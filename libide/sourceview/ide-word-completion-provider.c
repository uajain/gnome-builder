/* ide-word-completion-provider.c
 *
 * Copyright (C) 2017 Umang Jain <mailumangjain@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "ide-word-completion-provider"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <glib/gi18n.h>
#include <string.h>

#include "sourceview/ide-word-completion-provider.h"
#include "sourceview/ide-word-completion-item.h"
#include "sourceview/ide-word-completion-results.h"
#include "sourceview/ide-completion-provider.h"
#include "ide-debug.h"

enum
{
  PROP_0,
  PROP_NAME,
  PROP_ICON,
  PROP_INTERACTIVE_DELAY,
  PROP_PRIORITY,
  PROP_ACTIVATION,
  N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES];

static void ide_word_completion_provider_iface_init (GtkSourceCompletionProviderIface *iface);

struct _IdeWordCompletionProviderPrivate
{
  GtkSourceSearchContext        *search_context;
  GtkSourceSearchSettings       *search_settings;
  GtkSourceCompletionContext    *context;
  IdeWordCompletionResults      *results;
  GHashTable                    *all_proposals;
  GtkSourceCompletionActivation  activation;
  GIcon                         *icon;

  gchar                         *current_word;
  gulong                         cancel_id;
  gchar                         *name;
  gint                           interactive_delay;
  gint                           priority;
  gboolean                       wrap_around_flag;
};

G_DEFINE_TYPE_WITH_CODE (IdeWordCompletionProvider,
                         ide_word_completion_provider,
                         G_TYPE_OBJECT,
                         G_ADD_PRIVATE (IdeWordCompletionProvider)
                         G_IMPLEMENT_INTERFACE (GTK_SOURCE_TYPE_COMPLETION_PROVIDER, ide_word_completion_provider_iface_init)
                         G_IMPLEMENT_INTERFACE (IDE_TYPE_COMPLETION_PROVIDER, NULL))

static gboolean
refresh_iters (IdeWordCompletionProvider *self,
               GtkTextIter *match_start,
               GtkTextIter *match_end)
{
  GtkTextBuffer *buffer = NULL;
  GtkTextMark *insert_mark;
  GtkTextMark *start_mark = NULL;
  GtkTextMark *end_mark = NULL;

  g_assert (IDE_IS_COMPLETION_PROVIDER (self));

  insert_mark = GTK_TEXT_MARK (g_object_get_data (G_OBJECT (self), "insert-mark"));
  buffer = gtk_text_mark_get_buffer (insert_mark);

  start_mark = gtk_text_buffer_get_mark (buffer, "start-mark");
  end_mark = gtk_text_buffer_get_mark (buffer, "end-mark");

  if (start_mark != NULL && end_mark != NULL)
    {
      gtk_text_buffer_get_iter_at_mark (buffer, match_start, start_mark);
      gtk_text_buffer_get_iter_at_mark (buffer, match_end, end_mark);

      return TRUE;
    }

  return FALSE;
}

static void
forward_search_finished (GtkSourceSearchContext    *search_context,
                         GAsyncResult              *result,
                         IdeWordCompletionProvider *self)
{
  IdeWordCompletionItem *proposal;
  GtkTextBuffer *buffer = NULL;
  GtkTextMark *insert_mark;
  GtkTextIter match_start;
  GtkTextIter match_end;
  GError *error = NULL;
  gboolean has_wrapped_around;

  g_assert (IDE_IS_WORD_COMPLETION_PROVIDER (self));
  g_assert (G_IS_ASYNC_RESULT (result));

  insert_mark = GTK_TEXT_MARK (g_object_get_data (G_OBJECT (self), "insert-mark"));

  if (gtk_source_search_context_forward_finish2 (search_context,
                                                 result,
                                                 &match_start,
                                                 &match_end,
                                                 &has_wrapped_around,
                                                 &error))
    {
      GtkTextIter insert_iter;
      GtkTextMark *start_mark = NULL;
      GtkTextMark *end_mark = NULL;
      gchar *text = NULL;

      buffer = gtk_text_mark_get_buffer (insert_mark);
      g_assert (buffer != NULL);

      start_mark = gtk_text_buffer_create_mark (buffer, "start-mark", &match_start, FALSE);
      end_mark = gtk_text_buffer_create_mark (buffer, "end-mark", &match_end, FALSE);

      if (start_mark == NULL || end_mark == NULL)
        {
          g_print ("Marks not set\n"); // just for debugging
          return;
        }

      gtk_text_buffer_get_iter_at_mark (buffer, &insert_iter, insert_mark);

      if (gtk_text_iter_equal (&match_end, &insert_iter) && self->priv->wrap_around_flag)
        goto finish;

      if (error != NULL)
        {
          g_warning ("Unable to get word completion proposals: %s", error->message);
          g_clear_error (&error);
          goto finish;
        }

      refresh_iters (self, &match_start, &match_end);

      text = gtk_text_iter_get_text (&match_start, &match_end);

      if (!g_hash_table_contains (self->priv->all_proposals, text))
        {
          gint offset;

          /*  Scan must have wrapped around giving offset as negative */
          offset = gtk_text_iter_get_offset (&match_start) - gtk_text_iter_get_offset (&insert_iter);
          if (offset < 0)
            {
              GtkTextIter end_iter;

              gtk_text_buffer_get_end_iter (buffer, &end_iter);

              offset = gtk_text_iter_get_offset (&end_iter) -
                       gtk_text_iter_get_offset (&insert_iter) +
                       gtk_text_iter_get_offset (&match_start);

              self->priv->wrap_around_flag = TRUE;
             }

	  g_assert (offset > 0);

	  proposal = ide_word_completion_item_new (text, offset, NULL);
	  ide_completion_results_take_proposal (IDE_COMPLETION_RESULTS (self->priv->results),
                                                IDE_COMPLETION_ITEM (proposal));

	  g_hash_table_add (self->priv->all_proposals, g_steal_pointer (&text));
	}

      gtk_text_buffer_get_iter_at_mark (buffer, &match_end, end_mark);
      gtk_source_search_context_forward_async (self->priv->search_context,
                                               &match_end,
                                               NULL,
                                               (GAsyncReadyCallback) forward_search_finished,
                                               self);
      gtk_text_mark_get_deleted (start_mark);
      gtk_text_mark_get_deleted (end_mark);
      return;
    }

finish:

  ide_completion_results_present (IDE_COMPLETION_RESULTS (self->priv->results),
                                  GTK_SOURCE_COMPLETION_PROVIDER (self), self->priv->context);
  gtk_text_mark_get_deleted (insert_mark);
  g_hash_table_destroy (self->priv->all_proposals);
}

static gchar *
ide_word_completion_provider_get_name (GtkSourceCompletionProvider *self)
{
  return g_strdup (IDE_WORD_COMPLETION_PROVIDER (self)->priv->name);
}

static GIcon *
ide_word_completion_provider_get_gicon (GtkSourceCompletionProvider *self)
{
  return IDE_WORD_COMPLETION_PROVIDER (self)->priv->icon;
}

static void
completion_cleanup (IdeWordCompletionProvider *self)
{
  g_clear_pointer (&self->priv->current_word, g_free);

  if (self->priv->context != NULL)
    {
      if (self->priv->cancel_id)
        {
          g_signal_handler_disconnect (self->priv->context, self->priv->cancel_id);
          self->priv->cancel_id = 0;
        }

      g_clear_object (&self->priv->context);
    }

  g_clear_object (&self->priv->search_settings);
  g_clear_object (&self->priv->search_context);
  g_clear_object (&self->priv->results);
}

static void
completion_cancelled_cb (IdeWordCompletionProvider *self)
{
  g_assert (IDE_IS_WORD_COMPLETION_PROVIDER (self));

  //TODO : precondition checks ?
  completion_cleanup (self);
 }

static void
ide_word_completion_provider_populate (GtkSourceCompletionProvider *provider,
                                       GtkSourceCompletionContext  *context)
{
  IdeWordCompletionProvider *self = IDE_WORD_COMPLETION_PROVIDER (provider);
  gchar *search_text = NULL;
  GtkTextIter insert_iter;
  GtkSourceBuffer *buffer;
  GtkTextMark *insert_mark;

  if (!gtk_source_completion_context_get_iter (context, &insert_iter))
    {
      gtk_source_completion_context_add_proposals (context, provider, NULL, TRUE);
      IDE_EXIT;
    }

  buffer = GTK_SOURCE_BUFFER (gtk_text_iter_get_buffer (&insert_iter));

  g_assert (self->priv->search_settings == NULL);
  g_assert (self->priv->search_context == NULL);
  g_assert (buffer != NULL);

  g_clear_pointer (&self->priv->current_word, g_free);
  self->priv->current_word = ide_completion_provider_context_current_word (context);
  //TODO : handle "minimum word size"

  if (self->priv->results != NULL)
    {
      if (ide_completion_results_replay (IDE_COMPLETION_RESULTS (self->priv->results), self->priv->current_word))
        {
          ide_completion_results_present (IDE_COMPLETION_RESULTS (self->priv->results), provider, context);
          IDE_EXIT;
        }

      g_clear_pointer (&self->priv->results, g_object_unref);
    }

  self->priv->search_settings = g_object_new (GTK_SOURCE_TYPE_SEARCH_SETTINGS,
                                              "at-word-boundaries", TRUE,
                                              "regex-enabled", TRUE,
                                              "wrap-around", TRUE,
                                              NULL);

  self->priv->search_context = gtk_source_search_context_new (buffer, self->priv->search_settings);
  self->priv->context = g_object_ref (context);

  search_text = g_strconcat (self->priv->current_word, "[a-zA-Z0-9_]*", NULL);
  gtk_source_search_settings_set_search_text (self->priv->search_settings, search_text);
  g_free (search_text);

  self->priv->cancel_id = g_signal_connect_swapped (context, "cancelled", G_CALLBACK (completion_cancelled_cb), self);
  self->priv->wrap_around_flag = FALSE;
  self->priv->results = ide_word_completion_results_new (self->priv->current_word);

  insert_mark = gtk_text_buffer_get_insert (GTK_TEXT_BUFFER (buffer));
  g_object_set_data (G_OBJECT (self), "insert-mark", insert_mark);

  self->priv->all_proposals = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  gtk_source_search_context_forward_async (self->priv->search_context,
                                           &insert_iter,
                                           NULL,
                                           (GAsyncReadyCallback)forward_search_finished,
                                           self);
}

static gboolean
ide_word_completion_provider_match (GtkSourceCompletionProvider *provider,
                                    GtkSourceCompletionContext  *context)
{
  IdeWordCompletionProvider *self = (IdeWordCompletionProvider *) provider;
  GtkSourceCompletionActivation activation;
  GtkTextIter iter;

  g_assert (IDE_IS_WORD_COMPLETION_PROVIDER (self));
  g_assert (GTK_SOURCE_IS_COMPLETION_CONTEXT (context));

  if (!gtk_source_completion_context_get_iter (context, &iter))
    return FALSE;

  activation = gtk_source_completion_context_get_activation (context);

  if (activation == GTK_SOURCE_COMPLETION_ACTIVATION_USER_REQUESTED)
    {
      gunichar ch;

      if (gtk_text_iter_starts_line (&iter))
        return FALSE;

      gtk_text_iter_backward_char (&iter);

      ch = gtk_text_iter_get_char (&iter);

      if (g_unichar_isalnum (ch) || ch == '_')
        return TRUE;
    }

  return FALSE;
}

static gboolean
ide_word_completion_provider_get_start_iter (GtkSourceCompletionProvider *provider,
                                             GtkSourceCompletionContext  *context,
                                             GtkSourceCompletionProposal *proposal,
                                             GtkTextIter                 *iter)
{
  gchar *word;
  glong nb_chars;

  if (!gtk_source_completion_context_get_iter (context, iter))
    return FALSE;

  word = ide_completion_provider_context_current_word (context);
  g_return_val_if_fail (word != NULL, FALSE);

  nb_chars = g_utf8_strlen (word, -1);
  gtk_text_iter_backward_chars (iter, nb_chars);

  g_free (word);
  return TRUE;
}

static gint
ide_word_completion_provider_get_interactive_delay (GtkSourceCompletionProvider *provider)
{
  return IDE_WORD_COMPLETION_PROVIDER (provider)->priv->interactive_delay;
}

static gint
ide_word_completion_provider_get_priority (GtkSourceCompletionProvider *provider)
{
  return IDE_WORD_COMPLETION_PROVIDER (provider)->priv->priority;
}

static GtkSourceCompletionActivation
ide_word_completion_provider_get_activation (GtkSourceCompletionProvider *provider)
{
  return IDE_WORD_COMPLETION_PROVIDER (provider)->priv->activation;
}


static void ide_word_completion_provider_iface_init (GtkSourceCompletionProviderIface *iface)
{
  iface->get_name = ide_word_completion_provider_get_name;
  iface->get_gicon = ide_word_completion_provider_get_gicon;
  iface->populate = ide_word_completion_provider_populate;
  iface->match = ide_word_completion_provider_match;
  iface->get_start_iter = ide_word_completion_provider_get_start_iter;
  iface->get_interactive_delay = ide_word_completion_provider_get_interactive_delay;
  iface->get_priority = ide_word_completion_provider_get_priority;
  iface->get_activation = ide_word_completion_provider_get_activation;
}

static void
ide_word_completion_provider_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
  IdeWordCompletionProvider *self = IDE_WORD_COMPLETION_PROVIDER (object);

  switch (prop_id)
    {
      case PROP_NAME:
        g_free (self->priv->name);
        self->priv->name = g_value_dup_string (value);

        if (self->priv->name == NULL)
          {
            self->priv->name = g_strdup (_("Builder Word Completion"));
          }
        break;

      case PROP_ICON:
        g_clear_object (&self->priv->icon);
        self->priv->icon = g_value_dup_object (value);
        break;

      case PROP_INTERACTIVE_DELAY:
        self->priv->interactive_delay = g_value_get_int (value);
        break;

      case PROP_PRIORITY:
        self->priv->priority = g_value_get_int (value);
        break;

      case PROP_ACTIVATION:
        self->priv->activation = g_value_get_flags (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
ide_word_completion_provider_dispose (GObject *object)
{
  IdeWordCompletionProvider *self = IDE_WORD_COMPLETION_PROVIDER (object);

  completion_cleanup (self);

  g_free (self->priv->name);
  self->priv->name = NULL;

  g_clear_object (&self->priv->icon);
  g_clear_object (&self->priv->search_context);

  G_OBJECT_CLASS (ide_word_completion_provider_parent_class)->dispose (object);
}

static void
ide_word_completion_provider_get_property (GObject    *object,
                                           guint       prop_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
  IdeWordCompletionProvider *self = IDE_WORD_COMPLETION_PROVIDER (object);

  switch (prop_id)
    {
      case PROP_NAME:
        g_value_set_string (value, self->priv->name);
        break;

      case PROP_ICON:
        g_value_set_object (value, self->priv->icon);
        break;

      case PROP_INTERACTIVE_DELAY:
        g_value_set_int (value, self->priv->interactive_delay);
        break;

      case PROP_PRIORITY:
        g_value_set_int (value, self->priv->priority);
	break;

      case PROP_ACTIVATION:
        g_value_set_flags (value, self->priv->activation);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
ide_word_completion_provider_class_init (IdeWordCompletionProviderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = ide_word_completion_provider_get_property;
  object_class->set_property = ide_word_completion_provider_set_property;
  object_class->dispose = ide_word_completion_provider_dispose;

  properties[PROP_NAME] =
        g_param_spec_string ("name",
                             "Name",
                             "The provider name",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

    properties[PROP_ICON] =
        g_param_spec_object ("icon",
                             "Icon",
                             "The provider icon",
                             G_TYPE_ICON,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

    properties[PROP_INTERACTIVE_DELAY] =
        g_param_spec_int ("interactive-delay",
                          "Interactive Delay",
                          "The delay before initiating interactive completion",
                          -1,
                          G_MAXINT,
                          50,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

    properties[PROP_PRIORITY] =
        g_param_spec_int ("priority",
                          "Priority",
                          "Provider priority",
                          G_MININT,
                          G_MAXINT,
                          0,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

    properties[PROP_ACTIVATION] =
        g_param_spec_flags ("activation",
                            "Activation",
                            "The type of activation",
                            GTK_SOURCE_TYPE_COMPLETION_ACTIVATION,
                            GTK_SOURCE_COMPLETION_ACTIVATION_INTERACTIVE |
                            GTK_SOURCE_COMPLETION_ACTIVATION_USER_REQUESTED,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
ide_word_completion_provider_init (IdeWordCompletionProvider *self)
{
  self->priv = ide_word_completion_provider_get_instance_private (self);
}

IdeWordCompletionProvider *
ide_word_completion_provider_new (const gchar *name, GIcon *icon)
{
  return g_object_new (IDE_TYPE_WORD_COMPLETION_PROVIDER,
                       "name", name,
                       "icon", icon,
                       NULL);
}
