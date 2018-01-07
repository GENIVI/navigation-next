/*****************************************************************************
 *
 *  tssystem.h
 *
 *  Defines system information such as product name and version.
 *
 *  Copyright (C) 2002-2010 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TS_SYSTEM_H
#define TS_SYSTEM_H

#include "tstypes.h"


TS_BEGIN_HEADER

/*****************************************************************************
 *
 *  Macro that specifies company name.
 *  This string must be retained in the code.
 *  Do not modify the declaration given below.
 *
 */
#define TS_COMPANY_NAME "Monotype Imaging Inc."


/*****************************************************************************
 *
 *  Macro that specifies the product name for the WorldType Layout Engine.
 *  This string must be retained in the code.
 *  Do not modify the declaration given below.
 *
 */
#define TS_PRODUCT_NAME "WorldType Layout Engine"


/*****************************************************************************
 *
 *  Macro that specifies the product name for the WorldType Shaper.
 *  This string must be retained in the code.
 *  Do not modify the declaration given below.
 *
 */
#define TS_SHAPER_PRODUCT_NAME "WorldType Shaper"


/*****************************************************************************
 *
 *  Macro that specifies the copyright string.
 *  This string must be retained in the code.
 *  Do not modify the declaration given below.
 *
 */
#define TS_COPYRIGHT_STRING "Copyright (C) 2002-2010 Monotype Imaging Inc. All rights reserved."


/*****************************************************************************
 *
 *  Build configuration option that specifies a code version string.
 *  Defaults to the current release.  May be redefined in order to
 *  specify a particular working code revision, but in most cases this
 *  should be left as is.  If redefined then it must be as a string constant.
 *
 *  <GROUP sysinfo>
 */
#ifndef TS_VERSION_STRING
#define TS_VERSION_STRING "3.3.0"
#endif


/*****************************************************************************
 *
 *  Macro that returns the company name string.
 *
 *  <GROUP sysinfo>
 */
#define TsSystem_getCompanyName() (TS_COMPANY_NAME)


/*****************************************************************************
 *
 *  Macro that returns the product name string for the WorldType Layout Engine.
 *
 *  <GROUP sysinfo>
 */
#define TsSystem_getProductName() (TS_PRODUCT_NAME)


 /*****************************************************************************
 *
 *  Macro that returns the product name string for the WorldType Shaper.
 *
 *  <GROUP sysinfo>
 */
#define TsSystem_getShaperProductName() (TS_SHAPER_PRODUCT_NAME)

/*****************************************************************************
 *
 *  Macro that returns the copyright string.
 *
 *  <GROUP sysinfo>
 */
#define TsSystem_getCopyrightString() (TS_COPYRIGHT_STRING)


/*****************************************************************************
 *
 *  Macro that returns the product version string.
 *
 *  <GROUP sysinfo>
 */
#define TsSystem_getVersionString() (TS_VERSION_STRING)


/*****************************************************************************
 *
 *  Macro that returns the build configuration string.
 *
 *  <GROUP sysinfo>
 */
#define TsSystem_getConfigString() (TS_CONFIG_STRING)



TS_END_HEADER


#endif /* TS_SYSTEM_H */
