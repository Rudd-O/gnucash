/********************************************************************\
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

#ifndef __G_TABLE_H__
#define __G_TABLE_H__

#include "config.h"

#include <glib.h>


/* This is the API for GTables, a datatype for 2-dimensional tables
 * with automatic resizing and memory management. */

typedef struct GTable GTable;

typedef gpointer (*g_table_allocator)   (void);
typedef void     (*g_table_deallocator) (gpointer);


/* Create a new table with the given allocator and deallocator.
 * Both functions must be given. They are used to populate the
 * table and free extra members when resizing and destroying. */
GTable * g_table_new (g_table_allocator allocator,
                      g_table_deallocator deallocator);

/* Free the table and all associated table elements. */
void     g_table_destroy (GTable *gtable);

/* Return the element at the given row and column. If the coordinates
 * are out-of-bounds, return NULL */
gpointer g_table_index (GTable *gtable, int row, int col);

/* Resize the table, allocating and deallocating extra table
 * members if needed. The relationship between table members
 * before and after resizing is undefined. */
void     g_table_resize (GTable *gtable, int rows, int cols);

#endif
