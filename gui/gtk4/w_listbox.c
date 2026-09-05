/***************************************************************************
    begin       : Fri Jul 09 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


typedef struct _Gtk4GuiListBoxRow Gtk4GuiListBoxRow;
typedef struct _Gtk4GuiListBoxRowClass Gtk4GuiListBoxRowClass;

struct _Gtk4GuiListBoxRow {
  GObject parent_instance;
  GPtrArray *values;
};

struct _Gtk4GuiListBoxRowClass {
  GObjectClass parent_class;
};

G_DEFINE_TYPE(Gtk4GuiListBoxRow, gtk4_gui_list_box_row, G_TYPE_OBJECT)


static void gtk4_gui_list_box_row_finalize(GObject *object)
{
  Gtk4GuiListBoxRow *row=(Gtk4GuiListBoxRow *)object;

  g_clear_pointer(&row->values, g_ptr_array_unref);
  G_OBJECT_CLASS(gtk4_gui_list_box_row_parent_class)->finalize(object);
}


static void gtk4_gui_list_box_row_class_init(Gtk4GuiListBoxRowClass *klass)
{
  G_OBJECT_CLASS(klass)->finalize=gtk4_gui_list_box_row_finalize;
}


static void gtk4_gui_list_box_row_init(Gtk4GuiListBoxRow *row)
{
  row->values=g_ptr_array_new_with_free_func(g_free);
}


static Gtk4GuiListBoxRow *gtk4_gui_list_box_row_new(const char *text, guint columns)
{
  Gtk4GuiListBoxRow *row;
  gchar **parts;
  guint partCount;
  guint i;

  row=g_object_new(gtk4_gui_list_box_row_get_type(), NULL);
  parts=g_strsplit(text ? text : "", "\t", -1);
  partCount=g_strv_length(parts);
  for (i=0; i<columns; i++)
    g_ptr_array_add(row->values, g_strdup(i<partCount ? parts[i] : ""));
  g_strfreev(parts);
  return row;
}


static const char *gtk4_gui_list_box_row_get_value(Gtk4GuiListBoxRow *row, guint column)
{
  if (row==NULL || column>=row->values->len)
    return "";
  return g_ptr_array_index(row->values, column);
}


typedef struct W_LISTBOX W_LISTBOX;
struct W_LISTBOX {
  GtkColumnView *view;
  GListStore *store;
  GtkSortListModel *sortModel;
  GtkCustomSorter *sorter;
  GtkSelectionModel *selection;
  GPtrArray *headers;
  GArray *sortDirections;
  int selectionMode;
  int suppressSelectionChanged;
};


GWEN_INHERIT(GWEN_WIDGET, W_LISTBOX)


static int Gtk4Gui_WListBox_CompareRows(gconstpointer first,
                                        gconstpointer second,
                                        gpointer data)
{
  W_LISTBOX *xw=data;
  Gtk4GuiListBoxRow *firstRow=(Gtk4GuiListBoxRow *)first;
  Gtk4GuiListBoxRow *secondRow=(Gtk4GuiListBoxRow *)second;
  guint i;

  for (i=0; i<xw->sortDirections->len; i++) {
    int direction=g_array_index(xw->sortDirections, int, i);
    int comparison;

    if (direction==GWEN_DialogSortDirection_None)
      continue;

    comparison=g_utf8_collate(gtk4_gui_list_box_row_get_value(firstRow, i),
                              gtk4_gui_list_box_row_get_value(secondRow, i));
    if (comparison<0)
      return direction==GWEN_DialogSortDirection_Down ? 1 : -1;
    if (comparison>0)
      return direction==GWEN_DialogSortDirection_Down ? -1 : 1;
    return 0;
  }

  return 0;
}


static void Gtk4Gui_WListBox_Resort(W_LISTBOX *xw)
{
  xw->suppressSelectionChanged++;
  gtk_sorter_changed(GTK_SORTER(xw->sorter), GTK_SORTER_CHANGE_DIFFERENT);
  xw->suppressSelectionChanged--;
}


static void Gtk4Gui_WListBox_EmitActivated(GWEN_WIDGET *w)
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


static void Gtk4Gui_WListBox_SelectionChanged(GtkSelectionModel *model,
                                              guint position,
                                              guint n_items,
                                              gpointer data)
{
  GWEN_WIDGET *w=data;
  W_LISTBOX *xw;
  guint i;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_LISTBOX, w);
  assert(xw);

  if (xw->suppressSelectionChanged)
    return;

  for (i=position; i<position+n_items; i++) {
    if (gtk_selection_model_is_selected(model, i)) {
      Gtk4Gui_WListBox_EmitActivated(w);
      return;
    }
  }
}


static void Gtk4Gui_WListBox_ClearSelectionModel(GWEN_WIDGET *w, W_LISTBOX *xw)
{
  if (xw->selection)
    g_signal_handlers_disconnect_by_func(xw->selection,
                                         G_CALLBACK(Gtk4Gui_WListBox_SelectionChanged),
                                         w);
  gtk_column_view_set_model(xw->view, NULL);
  g_clear_object(&xw->selection);
}


static void Gtk4Gui_WListBox_FactorySetup(GtkSignalListItemFactory *factory,
                                          GtkListItem *item,
                                          gpointer data)
{
  GtkWidget *label=gtk_label_new(NULL);

  (void)factory;
  (void)data;
  gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
  gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
  gtk_list_item_set_child(item, label);
}


static void Gtk4Gui_WListBox_FactoryBind(GtkSignalListItemFactory *factory,
                                         GtkListItem *item,
                                         gpointer data)
{
  guint column=*((guint *)data);
  Gtk4GuiListBoxRow *row=(Gtk4GuiListBoxRow *)gtk_list_item_get_item(item);
  GtkWidget *label=gtk_list_item_get_child(item);

  (void)factory;
  gtk_label_set_text(GTK_LABEL(label), gtk4_gui_list_box_row_get_value(row, column));
}


static GtkColumnViewColumn *Gtk4Gui_WListBox_GetColumn(W_LISTBOX *xw, guint index)
{
  GListModel *columns;
  GtkColumnViewColumn *column;

  columns=gtk_column_view_get_columns(xw->view);
  if (index>=g_list_model_get_n_items(columns))
    return NULL;
  column=g_list_model_get_item(columns, index);
  return column;
}


static void Gtk4Gui_WListBox_ClearColumns(W_LISTBOX *xw)
{
  GtkColumnViewColumn *column;

  while ((column=Gtk4Gui_WListBox_GetColumn(xw, 0))) {
    gtk_column_view_remove_column(xw->view, column);
    g_object_unref(column);
  }
}


static void Gtk4Gui_WListBox_SetSelectionModel(GWEN_WIDGET *w, W_LISTBOX *xw, int mode)
{
  GtkSelectionModel *selection;

  Gtk4Gui_WListBox_ClearSelectionModel(w, xw);

  switch (mode) {
  case GWEN_Dialog_SelectionMode_None:
    selection=GTK_SELECTION_MODEL(gtk_no_selection_new(G_LIST_MODEL(g_object_ref(xw->sortModel))));
    break;
  case GWEN_Dialog_SelectionMode_Multi:
    selection=GTK_SELECTION_MODEL(gtk_multi_selection_new(G_LIST_MODEL(g_object_ref(xw->sortModel))));
    break;
  case GWEN_Dialog_SelectionMode_Single:
  default: {
    GtkSingleSelection *singleSelection;

    mode=GWEN_Dialog_SelectionMode_Single;
    singleSelection=gtk_single_selection_new(NULL);
    gtk_single_selection_set_autoselect(singleSelection, FALSE);
    gtk_single_selection_set_can_unselect(singleSelection, TRUE);
    gtk_single_selection_set_model(singleSelection, G_LIST_MODEL(xw->sortModel));
    selection=GTK_SELECTION_MODEL(singleSelection);
    break;
  }
  }

  gtk_column_view_set_model(xw->view, selection);
  xw->selection=selection;
  xw->selectionMode=mode;
  g_signal_connect(xw->selection,
                   "selection-changed",
                   G_CALLBACK(Gtk4Gui_WListBox_SelectionChanged),
                   w);
}


static void Gtk4Gui_WListBox_ClearValues(W_LISTBOX *xw)
{
  xw->suppressSelectionChanged++;
  g_list_store_remove_all(xw->store);
  xw->suppressSelectionChanged--;
}


static void Gtk4Gui_WListBox_Rebuild(GWEN_WIDGET *w, W_LISTBOX *xw, const char *title)
{
  gchar **parts;
  guint i;

  g_ptr_array_set_size(xw->headers, 0);
  parts=g_strsplit(title, "\t", -1);
  for (i=0; parts[i]; i++)
    g_ptr_array_add(xw->headers, g_strdup(parts[i]));
  g_strfreev(parts);

  Gtk4Gui_WListBox_ClearColumns(xw);
  Gtk4Gui_WListBox_ClearSelectionModel(w, xw);
  g_clear_object(&xw->sortModel);
  g_clear_object(&xw->sorter);
  g_clear_object(&xw->store);
  xw->store=g_list_store_new(gtk4_gui_list_box_row_get_type());
  xw->sorter=gtk_custom_sorter_new(Gtk4Gui_WListBox_CompareRows, xw, NULL);
  xw->sortModel=gtk_sort_list_model_new(G_LIST_MODEL(g_object_ref(xw->store)),
                                         GTK_SORTER(g_object_ref(xw->sorter)));
  gtk_sort_list_model_set_incremental(xw->sortModel, FALSE);
  Gtk4Gui_WListBox_SetSelectionModel(w, xw, xw->selectionMode);

  for (i=0; i<xw->headers->len; i++) {
    GtkListItemFactory *factory;
    GtkColumnViewColumn *column;
    guint *factoryColumn;

    factory=gtk_signal_list_item_factory_new();
    factoryColumn=g_new(guint, 1);
    *factoryColumn=i;
    g_object_set_data_full(G_OBJECT(factory), "gwen-list-column", factoryColumn, g_free);
    g_signal_connect(factory,
                     "setup",
                     G_CALLBACK(Gtk4Gui_WListBox_FactorySetup),
                     factoryColumn);
    g_signal_connect(factory,
                     "bind",
                     G_CALLBACK(Gtk4Gui_WListBox_FactoryBind),
                     factoryColumn);
    column=gtk_column_view_column_new(g_ptr_array_index(xw->headers, i), factory);
    gtk_column_view_column_set_resizable(column, TRUE);
    gtk_column_view_append_column(xw->view, column);
    g_object_unref(column);
  }

  g_array_set_size(xw->sortDirections, xw->headers->len);
  for (i=0; i<xw->sortDirections->len; i++)
    g_array_index(xw->sortDirections, int, i)=GWEN_DialogSortDirection_None;
  GWEN_Widget_SetColumns(w, (int)xw->headers->len);
}


static GWENHYWFAR_CB
int Gtk4Gui_WListBox_SetIntProperty(GWEN_WIDGET *w,
                                    GWEN_DIALOG_PROPERTY prop,
                                    int index,
                                    int value,
                                    int doSignal)
{
  GtkWidget *g;
  W_LISTBOX *xw;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_LISTBOX, w);
  assert(xw);
  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_CONTENT));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Enabled:
    gtk_widget_set_sensitive(g, value!=0);
    return 0;

  case GWEN_DialogProperty_Focus:
    gtk_widget_grab_focus(g);
    return 0;

  case GWEN_DialogProperty_Value: {
    gboolean success;
    guint count=g_list_model_get_n_items(G_LIST_MODEL(xw->selection));

    if (value<0 || (guint)value>=count)
      return GWEN_ERROR_INVALID;
    if (!doSignal)
      xw->suppressSelectionChanged++;
    success=gtk_selection_model_select_item(xw->selection, (guint)value, TRUE);
    if (!doSignal)
      xw->suppressSelectionChanged--;
    return success ? 0 : GWEN_ERROR_INVALID;
  }

  case GWEN_DialogProperty_SelectionMode:
    if (value!=GWEN_Dialog_SelectionMode_None &&
        value!=GWEN_Dialog_SelectionMode_Single &&
        value!=GWEN_Dialog_SelectionMode_Multi)
      return GWEN_ERROR_INVALID;
    Gtk4Gui_WListBox_SetSelectionModel(w, xw, value);
    return 0;

  case GWEN_DialogProperty_ColumnWidth: {
    GtkColumnViewColumn *column;

    if (index<0 || (column=Gtk4Gui_WListBox_GetColumn(xw, (guint)index))==NULL)
      return GWEN_ERROR_INVALID;
    gtk_column_view_column_set_fixed_width(column, value);
    g_object_unref(column);
    return 0;
  }

  case GWEN_DialogProperty_SortDirection: {
    guint i;

    if (index<0 || (guint)index>=xw->sortDirections->len)
      return GWEN_ERROR_INVALID;
    if (value!=GWEN_DialogSortDirection_None &&
        value!=GWEN_DialogSortDirection_Up &&
        value!=GWEN_DialogSortDirection_Down)
      return GWEN_ERROR_INVALID;
    for (i=0; i<xw->sortDirections->len; i++)
      g_array_index(xw->sortDirections, int, i)=GWEN_DialogSortDirection_None;
    if (value!=GWEN_DialogSortDirection_None)
      g_array_index(xw->sortDirections, int, index)=value;
    Gtk4Gui_WListBox_Resort(xw);
    return 0;
  }

  case GWEN_DialogProperty_ClearValues:
    Gtk4Gui_WListBox_ClearValues(xw);
    return 0;

  case GWEN_DialogProperty_Sort:
    Gtk4Gui_WListBox_Resort(xw);
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
int Gtk4Gui_WListBox_GetIntProperty(GWEN_WIDGET *w,
                                    GWEN_DIALOG_PROPERTY prop,
                                    int index,
                                    int defaultValue)
{
  GtkWidget *g;
  W_LISTBOX *xw;
  guint i;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_LISTBOX, w);
  assert(xw);
  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_CONTENT));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Enabled:
    return gtk_widget_get_sensitive(g) ? 1 : 0;

  case GWEN_DialogProperty_Focus:
    return gtk_widget_has_focus(g) ? 1 : 0;

  case GWEN_DialogProperty_Value:
    for (i=0; i<g_list_model_get_n_items(G_LIST_MODEL(xw->selection)); i++) {
      if (gtk_selection_model_is_selected(xw->selection, i))
        return (int)i;
    }
    return -1;

  case GWEN_DialogProperty_ColumnWidth: {
    GtkColumnViewColumn *column;
    int width;

    if (index<0 || (column=Gtk4Gui_WListBox_GetColumn(xw, (guint)index))==NULL)
      return -1;
    width=gtk_column_view_column_get_fixed_width(column);
    g_object_unref(column);
    return width;
  }

  case GWEN_DialogProperty_SortDirection:
    if (index<0 || (guint)index>=xw->sortDirections->len)
      return GWEN_DialogSortDirection_None;
    return g_array_index(xw->sortDirections, int, index);

  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function %d is not appropriate for this type of widget (%s)",
           prop,
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}


static GWENHYWFAR_CB
int Gtk4Gui_WListBox_SetCharProperty(GWEN_WIDGET *w,
                                     GWEN_DIALOG_PROPERTY prop,
                                     GWEN_UNUSED int index,
                                     const char *value,
                                     GWEN_UNUSED int doSignal)
{
  W_LISTBOX *xw;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_LISTBOX, w);
  assert(xw);

  switch (prop) {
  case GWEN_DialogProperty_Title:
    if (value==NULL || *value==0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No columns (empty title)");
      return GWEN_ERROR_INVALID;
    }
    Gtk4Gui_WListBox_Rebuild(w, xw, value);
    return 0;

  case GWEN_DialogProperty_ClearValues:
    Gtk4Gui_WListBox_ClearValues(xw);
    return 0;

  case GWEN_DialogProperty_AddValue: {
    Gtk4GuiListBoxRow *row;

    if (xw->headers->len==0)
      return GWEN_ERROR_INVALID;
    row=gtk4_gui_list_box_row_new(value, xw->headers->len);
    xw->suppressSelectionChanged++;
    g_list_store_append(xw->store, row);
    xw->suppressSelectionChanged--;
    g_object_unref(row);
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
const char *Gtk4Gui_WListBox_GetCharProperty(GWEN_WIDGET *w,
                                             GWEN_DIALOG_PROPERTY prop,
                                             int index,
                                             const char *defaultValue)
{
  W_LISTBOX *xw;
  GWEN_BUFFER *buffer;
  guint i;

  assert(w);
  xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_LISTBOX, w);
  assert(xw);

  switch (prop) {
  case GWEN_DialogProperty_Title:
    buffer=GWEN_Buffer_new(0, 256, 0, 1);
    for (i=0; i<xw->headers->len; i++) {
      if (i)
        GWEN_Buffer_AppendByte(buffer, '\t');
      GWEN_Buffer_AppendString(buffer, g_ptr_array_index(xw->headers, i));
    }
    GWEN_Widget_SetText(w, GTK4_DIALOG_STRING_TITLE, GWEN_Buffer_GetStart(buffer));
    GWEN_Buffer_free(buffer);
    return GWEN_Widget_GetText(w, GTK4_DIALOG_STRING_TITLE);

  case GWEN_DialogProperty_Value: {
    Gtk4GuiListBoxRow *row;
    const char *result;

    if (index<0 || (guint)index>=g_list_model_get_n_items(G_LIST_MODEL(xw->sortModel)))
      return defaultValue;
    row=g_list_model_get_item(G_LIST_MODEL(xw->sortModel), (guint)index);
    buffer=GWEN_Buffer_new(0, 256, 0, 1);
    for (i=0; i<xw->headers->len; i++) {
      if (i)
        GWEN_Buffer_AppendByte(buffer, '\t');
      GWEN_Buffer_AppendString(buffer, gtk4_gui_list_box_row_get_value(row, i));
    }
    GWEN_Widget_SetText(w, GTK4_DIALOG_STRING_VALUE, GWEN_Buffer_GetStart(buffer));
    GWEN_Buffer_free(buffer);
    result=GWEN_Widget_GetText(w, GTK4_DIALOG_STRING_VALUE);
    g_object_unref(row);
    return result;
  }

  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}


static void GWENHYWFAR_CB Gtk4Gui_WListBox_FreeData(void *bp, void *p)
{
  GWEN_WIDGET *w=bp;
  W_LISTBOX *xw=p;

  if (xw->selection)
    g_signal_handlers_disconnect_by_func(xw->selection,
                                         G_CALLBACK(Gtk4Gui_WListBox_SelectionChanged),
                                         w);
  g_clear_object(&xw->selection);
  g_clear_object(&xw->sortModel);
  g_clear_object(&xw->sorter);
  g_clear_object(&xw->store);
  g_clear_pointer(&xw->headers, g_ptr_array_unref);
  g_clear_pointer(&xw->sortDirections, g_array_unref);
  GWEN_FREE_OBJECT(xw);
}


int Gtk4Gui_WListBox_Setup(GWEN_WIDGET *w)
{
  W_LISTBOX *xw;
  GtkWidget *scroll;
  GWEN_WIDGET *wParent;

  wParent=GWEN_Widget_Tree_GetParent(w);
  GWEN_NEW_OBJECT(W_LISTBOX, xw);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, W_LISTBOX, w, xw, Gtk4Gui_WListBox_FreeData);
  xw->headers=g_ptr_array_new_with_free_func(g_free);
  xw->sortDirections=g_array_new(FALSE, TRUE, sizeof(int));
  xw->store=g_list_store_new(gtk4_gui_list_box_row_get_type());
  xw->sorter=gtk_custom_sorter_new(Gtk4Gui_WListBox_CompareRows, xw, NULL);
  xw->sortModel=gtk_sort_list_model_new(G_LIST_MODEL(g_object_ref(xw->store)),
                                         GTK_SORTER(g_object_ref(xw->sorter)));
  gtk_sort_list_model_set_incremental(xw->sortModel, FALSE);
  xw->view=GTK_COLUMN_VIEW(gtk_column_view_new(NULL));
  Gtk4Gui_WListBox_SetSelectionModel(w, xw, GWEN_Dialog_SelectionMode_Single);

  scroll=gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), GTK_WIDGET(xw->view));
  Gtk4Gui_ApplyFlags(scroll, GWEN_Widget_GetFlags(w));

  GWEN_Widget_SetImplData(w, GTK4_DIALOG_WIDGET_REAL, scroll);
  GWEN_Widget_SetImplData(w, GTK4_DIALOG_WIDGET_CONTENT, xw->view);
  GWEN_Widget_SetSetIntPropertyFn(w, Gtk4Gui_WListBox_SetIntProperty);
  GWEN_Widget_SetGetIntPropertyFn(w, Gtk4Gui_WListBox_GetIntProperty);
  GWEN_Widget_SetSetCharPropertyFn(w, Gtk4Gui_WListBox_SetCharProperty);
  GWEN_Widget_SetGetCharPropertyFn(w, Gtk4Gui_WListBox_GetCharProperty);

  if (wParent)
    GWEN_Widget_AddChildGuiWidget(wParent, w);

  return 0;
}
