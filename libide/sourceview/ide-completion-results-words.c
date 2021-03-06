/* ide-completion-results-words.c
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

#define G_LOG_DOMAIN "ide-completion-results-words"

#include "ide-completion-results-words.h"
#include "gtksourcecompletionvimwordsproposal.h"

struct _IdeCompletionResultsWords
{
  IdeCompletionResults parent_instance;
};


G_DEFINE_TYPE (IdeCompletionResultsWords,
               ide_completion_results_words,
               IDE_TYPE_COMPLETION_RESULTS)

static gint
ide_completion_results_words_compare (IdeCompletionResultsWords *results,
                                      IdeCompletionItem         *left,
                                      IdeCompletionItem         *right)
{
  IdeCompletionResultsWords *self = IDE_COMPLETION_RESULTS_WORDS (results);
  GtkSourceCompletionVimWordsProposal *p1 = (GtkSourceCompletionVimWordsProposal *) left;
  GtkSourceCompletionVimWordsProposal *p2 = (GtkSourceCompletionVimWordsProposal *) right;
  g_print ("===========================Using proper compare===============");
  if (gtk_source_completion_vim_words_proposal_get_offset (p1) <
      gtk_source_completion_vim_words_proposal_get_offset (p2))
    return -1;
  else
    return 1;
}

static void
ide_completion_results_words_init (IdeCompletionResultsWords *self)
{
}

static void
ide_completion_results_words_class_init (IdeCompletionResultsWordsClass *klass)
{
  IdeCompletionResultsClass *results_class = IDE_COMPLETION_RESULTS_CLASS (klass);

  results_class->compare = ide_completion_results_words_compare;
}

IdeCompletionResultsWords *
ide_completion_results_words_new (const gchar *query)
{
  return g_object_new (IDE_TYPE_COMPLETION_RESULTS_WORDS,
                       "query", query,
                       NULL);
}
