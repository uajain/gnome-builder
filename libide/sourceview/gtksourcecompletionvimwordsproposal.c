/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- *
 * gtksourcecompletionvim_wordsproposal.c
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

#include <glib/gi18n.h>
#include "sourceview/gtksourcecompletionvimwordsproposal.h"

struct _GtkSourceCompletionVimWordsProposalPrivate
{
  GIcon *icon;
  gchar *word;
  gint offset;
};

static void gtk_source_completion_proposal_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (GtkSourceCompletionVimWordsProposal,
                         gtk_source_completion_vim_words_proposal,
                         IDE_TYPE_COMPLETION_ITEM,
                         G_ADD_PRIVATE (GtkSourceCompletionVimWordsProposal)
                         G_IMPLEMENT_INTERFACE (GTK_SOURCE_TYPE_COMPLETION_PROPOSAL,
                                                gtk_source_completion_proposal_iface_init))

static gchar *
gtk_source_completion_vim_words_proposal_get_text (GtkSourceCompletionProposal *proposal)
{
  return g_strdup (GTK_SOURCE_COMPLETION_VIM_WORDS_PROPOSAL(proposal)->priv->word);
}

static GIcon *
gtk_source_completion_vim_words_proposal_get_gicon (GtkSourceCompletionProposal *proposal)
{
  return GTK_SOURCE_COMPLETION_VIM_WORDS_PROPOSAL(proposal)->priv->icon;
}

static void
gtk_source_completion_proposal_iface_init (gpointer g_iface,
                                           gpointer iface_data)
{
  GtkSourceCompletionProposalIface *iface = (GtkSourceCompletionProposalIface *)g_iface;

  /* Interface data getter implementations */
  iface->get_label = gtk_source_completion_vim_words_proposal_get_text;
  iface->get_text = gtk_source_completion_vim_words_proposal_get_text;
  iface->get_gicon = gtk_source_completion_vim_words_proposal_get_gicon;
}

static void
gtk_source_completion_vim_words_proposal_finalize (GObject *object)
{
  GtkSourceCompletionVimWordsProposal *proposal;

  proposal = GTK_SOURCE_COMPLETION_VIM_WORDS_PROPOSAL (object);
  g_free (proposal->priv->word);
  g_clear_object (&proposal->priv->icon);

  G_OBJECT_CLASS (gtk_source_completion_vim_words_proposal_parent_class)->finalize (object);
}

static void
gtk_source_completion_vim_words_proposal_class_init (GtkSourceCompletionVimWordsProposalClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtk_source_completion_vim_words_proposal_finalize;
}

static void
gtk_source_completion_vim_words_proposal_init (GtkSourceCompletionVimWordsProposal *self)
{
  self->priv = gtk_source_completion_vim_words_proposal_get_instance_private (self);
}

GtkSourceCompletionVimWordsProposal *
gtk_source_completion_vim_words_proposal_new (const gchar *word, gint offset, GIcon *icon)
{
  GtkSourceCompletionVimWordsProposal *proposal;
  
  g_return_val_if_fail (word != NULL, NULL);
  
  proposal = g_object_new (GTK_SOURCE_TYPE_COMPLETION_VIM_WORDS_PROPOSAL, NULL);

  proposal->priv->word = g_strdup (word);
  proposal->priv->offset = offset;
  proposal->priv->icon = icon;

  return proposal;
}

const gchar *
gtk_source_completion_vim_words_proposal_get_word (GtkSourceCompletionVimWordsProposal *proposal)
{
  g_return_val_if_fail (GTK_SOURCE_IS_COMPLETION_VIM_WORDS_PROPOSAL (proposal), NULL);
  return proposal->priv->word;
}

gint
gtk_source_completion_vim_words_proposal_get_offset (GtkSourceCompletionVimWordsProposal *proposal)
{
  g_return_val_if_fail (GTK_SOURCE_IS_COMPLETION_VIM_WORDS_PROPOSAL (proposal), 0);
  return proposal->priv->offset;
}
