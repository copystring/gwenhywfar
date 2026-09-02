/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "gtk4_gui_p.h"
#include "gtk4_gui_file_dialog_p.h"
#include "gtk4_gui_dialog_l.h"

#include <assert.h>
#include <string.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/i18n.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>

#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)

GWEN_INHERIT(GWEN_GUI, GTK4_GUI)

typedef enum
{
  GTK4_GUI_FILE_OPEN,
  GTK4_GUI_FILE_SAVE,
  GTK4_GUI_FILE_FOLDER
} GTK4_GUI_FILE_KIND;

typedef struct
{
  GMainLoop *loop;
  GtkFileDialog *dialog;
  GtkWindow *parent;
  GCancellable *cancellable;
  GFile *file;
  GError *error;
  GTK4_GUI_FILE_KIND kind;
} GTK4_GUI_FILE_REQUEST;

static GtkWindow *
Gtk4Gui_GetActiveWindow(void)
{
  GListModel *toplevels;
  guint i;

  toplevels=gtk_window_get_toplevels();
  for (i=0; i<g_list_model_get_n_items(toplevels); i++) {
    GtkWindow *window;

    window=GTK_WINDOW(g_list_model_get_item(toplevels, i));
    if (gtk_window_is_active(window)) {
      return window;
    }
    g_object_unref(window);
  }
  return NULL;
}


static void
Gtk4Gui_FileRequest_Free(GTK4_GUI_FILE_REQUEST *request)
{
  if (request==NULL)
    return;
  g_clear_pointer(&request->loop, g_main_loop_unref);
  g_clear_object(&request->dialog);
  g_clear_object(&request->parent);
  g_clear_object(&request->cancellable);
  g_clear_object(&request->file);
  g_clear_error(&request->error);
  g_free(request);
}

static void
Gtk4Gui_FileDialog_Finished(GObject *source,
                            GAsyncResult *result,
                            gpointer user_data)
{
  GTK4_GUI_FILE_REQUEST *request=user_data;
  GtkFileDialog *dialog=GTK_FILE_DIALOG(source);

  switch (request->kind) {
  case GTK4_GUI_FILE_OPEN:
    request->file=gtk_file_dialog_open_finish(dialog, result, &request->error);
    break;
  case GTK4_GUI_FILE_SAVE:
    request->file=gtk_file_dialog_save_finish(dialog, result, &request->error);
    break;
  case GTK4_GUI_FILE_FOLDER:
    request->file=gtk_file_dialog_select_folder_finish(dialog, result, &request->error);
    break;
  }

  g_main_loop_quit(request->loop);
}

static int
Gtk4Gui_RunFileDialog(const char *caption,
                      GWEN_GUI_FILENAME_TYPE fnt,
                      const char *folder,
                      const char *fileName,
                      GWEN_BUFFER *pathBuffer)
{
  GTK4_GUI_FILE_REQUEST *request;
  char *filename;

  request=g_new0(GTK4_GUI_FILE_REQUEST, 1);
  request->loop=g_main_loop_new(NULL, FALSE);
  request->dialog=gtk_file_dialog_new();
  request->parent=Gtk4Gui_GetActiveWindow();
  request->cancellable=g_cancellable_new();

  gtk_file_dialog_set_modal(request->dialog, TRUE);
  gtk_file_dialog_set_title(request->dialog, caption);
  if (folder && *folder) {
    GFile *initialFolder=g_file_new_for_path(folder);
    gtk_file_dialog_set_initial_folder(request->dialog, initialFolder);
    g_object_unref(initialFolder);
  }
  if (fileName && *fileName)
    gtk_file_dialog_set_initial_name(request->dialog, fileName);

  switch (fnt) {
  case GWEN_Gui_FileNameType_OpenFileName:
    request->kind=GTK4_GUI_FILE_OPEN;
    gtk_file_dialog_set_accept_label(request->dialog, I18N("Open"));
    gtk_file_dialog_open(request->dialog, request->parent, request->cancellable,
                         Gtk4Gui_FileDialog_Finished, request);
    break;
  case GWEN_Gui_FileNameType_SaveFileName:
    request->kind=GTK4_GUI_FILE_SAVE;
    gtk_file_dialog_set_accept_label(request->dialog, I18N("Save"));
    gtk_file_dialog_save(request->dialog, request->parent, request->cancellable,
                         Gtk4Gui_FileDialog_Finished, request);
    break;
  case GWEN_Gui_FileNameType_OpenDirectory:
    request->kind=GTK4_GUI_FILE_FOLDER;
    gtk_file_dialog_set_accept_label(request->dialog, I18N("Select"));
    gtk_file_dialog_select_folder(request->dialog, request->parent, request->cancellable,
                                  Gtk4Gui_FileDialog_Finished, request);
    break;
  default:
    Gtk4Gui_FileRequest_Free(request);
    return GWEN_ERROR_USER_ABORTED;
  }

  /* GWEN_GUI_GET_FILENAME_FN is synchronous. This is the narrow ABI adapter
   * around GTK4's asynchronous file dialog; no generic event pumping occurs. */
  g_main_loop_run(request->loop);

  if (!request->error && request->file) {
    filename=g_file_get_path(request->file);
    if (filename) {
      GWEN_Buffer_Reset(pathBuffer);
      GWEN_Buffer_AppendString(pathBuffer, filename);
      g_free(filename);
      Gtk4Gui_FileRequest_Free(request);
      return 0;
    }
  }

  if (request->error && !g_error_matches(request->error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    DBG_ERROR(GWEN_LOGDOMAIN, "GTK4 file dialog failed: %s", request->error->message);
  Gtk4Gui_FileRequest_Free(request);
  return GWEN_ERROR_USER_ABORTED;
}

GWEN_GUI *
Gtk4_Gui_new(void)
{
  GWEN_GUI *gui;
  GTK4_GUI *xgui;

  gui=GWEN_Gui_new();
  GWEN_NEW_OBJECT(GTK4_GUI, xgui);
  GWEN_INHERIT_SETDATA(GWEN_GUI, GTK4_GUI, gui, xgui, Gtk4_Gui_FreeData);

  GWEN_Gui_AddFlags(gui, GWEN_GUI_FLAGS_DIALOGSUPPORTED);
  GWEN_Gui_UseDialogs(gui);
  xgui->execDialogFn=GWEN_Gui_SetExecDialogFn(gui, GTK4_Gui_ExecDialog);
  xgui->openDialogFn=GWEN_Gui_SetOpenDialogFn(gui, GTK4_Gui_OpenDialog);
  xgui->closeDialogFn=GWEN_Gui_SetCloseDialogFn(gui, GTK4_Gui_CloseDialog);
  xgui->runDialogFn=GWEN_Gui_SetRunDialogFn(gui, GTK4_Gui_RunDialog);
  xgui->getFileNameDialogFn=GWEN_Gui_SetGetFileNameFn(gui, GTK4_Gui_GetFileName);

  return gui;
}

GWENHYWFAR_CB void
Gtk4_Gui_FreeData(GWEN_UNUSED void *bp, void *p)
{
  GTK4_GUI *xgui=p;

  GWEN_FREE_OBJECT(xgui);
}

GWENHYWFAR_CB int
GTK4_Gui_ExecDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, uint32_t guiid)
{
  int rv;

  assert(dlg);
  rv=GTK4_Gui_OpenDialog(gui, dlg, guiid);
  if (rv<0)
    return rv;

  rv=GTK4_Gui_RunDialog(gui, dlg, 1);
  GTK4_Gui_CloseDialog(gui, dlg);
  return rv;
}

GWENHYWFAR_CB int
GTK4_Gui_OpenDialog(GWEN_UNUSED GWEN_GUI *gui,
                    GWEN_DIALOG *dlg,
                    GWEN_UNUSED uint32_t guiid)
{
  int rv;
  GtkWidget *widget;

  assert(dlg);
  Gtk4Gui_Dialog_Extend(dlg);
  rv=Gtk4Gui_Dialog_Setup(dlg, NULL);
  if (rv<0) {
    Gtk4Gui_Dialog_Unextend(dlg);
    return rv;
  }

  widget=Gtk4Gui_Dialog_GetMainWidget(dlg);
  if (!widget) {
    Gtk4Gui_Dialog_Unextend(dlg);
    return GWEN_ERROR_INVALID;
  }

  rv=GWEN_Dialog_EmitSignalToAll(dlg, GWEN_DialogEvent_TypeInit, "");
  if (rv<0) {
    Gtk4Gui_Dialog_Unextend(dlg);
    return rv;
  }

  gtk_window_present(GTK_WINDOW(widget));
  return 0;
}

GWENHYWFAR_CB int
GTK4_Gui_CloseDialog(GWEN_UNUSED GWEN_GUI *gui, GWEN_DIALOG *dlg)
{
  GtkWidget *widget;
  int rv;

  assert(dlg);
  widget=Gtk4Gui_Dialog_GetMainWidget(dlg);
  if (!widget) {
    Gtk4Gui_Dialog_Unextend(dlg);
    return GWEN_ERROR_INVALID;
  }

  gtk_widget_set_visible(widget, FALSE);
  rv=GWEN_Dialog_EmitSignalToAll(dlg, GWEN_DialogEvent_TypeFini, "");
  Gtk4Gui_Dialog_Unextend(dlg);
  return rv;
}

GWENHYWFAR_CB int
GTK4_Gui_RunDialog(GWEN_UNUSED GWEN_GUI *gui, GWEN_DIALOG *dlg, int untilEnd)
{
  assert(dlg);
  return GTK4_Gui_Dialog_Run(dlg, untilEnd);
}

GWENHYWFAR_CB int
GTK4_Gui_GetFileName(GWEN_UNUSED GWEN_GUI *gui,
                     const char *caption,
                     GWEN_GUI_FILENAME_TYPE fnt,
                     GWEN_UNUSED uint32_t flags,
                     GWEN_UNUSED const char *patterns,
                     GWEN_BUFFER *pathBuffer,
                     GWEN_UNUSED uint32_t guiid)
{
  char *folder=NULL;
  char *fileName=NULL;
  const char *defaultCaption;

  switch (fnt) {
  case GWEN_Gui_FileNameType_OpenFileName:
    defaultCaption=I18N("Open File");
    break;
  case GWEN_Gui_FileNameType_SaveFileName:
    defaultCaption=I18N("Save File");
    break;
  case GWEN_Gui_FileNameType_OpenDirectory:
    defaultCaption=I18N("Select Folder");
    break;
  default:
    return GWEN_ERROR_USER_ABORTED;
  }

  if (GWEN_Buffer_GetUsedBytes(pathBuffer))
    Gtk4Gui_FileDialog_PreparePath(fnt, GWEN_Buffer_GetStart(pathBuffer),
                                   &folder, &fileName);

  if (!(caption && *caption))
    caption=defaultCaption;
  {
    int rv=Gtk4Gui_RunFileDialog(caption, fnt, folder, fileName, pathBuffer);
    g_free(folder);
    g_free(fileName);
    return rv;
  }
}
