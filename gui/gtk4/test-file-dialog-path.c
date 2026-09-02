/***************************************************************************
    begin       : Tue Sep 2 2026
    copyright   : (C) 2026 by the GnuCash contributors

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#include "config.h"

#include "gtk4_gui_file_dialog_p.h"

#include <glib.h>


typedef struct {
  GWEN_GUI_FILENAME_TYPE type;
  const char *path;
  const char *folder;
  const char *name;
} PathTest;


static void test_path(gconstpointer userData)
{
  const PathTest *test=userData;
  char *folder;
  char *name;

  Gtk4Gui_FileDialog_PreparePath(test->type, test->path, &folder, &name);
  g_assert_cmpstr(folder, ==, test->folder);
  g_assert_cmpstr(name, ==, test->name);
  g_free(folder);
  g_free(name);
}


int main(int argc, char **argv)
{
  static const PathTest openPosix={
    GWEN_Gui_FileNameType_OpenFileName, "/home/user/books.gnucash",
    "/home/user", "books.gnucash"
  };
  static const PathTest saveWindows={
    GWEN_Gui_FileNameType_SaveFileName, "C:\\Users\\user\\books.gnucash",
    "C:\\Users\\user", "books.gnucash"
  };
  static const PathTest rootPosix={
    GWEN_Gui_FileNameType_OpenFileName, "/books.gnucash",
    "/", "books.gnucash"
  };
  static const PathTest rootWindows={
    GWEN_Gui_FileNameType_SaveFileName, "C:\\books.gnucash",
    "C:\\", "books.gnucash"
  };
  static const PathTest directoryPosix={
    GWEN_Gui_FileNameType_OpenDirectory, "/home/user",
    "/home/user", NULL
  };
  static const PathTest directoryWindows={
    GWEN_Gui_FileNameType_OpenDirectory, "C:\\Users\\user",
    "C:\\Users\\user", NULL
  };
  static const PathTest directoryKeepsCompletePath={
    GWEN_Gui_FileNameType_OpenDirectory, "/home/user/books.gnucash",
    "/home/user/books.gnucash", NULL
  };
  static const PathTest relativeName={
    GWEN_Gui_FileNameType_OpenFileName, "books.gnucash",
    NULL, "books.gnucash"
  };

  g_test_init(&argc, &argv, NULL);
  g_test_add_data_func("/gtk4/file-dialog/open-posix", &openPosix, test_path);
  g_test_add_data_func("/gtk4/file-dialog/save-windows", &saveWindows, test_path);
  g_test_add_data_func("/gtk4/file-dialog/root-posix", &rootPosix, test_path);
  g_test_add_data_func("/gtk4/file-dialog/root-windows", &rootWindows, test_path);
  g_test_add_data_func("/gtk4/file-dialog/directory-posix", &directoryPosix, test_path);
  g_test_add_data_func("/gtk4/file-dialog/directory-windows", &directoryWindows, test_path);
  g_test_add_data_func("/gtk4/file-dialog/directory-complete-path", &directoryKeepsCompletePath, test_path);
  g_test_add_data_func("/gtk4/file-dialog/relative-name", &relativeName, test_path);
  return g_test_run();
}
