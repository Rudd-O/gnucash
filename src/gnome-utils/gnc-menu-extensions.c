/********************************************************************\
 * gnc-menu-extensions.c -- functions to build dynamic menus        *
 * Copyright (C) 1999 Rob Browning         	                    *
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

#include "config.h"

#include <ctype.h>
#include <gnome.h>

#include "guile-util.h"
#include "gnc-engine.h"
#include "gnc-menu-extensions.h"
#include "gnc-ui.h"

typedef struct _Getters Getters;
struct _Getters
{
  SCM type;
  SCM name;
  SCM documentation;
  SCM path;
  SCM script;
};

/* This static indicates the debugging module that this .o belongs to.  */
static QofLogModule log_module = GNC_MOD_GUI;

static GSList *extension_list = NULL;
static Getters getters = {0, 0, 0, 0, 0};

GSList *
gnc_extensions_get_menu_list (void)
{
  return extension_list;
}

static void
initialize_getters()
{
  static gboolean getters_initialized = FALSE;

  if (getters_initialized)
    return;

  getters.type = scm_c_eval_string("gnc:extension-type");
  getters.name = scm_c_eval_string("gnc:extension-name");
  getters.documentation = scm_c_eval_string("gnc:extension-documentation");
  getters.path = scm_c_eval_string("gnc:extension-path");
  getters.script = scm_c_eval_string("gnc:extension-script");

  getters_initialized = TRUE;
}


static gboolean
gnc_extension_type (SCM extension, GtkUIManagerItemType *type)
{
  char *string;

  initialize_getters();

  string = gnc_guile_call1_symbol_to_string(getters.type, extension);
  if (string == NULL)
  {
    PERR("bad type");
    return FALSE;
  }

  if (safe_strcmp(string, "menu-item") == 0) {
    *type = GTK_UI_MANAGER_MENUITEM;
  } else if (safe_strcmp(string, "menu") == 0) {
    *type = GTK_UI_MANAGER_MENU;
  } else if (safe_strcmp(string, "separator") == 0) {
    *type = GTK_UI_MANAGER_SEPARATOR;
  } else {
    PERR("bad type");
    return FALSE;
  }

  free(string);

  return TRUE;
}

/* returns malloc'd name */
static char *
gnc_extension_name (SCM extension)
{
  initialize_getters();

  return gnc_guile_call1_to_string(getters.name, extension);
}


/* returns malloc'd docs */
static char *
gnc_extension_documentation (SCM extension)
{
  initialize_getters();

  return gnc_guile_call1_to_string(getters.documentation, extension);
}

/* returns g_malloc'd path */
static void
gnc_extension_path (SCM extension, char **fullpath)
{
  SCM path;
  gchar **strings;
  gint i;

  initialize_getters();

  path = gnc_guile_call1_to_list(getters.path, extension);
  if ((path == SCM_UNDEFINED) || SCM_NULLP(path)) {
    *fullpath = g_strdup("");
    return;
  }

  strings = g_new0(gchar *, scm_ilength(path) + 2);
  strings[0] = "/menubar";

  i = 1;
  while (!SCM_NULLP(path))
  {
    SCM item;

    item = SCM_CAR(path);
    path = SCM_CDR(path);

    if (SCM_STRINGP(item))
    {
      strings[i] = g_strdup(SCM_STRING_CHARS(item));
    }
    else
    {
      g_free(strings);

      PERR("not a string");

      *fullpath = g_strdup("");
      return;
    }

    i++;
  }

  *fullpath = g_strjoinv("/", strings);

  g_free(strings);
}

/******************** New Menu Item ********************/

static gchar*
gnc_ext_gen_action_name (const gchar *name)
{
  //gchar *extName;
  const gchar *extChar;
  GString *actionName;

  actionName = g_string_sized_new( strlen( name ) + 7 );

  // 'Mum & ble' => 'Mumble'
  for ( extChar = name; *extChar != '\0'; extChar++ ) {
    if ( ! isalpha( *extChar ) )
      continue;
    g_string_append_c( actionName, *extChar );
  }

  // 'Mumble + 'Action' => 'MumbleAction'
  g_string_append_printf( actionName, "Action" );

  return g_string_free(actionName, FALSE);
}

/******************** Callback ********************/

void
gnc_extension_invoke_cb (SCM extension, SCM window)
{
  SCM script;

  initialize_getters();

  script = gnc_guile_call1_to_procedure(getters.script, extension);
  if (script == SCM_UNDEFINED)
  {
    PERR("not a procedure.");
    return;
  }

  scm_call_1(script, window);
}

/******************** New Menu Item ********************/

static gboolean
gnc_create_extension_info (SCM extension)
{
  ExtensionInfo *ext_info;
  gchar *typeStr;

  ext_info = g_new0(ExtensionInfo, 1);
  ext_info->extension = extension;
  gnc_extension_path(extension, &ext_info->path);
  if (!gnc_extension_type( extension, &ext_info->type )) {
    /* Can't parse the type passed to us.  Bail now. */
    g_free(ext_info);
    return FALSE;
  }

  /* Get all the pieces */
  ext_info->ae.label = gnc_extension_name(extension);
  ext_info->ae.name = gnc_ext_gen_action_name(ext_info->ae.label);
  ext_info->ae.tooltip = gnc_extension_documentation(extension);
  ext_info->ae.stock_id = "";
  ext_info->ae.accelerator = NULL;
  ext_info->ae.callback = NULL;

  switch (ext_info->type) {
    case GTK_UI_MANAGER_MENU: typeStr = "menu"; break;
    case GTK_UI_MANAGER_MENUITEM: typeStr = "menuitem"; break;
    default: typeStr = "unk"; break;
  }
  ext_info->typeStr = typeStr;

  DEBUG( "extension: %s/%s [%s] tip [%s] type %s\n",
	 ext_info->path, ext_info->ae.label, ext_info->ae.name,
	 ext_info->ae.tooltip, ext_info->typeStr );

  scm_gc_protect_object(extension);

  /* need to append so we can run them in order */
  extension_list = g_slist_append(extension_list, ext_info);

  return TRUE;
}

static void
cleanup_extension_info(gpointer extension_info, gpointer not_used)
{
  ExtensionInfo *ext_info = extension_info;

  if (ext_info->extension)
    scm_gc_unprotect_object(ext_info->extension);

  g_free((gchar *)ext_info->ae.name);
  g_free((gchar *)ext_info->ae.label);
  g_free((gchar *)ext_info->ae.tooltip);
  g_free(ext_info->path);
  g_free(ext_info);
}


void
gnc_add_scm_extension (SCM extension)
{
  if (!gnc_create_extension_info(extension))
  {
    PERR("bad extension");
    return;
  }
}

/******************** Install Menus ********************/

typedef struct {
  GtkUIManager *uiMerge;
  GtkActionGroup *group;
  guint merge_id;
} setup_data;

static void
gnc_extensions_menu_setup_one (ExtensionInfo *ext_info,
			       setup_data *data)
{
  DEBUG("Adding %s/%s [%s] as [%s]\n", ext_info->path, ext_info->ae.label,
        ext_info->ae.name, ext_info->typeStr);

  gtk_action_group_add_actions(data->group, &ext_info->ae, 1,
			       ext_info->extension);
  gtk_ui_manager_add_ui(data->uiMerge, data->merge_id, ext_info->path,
			ext_info->ae.label, ext_info->ae.name,
			ext_info->type, FALSE);
  gtk_ui_manager_ensure_update(data->uiMerge);
}

void
gnc_extensions_menu_setup (GtkUIManager *uiMerge )
{
  setup_data data;

  ENTER(" ");

  data.uiMerge = uiMerge;
  data.group = gtk_action_group_new("MainWindowActionsN");
  gtk_action_group_set_translation_domain(data.group, GETTEXT_PACKAGE);
  gtk_ui_manager_insert_action_group(uiMerge, data.group, 0);
  data.merge_id = gtk_ui_manager_new_merge_id(uiMerge);

  g_slist_foreach(extension_list, (GFunc)gnc_extensions_menu_setup_one, &data);
  LEAVE(" ");
}

/******************** Shutdown ********************/

void
gnc_extensions_shutdown (void)
{
  g_slist_foreach(extension_list, cleanup_extension_info, NULL);

  g_slist_free(extension_list);

  extension_list = NULL;
}
