/***************************************************************************
    begin       : Sat Sep 5 2026
    copyright   : (C) 2026 by the GnuCash contributors

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#include "config.h"

#include "gtk4_gui.h"

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/xml.h>

#include <string.h>


typedef struct {
  int activationCount;
  int expectedCount;
  int expectedSelection;
  const char *expectedText;
} ComboBoxTestState;


static ComboBoxTestState testState;


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

  memset(&testState, 0, sizeof(testState));
  g_assert_cmpint(GWEN_Gui_OpenDialog(dialog, 0), ==, 0);

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
                  ==, 0);

  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "combo",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Second", 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 0);

  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Value,
                                             0, 1, 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 0);
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
  g_assert_cmpint(testState.activationCount, ==, 1);

  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "combo",
                                              GWEN_DialogProperty_ClearValues,
                                              0, NULL, 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 1);
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
  g_assert_cmpint(testState.activationCount, ==, 1);
  g_assert_cmpint(GWEN_Dialog_GetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Value,
                                             0, -1),
                  ==, 0);

  g_assert_cmpint(GWEN_Dialog_SetCharProperty(dialog, "combo",
                                              GWEN_DialogProperty_AddValue,
                                              0, "Again", 0),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 1);

  testState.expectedCount=2;
  testState.expectedSelection=1;
  testState.expectedText="Again";
  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_Value,
                                             0, 1, 1),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 2);

  testState.expectedCount=0;
  testState.expectedSelection=-1;
  testState.expectedText=NULL;
  g_assert_cmpint(GWEN_Dialog_SetIntProperty(dialog, "combo",
                                             GWEN_DialogProperty_ClearValues,
                                             0, 0, 1),
                  ==, 0);
  g_assert_cmpint(testState.activationCount, ==, 3);
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
