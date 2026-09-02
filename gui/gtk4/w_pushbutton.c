/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/





static GWENHYWFAR_CB
int Gtk4Gui_WPushButton_SetIntProperty(GWEN_WIDGET *w,
                                       GWEN_DIALOG_PROPERTY prop,
                                       GWEN_UNUSED int index,
                                       int value,
                                       GWEN_UNUSED int doSignal)
{
  GtkButton *g;

  g=GTK_BUTTON(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Enabled:
    gtk_widget_set_sensitive(GTK_WIDGET(g), (value==0)?FALSE:TRUE);
    return 0;

  case GWEN_DialogProperty_Focus:
    gtk_widget_grab_focus(GTK_WIDGET(g));
    return 0;

  case GWEN_DialogProperty_Width:
  case GWEN_DialogProperty_Height:
    /* just ignore these for now */
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
int Gtk4Gui_WPushButton_GetIntProperty(GWEN_WIDGET *w,
                                       GWEN_DIALOG_PROPERTY prop,
                                       GWEN_UNUSED int index,
                                       int defaultValue)
{
  GtkButton *g;

  g=GTK_BUTTON(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Enabled:
    return (gtk_widget_get_sensitive(GTK_WIDGET(g))==TRUE)?1:0;

  case GWEN_DialogProperty_Focus:
    return (gtk_widget_has_focus(GTK_WIDGET(g))==TRUE)?1:0;
    return 0;

  case GWEN_DialogProperty_Width:
  case GWEN_DialogProperty_Height:
    /* just ignore these for now */
    return 0;

  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}



static GWENHYWFAR_CB
int Gtk4Gui_WPushButton_SetCharProperty(GWEN_WIDGET *w,
                                        GWEN_DIALOG_PROPERTY prop,
                                        GWEN_UNUSED int index,
                                        const char *value,
                                        GWEN_UNUSED int doSignal)
{
  GtkButton *g;

  g=GTK_BUTTON(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Title:
    GtkLabel *label=g_object_get_data(G_OBJECT(g), "gwen-button-label");
    if (label)
      gtk_label_set_text_with_mnemonic(label, value ? value : "");
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
const char *Gtk4Gui_WPushButton_GetCharProperty(GWEN_WIDGET *w,
                                                GWEN_DIALOG_PROPERTY prop,
                                                GWEN_UNUSED int index,
                                                const char *defaultValue)
{
  GtkButton *g;

  g=GTK_BUTTON(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Title:
    GtkLabel *label=g_object_get_data(G_OBJECT(g), "gwen-button-label");
    return label ? gtk_label_get_label(label) : defaultValue;
  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}



static void Gtk4Gui_WPushButton_Clicked_handler(GWEN_UNUSED GtkButton *button, gpointer data)
{
  GWEN_WIDGET *w;
  int rv;

  w=data;
  assert(w);
  rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
                            GWEN_DialogEvent_TypeActivated,
                            GWEN_Widget_GetName(w));
  if (rv==GWEN_DialogEvent_ResultAccept)
    Gtk4Gui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 1);
  else if (rv==GWEN_DialogEvent_ResultReject)
    Gtk4Gui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 0);
}



int Gtk4Gui_WPushButton_Setup(GWEN_WIDGET *w)
{
  GtkWidget *g;
  const char *s;
  GWEN_WIDGET *wParent;

  wParent=GWEN_Widget_Tree_GetParent(w);
  s=GWEN_Widget_GetText(w, 0);

  /* A GTK4 button has one child. Keep an explicit label so title changes and
   * icon buttons retain the existing Gwen property contract. */
  {
    GtkWidget *content=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *label=gtk_label_new_with_mnemonic(s ? s : "");

    g=gtk_button_new();
    gtk_box_append(GTK_BOX(content), label);
    gtk_button_set_child(GTK_BUTTON(g), content);
    g_object_set_data(G_OBJECT(g), "gwen-button-label", label);
  }

  s=GWEN_Widget_GetIconFileName(w);
  if (s && *s) {
    GWEN_STRINGLIST *sl;

    sl=GWEN_Dialog_GetMediaPaths(GWEN_Widget_GetDialog(w));
    if (sl) {
      int rv;
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=GWEN_Directory_FindFileInPaths(sl, s, tbuf);
      if (rv<0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Image file [%s] not found (%d)", s, rv);
        /* ignore result here, instead create GtkImage with "broken mage" later */
      }
      else {
        GtkWidget *gImage;
        GtkWidget *content;

        gImage=gtk_image_new_from_file(GWEN_Buffer_GetStart(tbuf));
        content=gtk_button_get_child(GTK_BUTTON(g));
        gtk_box_prepend(GTK_BOX(content), gImage);
      }
      GWEN_Buffer_free(tbuf);
    }
  }

  GWEN_Widget_SetImplData(w, GTK4_DIALOG_WIDGET_REAL, (void *) g);
  GWEN_Widget_SetImplData(w, GTK4_DIALOG_WIDGET_CONTENT, (void *) g);

  GWEN_Widget_SetSetIntPropertyFn(w, Gtk4Gui_WPushButton_SetIntProperty);
  GWEN_Widget_SetGetIntPropertyFn(w, Gtk4Gui_WPushButton_GetIntProperty);
  GWEN_Widget_SetSetCharPropertyFn(w, Gtk4Gui_WPushButton_SetCharProperty);
  GWEN_Widget_SetGetCharPropertyFn(w, Gtk4Gui_WPushButton_GetCharProperty);

  g_signal_connect(g,
                   "clicked",
                   G_CALLBACK(Gtk4Gui_WPushButton_Clicked_handler),
                   w);

  if (wParent)
    GWEN_Widget_AddChildGuiWidget(wParent, w);

  return 0;
}
