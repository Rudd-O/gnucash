
#ifndef _SIXTP_WRITERS_H_
#define _SIXTP_WRITERS_H_

#include <config.h>

#include <glib.h>

#include "gnc-xml-helper.h"
#include "Query.h"

gboolean xml_add_account_restorers(xmlNodePtr p, AccountGroup *g);

gboolean xml_add_commodity_restorers(xmlNodePtr p);

gboolean xml_add_commodity_ref(xmlNodePtr p, const char *tag,
                               const gnc_commodity *c);

gboolean xml_add_query_restorers(xmlNodePtr p, Query *q);

gboolean xml_add_txn_and_split_restorers(xmlNodePtr p, AccountGroup *g);



#endif /* _SIXTP_WRITERS_H_ */
