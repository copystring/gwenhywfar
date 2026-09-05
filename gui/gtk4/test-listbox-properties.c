/***************************************************************************
    begin       : Sat Sep 5 2026
    copyright   : (C) 2026 by the GnuCash contributors

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#include "config.h"

#include "gtk4_gui.h"

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/dialog_be.h>
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/widget_be.h>
#include <gwenhywfar/xml.h>

#include <string.h>


typedef struct {
  int activationCount;
  int expectedSelection;
  const char *expectedText;
} ListBoxTestState;


static ListBoxTestState testState;


static int GWENHYWFAR_CB signal_handler(GWEN_DIALOG *dialog,
                                        GWEN_DIALOG_EVENTTYPE type,
                                        const char *sender)
{
  if (type==GWEN_DialogEvent_TypeActivated &&
      sender && strcasecmp(sender, "list")==0) {
    int selection;

    testState.activationCount++;
    selection=GWEN_Dialog_GetIntProperty(dialog, "list",
                                         GWEN_DialogProperty_Value,
                                         0, -1);
    g_assert_cmpint(selection, ==, testState.expectedSelection);
    if (selection>=0)
      g_assert_cmpstr(GWEN_Dialog_GetCharProperty(dialog, "list",
                                                  GWEN_DialogProperty_Value,
                                                  selection, NULL),
                      ==, testState.expectedText);
  }
  return GWEN_DialogEvent_ResultHandled;
}


static GWEN_DIALOG *create_dialog(void)
{
  static const char xml[]=
    "<dialog type=\"dialog\" name=\"testdialog\">"
    "<widget type=\"vlayout\" name=\"layout\">"
    "<widget type=\"listBox\" name=\"list\"/>"
    "</widget>"
    "</dialog>";
  GWEN_XMLNODE *root;
  GWEN_XMLNODE *dialogNode;
  GWEN_DIALOG *dialog;

  root=GWEN_XMLNode_fromString(xml, sizeof(xml)-1, GWEN_XML_FLAGS_DEFAULT);
  g_assert_nonnull(root);
  dialogNode=GWEN_XMLNode_FindFirstTag(root, "dialog", NULL, NULL);
  g_assert_nonnull(dialogNode);

  dialog=GWEN_Dialog_new("listbox-properties");
  g_assert_nonnull(dialog);
  g_assert_cmpint(GWEN_Dialog_ReadXml(dialog, dialogNode), ==, 0);
  GWEN_XMLNode_free(root);
  GWEN_Dialog_SetSignalHandler(dialog, signal_handler);
  return dialog;
}


static GtkColumnView *get_column_view(GWEN_DIALOG *dialog)
{
  GWEN_WIDGET *widget=GWEN_Dialog_FindWidgetByName(dialog, "list");
  GtkWidget *view;

  g_assert_nonnull(widget);
  view=GTK_WIDGET(GWEN_Widget_GetImplData(widget, 1));
  g_assert_true(GTK_IS_COLUMN_VIEW(view));
  return GTK_COLUMN_VIEW(view);
}


static void assert_row(GWEN_DIALOG *dialog, int position, const char *text)
{
  g_assert_cmpstr(GWEN_Dialog_GetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_Value,
                                              position, NULL),
                  ==, text);
}


static void test_properties(void)
{
  GWEN_DIALOG *dialog=create_dialog();

  memset(&testState, 0, sizeof(testState));
  g_assert_cmpint(GWEN_Gui_OpenDialog(dialog, 0), ==, 0);
  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_Title,
                                              0, "Column", 0),
                  ==, 0);

  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_AddValue,
                                              0, "First", 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 0);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, -1);
  g_assert_cmpstr(GWEN_Dialog_GetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_Value,
                                              0, NULL),
                  ==, "First");

  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Second", 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 0);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, -1);

  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, 0, 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 0);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, 0);

  testState.expectedSelection=1;
  testState.expectedText="Second";
  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, 1, 1),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 1);

  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_ClearValues,
                                             0, 0, 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 1);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, -1);

  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Repopulated", 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 1);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, -1);
  g_assert_cmpstr(GWEN_Dialog_GetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_Value,
                                              0, NULL),
                  ==, "Repopulated");

  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_SelectionMode,
                                             0, GWEN_Dialog_SelectionMode_None, 0),
                  ==, 0);
  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_SelectionMode,
                                             0, GWEN_Dialog_SelectionMode_Single, 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 1);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, -1);

  g_assert_cmpint(GWEN_Gui_CloseDialog(dialog), ==, 0);
  GWEN_Dialog_free(dialog);
}


static void test_sorting(void)
{
  GWEN_DIALOG *dialog=create_dialog();
  GtkColumnView *view;
  GListModel *columns;
  GtkColumnViewColumn *column;

  memset(&testState, 0, sizeof(testState));
  g_assert_cmpint(GWEN_Gui_OpenDialog(dialog, 0), ==, 0);
  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_Title,
                                              0, "Name\tCode", 0),
                  ==, 0);
  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Zulu\t2", 0),
                  ==, 0);
  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Alpha\t3", 0),
                  ==, 0);
  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Mike\t1", 0),
                  ==, 0);

  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_SortDirection,
                                             0, GWEN_DialogSortDirection_Up, 0),
                  ==, 0);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_SortDirection,
                                             0, -1),
                  ==, GWEN_DialogSortDirection_Up);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_SortDirection,
                                             1, -1),
                  ==, GWEN_DialogSortDirection_None);
  assert_row(dialog, 0, "Alpha\t3");
  assert_row(dialog, 1, "Mike\t1");
  assert_row(dialog, 2, "Zulu\t2");
  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, 0, 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 0);

  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Beta\t0", 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 0);
  assert_row(dialog, 0, "Alpha\t3");
  assert_row(dialog, 1, "Beta\t0");
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, 0);

  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_SortDirection,
                                             0, GWEN_DialogSortDirection_Down, 0),
                  ==, 0);
  assert_row(dialog, 0, "Zulu\t2");
  assert_row(dialog, 3, "Alpha\t3");
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, 3);
  g_assert_cmpint(testState.activationCount, ==, 0);

  view=get_column_view(dialog);
  columns=gtk_column_view_get_columns(view);
  column=g_list_model_get_item(columns, 1);
  g_assert_nonnull(column);
  gtk_column_view_sort_by_column(view, column, GTK_SORT_ASCENDING);
  g_object_unref(column);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_SortDirection,
                                             0, -1),
                  ==, GWEN_DialogSortDirection_None);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_SortDirection,
                                             1, -1),
                  ==, GWEN_DialogSortDirection_Up);
  assert_row(dialog, 0, "Beta\t0");
  assert_row(dialog, 3, "Alpha\t3");
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, 3);
  g_assert_cmpint(testState.activationCount, ==, 0);

  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_SortDirection,
                                             1, GWEN_DialogSortDirection_None, 0),
                  ==, 0);
  assert_row(dialog, 0, "Zulu\t2");
  assert_row(dialog, 3, "Beta\t0");
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, 1);
  g_assert_cmpint(testState.activationCount, ==, 0);

  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_SortDirection,
                                             0, GWEN_DialogSortDirection_Up, 0),
                  ==, 0);
  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_Title,
                                              0, "Name\tCode", 0),
                  ==, 0);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "list",
                                             GWEN_DialogProperty_SortDirection,
                                             0, -1),
                  ==, GWEN_DialogSortDirection_Up);
  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Zulu\t2", 0),
                  ==, 0);
  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "list",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Alpha\t3", 0),
                  ==, 0);
  assert_row(dialog, 0, "Alpha\t3");
  assert_row(dialog, 1, "Zulu\t2");
  g_assert_cmpint(testState.activationCount, ==, 0);

  g_assert_cmpint(GWEN_Gui_CloseDialog(dialog), ==, 0);
  GWEN_Dialog_free(dialog);
}


int main(int argc, char **argv)
{
  GWEN_GUI *gui;
  int result;

  g_test_init(&argc, &argv, NULL);
  if (!gtk_init_check())
    return 77;
  g_log_set_always_fatal(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);
  if (GWEN_Init())
    return 1;

  gui=Gtk4_Gui_new();
  GWEN_Gui_SetGui(gui);
  g_test_add_func("/gtk4/listbox/properties", test_properties);
  g_test_add_func("/gtk4/listbox/sorting", test_sorting);
  result=g_test_run();
  GWEN_Gui_SetGui(NULL);
  GWEN_Gui_free(gui);
  GWEN_Fini();
  return result;
}
