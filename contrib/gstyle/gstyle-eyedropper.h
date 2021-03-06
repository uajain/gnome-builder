/* gstyle-eyedropper.h
 *
 * Copyright (C) 2016 sebastien lafargue <slafargue@gnome.org>
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

#ifndef GSTYLE_EYEDROPPER_H
#define GSTYLE_EYEDROPPER_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GSTYLE_TYPE_EYEDROPPER (gstyle_eyedropper_get_type())

G_DECLARE_FINAL_TYPE (GstyleEyedropper, gstyle_eyedropper, GSTYLE, EYEDROPPER, GtkWindow)

GstyleEyedropper       *gstyle_eyedropper_new       (GdkEvent       *event);

G_END_DECLS

#endif /* GSTYLE_EYEDROPPER_H */
