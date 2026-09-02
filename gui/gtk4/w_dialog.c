/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/





static GWENHYWFAR_CB
int Gtk4Gui_WDialog_SetIntProperty(GWEN_WIDGET *w,
                                   GWEN_DIALOG_PROPERTY prop,
                                   GWEN_UNUSED int index,
                                   int value,
                                   GWEN_UNUSED int doSignal)
{
  GtkWindow *g;

  g=GTK_WINDOW(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Enabled:
    gtk_widget_set_sensitive(GTK_WIDGET(g), (value==0)?FALSE:TRUE);
    return 0;

  case GWEN_DialogProperty_Focus:
    gtk_widget_grab_focus(GTK_WIDGET(g));
    return 0;

  case GWEN_DialogProperty_Width: {
    int width, height;

    gtk_window_get_default_size(g, &width, &height);
    gtk_window_set_default_size(g, value, height);
    return 0;
  }

  case GWEN_DialogProperty_Height: {
    int width, height;

    gtk_window_get_default_size(g, &width, &height);
    gtk_window_set_default_size(g, width, value);
    return 0;
  }

  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return GWEN_ERROR_INVALID;
}




static GWENHYWFAR_CB
int Gtk4Gui_WDialog_GetIntProperty(GWEN_WIDGET *w,
                                   GWEN_DIALOG_PROPERTY prop,
                                   GWEN_UNUSED int index,
                                   int defaultValue)
{
  GtkWindow *g;

  g=GTK_WINDOW(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Enabled:
    return (gtk_widget_get_sensitive(GTK_WIDGET(g))==TRUE)?1:0;

  case GWEN_DialogProperty_Focus:
    return (gtk_widget_has_focus(GTK_WIDGET(g))==TRUE)?1:0;
    return 0;

  case GWEN_DialogProperty_Width: {
    int width, height;

    gtk_window_get_default_size(g, &width, &height);
    return width;
  }

  case GWEN_DialogProperty_Height: {
    int width, height;

    gtk_window_get_default_size(g, &width, &height);
    return height;
  }

  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}



static GWENHYWFAR_CB
int Gtk4Gui_WDialog_SetCharProperty(GWEN_WIDGET *w,
                                    GWEN_DIALOG_PROPERTY prop,
                                    GWEN_UNUSED int index,
                                    const char *value,
                                    GWEN_UNUSED int doSignal)
{
  GtkWindow *g;

  g=GTK_WINDOW(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Title:
    gtk_window_set_title(GTK_WINDOW(g), value);
    return 0;
  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char *Gtk4Gui_WDialog_GetCharProperty(GWEN_WIDGET *w,
                                            GWEN_DIALOG_PROPERTY prop,
                                            GWEN_UNUSED int index,
                                            const char *defaultValue)
{
  GtkWindow *g;

  g=GTK_WINDOW(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Title:
    return gtk_window_get_title(GTK_WINDOW(g));
  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}



static GWENHYWFAR_CB
int Gtk4Gui_WDialog_AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild)
{
  GtkWidget *g;
  GtkWidget *gChild;

  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  gChild=GTK_WIDGET(GWEN_Widget_GetImplData(wChild, GTK4_DIALOG_WIDGET_REAL));
  assert(gChild);

  gtk_window_set_child(GTK_WINDOW(g), gChild);

  return 0;
}



int Gtk4Gui_WDialog_Setup(GWEN_WIDGET *w)
{
  GtkWidget *g;
  const char *s;

  s=GWEN_Widget_GetText(w, 0);

  g=gtk_window_new();
  gtk_window_set_modal(GTK_WINDOW(g), TRUE);
  if (s && *s)
    gtk_window_set_title(GTK_WINDOW(g), s);

  GWEN_Widget_SetImplData(w, GTK4_DIALOG_WIDGET_REAL, (void *) g);
  GWEN_Widget_SetImplData(w, GTK4_DIALOG_WIDGET_CONTENT, (void *) g);

  GWEN_Widget_SetSetIntPropertyFn(w, Gtk4Gui_WDialog_SetIntProperty);
  GWEN_Widget_SetGetIntPropertyFn(w, Gtk4Gui_WDialog_GetIntProperty);
  GWEN_Widget_SetSetCharPropertyFn(w, Gtk4Gui_WDialog_SetCharProperty);
  GWEN_Widget_SetGetCharPropertyFn(w, Gtk4Gui_WDialog_GetCharProperty);
  GWEN_Widget_SetAddChildGuiWidgetFn(w, Gtk4Gui_WDialog_AddChildGuiWidget);

  return 0;
}
