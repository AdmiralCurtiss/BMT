/**
* This file is part of Batman Tweak.
*
* Batman Tweak is free software : you can redistribute it and / or modify
* it under the terms of the GNU General Public License as published by
* The Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Batman Tweak is distributed in the hope that it will be useful,
* But WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Batman Tweak.If not, see <http://www.gnu.org/licenses/>.
**/

#include <Windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include "resource.h"

#include "bmt.h"

using namespace bmt;

INT_PTR
CALLBACK
ImportExport (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INITDIALOG:
  {
    HWND hWndDataSelect = GetDlgItem (hDlg, IDC_DATA);

    LONG style = GetWindowLong (hWndDataSelect, GWL_STYLE);
    SetWindowLong (hWndDataSelect, GWL_STYLE, style | TVS_CHECKBOXES | TVS_HASBUTTONS);

    wchar_t* default_sections [] { L"Engine.Engine",
      L"Engine.Client",
      L"Engine.GameEngine",
      L"TextureStreaming" };
    int num_def_secs = sizeof (default_sections) / sizeof (wchar_t *);

    INI::File* bm_engine_ini = engine.get_file ();

    int idx = 0;

    TVINSERTSTRUCT ins;
    ins.hParent = NULL;
    ins.hInsertAfter = TVI_ROOT;
    ins.item.pszText = L"Configuration Files";
    ins.item.mask = TVIF_TEXT;

    HTREEITEM hRoot = TreeView_InsertItem (hWndDataSelect, &ins);

    ins.hParent = hRoot;
    ins.hInsertAfter = TVI_ROOT;
    ins.item.pszText = L"BmEngine.ini";
    ins.item.mask = TVIF_TEXT;

    HTREEITEM hEngine = TreeView_InsertItem (hWndDataSelect, &ins);

    for (std::map <std::wstring, INI::File::Section>::const_iterator it = bm_engine_ini->get_sections ().begin ();
    it != bm_engine_ini->get_sections ().end ();
      ++it) {

#if 1
      ins.hParent = hEngine;
      ins.hInsertAfter = TVI_LAST;
      ins.item.pszText = (wchar_t *)(it->first.c_str ());
      ins.item.mask = TVIF_TEXT;

      HTREEITEM hItem = TreeView_InsertItem (hWndDataSelect, &ins);

      for (int i = 0; i < num_def_secs; i++) {
        if (lstrcmpW (ins.item.pszText, default_sections [i]) == 0) {
          TreeView_SetCheckState (hWndDataSelect, hItem, true);
          break;
        }
      }
#endif
    }

    TreeView_Expand (hWndDataSelect, hRoot, TVE_EXPAND);

    ins.hParent = hRoot;
    ins.hInsertAfter = TVI_ROOT;
    ins.item.pszText = L"BmSystemSettings.ini";
    ins.item.mask = TVIF_TEXT;

    HTREEITEM hSystem = TreeView_InsertItem (hWndDataSelect, &ins);

    INI::File* bm_system_ini = settings.get_file ();

    for (std::map <std::wstring, INI::File::Section>::const_iterator it = bm_system_ini->get_sections ().begin ();
    it != bm_system_ini->get_sections ().end ();
      ++it) {

#if 1
      ins.hParent = hSystem;
      ins.hInsertAfter = TVI_LAST;
      ins.item.pszText = (wchar_t *)(it->first.c_str ());
      ins.item.mask = TVIF_TEXT;

      HTREEITEM hItem = TreeView_InsertItem (hWndDataSelect, &ins);

      for (int i = 0; i < num_def_secs; i++) {
        if (lstrcmpW (ins.item.pszText, default_sections [i]) == 0) {
          TreeView_SetCheckState (hWndDataSelect, hItem, true);
          break;
        }
      }
#endif
    }

  } break;

  case WM_COMMAND:
  {
    switch (LOWORD (wParam))
    {
    case IDOK:
      EndDialog (hDlg, TRUE);
      break;
    }
  }
  }

  return (INT_PTR)FALSE;
}