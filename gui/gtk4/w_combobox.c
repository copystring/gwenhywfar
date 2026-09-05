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
  GtkSingleSelection *selection;
  GtkSingleSelection *popupSelection;
  GtkMenuButton *menuButton;
  GtkPopover *popover;
  GtkListView *listView;
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


static void Gtk4Gui_WComboBox_BeginSync(W_COMBOBOX *xw)
{
  xw->syncing++;
}


static void Gtk4Gui_WComboBox_EndSync(W_COMBOBOX *xw)
{
  assert(xw->syncing>0);
  xw->syncing--;
}


static void Gtk4Gui_WComboBox_FocusControl(W_COMBOBOX *xw)
{
  gtk_widget_grab_focus(xw->entry ? GTK_WIDGET(xw->entry) : GTK_WIDGET(xw->menuButton));
}


static gboolean Gtk4Gui_WComboBox_HasFocus(W_COMBOBOX *xw)
{
  GtkWidget *control=xw->entry ? GTK_WIDGET(xw->entry) : GTK_WIDGET(xw->menuButton);
  GtkRoot *root=gtk_widget_get_root(control);
  GtkWidget *focus=root ? gtk_root_get_focus(root) : NULL;
  GtkRoot *popupRoot=gtk_widget_get_root(GTK_WIDGET(xw->popover));
  GtkWidget *popupFocus=popupRoot ? gtk_root_get_focus(popupRoot) : NULL;

  return gtk_menu_button_get_active(xw->menuButton) ||
         (focus &&
          (focus==control || gtk_widget_is_ancestor(focus, control))) ||
         (popupFocus &&
          (popupFocus==GTK_WIDGET(xw->popover) ||
           gtk_widget_is_ancestor(popupFocus, GTK_WIDGET(xw->popover))));
}


static void Gtk4Gui_WComboBox_UpdatePresentation(W_COMBOBOX *xw)
{
  guint selected;
  const char *text=NULL;

  selected=gtk_single_selection_get_selected(xw->selection);
  if (selected!=GTK_INVALID_LIST_POSITION)
    text=gtk_string_list_get_string(xw->model, selected);

  if (xw->entry) {
    if (text) {
      Gtk4Gui_WComboBox_BeginSync(xw);
      gtk_editable_set_text(GTK_EDITABLE(xw->entry), text);
      Gtk4Gui_WComboBox_EndSync(xw);
    }
  }
  else {
    gtk_menu_button_set_label(xw->menuButton, text ? text : "");
    gtk_accessible_update_property(GTK_ACCESSIBLE(xw->menuButton),
                                   GTK_ACCESSIBLE_PROPERTY_LABEL,
                                   text && *text ? text : I18N("Select a value"),
                                   -1);
  }
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

  Gtk4Gui_WComboBox_UpdatePresentation(xw);
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


static void Gtk4Gui_WComboBox_FactorySetup(GtkSignalListItemFactory *factory,
                                            GtkListItem *item,
                                            gpointer data)
{
  GtkWidget *label=gtk_label_new(NULL);

  (void)factory;
  (void)data;
  gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
  gtk_list_item_set_child(item, label);
}


static void Gtk4Gui_WComboBox_FactoryBind(GtkSignalListItemFactory *factory,
                                           GtkListItem *item,
                                           gpointer data)
{
  GtkStringObject *stringObject=GTK_STRING_OBJECT(gtk_list_item_get_item(item));
  GtkWidget *label=gtk_list_item_get_child(item);

  (void)factory;
  (void)data;
  gtk_label_set_text(GTK_LABEL(label), gtk_string_object_get_string(stringObject));
}


static void Gtk4Gui_WComboBox_ListActivated(GtkListView *listView,
                                             guint position,
                                             gpointer data)
{
  GWEN_WIDGET *w=data;
  W_COMBOBOX *xw;

  (void)listView;
  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_COMBOBOX, w);
  assert(xw);
  if (position>=g_list_model_get_n_items(G_LIST_MODEL(xw->model)))
    return;

  /* Closing and returning focus must happen before the Gwen signal. Its
   * handler is allowed to destroy the dialog and therefore xw. */
  gtk_menu_button_popdown(xw->menuButton);
  Gtk4Gui_WComboBox_FocusControl(xw);
  gtk_single_selection_set_selected(xw->selection, position);
}


static void Gtk4Gui_WComboBox_PopoverShown(GtkWidget *popover, gpointer data)
{
  W_COMBOBOX *xw=data;
  guint count;
  guint selected;

  (void)popover;
  count=g_list_model_get_n_items(G_LIST_MODEL(xw->model));
  gtk_widget_grab_focus(GTK_WIDGET(xw->listView));
  if (count==0)
    return;

  selected=gtk_single_selection_get_selected(xw->selection);
  gtk_single_selection_set_selected(xw->popupSelection, selected);
  gtk_list_view_scroll_to(xw->listView,
                          selected==GTK_INVALID_LIST_POSITION ? 0 : selected,
                          GTK_LIST_SCROLL_FOCUS,
                          NULL);
}


static gboolean Gtk4Gui_WComboBox_PopupKeyPressed(GtkEventControllerKey *controller,
                                                   guint keyval,
                                                   guint keycode,
                                                   GdkModifierType state,
                                                   gpointer data)
{
  W_COMBOBOX *xw=data;

  (void)controller;
  (void)keycode;
  (void)state;
  if (keyval!=GDK_KEY_Escape)
    return FALSE;

  gtk_menu_button_popdown(xw->menuButton);
  Gtk4Gui_WComboBox_FocusControl(xw);
  return TRUE;
}


static void Gtk4Gui_WComboBox_Clear(W_COMBOBOX *xw, int doSignal)
{
  guint count;

  if (!doSignal)
    Gtk4Gui_WComboBox_BeginSync(xw);
  count=g_list_model_get_n_items(G_LIST_MODEL(xw->model));
  GWEN_StringList_Clear(xw->entries);
  gtk_string_list_splice(xw->model, 0, count, NULL);
  if (!doSignal)
    Gtk4Gui_WComboBox_EndSync(xw);
}


static GWENHYWFAR_CB
int Gtk4Gui_WComboBox_SetIntProperty(GWEN_WIDGET *w,
                                     GWEN_DIALOG_PROPERTY prop,
                                     GWEN_UNUSED int index,
                                     int value,
                                     int doSignal)
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
    Gtk4Gui_WComboBox_FocusControl(xw);
    return 0;

  case GWEN_DialogProperty_Value:
    if (!doSignal)
      Gtk4Gui_WComboBox_BeginSync(xw);
    if (value<0 || (guint)value>=g_list_model_get_n_items(G_LIST_MODEL(xw->model)))
      gtk_single_selection_set_selected(xw->selection, GTK_INVALID_LIST_POSITION);
    else
      gtk_single_selection_set_selected(xw->selection, (guint)value);
    if (!doSignal)
      Gtk4Gui_WComboBox_EndSync(xw);
    return 0;

  case GWEN_DialogProperty_ClearValues:
    Gtk4Gui_WComboBox_Clear(xw, doSignal);
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
    return Gtk4Gui_WComboBox_HasFocus(xw) ? 1 : 0;

  case GWEN_DialogProperty_Value: {
    guint selected=gtk_single_selection_get_selected(xw->selection);
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
                                      int doSignal)
{
  W_COMBOBOX *xw;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_COMBOBOX, w);
  assert(xw);

  switch (prop) {
  case GWEN_DialogProperty_Value:
    /* The legacy backend deliberately did not define text assignment. */
    return 0;

  case GWEN_DialogProperty_AddValue: {
    const char *text=value ? value : "";

    Gtk4Gui_WComboBox_BeginSync(xw);
    GWEN_StringList_AppendString(xw->entries, text, 0, 0);
    gtk_string_list_append(xw->model, text);
    Gtk4Gui_WComboBox_EndSync(xw);
    return 0;
  }

  case GWEN_DialogProperty_ClearValues:
    Gtk4Gui_WComboBox_Clear(xw, doSignal);
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
  if (xw->selection)
    g_object_unref(xw->selection);
  if (xw->popupSelection)
    g_object_unref(xw->popupSelection);
  GWEN_StringList_free(xw->entries);
  GWEN_FREE_OBJECT(xw);
}


int Gtk4Gui_WComboBox_Setup(GWEN_WIDGET *w)
{
  W_COMBOBOX *xw;
  GtkWidget *g;
  GtkListItemFactory *factory;
  GtkEventController *keyController;
  GtkWidget *scroll;
  uint32_t flags;
  GWEN_WIDGET *wParent;

  flags=GWEN_Widget_GetFlags(w);
  wParent=GWEN_Widget_Tree_GetParent(w);

  GWEN_NEW_OBJECT(W_COMBOBOX, xw);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, W_COMBOBOX, w, xw, Gtk4Gui_WComboBox_FreeData);
  xw->entries=GWEN_StringList_new();
  xw->model=gtk_string_list_new(NULL);
  xw->selection=gtk_single_selection_new(NULL);
  gtk_single_selection_set_autoselect(xw->selection, FALSE);
  gtk_single_selection_set_can_unselect(xw->selection, TRUE);
  gtk_single_selection_set_model(xw->selection, G_LIST_MODEL(xw->model));
  xw->popupSelection=gtk_single_selection_new(NULL);
  gtk_single_selection_set_autoselect(xw->popupSelection, FALSE);
  gtk_single_selection_set_can_unselect(xw->popupSelection, TRUE);
  gtk_single_selection_set_model(xw->popupSelection, G_LIST_MODEL(xw->model));

  factory=gtk_signal_list_item_factory_new();
  g_signal_connect(factory,
                   "setup",
                   G_CALLBACK(Gtk4Gui_WComboBox_FactorySetup),
                   NULL);
  g_signal_connect(factory,
                   "bind",
                   G_CALLBACK(Gtk4Gui_WComboBox_FactoryBind),
                   NULL);
  xw->listView=GTK_LIST_VIEW(gtk_list_view_new(GTK_SELECTION_MODEL(g_object_ref(xw->popupSelection)),
                                               factory));
  gtk_list_view_set_single_click_activate(xw->listView, TRUE);

  scroll=gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_NEVER,
                                 GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(scroll), TRUE);
  gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scroll), 300);
  gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroll), TRUE);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), GTK_WIDGET(xw->listView));
  xw->popover=GTK_POPOVER(gtk_popover_new());
  gtk_popover_set_child(xw->popover, scroll);

  xw->menuButton=GTK_MENU_BUTTON(g_object_new(GTK_TYPE_MENU_BUTTON,
                                               "accessible-role", GTK_ACCESSIBLE_ROLE_COMBO_BOX,
                                               NULL));
  gtk_menu_button_set_popover(xw->menuButton, GTK_WIDGET(xw->popover));
  gtk_menu_button_set_always_show_arrow(xw->menuButton, TRUE);

  if (flags & GWEN_WIDGET_FLAGS_READONLY) {
    gtk_menu_button_set_label(xw->menuButton, "");
    gtk_accessible_update_property(GTK_ACCESSIBLE(xw->menuButton),
                                   GTK_ACCESSIBLE_PROPERTY_LABEL,
                                   I18N("Select a value"),
                                   -1);
    g=GTK_WIDGET(xw->menuButton);
  }
  else {
    GtkWidget *box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    xw->entry=GTK_ENTRY(gtk_entry_new());
    gtk_menu_button_set_icon_name(xw->menuButton, "pan-down-symbolic");
    gtk_accessible_update_property(GTK_ACCESSIBLE(xw->menuButton),
                                   GTK_ACCESSIBLE_PROPERTY_LABEL,
                                   I18N("Show choices"),
                                   -1);
    gtk_widget_set_hexpand(GTK_WIDGET(xw->entry), TRUE);
    gtk_box_append(GTK_BOX(box), GTK_WIDGET(xw->entry));
    gtk_box_append(GTK_BOX(box), GTK_WIDGET(xw->menuButton));
    g=box;
  }

  Gtk4Gui_ApplyFlags(g, flags);

  GWEN_Widget_SetImplData(w, GTK4_DIALOG_WIDGET_REAL, g);
  GWEN_Widget_SetImplData(w, GTK4_DIALOG_WIDGET_CONTENT, g);

  GWEN_Widget_SetSetIntPropertyFn(w, Gtk4Gui_WComboBox_SetIntProperty);
  GWEN_Widget_SetGetIntPropertyFn(w, Gtk4Gui_WComboBox_GetIntProperty);
  GWEN_Widget_SetSetCharPropertyFn(w, Gtk4Gui_WComboBox_SetCharProperty);
  GWEN_Widget_SetGetCharPropertyFn(w, Gtk4Gui_WComboBox_GetCharProperty);

  g_signal_connect(xw->selection, "notify::selected", G_CALLBACK(selected_handler), w);
  g_signal_connect(xw->listView, "activate", G_CALLBACK(Gtk4Gui_WComboBox_ListActivated), w);
  g_signal_connect(xw->popover, "show", G_CALLBACK(Gtk4Gui_WComboBox_PopoverShown), xw);
  keyController=gtk_event_controller_key_new();
  g_signal_connect(keyController,
                   "key-pressed",
                   G_CALLBACK(Gtk4Gui_WComboBox_PopupKeyPressed),
                   xw);
  gtk_widget_add_controller(GTK_WIDGET(xw->listView), keyController);
  if (xw->entry)
    g_signal_connect(xw->entry, "changed", G_CALLBACK(entry_changed_handler), w);

  if (wParent)
    GWEN_Widget_AddChildGuiWidget(wParent, w);

  return 0;
}
