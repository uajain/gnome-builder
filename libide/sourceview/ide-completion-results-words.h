/* ide-completion-words.h
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

#ifndef IDE_COMPLETION_RESULTS_WORDS_H
#define IDE_COMPLETION_RESULTS_WORDS_H

#include <gtksourceview/gtksource.h>

#include "sourceview/ide-completion-results.h"
G_BEGIN_DECLS

#define IDE_TYPE_COMPLETION_RESULTS_WORDS            (ide_completion_results_words_get_type())
#define IDE_COMPLETION_RESULTS_WORDS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), IDE_TYPE_COMPLETION_RESULTS_WORDS, IdeCompletionResultsWords))
#define IDE_COMPLETION_RESULTS_WORDS_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), IDE_TYPE_COMPLETION_RESULTS_WORDS, IdeCompletionResultsWords const))
#define IDE_COMPLETION_RESULTS_WORDS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  IDE_TYPE_COMPLETION_RESULTS_WORDS, IdeCompletionResultsWordsClass))
#define IDE_IS_COMPLETION_RESULTS_WORDS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDE_TYPE_COMPLETION_RESULTS_WORDS))
#define IDE_IS_COMPLETION_RESULTS_WORDS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  IDE_TYPE_COMPLETION_RESULTS_WORDS))
#define IDE_COMPLETION_RESULTS_WORDS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  IDE_TYPE_COMPLETION_RESULTS_WORDS, IdeCompletionReultsWordsClass))


typedef struct _IdeCompletionResultsWords        IdeCompletionResultsWords;
typedef struct _IdeCompletionResultsWordsClass   IdeCompletionResultsWordsClass;

struct _IdeCompletionResultsWordsClass
{
  IdeCompletionResultsClass parent_class;
};

GType ide_completion_results_words_get_type (void);

IdeCompletionResultsWords* ide_completion_results_words_new (const gchar *query);

G_END_DECLS

#endif /* IDE_COMPLETION_RESULTS_WORDS_H */
