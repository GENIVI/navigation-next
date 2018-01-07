/*****************************************************************************
 *
 *  tsapi.h - A convenience file for including the full WTLE public API.
 *
 *  Copyright (C) 2002-2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TS_API_H
#define TS_API_H

/* Base Services */
#include "tsbase.h"     /* Base layer public API */

/* Font Services */
#include "tsfnteng.h"   /* TsFontEngine */
#include "tsfe_mgr.h"   /* TsFontEngineMgr */
#include "tsfont.h"     /* TsFont */
#include "tsfntref.h"   /* TsFontSpec */
#include "tsfntmgr.h"   /* TsFontMgr */
#include "tsfontst.h"   /* TsFontStyle */
#include "tsfs_mgr.h"   /* TsFontStyleMgr */

/* Display */
#include "tsdc.h"       /* Device context */
#include "tsimgbuf.h"   /* Simple image buffer */
#include "tsimgbdc.h"   /* Image buffer device context */
#include "tsaimgbuf.h"  /* Alpha image buffer */
#include "tsaimgbdc.h"  /* Alpha image buffer device context */

/* Layout */
#include "tstext.h"     /* Rich text */
#include "tstextat.h"   /* Text attributes */
#include "tstextcn.h"   /* Text containers */
#include "tslayout.h"   /* TsLayout */
#include "tslayctl.h"   /* TsLayoutControl */
#include "tslayopt.h"   /* TsLayoutOptions */

/* Unicode */
#include "tsstring.h"   /* TsString */
#include "tsunienc.h"   /* Unicode encoding/decoding functions */
#include "tsunicde.h"   /* Unicode character property functions */

#endif /* TS_API_H */


