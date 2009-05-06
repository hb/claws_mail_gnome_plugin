/* Gnome plugin for Claws-Mail
 * Copyright (C) 2009 Holger Berndt
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "pluginconfig.h"

#include "common/hooks.h"
#include "common/plugin.h"
#include "common/version.h"
#include "addr_compl.h"
#include "common/utils.h"

#include "gettext.h"

#include <libebook/e-book.h>


static gboolean my_address_completion_build_list_hook(gpointer, gpointer);

static guint hook_address_completion;

static EBook *eds_book = NULL;
static gboolean eds_waiting = FALSE;

static void eds_contacts_added_cb(EBookView *view, const GList *contacts, gpointer data)
{
  const GList *walk;
  GList **address_list = (GList**) data;

  for(walk = contacts; walk; walk = walk->next) {
    const char *name;
    GList *email_list, *email_entry;
    EContact *contact = walk->data;

    if(!E_IS_CONTACT(contact))
      continue;

    name = e_contact_get_const(contact, E_CONTACT_FULL_NAME);
    email_list = e_contact_get(contact, E_CONTACT_EMAIL);
    for(email_entry = email_list; email_entry; email_entry = email_entry->next) {
      address_entry *ae;
      const char *email_address = email_entry->data;

      ae = g_new0(address_entry, 1);
      ae->name = g_strdup(name);
      ae->address = g_strdup(email_address);
      ae->grp_emails = NULL;
      *address_list = g_list_prepend(*address_list, ae);

      addr_compl_add_address1(name, ae);
      if(email_address && *email_address != '\0')
        addr_compl_add_address1(email_address, ae);
    }
  }
}

static void eds_sequence_complete_cb(EBookView *view, const GList *contacts, gpointer data)
{
  eds_waiting = FALSE;
}

static void add_gnome_addressbook(GList **address_list)
{
  EBookQuery *query;
  EBookView *view;
  GError *error = NULL;

  /* create book accessor if necessary */
  if(!eds_book) {
    eds_book = e_book_new_system_addressbook(&error);
    if(!eds_book) {
      debug_print("Error: Could not get eds addressbook: %s\n", error->message);
      g_error_free(error);
      return;
    }
  }

  /* open book if necessary */
  if(!e_book_is_opened(eds_book) && !e_book_open(eds_book, TRUE, &error)) {
    debug_print("Error: Could not open eds addressbook: %s\n", error->message);
    g_error_free(error);
    return;
  }

  /* query book */
  query = e_book_query_field_exists(E_CONTACT_EMAIL);
  if(!e_book_get_book_view(eds_book, query, NULL, 0, &view, &error)) {
    debug_print("Error: Could not get eds addressbook view: %s\n", error->message);
    g_error_free(error);
  }
  e_book_query_unref(query);

  g_signal_connect(G_OBJECT(view), "contacts-added", G_CALLBACK(eds_contacts_added_cb), address_list);
  g_signal_connect(G_OBJECT(view), "sequence-complete", G_CALLBACK(eds_sequence_complete_cb), NULL);

  eds_waiting = TRUE;
  e_book_view_start(view);

  while(eds_waiting)
    gtk_main_iteration();

  e_book_view_stop(view);
  g_object_unref(view);
}

static gboolean my_address_completion_build_list_hook(gpointer source, gpointer data)
{
  add_gnome_addressbook(source);
  return FALSE;
}


gint plugin_init(gchar **error)
{
#ifdef G_OS_UNIX
	bindtextdomain(TEXTDOMAIN, LOCALEDIR);
#else
	bindtextdomain(TEXTDOMAIN, get_locale_dir());
#endif
  bind_textdomain_codeset(TEXTDOMAIN, "UTF-8");

  /* Version check */
  if(!check_plugin_version(MAKE_NUMERIC_VERSION(3,7,1,44),
			   VERSION_NUMERIC, _("Gnome"), error))
    return -1;

  hook_address_completion = hooks_register_hook(ADDDRESS_COMPLETION_BUILD_ADDRESS_LIST_HOOKLIST,
                                                my_address_completion_build_list_hook, NULL);
  if(hook_address_completion == (guint) -1) {
    *error = g_strdup(_("Failed to register address completion hook in the "
			"Gnome plugin"));
    return -1;
  }

  debug_print("Gnome plugin loaded\n");

  return 0;
}

gboolean plugin_done(void)
{
  hooks_unregister_hook(ADDDRESS_COMPLETION_BUILD_ADDRESS_LIST_HOOKLIST, hook_address_completion);

  if(eds_book) {
    g_object_unref(eds_book);
    eds_book = NULL;
  }

  debug_print("Gnome plugin unloaded\n");
  return TRUE;
}

const gchar *plugin_name(void)
{
  return _("Gnome");
}

const gchar *plugin_desc(void)
{
  return _("This plugin provides Gnome integration features.\n"
	   "Currently, the only implemented functionality is to "
	   "include the Gnome addressbook in Claws Mail's address "
	   "completion.\n"
           "\nFeedback to <berndth@gmx.de> is welcome.");
}

const gchar *plugin_type(void)
{
  return "GTK2";
}

const gchar *plugin_licence(void)
{
  return "GPL3+";
}

const gchar *plugin_version(void)
{
  return PLUGINVERSION;
}

struct PluginFeature *plugin_provides(void)
{
  static struct PluginFeature features[] =
    { {PLUGIN_UTILITY, N_("Gnome integration")},
      {PLUGIN_NOTHING, NULL}};
  return features;
}
