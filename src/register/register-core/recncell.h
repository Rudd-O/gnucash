/********************************************************************\
 * recncell.h -- reconcile checkbox cell                            *
 *                                                                  *
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

/*
 * FILE:
 * recncell.h
 *
 * FUNCTION:
 * The RecnCell object implements a cell handler
 * that will cycle through a series of single-character
 * values when clicked upon by the mouse.
 *
 * hack alert -- there should be a way of specifying what these values
 * are, instead of having them hard coded as they currently are.
 *
 * HISTORY:
 * Copyright (c) 1998 Linas Vepstas
 * Copyright (c) 2000 Dave Peticolas
 */

#ifndef RECN_CELL_H
#define RECN_CELL_H

#include <glib.h>

#include "basiccell.h"

typedef const char * (*RecnCellStringGetter) (char flag);
typedef gboolean (*RecnCellConfirm) (char old_flag, gpointer data);

typedef struct
{
  BasicCell cell;

  char reconciled_flag; /* The actual flag value */

  RecnCellConfirm confirm_cb;
  gpointer confirm_data;
} RecnCell;

BasicCell * gnc_recn_cell_new (void);

void        gnc_recn_cell_set_flag (RecnCell *cell, char reconciled_flag);
char        gnc_recn_cell_get_flag (RecnCell *cell);

void        gnc_recn_cell_set_confirm_cb (RecnCell *cell,
                                          RecnCellConfirm confirm_cb,
                                          gpointer data);
 
void        gnc_recn_cell_set_string_getter (RecnCellStringGetter getter);

#endif
