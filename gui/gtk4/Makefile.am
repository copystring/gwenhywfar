AM_CPPFLAGS=-I$(gwenhywfar_symlinkdir) $(GTK4_CFLAGS) \
  @visibility_cflags@
DEFS+=-DBUILDING_GTK4_GUI -DPACKAGE=\"$(PACKAGE)\"

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gwengui-gtk4.pc

lib_LTLIBRARIES=libgwengui-gtk4.la
noinst_LTLIBRARIES=libgtk4-file-dialog.la


libgwengui_gtk4_la_LIBADD=libgtk4-file-dialog.la $(GTK4_LIBS) $(top_builddir)/src/$(gwenhywfar_internal_libname)
libgwengui_gtk4_la_LDFLAGS = -no-undefined -version-info \
   $(GWENHYWFAR_SO_CURRENT):$(GWENHYWFAR_SO_REVISION):$(GWENHYWFAR_SO_AGE)

noinst_HEADERS=\
  gtk4_gui_p.h \
  gtk4_gui_file_dialog_p.h \
  gtk4_gui_dialog_l.h \
  gtk4_gui_dialog_p.h

libgwengui_gtk4_la_SOURCES=\
  gtk4_gui_dialog.c \
  gtk4_gui.c

libgtk4_file_dialog_la_SOURCES=\
  gtk4_gui_file_dialog.c


EXTRA_DIST=\
  w_combobox.c \
  w_label.c \
  w_dialog.c \
  w_gridlayout.c \
  w_groupbox.c \
  w_hlayout.c \
  w_hline.c \
  w_hspacer.c \
  w_lineedit.c \
  w_vlayout.c \
  w_vline.c \
  w_vspacer.c \
  w_pushbutton.c \
  w_textbrowser.c \
  w_textedit.c \
  w_stack.c \
  w_tabbook.c \
  w_progressbar.c \
  w_listbox.c \
  w_checkbox.c \
  w_scrollarea.c \
  w_radiobutton.c \
  w_spinbox.c \
  w_vsplitter.c \
  w_hsplitter.c



iheaderdir=${gwenhywfar_headerdir_am}/gwen-gui-gtk4
iheader_HEADERS= \
  gtk4_gui.h


noinst_PROGRAMS = libtest
libtest_SOURCES = libtest.c
libtest_LDADD = libgwengui-gtk4.la $(GTK4_LIBS) $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(builddir)/../testdialogs/libgwengui-test.la

check_PROGRAMS = test-listbox-model-ownership test-file-dialog-path test-combobox-properties
TESTS = test-listbox-model-ownership test-file-dialog-path test-combobox-properties
test_listbox_model_ownership_SOURCES = test-listbox-model-ownership.c
test_listbox_model_ownership_LDADD = libgwengui-gtk4.la $(GTK4_LIBS) \
  $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(builddir)/../testdialogs/libgwengui-test.la

test_file_dialog_path_SOURCES = test-file-dialog-path.c
test_file_dialog_path_LDADD = libgtk4-file-dialog.la libgwengui-gtk4.la $(GTK4_LIBS) \
  $(top_builddir)/src/$(gwenhywfar_internal_libname)

test_combobox_properties_SOURCES = test-combobox-properties.c
test_combobox_properties_LDADD = libgwengui-gtk4.la $(GTK4_LIBS) \
  $(top_builddir)/src/$(gwenhywfar_internal_libname)
