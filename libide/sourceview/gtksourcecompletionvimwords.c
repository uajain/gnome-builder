/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 * gtksourcecompletionproviderwords.h
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2017 - Umang Jain <mailumangjain@gmail.com>
 *
 * gtksourceview is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * gtksourceview is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gtksourceview/gtksource.h"
#include "gtksourcecompletionvimwordsproposal.h"
#include "gtksourceview/gtksourceview-enumtypes.h"
#include "gtksourceview/gtksourceview-i18n.h"

#include <string.h>

#include "gtksourcecompletionvimwords.h"

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

static void gtk_source_completion_vim_words_iface_init (GtkSourceCompletionProviderIface *iface);

struct _GtkSourceCompletionVimWordsPrivate {

  GtkSourceSearchContext *search_context;
  GtkSourceSearchSettings *search_settings;
  GtkSourceCompletionContext *context;
  GHashTable *all_proposals;
  GtkSourceCompletionActivation activation;
  GIcon *icon;
  GtkTextIter insert_iter;
  gchar *word;
  gulong cancel_id;
  gchar *name;
  gint interactive_delay;
  gint priority;
  gboolean wrap_around_flag;
};

G_DEFINE_TYPE_WITH_CODE (GtkSourceCompletionVimWords,
                         gtk_source_completion_vim_words,
                         G_TYPE_OBJECT,
                         G_ADD_PRIVATE (GtkSourceCompletionVimWords)
                         G_IMPLEMENT_INTERFACE (GTK_SOURCE_TYPE_COMPLETION_PROVIDER, gtk_source_completion_vim_words_iface_init)
                         G_IMPLEMENT_INTERFACE (IDE_TYPE_COMPLETION_PROVIDER, NULL))

static gint
sort_by_offset (gconstpointer a, gconstpointer b)
{
  GtkSourceCompletionVimWordsProposal *p1 = (GtkSourceCompletionVimWordsProposal *) a;
  GtkSourceCompletionVimWordsProposal *p2 = (GtkSourceCompletionVimWordsProposal *) b;

  if (gtk_source_completion_vim_words_proposal_get_offset (p1) <
      gtk_source_completion_vim_words_proposal_get_offset (p2))
    return -1;
  else
    return 1;
}

static void
forward_search_finished (GtkSourceSearchContext      *search_context,
                         GAsyncResult                *result,
                         GtkSourceCompletionVimWords *self)
{
  GtkSourceCompletionVimWordsProposal *proposal;
  GList *list = NULL;
  GtkTextIter match_start;
  GtkTextIter match_end;
  GError *error = NULL;
  gboolean has_wrapped_around;

  if (gtk_source_search_context_forward_finish (search_context,
                                                result,
                                                &match_start,
                                                &match_end,
                                                &has_wrapped_around,
                                                &error))
    {
      gchar *text = NULL;
      gint offset;

      offset = gtk_text_iter_get_offset (&match_start) - gtk_text_iter_get_offset (&self->priv->insert_iter);

      /* Scan overshoots the insert_iter so we break here by detecting the wrap around flag */
      if ((offset > 0) && self->priv->wrap_around_flag)
        goto finish;

      if (error != NULL)
        {
          g_warning ("Unable to get word completion proposals: %s", error->message);
          g_clear_error (&error);
        }

      text = gtk_text_iter_get_text (&match_start, &match_end);

      if (!g_hash_table_contains (self->priv->all_proposals, text))
        {
          /*  Scan must have wrapped around giving offset as negative */
          if (offset < 0)
            {
              GtkTextIter end_iter;

              gtk_text_buffer_get_end_iter (gtk_text_iter_get_buffer (&self->priv->insert_iter), &end_iter);

              offset = gtk_text_iter_get_offset (&end_iter) -
                       gtk_text_iter_get_offset (&self->priv->insert_iter) +
                       gtk_text_iter_get_offset (&match_start);

              self->priv->wrap_around_flag = TRUE;
             }

	  g_assert (offset > 0);

	  proposal = gtk_source_completion_vim_words_proposal_new (text, offset, NULL);
	  g_hash_table_insert (self->priv->all_proposals, g_steal_pointer (&text), g_object_ref (proposal));
	  g_object_unref (proposal);
	}

      gtk_source_search_context_forward_async (self->priv->search_context,
                                               &match_end,
                                               NULL,
                                               (GAsyncReadyCallback)forward_search_finished,
                                               self);
      g_free (text);
      return;
    }

finish:
  list = g_hash_table_get_values (self->priv->all_proposals);
  list = g_list_sort (list, sort_by_offset);

  gtk_source_completion_context_add_proposals (self->priv->context,
                                               GTK_SOURCE_COMPLETION_PROVIDER (self),
                                               list,
                                               TRUE);

  g_list_free (list);
}

static gchar *
gtk_source_completion_vim_words_get_name (GtkSourceCompletionProvider *self)
{
    return g_strdup (GTK_SOURCE_COMPLETION_VIM_WORDS (self)->priv->name);
}

static GIcon *
gtk_source_completion_vim_words_get_gicon (GtkSourceCompletionProvider *self)
{
  return GTK_SOURCE_COMPLETION_VIM_WORDS (self)->priv->icon;
}

static void
completion_cleanup (GtkSourceCompletionVimWords *self)
{
  g_free (self->priv->word);
  self->priv->word = NULL;

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
}

static void
completion_cancelled_cb (GtkSourceCompletionVimWords *self)
{
  g_assert (GTK_SOURCE_IS_COMPLETION_VIM_WORDS (self));

  //precondition checks ?
  completion_cleanup (self);
 }

static void
gtk_source_completion_vim_words_populate (GtkSourceCompletionProvider *provider,
                                          GtkSourceCompletionContext  *context)
{
  GtkSourceCompletionVimWords *self = GTK_SOURCE_COMPLETION_VIM_WORDS (provider);

  if (!gtk_source_completion_context_get_iter (context, &self->priv->insert_iter))
    {
      gtk_source_completion_context_add_proposals (context, provider, NULL, TRUE);
      return;
    }

  g_free (self->priv->word);
  self->priv->word = NULL;
  g_hash_table_remove_all (self->priv->all_proposals);

  g_assert (self->priv->search_settings == NULL);
  g_assert (self->priv->search_context == NULL);

  self->priv->search_settings = gtk_source_search_settings_new ();

  self->priv->search_context = gtk_source_search_context_new (GTK_SOURCE_BUFFER (gtk_text_iter_get_buffer (&self->priv->insert_iter)),
                                                              self->priv->search_settings);
  self->priv->context = g_object_ref (context);

  gtk_source_search_settings_set_regex_enabled (self->priv->search_settings, TRUE);
  gtk_source_search_settings_set_at_word_boundaries (self->priv->search_settings, TRUE);
  gtk_source_search_settings_set_wrap_around (self->priv->search_settings, TRUE);

  self->priv->word = g_strconcat (ide_completion_provider_context_current_word (context),
                                  "[a-zA-Z0-9_]*",
                                  NULL);

  gtk_source_search_settings_set_search_text (self->priv->search_settings, self->priv->word);

  self->priv->cancel_id = g_signal_connect_swapped (context, "cancelled", G_CALLBACK (completion_cancelled_cb), self);
  self->priv->wrap_around_flag = FALSE;

  gtk_source_search_context_forward_async (self->priv->search_context,
                                           &self->priv->insert_iter,
                                           NULL,
                                           (GAsyncReadyCallback)forward_search_finished,
                                           self);
}

static gboolean
gtk_source_completion_vim_words_match (GtkSourceCompletionProvider *provider,
                                       GtkSourceCompletionContext  *context)
{
  GtkSourceCompletionVimWords *self = (GtkSourceCompletionVimWords *)provider;
  GtkSourceCompletionActivation activation;
  GtkTextIter iter;

  g_assert (GTK_SOURCE_IS_COMPLETION_VIM_WORDS (self));
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
gtk_source_completion_vim_words_get_start_iter (GtkSourceCompletionProvider *provider,
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
gtk_source_completion_vim_words_get_interactive_delay (GtkSourceCompletionProvider *provider)
{
  return GTK_SOURCE_COMPLETION_VIM_WORDS (provider)->priv->interactive_delay;
}

static gint
gtk_source_completion_vim_words_get_priority (GtkSourceCompletionProvider *provider)
{
  return GTK_SOURCE_COMPLETION_VIM_WORDS (provider)->priv->priority;
}

static GtkSourceCompletionActivation
gtk_source_completion_vim_words_get_activation (GtkSourceCompletionProvider *provider)
{
  return GTK_SOURCE_COMPLETION_VIM_WORDS (provider)->priv->activation;
}


static void gtk_source_completion_vim_words_iface_init (GtkSourceCompletionProviderIface *iface)
{
  iface->get_name = gtk_source_completion_vim_words_get_name;
  iface->get_gicon = gtk_source_completion_vim_words_get_gicon;
  iface->populate = gtk_source_completion_vim_words_populate;
  iface->match = gtk_source_completion_vim_words_match;
  iface->get_start_iter = gtk_source_completion_vim_words_get_start_iter;
  iface->get_interactive_delay = gtk_source_completion_vim_words_get_interactive_delay;
  iface->get_priority = gtk_source_completion_vim_words_get_priority;
  iface->get_activation = gtk_source_completion_vim_words_get_activation;
}

static void
gtk_source_completion_vim_words_set_property (GObject      *object,
                                              guint         prop_id,
                                              const GValue *value,
                                              GParamSpec   *pspec)
{
  GtkSourceCompletionVimWords *self = GTK_SOURCE_COMPLETION_VIM_WORDS (object);

  switch (prop_id)
    {
      case PROP_NAME:
        g_free (self->priv->name);
        self->priv->name = g_value_dup_string (value);

        if (self->priv->name == NULL)
          {
            self->priv->name = g_strdup (_("Document Vim Words"));
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
gtk_source_completion_vim_words_dispose (GObject *object)
{
  GtkSourceCompletionVimWords *self = GTK_SOURCE_COMPLETION_VIM_WORDS (object);

  completion_cleanup (self);

  g_free (self->priv->name);
  self->priv->name = NULL;

  g_clear_object (&self->priv->icon);
  g_clear_object (&self->priv->search_context);
  g_clear_pointer (&self->priv->all_proposals, g_hash_table_unref);

  G_OBJECT_CLASS (gtk_source_completion_vim_words_parent_class)->dispose (object);
}

static void
gtk_source_completion_vim_words_get_property (GObject    *object,
                                              guint       prop_id,
                                              GValue     *value,
                                              GParamSpec *pspec)
{
  GtkSourceCompletionVimWords *self = GTK_SOURCE_COMPLETION_VIM_WORDS (object);

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
gtk_source_completion_vim_words_class_init (GtkSourceCompletionVimWordsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = gtk_source_completion_vim_words_get_property;
  object_class->set_property = gtk_source_completion_vim_words_set_property;
  object_class->dispose = gtk_source_completion_vim_words_dispose;

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
gtk_source_completion_vim_words_init (GtkSourceCompletionVimWords *self)
{
  self->priv = gtk_source_completion_vim_words_get_instance_private (self);
  self->priv->all_proposals = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}

GtkSourceCompletionVimWords *
gtk_source_completion_vim_words_new (const gchar *name, GIcon *icon)
{
  return g_object_new (GTK_SOURCE_TYPE_COMPLETION_VIM_WORDS,
                       "name", name,
                       "icon", icon,
                       NULL);
}
