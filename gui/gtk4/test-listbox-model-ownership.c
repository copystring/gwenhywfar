/***************************************************************************
    begin       : Mon Aug 31 2026
    copyright   : (C) 2026 by the GnuCash contributors

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#include "config.h"

#include "gtk4_gui.h"
#include "../testdialogs/dlg_test.h"

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/gui.h>


static gboolean close_test_window(GWEN_UNUSED gpointer userData)
{
  GListModel *windows=gtk_window_get_toplevels();
  guint i;

  for (i=0; i<g_list_model_get_n_items(windows); i++) {
    GtkWindow *window=g_list_model_get_item(windows, i);

    gtk_window_close(window);
    g_object_unref(window);
  }
  return G_SOURCE_REMOVE;
}


int main(void)
{
  GWEN_DIALOG *dialog;
  GWEN_GUI *gui;
  int result;

  if (!gtk_init_check())
    return 77;

  g_log_set_always_fatal(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);
  if (GWEN_Init())
    return 1;

  gui=Gtk4_Gui_new();
  GWEN_Gui_SetGui(gui);
  dialog=Dlg_Test1_new();
  if (dialog==NULL) {
    GWEN_Gui_SetGui(NULL);
    GWEN_Gui_free(gui);
    GWEN_Fini();
    return 1;
  }

  g_idle_add(close_test_window, NULL);
  result=GWEN_Gui_ExecDialog(dialog, 0);
  GWEN_Dialog_free(dialog);
  GWEN_Gui_SetGui(NULL);
  GWEN_Gui_free(gui);
  GWEN_Fini();
  return result<0 ? 1 : 0;
}
