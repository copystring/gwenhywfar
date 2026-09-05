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
  int expectedCount;
  int expectedSelection;
  const char *expectedText;
} ComboBoxTestState;


static ComboBoxTestState testState;


static GtkWidget *find_widget_of_type(GtkWidget *root, GType type)
{
  GtkWidget *child;

  if (G_TYPE_CHECK_INSTANCE_TYPE(root, type))
    return root;
  for (child=gtk_widget_get_first_child(root);
       child;
       child=gtk_widget_get_next_sibling(child)) {
    GtkWidget *result=find_widget_of_type(child, type);

    if (result)
      return result;
  }
  return NULL;
}


static GtkMenuButton *get_combo_menu_button(GWEN_DIALOG *dialog, const char *name)
{
  GWEN_WIDGET *widget=GWEN_Dialog_FindWidgetByName(dialog, name);
  GtkWidget *root;
  GtkWidget *menuButton;

  g_assert_nonnull(widget);
  root=GTK_WIDGET(GWEN_Widget_GetImplData(widget, 0));
  g_assert_nonnull(root);
  menuButton=find_widget_of_type(root, GTK_TYPE_MENU_BUTTON);
  g_assert_nonnull(menuButton);
  return GTK_MENU_BUTTON(menuButton);
}


static void press_escape(GtkListView *listView)
{
  GListModel *controllers=gtk_widget_observe_controllers(GTK_WIDGET(listView));
  gboolean handled=FALSE;
  guint i;

  for (i=0; i<g_list_model_get_n_items(controllers); i++) {
    GtkEventController *controller=g_list_model_get_item(controllers, i);

    if (GTK_IS_EVENT_CONTROLLER_KEY(controller))
      g_signal_emit_by_name(controller,
                            "key-pressed",
                            GDK_KEY_Escape,
                            0,
                            (GdkModifierType)0,
                            &handled);
    g_object_unref(controller);
    if (handled)
      break;
  }
  g_object_unref(controllers);
  g_assert_true(handled);
}


static GtkListView *get_combo_list_view(GtkMenuButton *menuButton)
{
  GtkPopover *popover=gtk_menu_button_get_popover(menuButton);
  GtkWidget *listView;

  g_assert_nonnull(popover);
  listView=find_widget_of_type(GTK_WIDGET(popover), GTK_TYPE_LIST_VIEW);
  g_assert_nonnull(listView);
  return GTK_LIST_VIEW(listView);
}


static GtkEntry *get_combo_entry(GWEN_DIALOG *dialog)
{
  GWEN_WIDGET *widget=GWEN_Dialog_FindWidgetByName(dialog, "combo");
  GtkWidget *root;
  GtkWidget *entry;

  g_assert_nonnull(widget);
  root=GTK_WIDGET(GWEN_Widget_GetImplData(widget, 0));
  g_assert_nonnull(root);
  entry=find_widget_of_type(root, GTK_TYPE_ENTRY);
  g_assert_nonnull(entry);
  return GTK_ENTRY(entry);
}


static void activate_list_item(GtkMenuButton *menuButton,
                               GtkListView *listView,
                               guint position)
{
  gtk_menu_button_popup(menuButton);
  while (g_main_context_pending(NULL))
    g_main_context_iteration(NULL, FALSE);
  g_assert_true(gtk_menu_button_get_active(menuButton));
  g_assert_true(gtk_widget_activate_action(GTK_WIDGET(listView),
                                           "list.activate-item",
                                           "u",
                                           position));
  g_assert_false(gtk_menu_button_get_active(menuButton));
}


static int GWENHYWFAR_CB signal_handler(GWEN_DIALOG *dialog,
                                        GWEN_DIALOG_EVENTTYPE type,
                                        const char *sender)
{
  if (type==GWEN_DialogEvent_TypeActivated &&
      sender && strcasecmp(sender, "combo")==0) {
    int selection;

    testState.activationCount++;
    g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                               GWEN_DialogProperty_ValueCount,
                                               0, -1),
                    ==, testState.expectedCount);
    selection=GWEN_Dialog_GetIntProperty(dialog, "combo",
                                         GWEN_DialogProperty_Value,
                                         0, -1);
    g_assert_cmpint(selection, ==, testState.expectedSelection);
    if (selection>=0)
      g_assert_cmpstr(GWEN_Dialog_GetCharProperty(dialog, "combo",
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
    "<widget type=\"comboBox\" name=\"combo\"/>"
    "<widget type=\"comboBox\" name=\"readonlyCombo\" flags=\"readOnly\"/>"
    "</widget>"
    "</dialog>";
  GWEN_XMLNODE *root;
  GWEN_XMLNODE *dialogNode;
  GWEN_DIALOG *dialog;

  root=GWEN_XMLNode_fromString(xml, sizeof(xml)-1, GWEN_XML_FLAGS_DEFAULT);
  g_assert_nonnull(root);
  dialogNode=GWEN_XMLNode_FindFirstTag(root, "dialog", NULL, NULL);
  g_assert_nonnull(dialogNode);

  dialog=GWEN_Dialog_new("combobox-properties");
  g_assert_nonnull(dialog);
  g_assert_cmpint(GWEN_Dialog_ReadXml(dialog, dialogNode), ==, 0);
  GWEN_XMLNode_free(root);
  GWEN_Dialog_SetSignalHandler(dialog, signal_handler);
  return dialog;
}


static void test_properties(void)
{
  GWEN_DIALOG *dialog=create_dialog();
  GtkMenuButton *menuButton;
  GtkMenuButton *readonlyMenuButton;
  GtkListView *listView;
  GtkEntry *entry;

  memset(&testState, 0, sizeof(testState));
  g_assert_cmpint(GWEN_Gui_OpenDialog(dialog, 0), ==, 0);
  menuButton=get_combo_menu_button(dialog, "combo");
  readonlyMenuButton=get_combo_menu_button(dialog, "readonlyCombo");
  listView=get_combo_list_view(menuButton);
  entry=get_combo_entry(dialog);
  g_assert_cmpint(gtk_accessible_get_accessible_role(GTK_ACCESSIBLE(menuButton)),
                  ==, GTK_ACCESSIBLE_ROLE_COMBO_BOX);
  g_assert_cmpint(gtk_accessible_get_accessible_role(GTK_ACCESSIBLE(readonlyMenuButton)),
                  ==, GTK_ACCESSIBLE_ROLE_COMBO_BOX);
  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "readonlyCombo",
                                             GWEN_DialogProperty_Focus,
                                             0, 1, 0),
                  ==, 0);
  while (g_main_context_pending(NULL))
    g_main_context_iteration(NULL, FALSE);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "readonlyCombo",
                                             GWEN_DialogProperty_Focus,
                                             0, 0),
                  ==, 1);

  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "combo",
                                              GWEN_DialogProperty_AddValue,
                                              0, "First", 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 0);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_ValueCount,
                                             0, -1),
                  ==, 1);
  g_assert_cmpstr(GWEN_Dialog_GetCharProperty(dialog, "combo",
                                              GWEN_DialogProperty_Value,
                                              0, NULL),
                  ==, "First");
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, -1);

  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "combo",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Second", 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 0);

  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Focus,
                                             0, 1, 0),
                  ==, 0);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Focus,
                                             0, 0),
                  ==, 1);
  gtk_menu_button_popup(menuButton);
  while (g_main_context_pending(NULL))
    g_main_context_iteration(NULL, FALSE);
  g_assert_true(gtk_menu_button_get_active(menuButton));
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Focus,
                                             0, 0),
                  ==, 1);
  g_assert_true(gtk_widget_activate_action(GTK_WIDGET(listView),
                                           "list.select-item",
                                           "(ubb)",
                                           1u,
                                           FALSE,
                                           FALSE));
  g_assert_cmpint(testState.activationCount, ==, 0);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, -1);
  press_escape(listView);
  g_assert_false(gtk_menu_button_get_active(menuButton));
  g_assert_cmpint(testState.activationCount, ==, 0);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, -1);

  testState.expectedCount=2;
  testState.expectedSelection=0;
  testState.expectedText="First";
  activate_list_item(menuButton, listView, 0);
  g_assert_cmpint(testState.activationCount, ==, 1);
  g_assert_cmpstr(gtk_editable_get_text(GTK_EDITABLE(entry)), ==, "First");

  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Value,
                                             0, 1, 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 1);
  g_assert_cmpstr(gtk_editable_get_text(GTK_EDITABLE(entry)), ==, "Second");
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, 1);

  testState.expectedCount=2;
  testState.expectedSelection=0;
  testState.expectedText="First";
  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Value,
                                             0, 0, 1),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 2);
  g_assert_cmpstr(gtk_editable_get_text(GTK_EDITABLE(entry)), ==, "First");

  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "combo",
                                              GWEN_DialogProperty_ClearValues,
                                              0, NULL, 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 2);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_ValueCount,
                                             0, -1),
                  ==, 0);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, -1);

  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "combo",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Repopulated", 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 2);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, -1);

  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "combo",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Again", 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 2);

  testState.expectedCount=2;
  testState.expectedSelection=1;
  testState.expectedText="Again";
  activate_list_item(menuButton, listView, 1);
  g_assert_cmpint(testState.activationCount, ==, 3);
  g_assert_cmpstr(gtk_editable_get_text(GTK_EDITABLE(entry)), ==, "Again");

  testState.expectedCount=0;
  testState.expectedSelection=-1;
  testState.expectedText=NULL;
  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_ClearValues,
                                             0, 0, 1),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 4);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_ValueCount,
                                             0, -1),
                  ==, 0);

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
  g_test_add_func("/gtk4/combobox/properties", test_properties);
  result=g_test_run();
  GWEN_Gui_SetGui(NULL);
  GWEN_Gui_free(gui);
  GWEN_Fini();
  return result;
}
