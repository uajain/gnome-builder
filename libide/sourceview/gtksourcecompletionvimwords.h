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

#ifndef GTK_SOURCE_COMPLETION_VIM_WORDS_H
#define GTK_SOURCE_COMPLETION_VIM_WORDS_H

#if !defined (GTK_SOURCE_H_INSIDE) && !defined (GTK_SOURCE_COMPILATION)
#error "Only <gtksourceview/gtksource.h> can be included directly."
#endif

#include <gtksourceview/gtksourcecompletionprovider.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_SOURCE_TYPE_COMPLETION_VIM_WORDS            (gtk_source_completion_vim_words_get_type ())
#define GTK_SOURCE_COMPLETION_VIM_WORDS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_SOURCE_TYPE_COMPLETION_VIM_WORDS, GtkSourceCompletionVimWords))
#define GTK_SOURCE_COMPLETION_VIM_WORDS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_SOURCE_TYPE_COMPLETION_VIM_WORDS, GtkSourceCompletionVimWordsClass))
#define GTK_SOURCE_IS_COMPLETION_VIM_WORDS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_SOURCE_TYPE_COMPLETION_VIM_WORDS))
#define GTK_SOURCE_IS_COMPLETION_VIM_WORDS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_SOURCE_TYPE_COMPLETION_VIM_WORDS))
#define GTK_SOURCE_COMPLETION_VIM_WORDS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_SOURCE_TYPE_COMPLETION_VIM_WORDS, GtkSourceCompletionVimWordsClass))

typedef struct _GtkSourceCompletionVimWords             GtkSourceCompletionVimWords;
typedef struct _GtkSourceCompletionVimWordsClass        GtkSourceCompletionVimWordsClass;
typedef struct _GtkSourceCompletionVimWordsPrivate      GtkSourceCompletionVimWordsPrivate;

struct _GtkSourceCompletionVimWords
{
  GObject parent;

  GtkSourceCompletionVimWordsPrivate *priv;
};

struct _GtkSourceCompletionVimWordsClass {
  GObjectClass parent_class;
};

GTK_SOURCE_AVAILABLE_IN_ALL
GType           gtk_source_completion_vim_words_get_type         (void) G_GNUC_CONST;

GTK_SOURCE_AVAILABLE_IN_ALL
GtkSourceCompletionVimWords *gtk_source_completion_vim_words_new (const gchar *name, GIcon *icon);

G_END_DECLS

#endif /* GTK_SOURCE_COMPLETION_VIM_WORDS_H */
