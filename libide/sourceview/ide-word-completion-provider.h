/* ide-word-completion-provider.h
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

#ifndef IDE_WORD_COMPLETION_PROVIDER_H
#define IDE_WORD_COMPLETION_PROVIDER_H

#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

#define IDE_TYPE_WORD_COMPLETION_PROVIDER            (ide_word_completion_provider_get_type ())
#define IDE_WORD_COMPLETION_PROVIDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), IDE_TYPE_WORD_COMPLETION_PROVIDER, IdeWordCompletionProvider))
#define IDE_WORD_COMPLETION_PROVIDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), IDE_TYPE_WORD_COMPLETION_PROVIDER, IdeWordCompletionProviderClass))
#define IDE_IS_WORD_COMPLETION_PROVIDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDE_TYPE_WORD_COMPLETION_PROVIDER))
#define IDE_IS_WORD_COMPLETION_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), IDE_TYPE_WORD_COMPLETION_PROVIDER))
#define IDE_WORD_COMPLETION_PROVIDER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), IDE_TYPE_WORD_COMPLETION_PROVIDER, IdeWordCompletionProviderClass))

typedef struct _IdeWordCompletionProvider             IdeWordCompletionProvider;
typedef struct _IdeWordCompletionProviderClass        IdeWordCompletionProviderClass;
typedef struct _IdeWordCompletionProviderPrivate      IdeWordCompletionProviderPrivate;

struct _IdeWordCompletionProvider
{
  GObject parent;

  IdeWordCompletionProviderPrivate *priv;
};

struct _IdeWordCompletionProviderClass {
  GObjectClass parent_class;
};

GType                      ide_word_completion_provider_get_type (void) G_GNUC_CONST;
IdeWordCompletionProvider *ide_word_completion_provider_new      (const gchar *name, GIcon *icon);

G_END_DECLS

#endif /* IDE_WORD_COMPLETION_PROVIDER_H */
