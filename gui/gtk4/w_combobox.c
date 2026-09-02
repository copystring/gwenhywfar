/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


typedef struct W_COMBOBOX W_COMBOBOX;
struct W_COMBOBOX {
  GWEN_STRINGLIST *entries;
  GtkStringList *model;
  GtkDropDown *dropDown;
  GtkEntry *entry;
  int syncing;
};


GWEN_INHERIT(GWEN_WIDGET, W_COMBOBOX)


static void Gtk4Gui_WComboBox_EmitActivated(GWEN_WIDGET *w)
{
  int rv;

  rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
                            GWEN_DialogEvent_TypeActivated,
                            GWEN_Widget_GetName(w));
  if (rv==GWEN_DialogEvent_ResultAccept)
    Gtk4Gui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 1);
  else if (rv==GWEN_DialogEvent_ResultReject)
    Gtk4Gui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 0);
}


static void Gtk4Gui_WComboBox_SetEntryFromSelection(W_COMBOBOX *xw)
{
  guint selected;
  const char *text;

  if (xw->entry==NULL)
    return;

  selected=gtk_drop_down_get_selected(xw->dropDown);
  if (selected==GTK_INVALID_LIST_POSITION)
    return;

  text=gtk_string_list_get_string(xw->model, selected);
  if (text==NULL)
    return;

  xw->syncing=1;
  gtk_editable_set_text(GTK_EDITABLE(xw->entry), text);
  xw->syncing=0;
}


static void selected_handler(GObject *object,
                             GParamSpec *pspec,
                             gpointer data)
{
  GWEN_WIDGET *w=data;
  W_COMBOBOX *xw;

  (void)object;
  (void)pspec;
  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_COMBOBOX, w);
  assert(xw);

  Gtk4Gui_WComboBox_SetEntryFromSelection(xw);
  if (!xw->syncing)
    Gtk4Gui_WComboBox_EmitActivated(w);
}


static void entry_changed_handler(GtkEditable *entry, gpointer data)
{
  GWEN_WIDGET *w=data;
  W_COMBOBOX *xw;

  (void)entry;
  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_COMBOBOX, w);
  assert(xw);

  if (!xw->syncing)
    Gtk4Gui_WComboBox_EmitActivated(w);
}


static void Gtk4Gui_WComboBox_Clear(W_COMBOBOX *xw)
{
  guint count;

  count=g_list_model_get_n_items(G_LIST_MODEL(xw->model));
  while (count>0) {
    count--;
    gtk_string_list_remove(xw->model, count);
  }
  GWEN_StringList_Clear(xw->entries);
}


static GWENHYWFAR_CB
int Gtk4Gui_WComboBox_SetIntProperty(GWEN_WIDGET *w,
                                     GWEN_DIALOG_PROPERTY prop,
                                     GWEN_UNUSED int index,
                                     int value,
                                     GWEN_UNUSED int doSignal)
{
  GtkWidget *g;
  W_COMBOBOX *xw;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_COMBOBOX, w);
  assert(xw);

  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Enabled:
    gtk_widget_set_sensitive(g, (value!=0));
    return 0;

  case GWEN_DialogProperty_Focus:
    gtk_widget_grab_focus(xw->entry ? GTK_WIDGET(xw->entry) : GTK_WIDGET(xw->dropDown));
    return 0;

  case GWEN_DialogProperty_Value:
    if (value<0 || (guint)value>=g_list_model_get_n_items(G_LIST_MODEL(xw->model)))
      gtk_drop_down_set_selected(xw->dropDown, GTK_INVALID_LIST_POSITION);
    else
      gtk_drop_down_set_selected(xw->dropDown, (guint)value);
    return 0;

  case GWEN_DialogProperty_ClearValues:
    Gtk4Gui_WComboBox_Clear(xw);
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
int Gtk4Gui_WComboBox_GetIntProperty(GWEN_WIDGET *w,
                                     GWEN_DIALOG_PROPERTY prop,
                                     GWEN_UNUSED int index,
                                     int defaultValue)
{
  GtkWidget *g;
  W_COMBOBOX *xw;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_COMBOBOX, w);
  assert(xw);

  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Enabled:
    return gtk_widget_get_sensitive(g) ? 1 : 0;

  case GWEN_DialogProperty_Focus:
    return gtk_widget_has_focus(xw->entry ? GTK_WIDGET(xw->entry) : GTK_WIDGET(xw->dropDown)) ? 1 : 0;

  case GWEN_DialogProperty_Value: {
    guint selected=gtk_drop_down_get_selected(xw->dropDown);
    return selected==GTK_INVALID_LIST_POSITION ? defaultValue : (int)selected;
  }

  case GWEN_DialogProperty_ValueCount:
    return (int)g_list_model_get_n_items(G_LIST_MODEL(xw->model));

  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}


static GWENHYWFAR_CB
int Gtk4Gui_WComboBox_SetCharProperty(GWEN_WIDGET *w,
                                      GWEN_DIALOG_PROPERTY prop,
                                      GWEN_UNUSED int index,
                                      const char *value,
                                      GWEN_UNUSED int doSignal)
{
  W_COMBOBOX *xw;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_COMBOBOX, w);
  assert(xw);

  switch (prop) {
  case GWEN_DialogProperty_Value:
    /* The legacy backend deliberately did not define text assignment. */
    return 0;

  case GWEN_DialogProperty_AddValue:
    gtk_string_list_append(xw->model, value ? value : "");
    GWEN_StringList_AppendString(xw->entries, value ? value : "", 0, 0);
    return 0;

  case GWEN_DialogProperty_ClearValues:
    Gtk4Gui_WComboBox_Clear(xw);
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
const char *Gtk4Gui_WComboBox_GetCharProperty(GWEN_WIDGET *w,
                                              GWEN_DIALOG_PROPERTY prop,
                                              int index,
                                              const char *defaultValue)
{
  W_COMBOBOX *xw;
  const char *text;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_COMBOBOX, w);
  assert(xw);

  switch (prop) {
  case GWEN_DialogProperty_Value:
    text=GWEN_StringList_StringAt(xw->entries, index);
    return (text && *text) ? text : defaultValue;

  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}


static void GWENHYWFAR_CB Gtk4Gui_WComboBox_FreeData(GWEN_UNUSED void *bp, void *p)
{
  W_COMBOBOX *xw=p;

  if (xw->model)
    g_object_unref(xw->model);
  GWEN_StringList_free(xw->entries);
  GWEN_FREE_OBJECT(xw);
}


int Gtk4Gui_WComboBox_Setup(GWEN_WIDGET *w)
{
  W_COMBOBOX *xw;
  GtkWidget *g;
  uint32_t flags;
  GWEN_WIDGET *wParent;

  flags=GWEN_Widget_GetFlags(w);
  wParent=GWEN_Widget_Tree_GetParent(w);

  GWEN_NEW_OBJECT(W_COMBOBOX, xw);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, W_COMBOBOX, w, xw, Gtk4Gui_WComboBox_FreeData);
  xw->entries=GWEN_StringList_new();
  xw->model=gtk_string_list_new(NULL);
  xw->dropDown=GTK_DROP_DOWN(gtk_drop_down_new(G_LIST_MODEL(g_object_ref(xw->model)), NULL));

  if (flags & GWEN_WIDGET_FLAGS_READONLY) {
    g=GTK_WIDGET(xw->dropDown);
  }
  else {
    GtkWidget *box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    xw->entry=GTK_ENTRY(gtk_entry_new());
    gtk_widget_set_hexpand(GTK_WIDGET(xw->entry), TRUE);
    gtk_box_append(GTK_BOX(box), GTK_WIDGET(xw->entry));
    gtk_box_append(GTK_BOX(box), GTK_WIDGET(xw->dropDown));
    g=box;
  }

  Gtk4Gui_ApplyFlags(g, flags);

  GWEN_Widget_SetImplData(w, GTK4_DIALOG_WIDGET_REAL, g);
  GWEN_Widget_SetImplData(w, GTK4_DIALOG_WIDGET_CONTENT, g);

  GWEN_Widget_SetSetIntPropertyFn(w, Gtk4Gui_WComboBox_SetIntProperty);
  GWEN_Widget_SetGetIntPropertyFn(w, Gtk4Gui_WComboBox_GetIntProperty);
  GWEN_Widget_SetSetCharPropertyFn(w, Gtk4Gui_WComboBox_SetCharProperty);
  GWEN_Widget_SetGetCharPropertyFn(w, Gtk4Gui_WComboBox_GetCharProperty);

  g_signal_connect(xw->dropDown, "notify::selected", G_CALLBACK(selected_handler), w);
  if (xw->entry)
    g_signal_connect(xw->entry, "changed", G_CALLBACK(entry_changed_handler), w);

  if (wParent)
    GWEN_Widget_AddChildGuiWidget(wParent, w);

  return 0;
}
