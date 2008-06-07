/********************************************************************
 * gnc-backend-sql.h: load and save data to SQL                     *
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
/** @file gnc-backend-sql.h
 *  @brief load and save data to SQL
 *  @author Copyright (c) 2006-2008 Phil Longstaff <plongstaff@rogers.com>
 *
 * This file implements the top-level QofBackend API for saving/
 * restoring data to/from an SQL database
 */

#ifndef GNC_BACKEND_SQL_H_
#define GNC_BACKEND_SQL_H_

#include <gmodule.h>

void gnc_sql_init( GncSqlBackend* be );
void gnc_sql_load( GncSqlBackend* be, QofBook *book );
void gnc_sql_sync_all( GncSqlBackend* be, QofBook *book );
void gnc_sql_begin_edit( QofBackend* qbe, QofInstance *inst );
void gnc_sql_rollback_edit( QofBackend* qbe, QofInstance *inst );
void gnc_sql_commit_edit( QofBackend* qbe, QofInstance *inst );

G_MODULE_EXPORT void
qof_backend_module_init(void);

#endif /* GNC_BACKEND_SQL_H_ */
