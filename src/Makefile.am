plugindir = $(CLAWS_MAIL_PLUGINDIR)

plugin_LTLIBRARIES = gnome_plugin.la

gnome_plugin_la_SOURCES = \
	gnome_plugin.c \
	gettext.h

gnome_plugin_la_LDFLAGS = \
	-avoid-version -module \
	$(GTK_LIBS) \
	$(libebook_LIBS)

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/gtk

AM_CPPFLAGS = \
	-Wall \
	$(CLAWS_MAIL_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GTK_CFLAGS) \
	$(libebook_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\"
