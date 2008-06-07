/********************************************************************
 * gnc-backend-gda.h: load and save data to SQL via libgda          *
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
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
\********************************************************************/
/** @file gnc-backend-gda.h
 *  @brief load and save data to SQL via libgda
 *  @author Copyright (c) 2006-2008 Phil Longstaff <plongstaff@rogers.com>
 *
 * This file implements the top-level QofBackend API for saving/
 * restoring data to/from an SQL database via libgda
 */

#ifndef GNC_BACKEND_GDA_H_
#define GNC_BACKEND_GDA_H_

#include <gmodule.h>

G_MODULE_EXPORT void
qof_backend_module_init(void);

struct GncGdaBackend_struct
{
  GncSqlBackend sql_be;

  GdaConnection* pConnection;
  GdaSqlParser* parser;
  GdaDataHandler* timespecDH;
  GdaDataHandler* dateDH;

  QofBook *primary_book;	/* The primary, main open book */
  gboolean	loading;		/* We are performing an initial load */
  gboolean  in_query;
  gboolean  supports_transactions;
  gboolean  is_pristine_db;	// Are we saving to a new pristine db?

  gint obj_total;			// Total # of objects (for percentage calculation)
  gint operations_done;		// Number of operations (save/load) done
  GHashTable* versions;		// Version number for each table
};
typedef struct GncGdaBackend_struct GncGdaBackend;

#endif /* GNC_BACKEND_GDA_H_ */
