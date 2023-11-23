#ifndef _MSPAINT_H
#define _MSPAINT_H

#ifdef NDEBUG
    #undef DBG
    #undef _DEBUG
#endif

#include <windef.h>
#include <winbase.h>
#include <winuser.h>
#include <wingdi.h>
#include <tchar.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlpath.h>
#include <atlstr.h>
#include <atlwin.h>
#include <atltypes.h>
#include <windowsx.h>
#include <commdlg.h>
#include <commctrl.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES /* for M_PI */
#include <math.h>
#include <shellapi.h>
#include <htmlhelp.h>
#include <strsafe.h>
#include "atlimagedx.h"

#ifdef __RPAINT__
    #ifndef max
        #define max(a, b) (((a) > (b)) ? (a) : (b))
    #endif
    #ifndef min
        #define min(a, b) (((a) < (b)) ? (a) : (b))
    #endif
#endif

#include <debug.h>

#include "resource.h"
#include "common.h"
#include "drawing.h"
#include "dib.h"
#include "fullscreen.h"
#include "history.h"
#include "miniature.h"
#include "palette.h"
#include "palettemodel.h"
#include "registry.h"
#include "selectionmodel.h"
#include "sizebox.h"
#include "canvas.h"
#include "textedit.h"
#include "toolbox.h"
#include "toolsettings.h"
#include "toolsmodel.h"
#include "main.h"
#include "dialogs.h"
#include <ui/CWaitCursor.h>
#include "globalvar.h"

#endif /* _MSPAINT_H */
