/***************************************************************************
    begin       : Tue Sep 2 2026
    copyright   : (C) 2026 by the GnuCash contributors

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GTK4_GUI_FILE_DIALOG_P_H
#define GTK4_GUI_FILE_DIALOG_P_H


#include <gwenhywfar/gui.h>


void Gtk4Gui_FileDialog_PreparePath(GWEN_GUI_FILENAME_TYPE fnt,
                                    const char *path,
                                    char **initialFolder,
                                    char **initialName);


#endif
