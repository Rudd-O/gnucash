/*
 * dialog-order.c -- Dialog for Order entry
 * Copyright (C) 2001,2002 Derek Atkins
 * Author: Derek Atkins <warlord@MIT.EDU>
 */

#include "config.h"

#include <gnome.h>

#include "dialog-utils.h"
#include "global-options.h"
#include "gnc-component-manager.h"
#include "gnc-ui.h"
#include "gnc-gui-query.h"
#include "gnc-ui-util.h"
#include "gnc-engine-util.h"
#include "gnucash-sheet.h"
#include "window-help.h"
#include "dialog-search.h"
#include "search-param.h"

#include "gncOrder.h"
#include "gncOrderP.h"

#include "gncEntryLedger.h"

#include "dialog-order.h"
#include "dialog-invoice.h"
#include "business-utils.h"
#include "dialog-date-close.h"

#define DIALOG_NEW_ORDER_CM_CLASS "dialog-new-order"
#define DIALOG_EDIT_ORDER_CM_CLASS "dialog-edit-order"
#define DIALOG_VIEW_ORDER_CM_CLASS "dialog-view-order"

typedef enum
{
  NEW_ORDER,
  EDIT_ORDER,
  VIEW_ORDER
} OrderDialogType;

struct _order_select_window {
  GNCBook *	book;
  GncOwner *	owner;
  QueryNew *	q;
  GncOwner	owner_def;
};

struct _order_window {
  GladeXML *	xml;

  GtkWidget *	dialog;

  GtkWidget *	id_entry;
  GtkWidget *	ref_entry;
  GtkWidget *	notes_text;
  GtkWidget *	opened_date;
  GtkWidget *	closed_date;
  GtkWidget *	active_check;

  GtkWidget *	owner_box;
  GtkWidget *	owner_label;
  GtkWidget *	owner_choice;

  GnucashRegister *	reg;
  GncEntryLedger *	ledger;

  OrderDialogType	dialog_type;
  GUID		order_guid;
  gint		component_id;
  GNCBook *	book;
  GncOrder *	created_order;
  GncOwner	owner;

};

static void gnc_order_update_window (OrderWindow *ow);

static GncOrder *
ow_get_order (OrderWindow *ow)
{
  if (!ow)
    return NULL;

  return gncOrderLookup (ow->book, &ow->order_guid);
}

static void gnc_ui_to_order (OrderWindow *ow, GncOrder *order)
{
  Timespec ts;
  time_t tt;

  /* Do nothing if this is view only */
  if (ow->dialog_type == VIEW_ORDER)
    return;

  gnc_suspend_gui_refresh ();
  
  gncOrderSetID (order, gtk_editable_get_chars
		 (GTK_EDITABLE (ow->id_entry), 0, -1));
  gncOrderSetNotes (order, gtk_editable_get_chars
		    (GTK_EDITABLE (ow->notes_text), 0, -1));
  gncOrderSetReference (order, gtk_editable_get_chars
			(GTK_EDITABLE (ow->ref_entry), 0, -1));

  tt = gnome_date_edit_get_date (GNOME_DATE_EDIT (ow->opened_date));
  timespecFromTime_t (&ts, tt);
  gncOrderSetDateOpened (order, ts);

  if (ow->active_check)
    gncOrderSetActive (order, gtk_toggle_button_get_active
		       (GTK_TOGGLE_BUTTON (ow->active_check)));

  gnc_owner_get_owner (ow->owner_choice, &(ow->owner));
  gncOrderSetOwner (order, &(ow->owner));

  gncOrderCommitEdit (order);
  gnc_resume_gui_refresh ();
}

static gboolean
gnc_order_window_verify_ok (OrderWindow *ow)
{
  const char *res;

  /* Check the ID */
  res = gtk_entry_get_text (GTK_ENTRY (ow->id_entry));
  if (safe_strcmp (res, "") == 0) {
    gnc_error_dialog_parented (GTK_WINDOW (ow->dialog),
			       _("The Order must be given an ID."));
    return FALSE;
  }

  /* Check the Owner */
  gnc_owner_get_owner (ow->owner_choice, &(ow->owner));
  res = gncOwnerGetName (&(ow->owner));
  if (res == NULL || safe_strcmp (res, "") == 0) {
    gnc_error_dialog_parented (GTK_WINDOW (ow->dialog),
  			       _("You need to supply Billing Information."));
    return FALSE;
  }

  return TRUE;
}

static gboolean
gnc_order_window_ok_save (OrderWindow *ow)
{
  if (!gnc_entry_ledger_check_close (ow->dialog, ow->ledger))
    return FALSE;

  if (!gnc_order_window_verify_ok (ow))
    return FALSE;
  
  /* Now save it off */
  {
    GncOrder *order = ow_get_order (ow);
    if (order) {
      gnc_ui_to_order (ow, order);

    }
    ow->created_order = order;
  }
  return TRUE;
}

static void
gnc_order_window_ok_cb (GtkWidget *widget, gpointer data)
{
  OrderWindow *ow = data;

  if (!gnc_order_window_ok_save (ow))
    return;

  /* Ok, we don't need this anymore */
  ow->order_guid = *xaccGUIDNULL ();

  gnc_close_gui_component (ow->component_id);
}

static void
gnc_order_window_cancel_cb (GtkWidget *widget, gpointer data)
{
  OrderWindow *ow = data;

  gnc_close_gui_component (ow->component_id);
}

static void
gnc_order_window_help_cb (GtkWidget *widget, gpointer data)
{
  char *help_file = "";		/* XXX: FIXME */

  helpWindow(NULL, NULL, help_file);
}

static void
gnc_order_window_invoice_cb (GtkWidget *widget, gpointer data)
{
  OrderWindow *ow = data;

  /* make sure we're ok */
  if (!gnc_order_window_verify_ok (ow))
      return;

  /* Ok, go make an invoice */
  gnc_invoice_search (NULL, &(ow->owner), ow->book); 

  /* refresh the window */
  gnc_order_update_window (ow);
}

static void
gnc_order_window_close_order_cb (GtkWidget *widget, gpointer data)
{
  OrderWindow *ow = data;
  GncOrder *order;
  GList *entries;
  char *message, *label;
  gboolean non_inv = FALSE;
  Timespec ts;

  /* Make sure the order is ok */
  if (!gnc_order_window_verify_ok (ow))
    return;

  /* Make sure the order exists */
  order = ow_get_order (ow);
  if (!order)
    return;

  /* Check that there is at least one Entry */
  if (gncOrderGetEntries (order) == NULL) {
    gnc_error_dialog_parented (GTK_WINDOW (ow->dialog),
			       _("The Order must have at least one Entry."));
    return;
  }

  /* Make sure we can close the order. Are there any uninvoiced entries? */
  entries = gncOrderGetEntries (order);
  for ( ; entries ; entries = entries->next) {
    GncEntry *entry = entries->data;
    if (gncEntryGetInvoice (entry) == NULL) {
      non_inv = TRUE;
      break;
    }
  }

  if (non_inv) {
    /* Damn; yes.  Well, ask the user to make sure they REALLY want to
     * close this order!
     */

    message = _("This order contains entries that have not been invoiced.\n"
		"Are you sure you want to close it out before\n"
		"you invoice all the entries?");

    if (gnc_verify_dialog_parented (ow->dialog, FALSE, message) == FALSE)
      return;
  }

  /* Ok, we can close this.  Ask for verification and set the closed date */
  message = _("Do you really want to close the order?");
  label = _("Close Date");

  timespecFromTime_t (&ts, time(NULL));
  if (!gnc_dialog_date_close_parented (ow->dialog, message, label, TRUE, &ts))
    return;

  gncOrderSetDateClosed (order, ts);

  /* save it off */
  gnc_order_window_ok_save (ow);

  /* Reset the type; change to read-only */
  ow->dialog_type = VIEW_ORDER;
  gnc_entry_ledger_set_readonly (ow->ledger);

  /* And redisplay the window */
  gnc_order_update_window (ow);
}

static void
gnc_order_window_destroy_cb (GtkWidget *widget, gpointer data)
{
  OrderWindow *ow = data;
  GncOrder *order = ow_get_order (ow);

  gnc_suspend_gui_refresh ();

  if (ow->dialog_type == NEW_ORDER && order != NULL) {
    gncOrderDestroy (order);
    ow->order_guid = *xaccGUIDNULL ();
  }

  if (ow->ledger)
    gnc_entry_ledger_destroy (ow->ledger);
  gnc_unregister_gui_component (ow->component_id);
  gnc_resume_gui_refresh ();

  g_free (ow);
}

static int
gnc_order_owner_changed_cb (GtkWidget *widget, gpointer data)
{
  OrderWindow *ow = data;
  GncOrder *order;

  if (!ow)
    return FALSE;

  if (ow->dialog_type == VIEW_ORDER)
    return FALSE;

  gnc_owner_get_owner (ow->owner_choice, &(ow->owner));

  /* Set the Order's owner now! */
  order = ow_get_order (ow);
  gncOrderSetOwner (order, &(ow->owner));

  if (ow->dialog_type == EDIT_ORDER)
    return FALSE;

  /* Only set the reference during the New Job dialog */
  switch (gncOwnerGetType (&(ow->owner))) {
  case GNC_OWNER_JOB:
  {
    char const *msg = gncJobGetReference (gncOwnerGetJob (&(ow->owner)));
    gtk_entry_set_text (GTK_ENTRY (ow->ref_entry), msg ? msg : "");
    break;
  }
  default:
    gtk_entry_set_text (GTK_ENTRY (ow->ref_entry), "");
    break;    
  }

  return FALSE;
}

static void
gnc_order_window_close_handler (gpointer user_data)
{
  OrderWindow *ow = user_data;

  gnome_dialog_close (GNOME_DIALOG (ow->dialog));
}

static void
gnc_order_window_refresh_handler (GHashTable *changes, gpointer user_data)
{
  OrderWindow *ow = user_data;
  const EventInfo *info;
  GncOrder *order = ow_get_order (ow);

  /* If there isn't a order behind us, close down */
  if (!order) {
    gnc_close_gui_component (ow->component_id);
    return;
  }

  /* Next, close if this is a destroy event */
  if (changes) {
    info = gnc_gui_get_entity_events (changes, &ow->order_guid);
    if (info && (info->event_mask & GNC_EVENT_DESTROY)) {
      gnc_close_gui_component (ow->component_id);
      return;
    }
  }
}

static void
gnc_configure_register_colors (void)
{
  GncEntryLedgerColors reg_colors;

  reg_colors.header_bg_color =
    gnc_lookup_color_option_argb("Register Colors",
                                 "Header color",
                                 0xffffff);

  reg_colors.primary_bg_color =
    gnc_lookup_color_option_argb("Register Colors",
                                 "Primary color",
                                 0xffffff);

  reg_colors.secondary_bg_color =
    gnc_lookup_color_option_argb("Register Colors",
                                 "Secondary color",
                                 0xffffff);

  reg_colors.primary_active_bg_color =
    gnc_lookup_color_option_argb("Register Colors",
                                 "Primary active color",
                                 0xffffff);

  reg_colors.secondary_active_bg_color =
    gnc_lookup_color_option_argb("Register Colors",
                                 "Secondary active color",
                                 0xffffff);

  gnc_entry_ledger_set_colors (reg_colors);
}

static void
gnc_order_update_window (OrderWindow *ow)
{
  GncOrder *order;
  GncOwner *owner;
  gboolean hide_cd = FALSE;

  order = ow_get_order (ow);
  owner = gncOrderGetOwner (order);

  if (ow->owner_choice) {
    gtk_container_remove (GTK_CONTAINER (ow->owner_box), ow->owner_choice);
    gtk_object_destroy (GTK_OBJECT (ow->owner_choice));
  }

  switch (ow->dialog_type) {
    case VIEW_ORDER:
    case EDIT_ORDER:
      ow->owner_choice =
	gnc_owner_edit_create (ow->owner_label, ow->owner_box, ow->book,
			       owner);
      break;
    case NEW_ORDER:
      ow->owner_choice =
	gnc_owner_select_create (ow->owner_label, ow->owner_box, ow->book,
				 owner);
      break;
    }

  gtk_signal_connect (GTK_OBJECT (ow->owner_choice), "changed",
		      GTK_SIGNAL_FUNC (gnc_order_owner_changed_cb),
		      ow);

  gtk_widget_show_all (ow->dialog);

  {
    const char *string;
    Timespec ts, ts_zero = {0,0};
    time_t tt;
    gint pos = 0;

    gtk_entry_set_text (GTK_ENTRY (ow->ref_entry),
			gncOrderGetReference (order));

    string = gncOrderGetNotes (order);
    gtk_editable_delete_text (GTK_EDITABLE (ow->notes_text), 0, -1);
    gtk_editable_insert_text (GTK_EDITABLE (ow->notes_text), string,
			      strlen (string), &pos);

    ts = gncOrderGetDateOpened (order);
    if (timespec_equal (&ts, &ts_zero)) {
      tt = time(NULL);
    } else {
      tt = ts.tv_sec;		/* XXX */
    }
    gnome_date_edit_set_time (GNOME_DATE_EDIT (ow->opened_date), tt);


    /* If this is a "New Order Window" we can stop here! */
    if (ow->dialog_type == NEW_ORDER)
      return;

    ts = gncOrderGetDateClosed (order);
    if (timespec_equal (&ts, &ts_zero)) {
      tt = time(NULL);
      hide_cd = TRUE;
    } else {
      tt = ts.tv_sec;		/* XXX */
    }
    gnome_date_edit_set_time (GNOME_DATE_EDIT (ow->closed_date), tt);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ow->active_check),
				  gncOrderGetActive (order));

  }

  gnc_gui_component_watch_entity_type (ow->component_id,
				       GNC_ORDER_MODULE_NAME,
				       GNC_EVENT_MODIFY | GNC_EVENT_DESTROY);

  gnc_table_refresh_gui (gnc_entry_ledger_get_table (ow->ledger), TRUE);

  if (hide_cd) {
    GtkWidget *hide;

    gtk_widget_hide_all (ow->closed_date);
    hide = glade_xml_get_widget (ow->xml, "cd_label");
    gtk_widget_hide_all (hide);

    hide = glade_xml_get_widget (ow->xml, "hide1");
    gtk_widget_hide_all (hide);
    hide = glade_xml_get_widget (ow->xml, "hide2");
    gtk_widget_hide_all (hide);
  }

  if (ow->dialog_type == VIEW_ORDER) {
    GtkWidget *hide;

    /* Setup viewer for read-only access */
    gtk_widget_set_sensitive (ow->id_entry, FALSE);
    gtk_widget_set_sensitive (ow->opened_date, FALSE);
    gtk_widget_set_sensitive (ow->closed_date, FALSE);
    gtk_widget_set_sensitive (ow->notes_text, FALSE); /* XXX: Should notes remain writable? */

    /* Hide the 'close order' button */
    hide = glade_xml_get_widget (ow->xml, "close_order_button");
    gtk_widget_hide_all (hide);
  }
}

static gboolean
find_handler (gpointer find_data, gpointer user_data)
{
  const GUID *order_guid = find_data;
  OrderWindow *ow = user_data;

  return(ow && guid_equal(&ow->order_guid, order_guid));
}

static OrderWindow *
gnc_order_new_window (GNCBook *bookp, OrderDialogType type,
		      GncOrder *order, GncOwner *owner)
{
  OrderWindow *ow;
  GladeXML *xml;
  GtkWidget *vbox, *regWidget;
  GncEntryLedger *entry_ledger = NULL;
  GnomeDialog *owd;
  const char * class_name;

  switch (type) {
   case EDIT_ORDER:
    class_name = DIALOG_EDIT_ORDER_CM_CLASS;
    break;
   case VIEW_ORDER:
   default:
    class_name = DIALOG_VIEW_ORDER_CM_CLASS;
    break;
  }

  /*
   * Find an existing window for this order.  If found, bring it to
   * the front.
   */
  if (order) {
    GUID order_guid;

    order_guid = *gncOrderGetGUID(order);
    ow = gnc_find_first_gui_component (class_name, find_handler,
				       &order_guid);
    if (ow) {
      gtk_window_present (GTK_WINDOW(ow->dialog));
      return(ow);
    }
  }
  
  /*
   * No existing order window found.  Build a new one.
   */
  gnc_configure_register_colors ();

  ow = g_new0 (OrderWindow, 1);
  ow->book = bookp;
  ow->dialog_type = type;

  /* Save this for later */
  gncOwnerCopy (owner, &(ow->owner));

  /* Find the dialog */
  ow->xml = xml = gnc_glade_xml_new ("order.glade", "Order Entry Dialog");
  ow->dialog = glade_xml_get_widget (xml, "Order Entry Dialog");
  owd = GNOME_DIALOG (ow->dialog);

  gtk_object_set_data (GTK_OBJECT (ow->dialog), "dialog_info", ow);

  /* Grab the widgets */
  ow->id_entry = glade_xml_get_widget (xml, "id_entry");
  ow->ref_entry = glade_xml_get_widget (xml, "ref_entry");
  ow->notes_text = glade_xml_get_widget (xml, "notes_text");
  ow->opened_date = glade_xml_get_widget (xml, "opened_date");
  ow->closed_date = glade_xml_get_widget (xml, "closed_date");
  ow->active_check = glade_xml_get_widget (xml, "active_check");
  ow->owner_box = glade_xml_get_widget (xml, "owner_hbox");
  ow->owner_label = glade_xml_get_widget (xml, "owner_label");

  /* default to ok */
  gnome_dialog_editable_enters (owd, GTK_EDITABLE (ow->id_entry));
  gnome_dialog_set_default (owd, 0);

  /* Build the ledger */
  switch (type) {
  case EDIT_ORDER:
    entry_ledger = gnc_entry_ledger_new (ow->book, GNCENTRY_ORDER_ENTRY);
    break;
  case VIEW_ORDER:
  default:
    entry_ledger = gnc_entry_ledger_new (ow->book, GNCENTRY_ORDER_VIEWER);
    break;
  }

  /* Save the entry ledger for later */
  ow->ledger = entry_ledger;

  /* Set the order for the entry_ledger */
  gnc_entry_ledger_set_default_order (entry_ledger, order);

  /* Set watches on entries */
  //  entries = gncOrderGetEntries (order);
  //  gnc_entry_ledger_load (entry_ledger, entries);

  /* Watch the order of operations, here... */
  gnucash_register_set_initial_rows( 10 );
  regWidget = gnucash_register_new (gnc_entry_ledger_get_table (entry_ledger));
  gnc_table_init_gui( regWidget, entry_ledger );
  ow->reg = GNUCASH_REGISTER (regWidget);
  GNUCASH_SHEET (ow->reg->sheet)->window = GTK_WIDGET(ow->dialog);
  gnc_entry_ledger_set_parent (entry_ledger, ow->dialog);

  vbox = glade_xml_get_widget (xml, "ledger_vbox");
  // gtk_box_pack_start (GTK_BOX(vbox), toolbar, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX(vbox), regWidget, TRUE, TRUE, 2);

  gtk_signal_connect (GTK_OBJECT (ow->dialog), "destroy",
		      GTK_SIGNAL_FUNC(gnc_order_window_destroy_cb), ow);

  gnome_dialog_button_connect (owd, 0,
			       GTK_SIGNAL_FUNC(gnc_order_window_ok_cb), ow);
  gnome_dialog_button_connect (owd, 1,
			       GTK_SIGNAL_FUNC(gnc_order_window_help_cb), ow);

  gnome_dialog_button_connect
    (owd, 2, GTK_SIGNAL_FUNC(gnc_order_window_invoice_cb), ow);

  gnome_dialog_button_connect
    (owd, 3, GTK_SIGNAL_FUNC(gnc_order_window_close_order_cb), ow);

  /* Setup initial values */
  ow->order_guid = *gncOrderGetGUID (order);

  gtk_entry_set_text (GTK_ENTRY (ow->id_entry), gncOrderGetID (order));

  ow->component_id =
    gnc_register_gui_component (class_name,
				gnc_order_window_refresh_handler,
				gnc_order_window_close_handler,
				ow);

  gnc_table_realize_gui (gnc_entry_ledger_get_table (entry_ledger));

  /* Now fill in a lot of the pieces and display properly */
  gnc_order_update_window (ow);

  /* Maybe set the reference */
  gnc_order_owner_changed_cb (ow->owner_choice, ow);

  return ow;
}

static OrderWindow *
gnc_order_window_new_order (GNCBook *bookp, GncOwner *owner)
{
  OrderWindow *ow;
  GladeXML *xml;
  GnomeDialog *owd;
  GncOrder *order;

  ow = g_new0 (OrderWindow, 1);
  ow->book = bookp;
  ow->dialog_type = NEW_ORDER;

  order = gncOrderCreate (bookp);
  gncOrderSetOwner (order, owner);

  /* Save this for later */
  gncOwnerCopy (owner, &(ow->owner));

  /* Find the dialog */
  xml = gnc_glade_xml_new ("order.glade", "New Order Dialog");
  ow->dialog = glade_xml_get_widget (xml, "New Order Dialog");
  owd = GNOME_DIALOG (ow->dialog);

  gtk_object_set_data (GTK_OBJECT (ow->dialog), "dialog_info", ow);

  /* Grab the widgets */
  ow->id_entry = glade_xml_get_widget (xml, "id_entry");
  ow->ref_entry = glade_xml_get_widget (xml, "ref_entry");
  ow->notes_text = glade_xml_get_widget (xml, "notes_text");
  ow->opened_date = glade_xml_get_widget (xml, "opened_date");
  ow->owner_box = glade_xml_get_widget (xml, "owner_hbox");
  ow->owner_label = glade_xml_get_widget (xml, "owner_label");

  /* default to ok */
  gnome_dialog_editable_enters (owd, GTK_EDITABLE (ow->id_entry));
  gnome_dialog_set_default (owd, 0);

  gtk_signal_connect (GTK_OBJECT (ow->dialog), "destroy",
		      GTK_SIGNAL_FUNC(gnc_order_window_destroy_cb), ow);

  gnome_dialog_button_connect (owd, 0,
			       GTK_SIGNAL_FUNC(gnc_order_window_ok_cb), ow);
  gnome_dialog_button_connect (owd, 1,
			       GTK_SIGNAL_FUNC(gnc_order_window_cancel_cb), ow);
  gnome_dialog_button_connect (owd, 2,
			       GTK_SIGNAL_FUNC(gnc_order_window_help_cb), ow);

  /* Setup initial values */
  ow->order_guid = *gncOrderGetGUID (order);
  gtk_entry_set_text (GTK_ENTRY (ow->id_entry),
		      g_strdup_printf ("%.6lld", gncOrderNextID(bookp)));
      
  ow->component_id =
    gnc_register_gui_component (DIALOG_NEW_ORDER_CM_CLASS,
				gnc_order_window_refresh_handler,
				gnc_order_window_close_handler,
				ow);

  /* Now fill in a lot of the pieces and display properly */
  gnc_order_update_window (ow);

  /* Maybe set the reference */
  gnc_order_owner_changed_cb (ow->owner_choice, ow);

  return ow;
}

OrderWindow *
gnc_ui_order_edit (GncOrder *order)
{
  OrderWindow *ow;
  OrderDialogType type;

  if (!order) return NULL;

  type = EDIT_ORDER;
  {
    Timespec ts = gncOrderGetDateClosed (order);
    if (ts.tv_sec || ts.tv_nsec)
      type = VIEW_ORDER;
  }

  ow = gnc_order_new_window (gncOrderGetBook(order), type, order, 
			     gncOrderGetOwner (order));

  return ow;
}

OrderWindow *
gnc_ui_order_new (GncOwner *ownerp, GNCBook *bookp)
{
  OrderWindow *ow;
  GncOwner owner;

  if (ownerp) {
    switch (gncOwnerGetType (ownerp)) {
    case GNC_OWNER_CUSTOMER:
    case GNC_OWNER_VENDOR:
    case GNC_OWNER_JOB:
      gncOwnerCopy (ownerp, &owner);
      break;
    default:
      g_warning ("Cannot deal with unknown Owner types");
      /* XXX: popup a warning? */
      return NULL;
    }
  } else
    gncOwnerInitJob (&owner, NULL); /* XXX: pass in the owner type? */

  /* Make sure required options exist */
  if (!bookp) return NULL;

  ow = gnc_order_window_new_order (bookp, &owner);

  return ow;
}

/* Functions for order selection widgets */

static void
edit_order_cb (gpointer *order_p, gpointer user_data)
{
  GncOrder *order;

  g_return_if_fail (order_p && user_data);

  order = *order_p;

  if (order)
    gnc_ui_order_edit (order);

  return;
}

static gpointer
new_order_cb (gpointer user_data)
{
  struct _order_select_window *sw = user_data;
  OrderWindow *ow;
  
  g_return_val_if_fail (user_data, NULL);

  ow = gnc_ui_order_new (sw->owner, sw->book);
  return ow_get_order (ow);
}

static void
free_order_cb (gpointer user_data)
{
  struct _order_select_window *sw = user_data;

  g_return_if_fail (sw);

  gncQueryDestroy (sw->q);
  g_free (sw);
}

GNCSearchWindow *
gnc_order_search (GncOrder *start, GncOwner *owner, GNCBook *book)
{
  GNCIdType type = GNC_ORDER_MODULE_NAME;
  struct _order_select_window *sw;
  QueryNew *q, *q2 = NULL;
  static GList *params = NULL;
  static GNCSearchCallbackButton buttons[] = { 
    { N_("View/Edit Order"), edit_order_cb},
    { NULL },
  };

  g_return_val_if_fail (book, NULL);

  /* Build parameter list in reverse order*/
  if (params == NULL) {
    params = gnc_search_param_prepend (params, _("Order Notes"), NULL, type,
				       ORDER_NOTES, NULL);
    params = gnc_search_param_prepend (params, _("Date Closed"), NULL, type,
				       ORDER_CLOSED, NULL);
    params = gnc_search_param_prepend (params, _("Is Closed?"), NULL, type,
				       ORDER_IS_CLOSED, NULL);
    params = gnc_search_param_prepend (params, _("Date Opened"), NULL, type,
				       ORDER_OPENED, NULL);
    params = gnc_search_param_prepend (params, _("Owner Name "), NULL, type,
				       ORDER_OWNER, OWNER_NAME, NULL);
    params = gnc_search_param_prepend (params, _("Order ID"), NULL, type,
				       ORDER_ID, NULL);
  }

  /* Build the queries */
  q = gncQueryCreate ();
  gncQuerySetBook (q, book);

  /* If owner is supplied, limit all searches to orders who's owner
   * (or parent) is the supplied owner!
   */
  if (owner && gncOwnerGetGUID (owner)) {
    QueryNew *tmp, *q3;

    q3 = gncQueryCreate ();
    gncQueryAddGUIDMatch (q3, g_slist_prepend
			  (g_slist_prepend (NULL, QUERY_PARAM_GUID),
			   ORDER_OWNER),
			  gncOwnerGetGUID (owner), QUERY_OR);
    gncQueryAddGUIDMatch (q3, g_slist_prepend
			  (g_slist_prepend (NULL, OWNER_PARENTG),
			   ORDER_OWNER),
			  gncOwnerGetGUID (owner), QUERY_OR);

    tmp = gncQueryMerge (q, q3, QUERY_AND);
    gncQueryDestroy (q);
    gncQueryDestroy (q3);
    q = tmp;
    q2 = gncQueryCopy (q);
  }

  if (start) {
    if (q2 == NULL)
      q2 = gncQueryCopy (q);

    gncQueryAddGUIDMatch (q2, g_slist_prepend (NULL, QUERY_PARAM_GUID),
			  gncOrderGetGUID (start), QUERY_AND);
  }

  /* launch select dialog and return the result */
  sw = g_new0 (struct _order_select_window, 1);

  if (owner) {
    gncOwnerCopy (owner, &(sw->owner_def));
    sw->owner = &(sw->owner_def);
  }
  sw->book = book;
  sw->q = q;

  return gnc_search_dialog_create (type, params, q, q2,
				   buttons, NULL, new_order_cb,
				   sw, free_order_cb);
}

GNCSearchWindow *
gnc_order_search_select (gpointer start, gpointer book)
{
  GncOrder *o = start;
  GncOwner owner, *ownerp;

  if (!book) return NULL;

  if (o) {
    ownerp = gncOrderGetOwner (o);
    gncOwnerCopy (ownerp, &owner);
  } else
    gncOwnerInitCustomer (&owner, NULL); /* XXX */

  return gnc_order_search (start, NULL, book);
}

GNCSearchWindow *
gnc_order_search_edit (gpointer start, gpointer book)
{
  if (start)
    gnc_ui_order_edit (start);

  return NULL;
}
