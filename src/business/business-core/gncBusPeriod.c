
XXX TODO:
-- billterms is incompletely cloned, not sure what to do 
   with refcount, ask warlord, need to explore.

-- taxtable is incompletely cloned, not sure what to do 
   with certain fields, ask warlord

-- customer incomplete cloned, neeed to handle jobs

-- Ask warlord if all bill terms should be copied, or just
   thiose on customers ... 


#include "gncBusiness.h"

/** Copy all customers from one book to another.  Do this
 *  by iterating over all existing customers in the src book,
 *  and creating a clone for the dest book. */

void
gncCustomerCopyAll (QofBook *dest_book, QofBook *src_book)
{
  
}

static void 
bill_term_copy_helper (gpointer object, gpointer user_data)
{
  QofBook *dest_book = user_data;
  gncBillTerm *src_term = object;
  gncCloneBillTerm (src_term, dest_book);
}

void
gncBillTermCopyAll (QofBook *dest_book, QofBook *src_book)
{
   qof_object_foreach (GNC_BILLTERM_MODULE_NAME, 
                   src_book, bill_term_copy_helper, dest_book);
}



partition (QofBook *dest_book, QofBook *src_book)
{

  /* Copy all bill terms first, since the CustomerCopy expects
   * these to be in place already. */
/* XXX not strictly needed, the customer can pull thier own .. ? */
  gncBillTermCopyAll (dest_book, src_book);
}


