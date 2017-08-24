/* ide-word-completion-item.c
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

#define G_LOG_DOMAIN "ide-word-completion-item"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sourceview/ide-word-completion-item.h"

struct _IdeWordCompletionItemPrivate
{
  GIcon *icon;
  gchar *word;
  gint   offset;
};

static void ide_word_completion_item_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (IdeWordCompletionItem,
                         ide_word_completion_item,
                         IDE_TYPE_COMPLETION_ITEM,
                         G_ADD_PRIVATE (IdeWordCompletionItem)
                         G_IMPLEMENT_INTERFACE (GTK_SOURCE_TYPE_COMPLETION_PROPOSAL,
                                                ide_word_completion_item_iface_init))

static gchar *
ide_word_completion_item_get_text (GtkSourceCompletionProposal *proposal)
{
  return g_strdup (IDE_WORD_COMPLETION_ITEM(proposal)->priv->word);
}

static GIcon *
ide_word_completion_item_get_gicon (GtkSourceCompletionProposal *proposal)
{
  return IDE_WORD_COMPLETION_ITEM(proposal)->priv->icon;
}

static void
ide_word_completion_item_iface_init (gpointer g_iface,
                                     gpointer iface_data)
{
  GtkSourceCompletionProposalIface *iface = (GtkSourceCompletionProposalIface *)g_iface;

  /* Interface data getter implementations */
  iface->get_label = ide_word_completion_item_get_text;
  iface->get_text = ide_word_completion_item_get_text;
  iface->get_gicon = ide_word_completion_item_get_gicon;
}

static void
ide_word_completion_item_finalize (GObject *object)
{
  IdeWordCompletionItem *proposal;

  proposal = IDE_WORD_COMPLETION_ITEM (object);
  g_free (proposal->priv->word);
  g_clear_object (&proposal->priv->icon);

  G_OBJECT_CLASS (ide_word_completion_item_parent_class)->finalize (object);
}

static void
ide_word_completion_item_class_init (IdeWordCompletionItemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = ide_word_completion_item_finalize;
}

static void
ide_word_completion_item_init (IdeWordCompletionItem *self)
{
  self->priv = ide_word_completion_item_get_instance_private (self);
}

IdeWordCompletionItem *
ide_word_completion_item_new (const gchar *word, gint offset, GIcon *icon)
{
  IdeWordCompletionItem *proposal;
  
  g_return_val_if_fail (word != NULL, NULL);
  
  proposal = g_object_new (IDE_TYPE_WORD_COMPLETION_ITEM, NULL);

  proposal->priv->word = g_strdup (word);
  proposal->priv->offset = offset;
  proposal->priv->icon = icon;

  return proposal;
}

const gchar *
ide_word_completion_item_get_word (IdeWordCompletionItem *proposal)
{
  g_return_val_if_fail (IDE_IS_WORD_COMPLETION_ITEM (proposal), NULL);
  return proposal->priv->word;
}

gint
ide_word_completion_item_get_offset (IdeWordCompletionItem *proposal)
{
  g_return_val_if_fail (IDE_IS_WORD_COMPLETION_ITEM (proposal), 0);
  return proposal->priv->offset;
}
