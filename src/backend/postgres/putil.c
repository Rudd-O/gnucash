/********************************************************************\
 * putil.c -- utility macros for the postgres backend               *
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
\********************************************************************/

/* 
 * FILE:
 * putil.c
 *
 * FUNCTION:
 * Postgres backend utility macros
 *
 * HISTORY:
 * Copyright (c) 2002 Matthew Vanecek <mevanecek@yahoo.com>
 * 
 */

#include <glib.h> 
#include <libpq-fe.h>  
#include <stdlib.h>  
#include "qofbackend.h"
#include "qofbackend-p.h"
#include "gnc-engine-util.h" 
#include "PostgresBackend.h"

static short module = MOD_BACKEND;

#include "putil.h"

/* ============================================================= */
/* The sendQuery function sends the sql statement off to the server. 
 * It performs a minimal check to see that the send succeeded. The
 * return value indicates success or failure of the send.
 */
int sendQuery(PGBackend *be,char * buff) {
   int rc = 0;

   ENTER(" ");
   if (NULL == be->connection) return ERR_BACKEND_CONN_LOST;
   PINFO("Connectionn is %p", be->connection);
   PINFO ("sending query %s", buff);
   rc = PQsendQuery (be->connection, buff);
   if (!rc) {
      gchar * msg = (gchar *)PQerrorMessage(be->connection);
      PERR("send query failed:\n"
           "\t%s", msg);
      qof_backend_set_message(&be->be, msg);
      qof_backend_set_error (&be->be, ERR_BACKEND_SERVER_ERR);
      return ERR_BACKEND_SERVER_ERR;
   }
   LEAVE("PQsendQuery rc = %d", rc);
   return ERR_BACKEND_NO_ERR;
}

/* --------------------------------------------------------------- */
/* The finishQuery function makes sure that the previously sent
 * query completed with no errors.  It assumes that the query
 * does not produce any results; if it did those results are
 * discarded (only error conditions are checked for).  The number of
 * rows affected by the query is returned.
 */

int finishQuery(PGBackend *be) {
   int i=0;
   PGresult *result;

   ENTER(" ");
   /* complete/commit the transaction, check the status */
   PINFO("Connection is %p", be->connection);
   do {
      int x;
      ExecStatusType status;
      result = PQgetResult(be->connection);
      if (!result) {
          PINFO("Result is (null)");
          break;
      }
      
      status = PQresultStatus(result);
      x = atoi(PQcmdTuples(result));
      PINFO("Result status: %s, rows affected: %d, by %s",
            PQresStatus(status), x, PQcmdStatus(result));
      i += x;

      if (PGRES_COMMAND_OK != status) {
         gchar * msg = (gchar *)PQerrorMessage(be->connection);
         PERR("finish query failed:\n\t%s", msg);
         PQclear(result);
         qof_backend_set_message(&be->be, msg);
         qof_backend_set_error (&be->be, ERR_BACKEND_SERVER_ERR);
         break; 
      }
      PQclear(result);
   } while (result);

   LEAVE("%d rows affected by SQL statement", i);
   return i;
}
