/*
 * gncEntryLedgerModel.c -- Model for GncEntry ledger
 * Copyright (C) 2001,2002 Derek Atkins
 * Author: Derek Atkins <warlord@MIT.EDU>
 */

#define _GNU_SOURCE

#include "config.h"

#include <glib.h>

#include "messages.h"
#include "Account.h"
#include "gnc-ui-util.h"

#include "datecell.h"

#include "gncEntryLedgerP.h"
#include "gncEntryLedgerModel.h"

static GncEntryLedgerColors reg_colors =
{
  0xffffff, /* white */
  0xffffff,
  0xffffff,

  0xffffff,
  0xffffff
};

/** Private Interfaces ***********************************************/

/* GET_LABEL */

static const char * get_acct_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Account");
}

static const char * get_actn_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Action");
}

static const char * get_date_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Date");
}

static const char * get_desc_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Description");
}

static const char * get_disc_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Discount");
}

static const char * get_distype_label (VirtualLocation virt_loc, gpointer data)
{
  return _("DT");
}

static const char * get_dishow_label (VirtualLocation virt_loc, gpointer data)
{
  return _("DH");
}

static const char * get_pric_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Unit Price");
}

static const char * get_qty_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Quantity");
}

static const char * get_taxtable_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Tax Table");
}

static const char * get_taxable_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Taxable?");
}

static const char * get_taxincluded_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Tax Included?");
}

static const char * get_inv_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Invoiced?");
}

static const char * get_value_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Subtotal");
}

static const char * get_taxval_label (VirtualLocation virt_loc, gpointer data)
{
  return _("Tax");
}

/* GET_ENTRY */

static const char * get_acct_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  static char *name = NULL;

  GncEntryLedger *ledger = user_data;
  GncEntry *entry;

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);

  g_free (name);
  name = xaccAccountGetFullName (gncEntryGetAccount (entry),
				 gnc_get_account_separator ());
  return name;
}

static const char * get_actn_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry;

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);
  return gncEntryGetAction  (entry);
}

static const char * get_date_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry;
  Timespec ts;

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);

  ts = gncEntryGetDate (entry);
  return gnc_print_date (ts);
}

static const char * get_desc_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry;

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);
  return gncEntryGetDescription (entry);
}

static const char * get_disc_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry;
  gnc_numeric discount;

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);
  discount = gncEntryGetDiscount (entry);
  if (gnc_numeric_zero_p (discount))
    return NULL;

  return xaccPrintAmount (discount, gnc_default_print_info (FALSE));
}

static const char * get_distype_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry;
  char type;

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);
  type = gncEntryGetDiscountType (entry);

  if (translate) {
    return gnc_entry_ledger_type_string_getter (type + '0');
  } else {
    static char s[2];
    s[0] = '0' + type;
    s[1] = '\0';
    return s;
  }
}

static const char * get_dishow_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry;
  char type;

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);
  type = gncEntryGetDiscountHow (entry);

  if (translate) {
    return gnc_entry_ledger_how_string_getter (type + '0');
  } else {
    static char s[2];
    s[0] = '0' + type;
    s[1] = '\0';
    return s;
  }
}

static const char * get_pric_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry;
  gnc_numeric price;

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);
  price = gncEntryGetPrice (entry);

  if (gnc_numeric_zero_p (price))
    return NULL;

  return xaccPrintAmount (price, gnc_default_print_info (FALSE));
}

static const char * get_qty_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry;
  gnc_numeric qty;

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);
  qty = gncEntryGetQuantity (entry);

  if (gnc_numeric_zero_p (qty))
    return NULL;

  return xaccPrintAmount (qty, gnc_default_print_info (FALSE));
}

static const char * get_taxable_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry;
  static char s[2] = { ' ', '\0' };

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);
  if (gncEntryGetTaxable (entry))
    s[0] = 'X';
  else
    s[0] = ' ';

  return s;
}

static gboolean
gnc_entry_ledger_get_taxable_value (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  gboolean is_current;

  is_current = virt_cell_loc_equal(ledger->table->current_cursor_loc.vcell_loc,
				   virt_loc.vcell_loc);
  if (is_current)
    return gnc_entry_ledger_get_checkmark (ledger, ENTRY_TAXABLE_CELL);
  else {
    const char *valstr =
      get_taxable_entry (virt_loc, translate, conditionally_changed,
			 user_data);
    if (valstr && *valstr == 'X')
      return TRUE;
  }
  return FALSE;
}

static const char * get_taxtable_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry;
  GncTaxTable *table;
  gboolean taxable;

  /* load the cell properly; just shadow the value */
  if (!conditionally_changed) {
    taxable = gnc_entry_ledger_get_taxable_value (virt_loc, translate,
						  conditionally_changed,
						  user_data);
    if (!taxable)
      return NULL;
  }

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);
  table = gncEntryGetTaxTable (entry);
  
  return gncTaxTableGetName (table);
}

static const char * get_taxincluded_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry;
  static char s[2] = { ' ', '\0' };
  gboolean taxable;

  /* load the cell properly; just shadow the value */
  if (!conditionally_changed) {
    taxable = gnc_entry_ledger_get_taxable_value (virt_loc, translate,
						  conditionally_changed,
						  user_data);
    if (!taxable)
      return NULL;
  }

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);
  if (gncEntryGetTaxIncluded (entry))
    s[0] = 'X';
  else
    s[0] = ' ';

  return s;
}

static const char * get_inv_entry (VirtualLocation virt_loc,
				    gboolean translate,
				    gboolean *conditionally_changed,
				    gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry;
  static char s[2] = { ' ', '\0' };

  entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);
  if (gncEntryGetInvoice (entry) == NULL)
    s[0] = ' ';
  else
    s[0] = 'X';

  /* XXX: what if this entry doesn't belong to this invoice?
   * Or, better question, what if this is the blank_entry on
   * an invoice page?  For the latter, don't worry about it;
   * it will be added automatically during the Save operation
   */

  return s;
}

static const char * get_value_entry (VirtualLocation virt_loc,
                                      gboolean translate,
                                      gboolean *conditionally_changed,
                                      gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  gnc_numeric value;

  /* Check if this is the current cursor */
  if (virt_cell_loc_equal (ledger->table->current_cursor_loc.vcell_loc,
			   virt_loc.vcell_loc)) {
    gnc_entry_ledger_compute_value (ledger, &value, NULL);
  } else {
    GncEntry *entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);

    if (entry == gnc_entry_ledger_get_blank_entry (ledger))
      return NULL;

    value = gncEntryReturnValue (entry);
  }
  return xaccPrintAmount (value, gnc_default_print_info (TRUE));
}

static const char * get_taxval_entry (VirtualLocation virt_loc,
                                      gboolean translate,
                                      gboolean *conditionally_changed,
                                      gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  gnc_numeric value;

  /* Check if this is the current cursor */
  if (virt_cell_loc_equal (ledger->table->current_cursor_loc.vcell_loc,
			   virt_loc.vcell_loc)) {
    gnc_entry_ledger_compute_value (ledger, NULL, &value);
  } else {
    GncEntry *entry = gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);

    if (entry == gnc_entry_ledger_get_blank_entry (ledger))
      return NULL;

    value = gncEntryReturnTaxValue (entry);
  }

  return xaccPrintAmount (value, gnc_default_print_info (TRUE));
}

/* GET_HELP */

static char * get_acct_help (VirtualLocation virt_loc, gpointer user_data)
{
  const char *help;
  GncEntryLedger *ledger = user_data;

  help = gnc_table_get_entry (ledger->table, virt_loc);
  if (!help || *help == '\0')
    help = _("Enter the income/expense account for the Entry, "
	     "or choose one from the list");

  return g_strdup (help);
}

static char * get_actn_help (VirtualLocation virt_loc, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  const char *help;

  help = gnc_table_get_entry (ledger->table, virt_loc);
  if (!help || *help == '\0')
    help = _("Enter the type of Entry");

  return g_strdup (help);
}

static char * get_date_help (VirtualLocation virt_loc, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  BasicCell *cell;
  char string[1024];
  struct tm *tm;
  Timespec ts;
  time_t tt;

  cell = gnc_table_get_cell (ledger->table, virt_loc);
  if (!cell)
    return NULL;

  if (!cell->value || *cell->value == '\0')
    return NULL;

  gnc_date_cell_get_date ((DateCell *) cell, &ts);
  tt = ts.tv_sec;
  tm = localtime (&tt);
  strftime (string, sizeof(string), "%A %d %B %Y", tm);

  return g_strdup (string);
}

static char * get_desc_help (VirtualLocation virt_loc, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  const char *help;

  help = gnc_table_get_entry (ledger->table, virt_loc);
  if (!help || *help == '\0')
    help = _("Enter the Entry Description");

  return g_strdup (help);
}

static char * get_disc_help (VirtualLocation virt_loc, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  const char *help;
  gint type;

  type = gnc_entry_ledger_get_type (ledger, ENTRY_DISTYPE_CELL);

  switch (type) {
  case GNC_AMT_TYPE_VALUE:
    help = _("Enter the Discount Amount");
    break;
  case GNC_AMT_TYPE_PERCENT:
    help = _("Enter the Discount Percent");
    break;
  default:
    help = _("Enter the Discount ... unknown type");
    break;
  }

  return g_strdup (help);
}

static char * get_distype_help (VirtualLocation virt_loc, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  const char *help;
  gint type;

  type = gnc_entry_ledger_get_type (ledger, ENTRY_DISTYPE_CELL);

  switch (type) {
  case GNC_AMT_TYPE_VALUE:
    help = _("Discount Type: Monetary Value");
    break;
  case GNC_AMT_TYPE_PERCENT:
    help = _("Discount Type: Percent");
    break;
  default:
    help = _("Select the Discount Type");
    break;
  }
  return g_strdup (help);
}

static char * get_dishow_help (VirtualLocation virt_loc, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  const char *help;
  gint type;

  type = gnc_entry_ledger_get_type (ledger, ENTRY_DISHOW_CELL);

  switch (type) {
  case GNC_DISC_PRETAX:
    help = _("Tax computed after discount is applied");
    break;
  case GNC_DISC_SAMETIME:
    help = _("Discount and tax both applied on pretax value");
    break;
  case GNC_DISC_POSTTAX:
    help = _("Discount computed after tax is applied");
    break;
  default:
    help = _("Select how to compute the Discount and Taxes");
    break;
  }
  return g_strdup (help);
}

static char * get_pric_help (VirtualLocation virt_loc, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  const char *help;

  help = gnc_table_get_entry (ledger->table, virt_loc);
  if (!help || *help == '\0')
    help = _("Enter the unit-Price for this Entry");

  return g_strdup (help);
}

static char * get_qty_help (VirtualLocation virt_loc, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  const char *help;

  help = gnc_table_get_entry (ledger->table, virt_loc);
  if (!help || *help == '\0')
    help = _("Enter the Quantity of units for this Entry");

  return g_strdup (help);
}

static char * get_taxtable_help (VirtualLocation virt_loc, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  const char *help;

  help = gnc_table_get_entry (ledger->table, virt_loc);
  if (!help || *help == '\0')
    help = _("Enter the Tax Table to apply to this entry");

  return g_strdup (help);
}

static char * get_taxable_help (VirtualLocation virt_loc, gpointer user_data)
{
  const char *help;

  help = _("Is this entry taxable?");

  return g_strdup (help);
}

static char * get_taxincluded_help (VirtualLocation virt_loc, gpointer user_data)
{
  const char *help;

  help = _("Is the tax already included in the price of this entry?");

  return g_strdup (help);
}

static char * get_inv_help (VirtualLocation virt_loc, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  const char *help;

  switch (ledger->type) {
  case GNCENTRY_ORDER_ENTRY:
  case GNCENTRY_ORDER_VIEWER:
    help = _("Is this entry Invoiced?");
    break;
  case GNCENTRY_INVOICE_ENTRY:
  case GNCENTRY_INVOICE_VIEWER:
    help = _("Include this entry on this invoice?");
    break;
  default:
    help = _("Unknown EntryLedger Type");
  }

  return g_strdup (help);
}

static char * get_value_help (VirtualLocation virt_loc, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  const char *help;

  help = gnc_table_get_entry (ledger->table, virt_loc);
  if (!help || *help == '\0')
    help = _("The subtotal value of this entry ");

  return g_strdup (help);
}

static char * get_taxval_help (VirtualLocation virt_loc, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  const char *help;

  help = gnc_table_get_entry (ledger->table, virt_loc);
  if (!help || *help == '\0')
    help = _("The total tax of this entry ");

  return g_strdup (help);
}

/* GET_IO_FLAGS */

static CellIOFlags get_standard_io_flags (VirtualLocation virt_loc,
					  gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  switch (ledger->type) {
  case GNCENTRY_ORDER_ENTRY:
    {
      GncEntry *entry =
	gnc_entry_ledger_get_entry (ledger, virt_loc.vcell_loc);

      /*
       * If the type is an order_entry and the entry was invoiced,
       * make the entry immutable
       */
      if (gncEntryGetInvoice (entry) != NULL)
	return XACC_CELL_ALLOW_SHADOW;
    }
    /* FALLTHROUGH */
  default:
    return XACC_CELL_ALLOW_ALL;
  }
}

static CellIOFlags get_typecell_io_flags (VirtualLocation virt_loc,
					  gpointer user_data)
{
  return (get_standard_io_flags (virt_loc, user_data) |
	  XACC_CELL_ALLOW_EXACT_ONLY);
}

static CellIOFlags get_inv_io_flags (VirtualLocation virt_loc,
				     gpointer user_data)
{
  GncEntryLedger *ledger = user_data;

  switch (ledger->type) {
  case GNCENTRY_ORDER_ENTRY:
  case GNCENTRY_ORDER_VIEWER:
  case GNCENTRY_INVOICE_VIEWER:
    return XACC_CELL_ALLOW_SHADOW;
  default:
    return XACC_CELL_ALLOW_ALL | XACC_CELL_ALLOW_EXACT_ONLY;
  }
}

static CellIOFlags get_value_io_flags (VirtualLocation virt_loc,
				       gpointer user_data)
{
  return XACC_CELL_ALLOW_SHADOW;
}

static CellIOFlags get_tax_io_flags (VirtualLocation virt_loc,
				     gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  gboolean taxable;

  taxable = gnc_entry_ledger_get_checkmark (ledger, ENTRY_TAXABLE_CELL);

  /* Only print the taxtable and taxincluded cells if taxable is true */
  if (taxable)
    return get_standard_io_flags (virt_loc, user_data);

  /* Shadow the value, so the cell is loaded properly */
  return XACC_CELL_ALLOW_SHADOW;
}

static CellIOFlags get_taxincluded_io_flags (VirtualLocation virt_loc,
					     gpointer user_data)
{
  CellIOFlags flags = get_tax_io_flags (virt_loc, user_data);
  if (flags == XACC_CELL_ALLOW_SHADOW)
    return flags;
  return flags | XACC_CELL_ALLOW_EXACT_ONLY;
}

/* GET BG_COLORS */

static guint32
gnc_entry_ledger_get_bg_color (VirtualLocation virt_loc,
			       gboolean *hatching, gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  VirtualCell *vcell;
  guint32 bg_color;
  gboolean is_current;

  if (hatching)
    *hatching = FALSE;

  bg_color = 0xffffff; /* white */

  if (!ledger) return bg_color;

  if (gnc_table_virtual_location_in_header (ledger->table, virt_loc))
    return reg_colors.header_bg_color;

  vcell = gnc_table_get_virtual_cell (ledger->table, virt_loc.vcell_loc);
  if (!vcell || !vcell->cellblock)
    return bg_color;

  if ((virt_loc.phys_col_offset < vcell->cellblock->start_col) ||
      (virt_loc.phys_col_offset > vcell->cellblock->stop_col))
    return bg_color;

  is_current = virt_cell_loc_equal
    (ledger->table->current_cursor_loc.vcell_loc, virt_loc.vcell_loc);

  if (is_current)
    return vcell->start_primary_color ?
      reg_colors.primary_active_bg_color :
    reg_colors.secondary_active_bg_color;

  return vcell->start_primary_color ?
    reg_colors.primary_bg_color : reg_colors.secondary_bg_color;
}

/* SAVE CELLS */

static void gnc_entry_ledger_save_cells (gpointer save_data,
					 gpointer user_data)
{
  GncEntryLedger *ledger = user_data;
  GncEntry *entry = save_data;

  g_return_if_fail (entry != NULL);

  /* copy the contents from the cursor to the split */

  if (gnc_table_layout_get_cell_changed (ledger->table->layout,
					 ENTRY_ACCT_CELL, TRUE)) {
    Account *acc;

    acc = gnc_entry_ledger_get_account (ledger, ENTRY_ACCT_CELL);

    if (acc != NULL)
      gncEntrySetAccount (entry, acc);
  }

  if (gnc_table_layout_get_cell_changed (ledger->table->layout,
					 ENTRY_ACTN_CELL, TRUE)) {
    const char *value;

    value = gnc_table_layout_get_cell_value (ledger->table->layout,
					     ENTRY_ACTN_CELL);
    gncEntrySetAction (entry, value);
  }

  if (gnc_table_layout_get_cell_changed (ledger->table->layout,
					 ENTRY_DATE_CELL, TRUE)) {
    BasicCell *cell;
    Timespec ts;

    cell = gnc_table_layout_get_cell (ledger->table->layout, ENTRY_DATE_CELL);

    /* commit any pending changes */
    gnc_date_cell_commit ((DateCell *) cell);

    gnc_date_cell_get_date ((DateCell *) cell, &ts);
    gncEntrySetDate (entry, ts);
  }

  if (gnc_table_layout_get_cell_changed (ledger->table->layout,
					 ENTRY_DESC_CELL, TRUE)) {
    const char *value;

    value = gnc_table_layout_get_cell_value (ledger->table->layout,
					     ENTRY_DESC_CELL);
    gncEntrySetDescription (entry, value);
  }

  if (gnc_table_layout_get_cell_changed (ledger->table->layout,
					 ENTRY_DISC_CELL, TRUE)) {
    gnc_numeric amount;

    if (gnc_entry_ledger_get_numeric (ledger, ENTRY_DISC_CELL, &amount))
	gncEntrySetDiscount (entry, amount);
  }

  if (gnc_table_layout_get_cell_changed (ledger->table->layout,
					 ENTRY_DISTYPE_CELL, TRUE)) {
    gint type;

    type = gnc_entry_ledger_get_type (ledger, ENTRY_DISTYPE_CELL);

    if (type != -1)
      gncEntrySetDiscountType (entry, type);
  }

  if (gnc_table_layout_get_cell_changed (ledger->table->layout,
					 ENTRY_DISHOW_CELL, TRUE)) {
    gint type;

    type = gnc_entry_ledger_get_type (ledger, ENTRY_DISHOW_CELL);

    if (type != -1)
      gncEntrySetDiscountHow (entry, type);
  }

  if (gnc_table_layout_get_cell_changed (ledger->table->layout,
					  ENTRY_PRIC_CELL, TRUE)) {
    gnc_numeric amount;

    if (gnc_entry_ledger_get_numeric (ledger, ENTRY_PRIC_CELL, &amount))
	gncEntrySetPrice (entry, amount);
  }

  if (gnc_table_layout_get_cell_changed (ledger->table->layout,
					 ENTRY_QTY_CELL, TRUE)) {
    gnc_numeric amount;

    if (gnc_entry_ledger_get_numeric (ledger, ENTRY_QTY_CELL, &amount))
	gncEntrySetQuantity (entry, amount);
  }

  if (gnc_table_layout_get_cell_changed (ledger->table->layout,
					 ENTRY_TAXABLE_CELL, TRUE)) {
    gboolean taxable;

    taxable = gnc_entry_ledger_get_checkmark (ledger, ENTRY_TAXABLE_CELL);
    gncEntrySetTaxable (entry, taxable);
  }

  /* XXX: Only (re-set) these if taxable is TRUE? */
  if (gnc_table_layout_get_cell_changed (ledger->table->layout,
					 ENTRY_TAXTABLE_CELL, TRUE)) {
    GncTaxTable *table;

    table = gnc_entry_ledger_get_taxtable (ledger, ENTRY_TAXTABLE_CELL);
    if (table)
      gncEntrySetTaxTable (entry, table);
  }

  if (gnc_table_layout_get_cell_changed (ledger->table->layout,
					 ENTRY_TAXINCLUDED_CELL, TRUE)) {
    gboolean taxincluded;

    taxincluded = gnc_entry_ledger_get_checkmark (ledger,
						  ENTRY_TAXINCLUDED_CELL);
    gncEntrySetTaxIncluded (entry, taxincluded);
  }

  {
    char inv_value;

    inv_value = gnc_entry_ledger_get_inv (ledger, ENTRY_INV_CELL);

    if (inv_value != '\0') {
      /* It's changed.  Note that this should only be changed in
       * INVOICE_ENTRY.  But we don't check here (should we)?
       */

      if (inv_value == 'X') {
	/* Add this to the invoice (if it's not already attached) */
	if (gncEntryGetInvoice (entry) == NULL)
	  gncInvoiceAddEntry (ledger->invoice, entry);

      } else {
	/* Remove from the invoice iff we're attached to an order */
	if (gncEntryGetOrder (entry) != NULL)
	  gncInvoiceRemoveEntry (ledger->invoice, entry);
      }
    }
  }
}

/* Set Cell Handlers */

static void gnc_entry_ledger_model_new_handlers (TableModel *model,
						 GncEntryLedgerType type)
{
  struct model_desc {
    const char * cell;
    gpointer entry_handler;
    gpointer label_handler;
    gpointer help_handler;
    gpointer io_flags_handler;
  } models[] = {
    { ENTRY_ACCT_CELL, get_acct_entry, get_acct_label, get_acct_help, get_standard_io_flags },
    { ENTRY_ACTN_CELL, get_actn_entry, get_actn_label, get_actn_help, get_standard_io_flags },
    { ENTRY_DATE_CELL, get_date_entry, get_date_label, get_date_help, get_standard_io_flags },
    { ENTRY_DESC_CELL, get_desc_entry, get_desc_label, get_desc_help, get_standard_io_flags },
    { ENTRY_DISC_CELL, get_disc_entry, get_disc_label, get_disc_help, get_standard_io_flags },
    { ENTRY_DISTYPE_CELL, get_distype_entry, get_distype_label, get_distype_help,  get_typecell_io_flags },
    { ENTRY_DISHOW_CELL, get_dishow_entry, get_dishow_label, get_dishow_help, get_typecell_io_flags },
    { ENTRY_PRIC_CELL, get_pric_entry, get_pric_label, get_pric_help, get_standard_io_flags },
    { ENTRY_QTY_CELL, get_qty_entry, get_qty_label, get_qty_help, get_standard_io_flags },
    { ENTRY_TAXABLE_CELL, get_taxable_entry, get_taxable_label, get_taxable_help,  get_typecell_io_flags },
    { ENTRY_TAXTABLE_CELL, get_taxtable_entry, get_taxtable_label, get_taxtable_help, get_tax_io_flags },
    { ENTRY_TAXINCLUDED_CELL, get_taxincluded_entry, get_taxincluded_label, get_taxincluded_help, get_taxincluded_io_flags },
    { ENTRY_INV_CELL, get_inv_entry, get_inv_label, get_inv_help, get_inv_io_flags },
    { ENTRY_VALUE_CELL, get_value_entry, get_value_label, get_value_help, get_value_io_flags },
    { ENTRY_TAXVAL_CELL, get_taxval_entry, get_taxval_label, get_taxval_help, get_value_io_flags },
  };
  int i;

  gnc_table_model_set_default_bg_color_handler
    (model, gnc_entry_ledger_get_bg_color);


  for (i = 0; i < (sizeof(models)/sizeof(*models)); i++) {
    if (models[i].entry_handler)
      gnc_table_model_set_entry_handler (model, models[i].entry_handler,
					 models[i].cell);
    if (models[i].label_handler)
      gnc_table_model_set_label_handler (model, models[i].label_handler,
					 models[i].cell);
    if (models[i].help_handler)
      gnc_table_model_set_help_handler (model, models[i].help_handler,
					models[i].cell);
    if (models[i].io_flags_handler)
      gnc_table_model_set_io_flags_handler (model, models[i].io_flags_handler,
					    models[i].cell);
  } /* for */

  /*
  model->cell_data_allocator = ;
  model->cell_data_deallocator = ;
  model->cell_data_copy = ;
  */

  gnc_table_model_set_post_save_handler (model, gnc_entry_ledger_save_cells);

  switch (type) {
  case GNCENTRY_ORDER_VIEWER:
  case GNCENTRY_INVOICE_VIEWER:
    /* make this table read-only */
    gnc_table_model_set_read_only (model, TRUE);
    break;
  default:
    break;
  }
}

/** Public Interface ***********************************************/

TableModel * gnc_entry_ledger_model_new (GncEntryLedgerType type)
{
  TableModel * model;

  model = gnc_table_model_new ();
  gnc_entry_ledger_model_new_handlers (model, type);

  return model;
}

void gnc_entry_ledger_set_colors (GncEntryLedgerColors reg_colors_new)
{
  reg_colors = reg_colors_new;
}
