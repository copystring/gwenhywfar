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

#include "gtk4_gui_file_dialog_p.h"

#include <assert.h>
#include <glib.h>


static int
Gtk4Gui_IsDirectorySeparator(char c)
{
  return c=='/' || c=='\\';
}


void
Gtk4Gui_FileDialog_PreparePath(GWEN_GUI_FILENAME_TYPE fnt,
                               const char *path,
                               char **initialFolder,
                               char **initialName)
{
  const char *p;
  const char *separator=NULL;

  assert(initialFolder);
  assert(initialName);
  *initialFolder=NULL;
  *initialName=NULL;

  if (path==NULL || *path==0)
    return;

  /* A directory chooser has no filename field. Keep its complete path,
   * including a root or a trailing separator, as the initial directory. */
  if (fnt==GWEN_Gui_FileNameType_OpenDirectory) {
    *initialFolder=g_strdup(path);
    return;
  }

  for (p=path; *p; p++) {
    if (Gtk4Gui_IsDirectorySeparator(*p))
      separator=p;
  }

  if (separator==NULL) {
    *initialName=g_strdup(path);
    return;
  }

  if (separator[1]==0) {
    *initialFolder=g_strdup(path);
    return;
  }

  /* Keep the separator for POSIX and Windows filesystem roots. */
  if (separator==path ||
      (separator==path+2 && path[0] && path[1]==':'))
    *initialFolder=g_strndup(path, separator-path+1);
  else
    *initialFolder=g_strndup(path, separator-path);
  *initialName=g_strdup(separator+1);
}
