/********************************************************************\
 * table-allgui.c -- 2D grid table object, embeds cells for i/o     *
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
 * table-allgui.c
 *
 * FUNCTION:
 * Implements the gui-independent parts of the table infrastructure.
 *
 * HISTORY:
 * Copyright (c) 1998,1999,2000 Linas Vepstas
 * Copyright (c) 2000 Dave Peticolas
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "table-allgui.h"
#include "cellblock.h"
#include "gnc-engine-util.h"


/** Static Globals *****************************************************/

/* This static indicates the debugging module that this .o belongs to. */
static short module = MOD_REGISTER;


/** Prototypes *********************************************************/
static void gnc_table_init (Table * table);
static void gnc_table_free_data (Table * table);
static void gnc_virtual_cell_construct (gpointer vcell, gpointer user_data);
static void gnc_virtual_location_init (VirtualLocation *vloc);
static void gnc_virtual_cell_destroy (gpointer vcell, gpointer user_data);
static void gnc_table_resize (Table * table, int virt_rows, int virt_cols);


/** Implementation *****************************************************/

Table * 
gnc_table_new (TableGetEntryHandler entry_handler,
               TableGetLabelHandler label_handler,
               TableGetCellIOFlags io_flag_handler,
               TableGetFGColorHandler fg_color_handler,
               TableGetBGColorHandler bg_color_handler,
               TableGetCellBorderHandler cell_border_handler,
               gpointer handler_user_data,
               VirtCellDataAllocator allocator,
               VirtCellDataDeallocator deallocator,
               VirtCellDataCopy copy)
{
   Table *table;

   g_assert (entry_handler != NULL);

   table = g_new0 (Table, 1);

   table->entry_handler = entry_handler;
   table->label_handler = label_handler;
   table->io_flag_handler = io_flag_handler;
   table->fg_color_handler = fg_color_handler;
   table->bg_color_handler = bg_color_handler;
   table->cell_border_handler = cell_border_handler;
   table->handler_user_data = handler_user_data;
   table->vcell_data_allocator = allocator;
   table->vcell_data_deallocator = deallocator;
   table->vcell_data_copy = copy;

   gnc_table_init (table);

   table->virt_cells = g_table_new (sizeof (VirtualCell),
                                    gnc_virtual_cell_construct,
                                    gnc_virtual_cell_destroy, table);

   return table;
}

/* ==================================================== */

static void
gnc_table_init (Table * table)
{
   table->num_virt_rows = -1;
   table->num_virt_cols = -1;

   table->num_header_phys_rows = -1;

   table->current_cursor = NULL;

   gnc_virtual_location_init (&table->current_cursor_loc);

   table->move_cursor = NULL;
   table->traverse = NULL;
   table->set_help = NULL;
   table->user_data = NULL;

   table->alternate_bg_colors = FALSE;

   table->dividing_row = -1;

   /* initialize private data */

   table->virt_cells = NULL;

   table->ui_data = NULL;
   table->destroy = NULL;
}

/* ==================================================== */

void 
gnc_table_destroy (Table * table)
{
   /* invoke destroy callback */
   if (table->destroy)
     table->destroy(table);

   /* free the dynamic structures */
   gnc_table_free_data (table);

   /* free the cell tables */
   g_table_destroy(table->virt_cells);

   /* intialize vars to null value so that any access is voided. */
   gnc_table_init (table);

   g_free (table);
}

/* ==================================================== */

VirtualCell *
gnc_table_get_virtual_cell (Table *table, VirtualCellLocation vcell_loc)
{
  if (table == NULL)
    return NULL;

  return g_table_index (table->virt_cells,
                        vcell_loc.virt_row, vcell_loc.virt_col);
}

/* ==================================================== */

VirtualCell *
gnc_table_get_header_cell (Table *table)
{
  VirtualCellLocation vcell_loc = { 0, 0 };

  return gnc_table_get_virtual_cell (table, vcell_loc);
}

/* ==================================================== */

static const char *
gnc_table_get_entry_internal (Table *table, VirtualLocation virt_loc,
                              gboolean *conditionally_changed)
{
  const char *entry;

  entry = table->entry_handler (virt_loc,
                                conditionally_changed,
                                table->handler_user_data);
  if (!entry)
    entry = "";

  return entry;
}

const char *
gnc_table_get_entry (Table *table, VirtualLocation virt_loc)
{
  VirtualCell *vcell;
  CellBlockCell *cb_cell;
  const char *entry;

  vcell = gnc_table_get_virtual_cell (table, virt_loc.vcell_loc);
  if (vcell == NULL)
    return "";

  cb_cell = gnc_cellblock_get_cell (vcell->cellblock,
                                    virt_loc.phys_row_offset,
                                    virt_loc.phys_col_offset);
  if (cb_cell == NULL)
    return "";
  if (cb_cell->cell_type < 0)
    return "";

  if (virt_cell_loc_equal (table->current_cursor_loc.vcell_loc,
                           virt_loc.vcell_loc))
  {
    CellIOFlags io_flags;

    io_flags = gnc_table_get_io_flags (table, virt_loc);

    if (io_flags & XACC_CELL_ALLOW_INPUT)
      return cb_cell->cell->value;
  }

  entry = table->entry_handler (virt_loc, NULL, table->handler_user_data);
  if (!entry)
    entry = "";

  return entry;
}

/* ==================================================== */

CellIOFlags
gnc_table_get_io_flags (Table *table, VirtualLocation virt_loc)
{
  if (!table->io_flag_handler)
    return XACC_CELL_ALLOW_NONE;

  return table->io_flag_handler (virt_loc, table->handler_user_data);
}

/* ==================================================== */

const char *
gnc_table_get_label (Table *table, VirtualLocation virt_loc)
{
  if (!table->label_handler)
    return "";

  return table->label_handler (virt_loc, table->handler_user_data);
}

/* ==================================================== */

guint32
gnc_table_get_fg_color (Table *table, VirtualLocation virt_loc)
{
  if (!table->fg_color_handler)
    return 0x0; /* black */

  return table->fg_color_handler (virt_loc, table->handler_user_data);
}

/* ==================================================== */

guint32
gnc_table_get_bg_color (Table *table, VirtualLocation virt_loc,
                        gboolean *hatching)
{
  if (!table->bg_color_handler)
    return 0xffffff; /* white */

  return table->bg_color_handler (virt_loc, hatching,
                                  table->handler_user_data);
}

/* ==================================================== */

void
gnc_table_get_borders (Table *table, VirtualLocation virt_loc,
                       PhysicalCellBorders *borders)
{
  if (!table->cell_border_handler)
    return;

  table->cell_border_handler (virt_loc, borders, table->handler_user_data);
}

/* ==================================================== */

CellAlignment
gnc_table_get_align (Table *table, VirtualLocation virt_loc)
{
  VirtualCell *vcell;
  CellBlockCell *cb_cell;

  vcell = gnc_table_get_virtual_cell (table, virt_loc.vcell_loc);
  if (vcell == NULL)
    return CELL_ALIGN_RIGHT;

  cb_cell = gnc_cellblock_get_cell (vcell->cellblock,
                                    virt_loc.phys_row_offset,
                                    virt_loc.phys_col_offset);
  if (cb_cell == NULL)
    return CELL_ALIGN_RIGHT;

  return cb_cell->alignment;
}

/* ==================================================== */

gboolean
gnc_table_is_popup (Table *table, VirtualLocation virt_loc)
{
  VirtualCell *vcell;
  CellBlockCell *cb_cell;

  vcell = gnc_table_get_virtual_cell (table, virt_loc.vcell_loc);
  if (vcell == NULL)
    return FALSE;

  cb_cell = gnc_cellblock_get_cell (vcell->cellblock,
                                    virt_loc.phys_row_offset,
                                    virt_loc.phys_col_offset);
  if (cb_cell == NULL || cb_cell->cell == NULL)
    return FALSE;

  return cb_cell->cell->is_popup;
}

/* ==================================================== */

void 
gnc_table_set_size (Table * table, int virt_rows, int virt_cols)
{
  /* Invalidate the current cursor position, if the array is
   * shrinking. This must be done since the table is probably
   * shrinking because some rows were deleted, and the cursor
   * could be on the deleted rows. */
  if ((virt_rows < table->num_virt_rows) ||
      (virt_cols < table->num_virt_cols))
  {
    gnc_virtual_location_init (&table->current_cursor_loc);
    table->current_cursor = NULL;
  }

  gnc_table_resize (table, virt_rows, virt_cols);
}

/* ==================================================== */

static void
gnc_table_free_data (Table * table)
{
  if (table == NULL)
    return;

  g_table_resize (table->virt_cells, 0, 0);
}

/* ==================================================== */

static void
gnc_virtual_location_init (VirtualLocation *vloc)
{
  if (vloc == NULL)
    return;

  vloc->phys_row_offset = -1;
  vloc->phys_col_offset = -1;
  vloc->vcell_loc.virt_row = -1;
  vloc->vcell_loc.virt_col = -1;
}

/* ==================================================== */

static void
gnc_virtual_cell_construct (gpointer _vcell, gpointer user_data)
{
  VirtualCell *vcell = _vcell;
  Table *table = user_data;

  vcell->cellblock = NULL;

  if (table && table->vcell_data_allocator)
    vcell->vcell_data = table->vcell_data_allocator();
  else
    vcell->vcell_data = NULL;

  vcell->visible = 1;
}

/* ==================================================== */

static void
gnc_virtual_cell_destroy (gpointer _vcell, gpointer user_data)
{
  VirtualCell *vcell = _vcell;
  Table *table = user_data;

  if (vcell->vcell_data && table && table->vcell_data_deallocator)
    table->vcell_data_deallocator (vcell->vcell_data);

  vcell->vcell_data = NULL;
}

/* ==================================================== */

static void 
gnc_table_resize (Table * table, int new_virt_rows, int new_virt_cols)
{
  if (!table) return;

  g_table_resize (table->virt_cells, new_virt_rows, new_virt_cols);

  table->num_virt_rows = new_virt_rows;
  table->num_virt_cols = new_virt_cols;
}

/* ==================================================== */

void
gnc_table_set_vcell (Table *table,
                     CellBlock *cursor,
                     gconstpointer vcell_data,
                     gboolean visible,
                     gboolean start_primary_color,
                     VirtualCellLocation vcell_loc)
{
  VirtualCell *vcell;

  if ((table == NULL) || (cursor == NULL))
    return;

  if ((vcell_loc.virt_row >= table->num_virt_rows) ||
      (vcell_loc.virt_col >= table->num_virt_cols))
    gnc_table_resize (table,
                      MAX (table->num_virt_rows, vcell_loc.virt_row + 1),
                      MAX (table->num_virt_cols, vcell_loc.virt_col + 1));

  vcell = gnc_table_get_virtual_cell (table, vcell_loc);
  if (vcell == NULL)
    return;

  /* this cursor is the handler for this block */
  vcell->cellblock = cursor;

  /* copy the vcell user data */
  if (table->vcell_data_copy)
    table->vcell_data_copy (vcell->vcell_data, vcell_data);
  else
    vcell->vcell_data = (gpointer) vcell_data;

  vcell->visible = visible ? 1 : 0;
  vcell->start_primary_color = start_primary_color ? 1 : 0;
}

/* ==================================================== */

void
gnc_table_set_virt_cell_data (Table *table,
                              VirtualCellLocation vcell_loc,
                              gconstpointer vcell_data)
{
  VirtualCell *vcell;

  if (table == NULL)
    return;

  vcell = gnc_table_get_virtual_cell (table, vcell_loc);
  if (vcell == NULL)
    return;

  if (table->vcell_data_copy)
    table->vcell_data_copy (vcell->vcell_data, vcell_data);
  else
    vcell->vcell_data = (gpointer) vcell_data;
}

/* ==================================================== */

void
gnc_table_set_virt_cell_visible (Table *table,
                                 VirtualCellLocation vcell_loc,
                                 gboolean visible)
{
  VirtualCell *vcell;

  if (table == NULL)
    return;

  vcell = gnc_table_get_virtual_cell (table, vcell_loc);
  if (vcell == NULL)
    return;

  vcell->visible = visible ? 1 : 0;
}

/* ==================================================== */

void
gnc_table_set_virt_cell_cursor (Table *table,
                                VirtualCellLocation vcell_loc,
                                CellBlock *cursor)
{
  VirtualCell *vcell;

  if (table == NULL)
    return;

  vcell = gnc_table_get_virtual_cell (table, vcell_loc);
  if (vcell == NULL)
    return;

  vcell->cellblock = cursor;
}

/* ==================================================== */

static void 
gnc_table_move_cursor_internal (Table *table,
                                VirtualLocation new_virt_loc,
                                gboolean do_move_gui)
{
  int cell_row, cell_col;
  VirtualLocation virt_loc;
  VirtualCell *vcell;
  CellBlock *curs;

  ENTER("new_virt=(%d %d) do_move_gui=%d\n", 
        new_virt_loc.vcell_loc.virt_row,
        new_virt_loc.vcell_loc.virt_col, do_move_gui);

  /* call the callback, allowing the app to commit any changes
   * associated with the current location of the cursor. Note that
   * this callback may recursively call this routine. */
  if (table->move_cursor)
  {
    (table->move_cursor) (table, &new_virt_loc);

    /* The above callback can cause this routine to be called
     * recursively. As a result of this recursion, the cursor may
     * have gotten repositioned. We need to make sure we make
     * passive again. */
    if (do_move_gui)
      gnc_table_refresh_current_cursor_gui (table, FALSE);
  }

  /* invalidate the cursor for now; we'll fix it back up below */
  gnc_virtual_location_init (&table->current_cursor_loc);

  curs = table->current_cursor;
  table->current_cursor = NULL;

  /* check for out-of-bounds conditions (which may be deliberate) */
  if ((new_virt_loc.vcell_loc.virt_row < 0) ||
      (new_virt_loc.vcell_loc.virt_col < 0))
  {
    /* if the location is invalid, then we should take this 
     * as a command to unmap the cursor gui. */
    if (do_move_gui && curs)
    {
      for (cell_row = 0; cell_row < curs->num_rows; cell_row++)
        for (cell_col = 0; cell_col < curs->num_cols; cell_col++)
        {
          CellBlockCell *cb_cell;

          cb_cell = gnc_cellblock_get_cell (curs, cell_row, cell_col);
          if (cb_cell && cb_cell->cell)
          {
            BasicCell *cell = cb_cell->cell;

            cell->changed = 0;
            cell->conditionally_changed = 0;

            if (cell->move)
            {
              VirtualLocation vloc;

              vloc.vcell_loc.virt_row = -1;
              vloc.vcell_loc.virt_col = -1;
              vloc.phys_row_offset = -1;
              vloc.phys_col_offset = -1;

              cell->move (cell, vloc);
            }
          }
        }
    }

    LEAVE("out of bounds\n");

    return;
  }

  if (!gnc_table_virtual_loc_valid (table, new_virt_loc, TRUE))
  {
    PWARN("bad table location");
    return;
  }

  /* ok, we now have a valid position. Find the new cursor to use,
   * and initialize its cells */
  vcell = gnc_table_get_virtual_cell (table, new_virt_loc.vcell_loc);
  curs = vcell->cellblock;
  table->current_cursor = curs;

  /* record the new position */
  table->current_cursor_loc = new_virt_loc;

  virt_loc.vcell_loc = new_virt_loc.vcell_loc;

  /* update the cell values to reflect the new position */
  for (cell_row = 0; cell_row < curs->num_rows; cell_row++)
    for (cell_col = 0; cell_col < curs->num_cols; cell_col++)
    {
      CellBlockCell *cb_cell;
      CellIOFlags io_flags;

      virt_loc.phys_row_offset = cell_row;
      virt_loc.phys_col_offset = cell_col;

      cb_cell = gnc_cellblock_get_cell(curs, cell_row, cell_col);
      if (cb_cell && cb_cell->cell)
      {
        BasicCell *cell = cb_cell->cell;

        /* if a cell has a GUI, move that first, before setting
         * the cell value.  Otherwise, we'll end up putting the 
         * new values in the old cell locations, and that would 
         * lead to confusion of all sorts. */
        if (do_move_gui && cell->move)
          cell->move (cell, virt_loc);

        /* OK, now copy the string value from the table at large 
         * into the cell handler. */
        io_flags = gnc_table_get_io_flags (table, virt_loc);
        if (io_flags & XACC_CELL_ALLOW_SHADOW)
        {
          const char *entry;
          gboolean conditionally_changed = FALSE;

          entry = gnc_table_get_entry_internal (table, virt_loc,
                                                &conditionally_changed);

          xaccSetBasicCellValue (cell, entry);

          cell->changed = 0;
          cell->conditionally_changed =
            conditionally_changed ? GNC_CELL_CHANGED : 0;
        }
      }
    }

  LEAVE("did move\n");
}

/* ==================================================== */

void
gnc_table_move_cursor (Table *table, VirtualLocation new_virt_loc)
{
  if (!table) return;

  gnc_table_move_cursor_internal (table, new_virt_loc, FALSE);
}

/* same as above, but be sure to deal with GUI elements as well */
void
gnc_table_move_cursor_gui (Table *table, VirtualLocation new_virt_loc)
{
  if (!table) return;

  gnc_table_move_cursor_internal (table, new_virt_loc, TRUE);
}

/* ==================================================== */

/* gnc_table_verify_cursor_position checks the location of the cursor
 * with respect to a virtual location, and repositions the cursor
 * if necessary. Returns true if the cell cursor was repositioned. */
gboolean
gnc_table_verify_cursor_position (Table *table, VirtualLocation virt_loc)
{
  gboolean do_move = FALSE;
  gboolean moved_cursor = FALSE;

  if (!table) return FALSE;

  /* Someone may be trying to intentionally invalidate the cursor, in
   * which case the physical addresses could be out of bounds. For
   * example, in order to unmap it in preparation for a reconfig.
   * So, if the specified location is out of bounds, then the cursor
   * MUST be moved. */
  if (gnc_table_virtual_cell_out_of_bounds (table, virt_loc.vcell_loc))
    do_move = TRUE;

  if (!virt_cell_loc_equal (virt_loc.vcell_loc,
                            table->current_cursor_loc.vcell_loc))
    do_move = TRUE;

  if (do_move)
  {
    gnc_table_move_cursor_gui (table, virt_loc);
    moved_cursor = TRUE;
  }
  else if (!virt_loc_equal (virt_loc, table->current_cursor_loc))
  {
    table->current_cursor_loc = virt_loc;
    moved_cursor = TRUE;
  }

  return moved_cursor;
}

/* ==================================================== */

void *
gnc_table_get_vcell_data (Table *table, VirtualCellLocation vcell_loc)
{
  VirtualCell *vcell;

  if (!table) return NULL;

  vcell = gnc_table_get_virtual_cell (table, vcell_loc);
  if (vcell == NULL)
    return NULL;

  return vcell->vcell_data;
}

/* ==================================================== */

/* if any of the cells have GUI specific components that need 
 * initialization, initialize them now. The realize() callback
 * on the cursor cell is how we inform the cell handler that 
 * now is the time to initialize its GUI.  */
void
gnc_table_create_cursor (Table * table, CellBlock *curs)
{
  int cell_row, cell_col;

  if (!curs || !table) return;  
  if (!table->ui_data) return;

  for (cell_row = 0; cell_row < curs->num_rows; cell_row++)
    for (cell_col = 0; cell_col < curs->num_cols; cell_col++)
    {
      CellBlockCell *cb_cell;

      cb_cell = gnc_cellblock_get_cell (curs, cell_row, cell_col);

      if (cb_cell && cb_cell->cell && cb_cell->cell->realize)
        cb_cell->cell->realize (cb_cell->cell, table->ui_data);
    }
}

/* ==================================================== */

void
gnc_table_wrap_verify_cursor_position (Table *table, VirtualLocation virt_loc)
{
   VirtualLocation save_loc;
   gboolean moved_cursor;

   if (!table) return;

   ENTER("(%d %d)", virt_loc.vcell_loc.virt_row, virt_loc.vcell_loc.virt_col);

   save_loc = table->current_cursor_loc;

   /* VerifyCursor will do all sorts of gui-independent machinations */
   moved_cursor = gnc_table_verify_cursor_position (table, virt_loc);

   if (moved_cursor)
   {
      /* make sure *both* the old and the new cursor rows get redrawn */
      gnc_table_refresh_current_cursor_gui (table, TRUE);
      gnc_table_refresh_cursor_gui (table, save_loc.vcell_loc, FALSE);
   }

   LEAVE ("\n");
}

/* ==================================================== */

void        
gnc_table_refresh_current_cursor_gui (Table * table, gboolean do_scroll)
{
  if (!table) return;

  gnc_table_refresh_cursor_gui (table, table->current_cursor_loc.vcell_loc,
                                do_scroll);
}

/* ==================================================== */

gboolean
gnc_table_virtual_loc_valid(Table *table,
                            VirtualLocation virt_loc,
                            gboolean exact_pointer)
{
  VirtualCell *vcell;
  CellIOFlags io_flags;

  if (!table) return FALSE;

  /* header rows cannot be modified */
  if (virt_loc.vcell_loc.virt_row == 0)
    return FALSE;

  vcell = gnc_table_get_virtual_cell(table, virt_loc.vcell_loc);
  if (vcell == NULL)
    return FALSE;

  if (!vcell->visible)
    return FALSE;

  /* verify that offsets are valid. This may occur if the app that is
   * using the table has a paritally initialized cursor. (probably due
   * to a programming error, but maybe they meant to do this). */
  if ((0 > virt_loc.phys_row_offset) || (0 > virt_loc.phys_col_offset))
    return FALSE;

  /* check for a cell handler, but only if cell address is valid */
  if (vcell->cellblock == NULL) return FALSE;

  io_flags = gnc_table_get_io_flags (table, virt_loc);
  /* if cell is marked as output-only, you can't enter */
  if (0 == (XACC_CELL_ALLOW_INPUT & io_flags)) return FALSE;

  /* if cell is pointer only and this is not an exact pointer test,
   * it cannot be entered. */
  if (!exact_pointer && ((XACC_CELL_ALLOW_EXACT_ONLY & io_flags) != 0))
    return FALSE;

  return TRUE;
}

/* ==================================================== */

/* Handle the non gui-specific parts of a cell enter callback */
gboolean
gnc_table_enter_update(Table *table,
                       VirtualLocation virt_loc,
                       int *cursor_position,
                       int *start_selection,
                       int *end_selection)
{
  gboolean can_edit = TRUE;
  CellEnterFunc enter;
  CellBlockCell *cb_cell;
  BasicCell *cell;
  CellBlock *cb;
  int cell_row;
  int cell_col;

  if (table == NULL)
    return FALSE;

  cb = table->current_cursor;

  cell_row = virt_loc.phys_row_offset;
  cell_col = virt_loc.phys_col_offset;

  ENTER("enter %d %d (relrow=%d relcol=%d)",
        virt_loc.vcell_loc.virt_row,
        virt_loc.vcell_loc.virt_col,
        cell_row, cell_col);

  /* OK, if there is a callback for this cell, call it */
  cb_cell = gnc_cellblock_get_cell (cb, cell_row, cell_col);
  cell = cb_cell->cell;
  enter = cell->enter_cell;

  if (enter)
  {
    char * old_value;

    DEBUG("gnc_table_enter_update(): %d %d has enter handler\n",
          cell_row, cell_col);

    old_value = g_strdup(cell->value);

    can_edit = enter(cell, cursor_position, start_selection, end_selection);

    if (safe_strcmp(old_value, cell->value) != 0)
      cell->changed = GNC_CELL_CHANGED;

    g_free (old_value);
  }

  if (table->set_help)
  {
    char *help_str;

    help_str = xaccBasicCellGetHelp(cell);

    table->set_help(table, help_str);

    g_free(help_str);
  }

  LEAVE("return %d\n", can_edit);

  return can_edit;
}

/* ==================================================== */

void
gnc_table_leave_update(Table *table, VirtualLocation virt_loc)
{
  gboolean changed = FALSE;
  CellLeaveFunc leave;
  CellBlockCell *cb_cell;
  BasicCell *cell;
  CellBlock *cb;
  int cell_row;
  int cell_col;

  if (table == NULL)
    return;

  cb = table->current_cursor;

  cell_row = virt_loc.phys_row_offset;
  cell_col = virt_loc.phys_col_offset;

  ENTER("proposed (%d %d) rel(%d %d)\n",
        virt_loc.vcell_loc.virt_row,
        virt_loc.vcell_loc.virt_col,
        cell_row, cell_col);

  /* OK, if there is a callback for this cell, call it */
  cb_cell = gnc_cellblock_get_cell (cb, cell_row, cell_col);
  if (!cb_cell || !cb_cell->cell)
    return;

  cell = cb_cell->cell;
  leave = cell->leave_cell;

  if (leave)
  {
    char * old_value;

    old_value = g_strdup(cell->value);

    leave (cell);

    if (safe_strcmp(old_value, cell->value) != 0)
    {
      changed = TRUE;
      cell->changed = GNC_CELL_CHANGED;
    }

    g_free (old_value);
  }
}

/* ==================================================== */

/* returned result should not be touched by the caller.
 * NULL return value means the edit was rejected. */
const char *
gnc_table_modify_update(Table *table,
                        VirtualLocation virt_loc,
                        const GdkWChar *change,
                        int change_len,
                        const GdkWChar *newval,
                        int newval_len,
                        int *cursor_position,
                        int *start_selection,
                        int *end_selection)
{
  gboolean changed = FALSE;
  CellModifyVerifyFunc mv;
  CellBlockCell *cb_cell;
  BasicCell *cell;
  CellBlock *cb;
  int cell_row;
  int cell_col;
  char * old_value;

  if (table == NULL)
    return NULL;

  cb = table->current_cursor;

  cell_row = virt_loc.phys_row_offset;
  cell_col = virt_loc.phys_col_offset;

  ENTER ("\n");

  /* OK, if there is a callback for this cell, call it */
  cb_cell = gnc_cellblock_get_cell (cb, cell_row, cell_col);
  cell = cb_cell->cell;
  mv = cell->modify_verify;

  old_value = g_strdup (cell->value);

  if (mv)
    mv (cell, change, change_len, newval, newval_len,
        cursor_position, start_selection, end_selection);
  else
  {
    char *newval_mb = gnc_wcstombs (newval);
    xaccSetBasicCellValue (cell, newval_mb);
    g_free (newval_mb);
  }

  if (safe_strcmp (old_value, cell->value) != 0)
  {
    changed = TRUE;
    cell->changed = GNC_CELL_CHANGED;
  }

  g_free (old_value);

  if (table->set_help)
  {
    char *help_str;

    help_str = xaccBasicCellGetHelp (cell);

    table->set_help (table, help_str);

    g_free (help_str);
  }

  LEAVE ("change %d %d (relrow=%d relcol=%d) val=%s\n", 
         virt_loc.vcell_loc.virt_row, virt_loc.vcell_loc.virt_col,
         cell_row, cell_col, cell->value);

  if (changed)
    return cell->value;
  else
    return NULL;
}

/* ==================================================== */

gboolean
gnc_table_direct_update(Table *table,
                        VirtualLocation virt_loc,
                        char **newval_ptr,
                        int *cursor_position,
                        int *start_selection,
                        int *end_selection,
                        void *gui_data)
{
  CellBlockCell *cb_cell;
  gboolean result;
  BasicCell *cell;
  CellBlock *cb;
  int cell_row;
  int cell_col;
  char * old_value;

  if (table == NULL)
    return FALSE;

  cb = table->current_cursor;

  cell_row = virt_loc.phys_row_offset;
  cell_col = virt_loc.phys_col_offset;

  cb_cell = gnc_cellblock_get_cell (cb, cell_row, cell_col);
  cell = cb_cell->cell;

  ENTER ("\n");

  if (cell->direct_update == NULL)
    return FALSE;

  old_value = g_strdup(cell->value);

  result = cell->direct_update(cell, cursor_position, start_selection,
                               end_selection, gui_data);

  if (safe_strcmp(old_value, cell->value) != 0)
  {
    cell->changed = GNC_CELL_CHANGED;
    *newval_ptr = cell->value;
  }
  else
    *newval_ptr = NULL;

  g_free (old_value);

  if (table->set_help)
  {
    char *help_str;

    help_str = xaccBasicCellGetHelp(cell);

    table->set_help(table, help_str);

    g_free(help_str);
  }

  return result;
}

/* ==================================================== */

static gboolean
gnc_table_find_valid_row_vert (Table *table, VirtualLocation *virt_loc)
{
  VirtualLocation vloc;
  VirtualCell *vcell = NULL;
  int top;
  int bottom;

  if (table == NULL)
    return FALSE;

  if (virt_loc == NULL)
    return FALSE;

  vloc = *virt_loc;

  if (vloc.vcell_loc.virt_row < 1)
    vloc.vcell_loc.virt_row = 1;
  if (vloc.vcell_loc.virt_row >= table->num_virt_rows)
    vloc.vcell_loc.virt_row = table->num_virt_rows - 1;

  top  = vloc.vcell_loc.virt_row;
  bottom = vloc.vcell_loc.virt_row + 1;

  while (top >= 1 || bottom < table->num_virt_rows)
  {
    vloc.vcell_loc.virt_row = top;
    vcell = gnc_table_get_virtual_cell (table, vloc.vcell_loc);
    if (vcell && vcell->cellblock && vcell->visible)
      break;

    vloc.vcell_loc.virt_row = bottom;
    vcell = gnc_table_get_virtual_cell (table, vloc.vcell_loc);
    if (vcell && vcell->cellblock && vcell->visible)
      break;

    top--;
    bottom++;
  }

  if (!vcell || !vcell->cellblock || !vcell->visible)
    return FALSE;

  if (vloc.phys_row_offset < 0)
    vloc.phys_row_offset = 0;
  if (vloc.phys_row_offset >= vcell->cellblock->num_rows)
    vloc.phys_row_offset = vcell->cellblock->num_rows - 1;

  *virt_loc = vloc;

  return TRUE;
}

/* ==================================================== */

static gboolean
gnc_table_find_valid_cell_horiz(Table *table,
                                VirtualLocation *virt_loc,
                                gboolean exact_cell)
{
  VirtualLocation vloc;
  VirtualCell *vcell;
  int left;
  int right;

  if (table == NULL)
    return FALSE;

  if (virt_loc == NULL)
    return FALSE;

  if (gnc_table_virtual_cell_out_of_bounds (table, virt_loc->vcell_loc))
    return FALSE;

  if (gnc_table_virtual_loc_valid(table, *virt_loc, exact_cell))
    return TRUE;

  vloc = *virt_loc;

  vcell = gnc_table_get_virtual_cell (table, vloc.vcell_loc);
  if (vcell == NULL)
    return FALSE;
  if (vcell->cellblock == NULL)
    return FALSE;

  if (vloc.phys_col_offset < 0)
    vloc.phys_col_offset = 0;
  if (vloc.phys_col_offset >= vcell->cellblock->num_cols)
    vloc.phys_col_offset = vcell->cellblock->num_cols - 1;

  left  = vloc.phys_col_offset - 1;
  right = vloc.phys_col_offset + 1;

  while (left >= 0 || right < vcell->cellblock->num_cols)
  {
    vloc.phys_col_offset = right;
    if (gnc_table_virtual_loc_valid(table, vloc, FALSE))
    {
      *virt_loc = vloc;
      return TRUE;
    }

    vloc.phys_col_offset = left;
    if (gnc_table_virtual_loc_valid(table, vloc, FALSE))
    {
      *virt_loc = vloc;
      return TRUE;
    }

    left--;
    right++;
  }

  return FALSE;
}

/* ==================================================== */

gboolean
gnc_table_find_close_valid_cell (Table *table, VirtualLocation *virt_loc,
                                 gboolean exact_pointer)
{
  if (!gnc_table_find_valid_row_vert (table, virt_loc))
    return FALSE;

  return gnc_table_find_valid_cell_horiz (table, virt_loc, exact_pointer);
}

/* ==================================================== */

gboolean
gnc_table_move_tab (Table *table,
                    VirtualLocation *virt_loc,
                    gboolean move_right)
{
  VirtualCell *vcell;
  VirtualLocation vloc;

  if ((table == NULL) || (virt_loc == NULL))
    return FALSE;

  vloc = *virt_loc;

  vcell = gnc_table_get_virtual_cell (table, vloc.vcell_loc);
  if ((vcell == NULL) || (vcell->cellblock == NULL) || !vcell->visible)
    return FALSE;

  while (1)
  {
    CellBlockCell *cb_cell;
    CellIOFlags io_flags;

    if (move_right)
    {
      vloc.phys_col_offset++;

      if (vloc.phys_col_offset >= vcell->cellblock->num_cols)
      {
        if (!gnc_table_move_vertical_position (table, &vloc, 1))
          return FALSE;

        vloc.phys_col_offset = 0;
      }
    }
    else
    {
      vloc.phys_col_offset--;

      if (vloc.phys_col_offset < 0)
      {
        if (!gnc_table_move_vertical_position (table, &vloc, -1))
          return FALSE;

        vloc.phys_col_offset = vcell->cellblock->num_cols - 1;
      }
    }

    vcell = gnc_table_get_virtual_cell (table, vloc.vcell_loc);
    if ((vcell == NULL) || (vcell->cellblock == NULL) || !vcell->visible)
      return FALSE;

    cb_cell = gnc_cellblock_get_cell (vcell->cellblock,
                                      vloc.phys_row_offset,
                                      vloc.phys_col_offset);
    if ((cb_cell == NULL) || (cb_cell->cell == NULL))
      continue;

    io_flags = gnc_table_get_io_flags (table, vloc);

    if (!(io_flags & XACC_CELL_ALLOW_INPUT))
      continue;

    if (io_flags & XACC_CELL_ALLOW_EXACT_ONLY)
      continue;

    break;
  }

  {
    gboolean changed = !virt_loc_equal (vloc, *virt_loc);

    *virt_loc = vloc;

    return changed;
  }
}

/* ==================================================== */

gboolean
gnc_table_move_vertical_position (Table *table,
                                  VirtualLocation *virt_loc,
                                  int phys_row_offset)
{
  VirtualLocation vloc;
  VirtualCell *vcell;
  gint last_visible_row;

  if ((table == NULL) || (virt_loc == NULL))
    return FALSE;

  vloc = *virt_loc;
  last_visible_row = vloc.vcell_loc.virt_row;

  vcell = gnc_table_get_virtual_cell (table, vloc.vcell_loc);
  if ((vcell == NULL) || (vcell->cellblock == NULL))
    return FALSE;

  while (phys_row_offset != 0)
  {
    /* going up */
    if (phys_row_offset < 0)
    {
      phys_row_offset++;

      /* room left in the current cursor */
      if (vloc.phys_row_offset > 0)
      {
        vloc.phys_row_offset--;
        continue;
      }

      /* end of the line */
      if (vloc.vcell_loc.virt_row == 1)
        break;

      do
      {
        vloc.vcell_loc.virt_row--;

        vcell = gnc_table_get_virtual_cell (table, vloc.vcell_loc);
      } while (vcell && vcell->cellblock && !vcell->visible);

      if (!vcell || !vcell->cellblock)
        break;

      last_visible_row = vloc.vcell_loc.virt_row;
      vloc.phys_row_offset = vcell->cellblock->num_rows - 1;
    }
    /* going down */
    else
    {
      phys_row_offset--;

      /* room left in the current cursor */
      if (vloc.phys_row_offset < (vcell->cellblock->num_rows - 1))
      {
        vloc.phys_row_offset++;
        continue;
      }

      /* end of the line */
      if (vloc.vcell_loc.virt_row == (table->num_virt_rows - 1))
        break;

      do
      {
        vloc.vcell_loc.virt_row++;

        vcell = gnc_table_get_virtual_cell (table, vloc.vcell_loc);
      } while (vcell && vcell->cellblock && !vcell->visible);

      if (!vcell || !vcell->cellblock)
        break;

      last_visible_row = vloc.vcell_loc.virt_row;
      vloc.phys_row_offset = 0;
    }
  }

  vloc.vcell_loc.virt_row = last_visible_row;

  {
    gboolean changed = !virt_loc_equal (vloc, *virt_loc);

    *virt_loc = vloc;

    return changed;
  }
}

/* ==================================================== */

gboolean
gnc_table_traverse_update(Table *table,
                          VirtualLocation virt_loc,
                          gncTableTraversalDir dir,
                          VirtualLocation *dest_loc)
{
  CellBlock *cb;
  gboolean abort_move;

  if ((table == NULL) || (dest_loc == NULL))
    return FALSE;

  cb = table->current_cursor;

  ENTER("proposed (%d %d) -> (%d %d)\n",
        virt_loc.vcell_loc.virt_row, virt_loc.vcell_loc.virt_row,
        dest_loc->vcell_loc.virt_row, dest_loc->vcell_loc.virt_col);

  /* first, make sure our destination cell is valid. If it is out
   * of bounds report an error. I don't think this ever happens. */
  if (gnc_table_virtual_cell_out_of_bounds (table, dest_loc->vcell_loc))
  {
    PERR("destination (%d, %d) out of bounds (%d, %d)\n",
         dest_loc->vcell_loc.virt_row, dest_loc->vcell_loc.virt_col,
         table->num_virt_rows, table->num_virt_cols);
    return TRUE;
  }

  /* next, check the current row and column.  If they are out of bounds
   * we can recover by treating the traversal as a mouse point. This can
   * occur whenever the register widget is resized smaller, maybe?. */
  if (!gnc_table_virtual_loc_valid (table, virt_loc, TRUE))
  {
    PINFO("source (%d, %d) out of bounds (%d, %d)\n",
	  virt_loc.vcell_loc.virt_row, virt_loc.vcell_loc.virt_col,
          table->num_virt_rows, table->num_virt_cols);

    dir = GNC_TABLE_TRAVERSE_POINTER;
  }

  /* process forward-moving traversals */
  switch (dir)
  {
    case GNC_TABLE_TRAVERSE_RIGHT:
    case GNC_TABLE_TRAVERSE_LEFT:      
      gnc_table_find_valid_cell_horiz(table, dest_loc, FALSE);

      break;

    case GNC_TABLE_TRAVERSE_UP:
    case GNC_TABLE_TRAVERSE_DOWN:
      {
	VirtualLocation new_loc = *dest_loc;
	int increment;

	/* Keep going in the specified direction until we find a valid
	 * row to land on, or we hit the end of the table. At the end,
	 * turn around and go back until we find a valid row or we get
	 * to where we started. If we still can't find anything, try
	 * going left and right. */
	increment = (dir == GNC_TABLE_TRAVERSE_DOWN) ? 1 : -1;

	while (!gnc_table_virtual_loc_valid(table, new_loc, FALSE))
	{
	  if (virt_loc_equal (new_loc, virt_loc))
          {
            new_loc = *dest_loc;
            gnc_table_find_valid_cell_horiz(table, &new_loc, FALSE);
            break;
          }

          if (!gnc_table_move_vertical_position (table, &new_loc, increment))
          {
	    increment *= -1;
	    new_loc = *dest_loc;
	  }
	}

	*dest_loc = new_loc;
      }

      if (!gnc_table_virtual_loc_valid(table, *dest_loc, FALSE))
	return TRUE;

      break;

    case GNC_TABLE_TRAVERSE_POINTER:
      if (!gnc_table_find_valid_cell_horiz(table, dest_loc, TRUE))
        return TRUE;

      break;

    default:
      /* shouldn't be reached */
      assert(0);
      break;
  }

  /* Call the table traverse callback for any modifications. */
  if (table->traverse)
    abort_move = table->traverse (table, dest_loc, dir);
  else
    abort_move = FALSE;

  LEAVE("dest_row = %d, dest_col = %d\n",
        dest_loc->vcell_loc.virt_row, dest_loc->vcell_loc.virt_col);

  return abort_move;
}

/* ================== end of file ======================= */
