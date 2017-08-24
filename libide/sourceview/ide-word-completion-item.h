/* ide-word-completion-item.h
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

#ifndef IDE_WORD_COMPLETION_ITEM_H
#define IDE_WORD_COMPLETION_ITEM_H

#include <gtksourceview/gtksource.h>

#include "sourceview/ide-completion-item.h"

G_BEGIN_DECLS

#define IDE_TYPE_WORD_COMPLETION_ITEM                   (ide_word_completion_item_get_type ())
#define IDE_WORD_COMPLETION_ITEM(obj)                   (G_TYPE_CHECK_INSTANCE_CAST ((obj), IDE_TYPE_WORD_COMPLETION_ITEM, IdeWordCompletionItem))
#define IDE_WORD_COMPLETION_ITEM_CONST(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), IDE_TYPE_WORD_COMPLETION_ITEM, IdeWordCompletionItem const))
#define IDE_WORD_COMPLETION_ITEM_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), IDE_TYPE_WORD_COMPLETION_ITEM, IdeWordCompletionItemClass))
#define IDE_IS_WORD_COMPLETION_ITEM(obj)                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDE_TYPE_WORD_COMPLETION_ITEM))
#define IDE_IS_WORD_COMPLETION_ITEM_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), IDE_TYPE_WORD_COMPLETION_ITEM))
#define IDE_WORD_COMPLETION_ITEM_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), IDE_TYPE_WORD_COMPLETION_ITEM, IdeWordCompletionItemClass))

typedef struct _IdeWordCompletionItem             IdeWordCompletionItem;
typedef struct _IdeWordCompletionItemClass        IdeWordCompletionItemClass;
typedef struct _IdeWordCompletionItemPrivate      IdeWordCompletionItemPrivate;

struct _IdeWordCompletionItem
{
  IdeCompletionItem parent;

  IdeWordCompletionItemPrivate *priv;
};

struct _IdeWordCompletionItemClass
{
  IdeCompletionItemClass parent_class;
};

GType                  ide_word_completion_item_get_type  (void) G_GNUC_CONST;
IdeWordCompletionItem *ide_word_completion_item_new        (const gchar *word,
                                                            gint         offset,
                                                            GIcon       *icon);
const gchar           *ide_word_completion_item_get_word   (IdeWordCompletionItem *proposal);
gint                   ide_word_completion_item_get_offset (IdeWordCompletionItem *proposal);

G_END_DECLS

#endif /* IDE_WORD_COMPLETION_ITEM_H */
