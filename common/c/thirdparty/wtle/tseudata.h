/*****************************************************************************
 *
 *  tseudata.h  - Defines interface for external data access in WTLE objects
 *
 *  Description:
 *      The external user data mechanism allows addition of external data
 *      to every WTLE object. To enable the mechanism the integrator must:
 *      - Define TS_USE_EXTERNAL_USERDATA within tsport.h.
 *      - Set the value of TS_EXTERNAL_USERDATA_SIZE within tsport.h.
 *      - Implement port-specific tseudata.c file.
 *
 *      Several operations are wrapped in macros to avoid having to put
 *      #ifdef's around them in each file. If TS_USE_EXTERNAL_USERDATA is
 *      not defined then these macros map to no-ops.
 *
 *  Copyright (C) 2007, 2010 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSEUDATA_H
#define TSEUDATA_H

#include "tsproj.h"     /* basic project includes and definitions */

TS_BEGIN_HEADER

#ifndef TS_USE_EXTERNAL_USERDATA

typedef void * TsExternalUserData;          /* so that _getExternalUserData functions can return NULL */

#define TS_SYNCHRONIZE_EXTERNAL_USERDATA_REFCOUNT(p)    /* do nothing */
#define TS_DELETE_EXTERNAL_USERDATA(p)                  /* do nothing */
#define TS_RETURN_EXTERNAL_USERDATA(p)  TS_UNUSED_PARAMETER(p); return(NULL)


#else


/*****************************************************************************
 *
 *  An enumeration of IDs for the major WTLE objects for which external user
 *  data functionality is available.
 *
 *  <GROUP externaluserdata>
 */
typedef enum
{
    TsObjectId_TsText = 1,          /* identifier for a TsText object */
    TsObjectId_TsLayout,            /* identifier for a TsLayout object */
    TsObjectId_TsLayoutControl,     /* identifier for a TsLayoutControl object */
    TsObjectId_TsLayoutOptions,     /* identifier for a TsLayoutOptions object */
    TsObjectId_TsTextContainer,     /* identifier for a TsTextContainer object */
    TsObjectId_TsFontEngine,        /* identifier for a TsFontEngine object */
    TsObjectId_TsFontEngineMgr,     /* identifier for a TsFontEngineMgr object */
    TsObjectId_TsFont,              /* identifier for a TsFont object */
    TsObjectId_TsFontMgr,           /* identifier for a TsFontMgr object */
    TsObjectId_TsFontStyle,         /* identifier for a TsFontStyle object */
    TsObjectId_TsFontStyleMgr,      /* identifier for a TsFontStyleMgr object */
    TsObjectId_TsPolygon,           /* identifier for a TsPolygon object */
    TsObjectId_TsMemMgr,            /* identifier for a TsMemMgr object */
    TsObjectId_TsShaper,            /* identifier for a TsShaper object */
    TsObjectId_TsShaperFont         /* identifier for a TsShaperFont object */
} TsObjectId;

typedef struct TsExternalUserData_ TsExternalUserData;


/*****************************************************************************
 *
 *  A structure to contain the user's external data.
 *
 *  <GROUP externaluserdata>
 */
struct TsExternalUserData_
{
    TsUInt8 data[TS_EXTERNAL_USERDATA_SIZE]; /* customer specific data area, TS_EXTERNAL_USERDATA_SIZE is defined in tsport.h */
    void (*deleteIt)(TsExternalUserData*);   /* function to free the user data. This is initialized to NULL in the object's constructor */
    TsRefCount *pRefCount;                   /* pointer to object reference counter */
};


/*****************************************************************************
 *
 *  A macro to synchronize the private user data refCount with an
 *  object's refCount.

 *  Notes:
 *     This macro evaluates the p argument more than once so the
 *     argument should never be an expression with side effects.
 *
 *  <GROUP externaluserdata>
 */
#define TS_SYNCHRONIZE_EXTERNAL_USERDATA_REFCOUNT(p)  /*lint -e717 */ do \
            {  \
                if ((p)->externalUserData.pRefCount != NULL) \
                { \
                    *((p)->externalUserData.pRefCount) = (p)->refCount; \
                } \
            } while (FALSE)


/*****************************************************************************
 *
 *  A macro to call external user data deleteIt functions.
 *
 *  Notes:
 *     This macro evaluates the p argument more than once so the
 *     argument should never be an expression with side effects.
 *
 *  <GROUP externaluserdata>
 */
#define TS_DELETE_EXTERNAL_USERDATA(p)  /*lint -e717 */ do \
            {  \
                if ((p)->externalUserData.deleteIt != NULL) \
                { \
                    (p)->externalUserData.deleteIt(&((p)->externalUserData)); \
                } \
            } while (FALSE)


/*****************************************************************************
 *
 *  A macro to get external user data.
 *
 *  Notes:
 *     This macro evaluates the p argument more than once so the
 *     argument should never be an expression with side effects.
 *
 *  <GROUP externaluserdata>
 */
#define TS_RETURN_EXTERNAL_USERDATA(p) /*lint -e717 */ do \
            { \
                if ((p) != NULL) \
                { \
                    return(&((p)->externalUserData)); \
                } \
                else \
                { \
                    return(NULL); \
                } \
            } while (FALSE)




/*****************************************************************************
 *
 *  Set external user data into a WTLE object
 *
 *  Description:
 *      This function is implemented by the client. In it, code has access to
 *      both the WTLE types and client specific types. The function should
 *      have appropriate handling for each of the WTLE types for which
 *      external data usage has been enabled. See tseudata.c in the port
 *      directory for an example implementation.
 *
 *  Parameters:
 *      id      - [in] a TsObjectId that indicating the type of WTLE object
 *                     that parameter obj points to
 *      obj     - [in] pointer to the WTLE object that external data is being
 *                     set into
 *      data    - [in] pointer to structure containing user's data
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP externaluserdata>
 */
TS_EXPORT(TsResult)
TsExternalUserData_init(TsObjectId id, void* obj, TsExternalUserData* data);


#endif /* TS_EXTERNAL_USERDATA */

TS_END_HEADER


#endif /* TSEUDATA_H */
