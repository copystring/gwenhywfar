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


#include "gtk4_gui_dialog_p.h"
#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/i18n.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>

#include <ctype.h>


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)


#define GTK4_DIALOG_WIDGET_REAL    0
#define GTK4_DIALOG_WIDGET_CONTENT 1
#define GTK4_DIALOG_WIDGET_DATA    2

#define GTK4_DIALOG_STRING_TITLE  0
#define GTK4_DIALOG_STRING_VALUE  1


#include "w_combobox.c"
#include "w_label.c"
#include "w_dialog.c"
#include "w_gridlayout.c"
#include "w_hlayout.c"
#include "w_hline.c"
#include "w_hspacer.c"
#include "w_vlayout.c"
#include "w_vline.c"
#include "w_vspacer.c"
#include "w_pushbutton.c"
#include "w_lineedit.c"
#include "w_textedit.c"
#include "w_textbrowser.c"
#include "w_stack.c"
#include "w_tabbook.c"
#include "w_groupbox.c"
#include "w_progressbar.c"
#include "w_listbox.c"
#include "w_checkbox.c"
#include "w_scrollarea.c"
#include "w_radiobutton.c"
#include "w_spinbox.c"
#include "w_vsplitter.c"
#include "w_hsplitter.c"



GWEN_INHERIT(GWEN_DIALOG, GTK4_GUI_DIALOG)




void Gtk4Gui_Dialog_Extend(GWEN_DIALOG *dlg)
{
  GTK4_GUI_DIALOG *xdlg;

  GWEN_NEW_OBJECT(GTK4_GUI_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, GTK4_GUI_DIALOG, dlg, xdlg, Gtk4Gui_Dialog_FreeData);

  /* set virtual functions */
  xdlg->setIntPropertyFn=GWEN_Dialog_SetSetIntPropertyFn(dlg, Gtk4Gui_Dialog_SetIntProperty);
  xdlg->getIntPropertyFn=GWEN_Dialog_SetGetIntPropertyFn(dlg, Gtk4Gui_Dialog_GetIntProperty);
  xdlg->setCharPropertyFn=GWEN_Dialog_SetSetCharPropertyFn(dlg, Gtk4Gui_Dialog_SetCharProperty);
  xdlg->getCharPropertyFn=GWEN_Dialog_SetGetCharPropertyFn(dlg, Gtk4Gui_Dialog_GetCharProperty);

}



void Gtk4Gui_Dialog_Unextend(GWEN_DIALOG *dlg)
{
  GTK4_GUI_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GTK4_GUI_DIALOG, dlg);
  assert(xdlg);

  /* reset virtual functions */
  GWEN_Dialog_SetSetIntPropertyFn(dlg, xdlg->setIntPropertyFn);
  GWEN_Dialog_SetGetIntPropertyFn(dlg, xdlg->getIntPropertyFn);
  GWEN_Dialog_SetSetCharPropertyFn(dlg, xdlg->setCharPropertyFn);
  GWEN_Dialog_SetGetCharPropertyFn(dlg, xdlg->getCharPropertyFn);

  GWEN_INHERIT_UNLINK(GWEN_DIALOG, GTK4_GUI_DIALOG, dlg);
}



void GWENHYWFAR_CB Gtk4Gui_Dialog_FreeData(GWEN_UNUSED void *bp, void *p)
{
  GTK4_GUI_DIALOG *xdlg;

  xdlg=(GTK4_GUI_DIALOG *) p;

  if (xdlg->mainWidget && GTK_IS_WINDOW(xdlg->mainWidget))
    gtk_window_destroy(GTK_WINDOW(xdlg->mainWidget));

  GWEN_FREE_OBJECT(xdlg);
}



GtkWidget *Gtk4Gui_Dialog_GetMainWidget(const GWEN_DIALOG *dlg)
{
  GTK4_GUI_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GTK4_GUI_DIALOG, dlg);
  assert(xdlg);

  return xdlg->mainWidget;
}



GWENHYWFAR_CB int Gtk4Gui_Dialog_SetIntProperty(GWEN_UNUSED GWEN_DIALOG *dlg,
                                                GWEN_WIDGET *w,
                                                GWEN_DIALOG_PROPERTY prop,
                                                int index,
                                                int value,
                                                int doSignal)
{
  return GWEN_Widget_SetIntProperty(w, prop, index, value, doSignal);
}



GWENHYWFAR_CB int Gtk4Gui_Dialog_GetIntProperty(GWEN_UNUSED GWEN_DIALOG *dlg,
                                                GWEN_WIDGET *w,
                                                GWEN_DIALOG_PROPERTY prop,
                                                int index,
                                                int defaultValue)
{
  return GWEN_Widget_GetIntProperty(w, prop, index, defaultValue);
}



GWENHYWFAR_CB int Gtk4Gui_Dialog_SetCharProperty(GWEN_UNUSED GWEN_DIALOG *dlg,
                                                 GWEN_WIDGET *w,
                                                 GWEN_DIALOG_PROPERTY prop,
                                                 int index,
                                                 const char *value,
                                                 int doSignal)
{
  return GWEN_Widget_SetCharProperty(w, prop, index, value, doSignal);
}



GWENHYWFAR_CB const char *Gtk4Gui_Dialog_GetCharProperty(GWEN_UNUSED GWEN_DIALOG *dlg,
                                                         GWEN_WIDGET *w,
                                                         GWEN_DIALOG_PROPERTY prop,
                                                         int index,
                                                         const char *defaultValue)
{
  return GWEN_Widget_GetCharProperty(w, prop, index, defaultValue);
}



static GtkWindow *Gtk4Gui_Dialog_FindActiveWindow(GtkWindow *exclude)
{
  GListModel *topLevels;
  GtkWindow *active=NULL;
  guint i;

  topLevels=gtk_window_get_toplevels();
  for (i=0; i<g_list_model_get_n_items(topLevels); i++) {
    GtkWindow *candidate=g_list_model_get_item(topLevels, i);

    if (candidate!=exclude && gtk_window_is_active(candidate)) {
      active=candidate;
      break;
    }
    g_object_unref(candidate);
  }
  return active;
}


int Gtk4Gui_Dialog_Setup(GWEN_DIALOG *dlg, GtkWidget *parentWindow)
{
  GTK4_GUI_DIALOG *xdlg;
  GWEN_WIDGET_TREE *wtree;
  GWEN_WIDGET *w;
  GtkWindow *gw;
  GtkWindow *topLevel=NULL;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GTK4_GUI_DIALOG, dlg);
  assert(xdlg);

  wtree=GWEN_Dialog_GetWidgets(dlg);
  if (wtree==NULL)
    return GWEN_ERROR_NOT_FOUND;
  w=GWEN_Widget_Tree_GetFirst(wtree);
  if (w==NULL)
    return GWEN_ERROR_NOT_FOUND;

  rv=Gtk4Gui_Dialog_SetupTree(w);
  if (rv<0)
    return rv;

  gw=GTK_WINDOW(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  xdlg->mainWidget=GTK_WIDGET(gw);
  if (parentWindow && GTK_IS_WINDOW(parentWindow))
    topLevel=GTK_WINDOW(parentWindow);
  else
    topLevel=Gtk4Gui_Dialog_FindActiveWindow(gw);

  if (topLevel) {
    gtk_window_set_transient_for(gw, topLevel);
    if (!(parentWindow && topLevel==GTK_WINDOW(parentWindow)))
      g_object_unref(topLevel);
  }

  return 0;
}

void Gtk4Gui_Dialog_Leave(GWEN_DIALOG *dlg, int result)
{
  GTK4_GUI_DIALOG *xdlg;
  GWEN_DIALOG *parent;

  /* get toplevel dialog, the one which actually is the GUI dialog */
  while ((parent=GWEN_Dialog_GetParentDialog(dlg)))
    dlg=parent;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GTK4_GUI_DIALOG, dlg);
  assert(xdlg);

  xdlg->response=result;
  if (xdlg->loop && g_main_loop_is_running(xdlg->loop))
    g_main_loop_quit(xdlg->loop);
}



static void run_unmap_handler(GWEN_UNUSED GtkWindow *window, gpointer data)
{
  GWEN_DIALOG *dlg;
  GTK4_GUI_DIALOG *xdlg;

  dlg=data;
  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GTK4_GUI_DIALOG, dlg);
  assert(xdlg);

  Gtk4Gui_Dialog_Leave(dlg, 0);
}



static gboolean run_close_request_handler(GWEN_UNUSED GtkWindow *window,
                                          gpointer data)
{
  GWEN_DIALOG *dlg=data;

  assert(dlg);
  Gtk4Gui_Dialog_Leave(dlg, 0);
  return TRUE;
}


static void run_destroy_handler(GtkWindow *window, gpointer data)
{
  GWEN_DIALOG *dlg=data;
  GTK4_GUI_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GTK4_GUI_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->mainWidget==GTK_WIDGET(window))
    xdlg->mainWidget=NULL;
  xdlg->destroyed=1;
  Gtk4Gui_Dialog_Leave(dlg, 0);
}


int GTK4_Gui_Dialog_Run(GWEN_DIALOG *dlg, int untilEnd)
{
  GTK4_GUI_DIALOG *xdlg;
  GtkWidget *g;
  GMainLoop *loop;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GTK4_GUI_DIALOG, dlg);
  assert(xdlg);
  g=Gtk4Gui_Dialog_GetMainWidget(dlg);
  if (g==NULL || !GTK_IS_WINDOW(g))
    return GWEN_ERROR_INVALID;
  if (xdlg->loop)
    return GWEN_ERROR_INVALID;

  xdlg->destroyed=0;
  xdlg->unmap_handler=g_signal_connect(g, "unmap", G_CALLBACK(run_unmap_handler), dlg);
  xdlg->delete_handler=g_signal_connect(g, "close-request", G_CALLBACK(run_close_request_handler), dlg);
  xdlg->destroy_handler=g_signal_connect(g, "destroy", G_CALLBACK(run_destroy_handler), dlg);
  xdlg->loop=g_main_loop_new(NULL, FALSE);

  if (untilEnd) {
    /* Gwen requires a synchronous dialog result; this is its isolated ABI
     * compatibility boundary until Gwen core offers a continuation. */
    g_main_loop_run(xdlg->loop);
  }
  else {
    GMainContext *ctx=g_main_loop_get_context(xdlg->loop);
    while (g_main_context_pending(ctx))
      g_main_context_iteration(ctx, FALSE);
  }

  loop=xdlg->loop;
  xdlg->loop=NULL;
  g_main_loop_unref(loop);
  if (!xdlg->destroyed) {
    g_signal_handler_disconnect(g, xdlg->unmap_handler);
    g_signal_handler_disconnect(g, xdlg->delete_handler);
    g_signal_handler_disconnect(g, xdlg->destroy_handler);
  }
  xdlg->unmap_handler=0;
  xdlg->delete_handler=0;
  xdlg->destroy_handler=0;
  return xdlg->response;
}

int Gtk4Gui_Dialog_SetupTree(GWEN_WIDGET *w)
{
  int rv;

  switch (GWEN_Widget_GetType(w)) {
  case GWEN_Widget_TypeDialog:
    rv=Gtk4Gui_WDialog_Setup(w);
    break;
  case GWEN_Widget_TypeLabel:
    rv=Gtk4Gui_WLabel_Setup(w);
    break;
  case GWEN_Widget_TypeGridLayout:
    rv=Gtk4Gui_WGridLayout_Setup(w);
    break;
  case GWEN_Widget_TypeVLayout:
    rv=Gtk4Gui_WVLayout_Setup(w);
    break;
  case GWEN_Widget_TypeHLayout:
    rv=Gtk4Gui_WHLayout_Setup(w);
    break;
  case GWEN_Widget_TypePushButton:
    rv=Gtk4Gui_WPushButton_Setup(w);
    break;
  case GWEN_Widget_TypeLineEdit:
    rv=Gtk4Gui_WLineEdit_Setup(w);
    break;
  case GWEN_Widget_TypeHLine:
    rv=Gtk4Gui_WHLine_Setup(w);
    break;
  case GWEN_Widget_TypeVLine:
    rv=Gtk4Gui_WVLine_Setup(w);
    break;
  case GWEN_Widget_TypeVSpacer:
    rv=Gtk4Gui_WVSpacer_Setup(w);
    break;
  case GWEN_Widget_TypeHSpacer:
    rv=Gtk4Gui_WHSpacer_Setup(w);
    break;
  case GWEN_Widget_TypeComboBox:
    rv=Gtk4Gui_WComboBox_Setup(w);
    break;
  case GWEN_Widget_TypeTextEdit:
    rv=Gtk4Gui_WTextEdit_Setup(w);
    break;
  case GWEN_Widget_TypeWidgetStack:
    rv=Gtk4Gui_WStack_Setup(w);
    break;
  case GWEN_Widget_TypeTabBook:
    rv=Gtk4Gui_WTabBook_Setup(w);
    break;
  case GWEN_Widget_TypeTabPage:
    /* just re-use vbox */
    GWEN_Widget_AddFlags(w, GWEN_WIDGET_FLAGS_FILLX | GWEN_WIDGET_FLAGS_FILLY);
    rv=Gtk4Gui_WVLayout_Setup(w);
    break;
  case GWEN_Widget_TypeGroupBox:
    rv=Gtk4Gui_WGroupBox_Setup(w);
    break;
  case GWEN_Widget_TypeTextBrowser:
    rv=Gtk4Gui_WTextBrowser_Setup(w);
    break;
  case GWEN_Widget_TypeProgressBar:
    rv=Gtk4Gui_WProgressBar_Setup(w);
    break;
  case GWEN_Widget_TypeSpinBox:
    rv=Gtk4Gui_WSpinBox_Setup(w);
    break;
  case GWEN_Widget_TypeListBox:
    rv=Gtk4Gui_WListBox_Setup(w);
    break;
  case GWEN_Widget_TypeCheckBox:
    rv=Gtk4Gui_WCheckBox_Setup(w);
    break;
  case GWEN_Widget_TypeScrollArea:
    rv=Gtk4Gui_WScrollArea_Setup(w);
    break;
  case GWEN_Widget_TypeRadioButton:
    rv=Gtk4Gui_WRadioButton_Setup(w);
    break;
  case GWEN_Widget_TypeVSplitter:
    rv=Gtk4Gui_WVSplitter_Setup(w);
    break;
  case GWEN_Widget_TypeHSplitter:
    rv=Gtk4Gui_WHSplitter_Setup(w);
    break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unhandled widget type %d", GWEN_Widget_GetType(w));
    rv=GWEN_ERROR_INVALID;
    break;
  }

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else {
    GWEN_WIDGET *wChild;

    /* handle children */
    wChild=GWEN_Widget_Tree_GetFirstChild(w);
    while (wChild) {
      /* recursion */
      rv=Gtk4Gui_Dialog_SetupTree(wChild);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
      wChild=GWEN_Widget_Tree_GetNext(wChild);
    }
  }

  return 0;
}




int Gtk4Gui_GetRawText(const char *text, GWEN_BUFFER *tbuf)
{
  const char *p=0;
  const char *p2=0;

  if (text==NULL)
    return 0;

  /* find begin of HTML area */
  p=text;
  while ((p=strchr(p, '<'))) {
    const char *t;

    t=p;
    t++;
    if (toupper(*t)=='H') {
      t++;
      if (toupper(*t)=='T') {
        t++;
        if (toupper(*t)=='M') {
          t++;
          if (toupper(*t)=='L') {
            t++;
            if (toupper(*t)=='>') {
              break;
            }
          }
        }
      }
    }
    p++;
  } /* while */

  /* find end of HTML area */
  if (p) {
    p2=p;
    p2+=6; /* skip "<html>" */
    while ((p2=strchr(p2, '<'))) {
      const char *t;

      t=p2;
      t++;
      if (toupper(*t)=='/') {
        t++;
        if (toupper(*t)=='H') {
          t++;
          if (toupper(*t)=='T') {
            t++;
            if (toupper(*t)=='M') {
              t++;
              if (toupper(*t)=='L') {
                t++;
                if (toupper(*t)=='>') {
                  break;
                }
              }
            }
          }
        }
      }
      p2++;
    } /* while */
  }

  if (p && p2) {
    int startPos;

    p2+=7; /* skip "</html>" */

    startPos=(p-text);

    /* append stuff before startPos */
    if (startPos)
      GWEN_Buffer_AppendBytes(tbuf, text, startPos);
    if (*p2)
      GWEN_Buffer_AppendString(tbuf, p2);
    return 0;
  }
  else {
    GWEN_Buffer_AppendString(tbuf, text);
    return 0;
  }
}



void Gtk4Gui_ApplyFlags(GtkWidget *g, uint32_t flags)
{
  gtk_widget_set_hexpand(g, (flags & GWEN_WIDGET_FLAGS_FILLX)?TRUE:FALSE);
  gtk_widget_set_vexpand(g, (flags & GWEN_WIDGET_FLAGS_FILLY)?TRUE:FALSE);

}
