/*
 * PROJECT:    PAINT for ReactOS
 * LICENSE:    LGPL-2.0-or-later (https://spdx.org/licenses/LGPL-2.0-or-later)
 * PURPOSE:    Initializing everything
 * COPYRIGHT:  Copyright 2015 Benedikt Freisen <b.freisen@gmx.net>
 */

#include "precomp.h"

#include <mapi.h>
#include <mapicode.h>

POINT g_ptStart, g_ptEnd;
BOOL g_askBeforeEnlarging = FALSE;  // TODO: initialize from registry
HINSTANCE g_hinstExe = NULL;
TCHAR g_szFileName[MAX_LONG_PATH] = { 0 };
WCHAR g_szMailTempFile[MAX_LONG_PATH] = { 0 };
BOOL g_isAFile = FALSE;
BOOL g_imageSaved = FALSE;
BOOL g_showGrid = FALSE;

CMainWindow mainWindow;

/* FUNCTIONS ********************************************************/

// get file name extension from filter string
static BOOL
FileExtFromFilter(LPTSTR pExt, OPENFILENAME *pOFN)
{
    LPTSTR pchExt = pExt;
    *pchExt = 0;

    DWORD nIndex = 1;
    for (LPCTSTR pch = pOFN->lpstrFilter; *pch; ++nIndex)
    {
        pch += lstrlen(pch) + 1;
        if (pOFN->nFilterIndex == nIndex)
        {
            for (++pch; *pch && *pch != _T(';'); ++pch)
            {
                *pchExt++ = *pch;
            }
            *pchExt = 0;
            CharLower(pExt);
            return TRUE;
        }
        pch += lstrlen(pch) + 1;
    }
    return FALSE;
}

// Hook procedure for OPENFILENAME to change the file name extension
static UINT_PTR APIENTRY
OFNHookProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hParent;
    OFNOTIFY *pon;
    switch (uMsg)
    {
    case WM_NOTIFY:
        pon = (OFNOTIFY *)lParam;
        if (pon->hdr.code == CDN_TYPECHANGE)
        {
            hParent = GetParent(hwnd);
            TCHAR Path[MAX_PATH];
            SendMessage(hParent, CDM_GETFILEPATH, _countof(Path), (LPARAM)Path);
            FileExtFromFilter(PathFindExtension(Path), pon->lpOFN);
            SendMessage(hParent, CDM_SETCONTROLTEXT, 0x047c, (LPARAM)PathFindFileName(Path));
            lstrcpyn(pon->lpOFN->lpstrFile, Path, pon->lpOFN->nMaxFile);
        }
        break;
    }
    return 0;
}

typedef ULONG (WINAPI *FN_MAPISendMail)(LHANDLE, ULONG_PTR, lpMapiMessage, FLAGS, ULONG);
typedef ULONG (WINAPI *FN_MAPISendMailW)(LHANDLE, ULONG_PTR, lpMapiMessageW, FLAGS, ULONG);

BOOL OpenMailer(HWND hWnd, LPCWSTR pszPathName)
{
    // Delete the temporary file if any
    if (g_szMailTempFile[0])
    {
        ::DeleteFileW(g_szMailTempFile);
        g_szMailTempFile[0] = UNICODE_NULL;
    }

    CStringW strFileTitle;
    if (PathFileExistsW(pszPathName) && imageModel.IsImageSaved())
    {
        strFileTitle = PathFindFileNameW(pszPathName);
    }
    else // Not existing or not saved
    {
        // Get the name of a temporary file
        WCHAR szTempDir[MAX_PATH];
        ::GetTempPathW(_countof(szTempDir), szTempDir);
        if (!::GetTempFileNameW(szTempDir, L"afx", 0, g_szMailTempFile))
            return FALSE; // Failure

        if (PathFileExistsW(g_szFileName))
        {
            // Set file title
            strFileTitle = PathFindFileNameW(g_szFileName);

            // Copy to the temporary file
            if (!::CopyFileW(g_szFileName, g_szMailTempFile, FALSE))
            {
                g_szMailTempFile[0] = UNICODE_NULL;
                return FALSE; // Failure
            }
        }
        else
        {
            // Set file title
            strFileTitle.LoadString(IDS_DEFAULTFILENAME);
            strFileTitle += L".png";

            // Save it to the temporary file
            HBITMAP hbmLocked = imageModel.LockBitmap();
            BOOL ret = SaveDIBToFile(hbmLocked, g_szMailTempFile, FALSE, Gdiplus::ImageFormatPNG);
            imageModel.UnlockBitmap(hbmLocked);
            if (!ret)
            {
                g_szMailTempFile[0] = UNICODE_NULL;
                return FALSE; // Failure
            }
        }

        // Use the temporary file 
        pszPathName = g_szMailTempFile;
    }

    // Load "mapi32.dll"
    HINSTANCE hMAPI = LoadLibraryW(L"mapi32.dll");
    if (!hMAPI)
        return FALSE; // Failure

    // Attachment
    MapiFileDescW attachmentW = { 0 };
    attachmentW.nPosition = (ULONG)-1;
    attachmentW.lpszPathName = (LPWSTR)pszPathName;
    attachmentW.lpszFileName = (LPWSTR)(LPCWSTR)strFileTitle;

    // Message with attachment
    MapiMessageW messageW = { 0 };
    messageW.lpszSubject = NULL;
    messageW.nFileCount = 1;
    messageW.lpFiles = &attachmentW;

    // First, try to open the mailer by the function of Unicode version
    FN_MAPISendMailW pMAPISendMailW = (FN_MAPISendMailW)::GetProcAddress(hMAPI, "MAPISendMailW");
    if (pMAPISendMailW)
    {
        pMAPISendMailW(0, (ULONG_PTR)hWnd, &messageW, MAPI_DIALOG | MAPI_LOGON_UI, 0);
        ::FreeLibrary(hMAPI);
        return TRUE; // MAPISendMailW will show an error message on failure
    }

    // Convert to ANSI strings
    CStringA szPathNameA(pszPathName), szFileTitleA(strFileTitle);

    MapiFileDesc attachment = { 0 };
    attachment.nPosition = (ULONG)-1;
    attachment.lpszPathName = (LPSTR)(LPCSTR)szPathNameA;
    attachment.lpszFileName = (LPSTR)(LPCSTR)szFileTitleA;

    MapiMessage message = { 0 };
    message.lpszSubject = NULL;
    message.nFileCount = 1;
    message.lpFiles = &attachment;

    // Try again but in ANSI version
    FN_MAPISendMail pMAPISendMail = (FN_MAPISendMail)::GetProcAddress(hMAPI, "MAPISendMail");
    if (pMAPISendMail)
    {
        pMAPISendMail(0, (ULONG_PTR)hWnd, &message, MAPI_DIALOG | MAPI_LOGON_UI, 0);
        ::FreeLibrary(hMAPI);
        return TRUE; // MAPISendMail will show an error message on failure
    }

    ::FreeLibrary(hMAPI);
    return FALSE; // Failure
}

BOOL CMainWindow::GetOpenFileName(IN OUT LPTSTR pszFile, INT cchMaxFile)
{
    static OPENFILENAME ofn = { 0 };
    static CString strFilter;

    if (ofn.lStructSize == 0)
    {
        // The "All Files" item text
        CString strAllPictureFiles;
        strAllPictureFiles.LoadString(g_hinstExe, IDS_ALLPICTUREFILES);

        // Get the import filter
        CSimpleArray<GUID> aguidFileTypesI;
        CImage::GetImporterFilterString(strFilter, aguidFileTypesI, strAllPictureFiles,
                                        CImage::excludeDefaultLoad, _T('\0'));

        // Initializing the OPENFILENAME structure for GetOpenFileName
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner   = m_hWnd;
        ofn.hInstance   = g_hinstExe;
        ofn.lpstrFilter = strFilter;
        ofn.Flags       = OFN_EXPLORER | OFN_HIDEREADONLY;
        ofn.lpstrDefExt = L"png";
    }

    ofn.lpstrFile = pszFile;
    ofn.nMaxFile  = cchMaxFile;
    return ::GetOpenFileName(&ofn);
}

BOOL CMainWindow::GetSaveFileName(IN OUT LPTSTR pszFile, INT cchMaxFile)
{
    static OPENFILENAME sfn = { 0 };
    static CString strFilter;

    if (sfn.lStructSize == 0)
    {
        // Get the export filter
        CSimpleArray<GUID> aguidFileTypesE;
        CImage::GetExporterFilterString(strFilter, aguidFileTypesE, NULL,
                                        CImage::excludeDefaultSave, _T('\0'));

        // Initializing the OPENFILENAME structure for GetSaveFileName
        ZeroMemory(&sfn, sizeof(sfn));
        sfn.lStructSize = sizeof(sfn);
        sfn.hwndOwner   = m_hWnd;
        sfn.hInstance   = g_hinstExe;
        sfn.lpstrFilter = strFilter;
        sfn.Flags       = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_ENABLEHOOK;
        sfn.lpfnHook    = OFNHookProc;
        sfn.lpstrDefExt = L"png";

        LPWSTR pchDotExt = PathFindExtensionW(pszFile);
        if (*pchDotExt == UNICODE_NULL)
        {
            // Choose PNG
            wcscat(pszFile, L".png");
            for (INT i = 0; i < aguidFileTypesE.GetSize(); ++i)
            {
                if (aguidFileTypesE[i] == Gdiplus::ImageFormatPNG)
                {
                    sfn.nFilterIndex = i + 1;
                    break;
                }
            }
        }
    }

    sfn.lpstrFile = pszFile;
    sfn.nMaxFile  = cchMaxFile;
    return ::GetSaveFileName(&sfn);
}

BOOL CMainWindow::ChooseColor(IN OUT COLORREF *prgbColor)
{
    static CHOOSECOLOR choosecolor = { 0 };
    static COLORREF custColors[16] =
    {
        0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff,
        0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff
    };

    if (choosecolor.lStructSize == 0)
    {
        // Initializing the CHOOSECOLOR structure for ChooseColor
        ZeroMemory(&choosecolor, sizeof(choosecolor));
        choosecolor.lStructSize  = sizeof(choosecolor);
        choosecolor.hwndOwner    = m_hWnd;
        choosecolor.lpCustColors = custColors;
    }

    choosecolor.Flags = CC_RGBINIT;
    choosecolor.rgbResult = *prgbColor;
    if (!::ChooseColor(&choosecolor))
        return FALSE;

    *prgbColor = choosecolor.rgbResult;
    return TRUE;
}

HWND CMainWindow::DoCreate()
{
    ::LoadString(g_hinstExe, IDS_DEFAULTFILENAME, g_szFileName, _countof(g_szFileName));

    CString strTitle;
    strTitle.Format(IDS_WINDOWTITLE, PathFindFileName(g_szFileName));

    RECT& rc = registrySettings.WindowPlacement.rcNormalPosition;
    return Create(HWND_DESKTOP, rc, strTitle, WS_OVERLAPPEDWINDOW, WS_EX_ACCEPTFILES);
}

// entry point
INT WINAPI
_tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, INT nCmdShow)
{
    g_hinstExe = hInstance;

    // Initialize common controls library
    INITCOMMONCONTROLSEX iccx;
    iccx.dwSize = sizeof(iccx);
    iccx.dwICC = ICC_STANDARD_CLASSES | ICC_USEREX_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&iccx);

    // Load settings from registry
    registrySettings.Load(nCmdShow);

    // Create the main window
    if (!mainWindow.DoCreate())
    {
        MessageBox(NULL, TEXT("Failed to create main window."), NULL, MB_ICONERROR);
        return 1;
    }

    // Initialize imageModel
    if (__argc < 2 || !DoLoadImageFile(mainWindow, __targv[1], TRUE))
        InitializeImage(NULL, NULL, FALSE);

    // Make the window visible on the screen
    mainWindow.ShowWindow(registrySettings.WindowPlacement.showCmd);

    // Load the access keys
    HACCEL hAccel = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(800));

    // The message loop
    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0))
    {
        if (fontsDialog.IsWindow() && fontsDialog.IsDialogMessage(&msg))
            continue;

        if (::TranslateAccelerator(mainWindow, hAccel, &msg))
            continue;

        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    // Unload the access keys
    ::DestroyAcceleratorTable(hAccel);

    // Write back settings to registry
    registrySettings.Store();

    if (g_szMailTempFile[0])
        ::DeleteFileW(g_szMailTempFile);

    // Return the value that PostQuitMessage() gave
    return (INT)msg.wParam;
}
