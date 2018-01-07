/*****************************************************************************
 *
 *  tsfont.h - Interface to TsFont class.
 *
 *  Copyright (C) 2003-2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSFONT_H
#define TSFONT_H

#include "tstextat.h"
#include "tsfontst.h"
#include "tsport.h"
#include "tstypes.h"
#include "tstag.h"
#include "tsfntref.h"
#ifndef TS_NO_OPENTYPE
#include "otlayout.h"
#endif
#include "tseudata.h"


TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  Enumerated list of font design metrics.
 *
 *  Description:
 *      These metrics are used as arguments to TsFont_getBoolMetric
 *      which returns boolean font design metrics.
 *
 *  <GROUP font>
 */
typedef enum
{
    TS_FONT_DESIGN_IS_BOLD,
    TS_FONT_DESIGN_IS_ITALIC,
    TS_FONT_DESIGN_IS_MONOSPACE
} TsFontDesignMetric;


/*****************************************************************************
 *
 *  The font virtual function table.
 *
 *  Description:
 *      This structure provides the "virtual function" pointers that are
 *      necessary for the "abstract class" TsFont to provide font
 *      services.  This structure must be properly initialized and passed
 *      to the TsFont constructor via TsFont_new or TsFont_newex.
 *
 *  <GROUP font>
 */
typedef struct TsFontFuncs_
{
    void (*deleteIt)(void *data); /* delete the derived class */
    TsSize (*getName)(void *data, TsSize, TsChar *); /* get the font name */
    TsSize (*getFamilyName)(void *data, TsSize, TsChar *); /* get the font family name */
    TsSize (*getNumEmbeddedFonts)(void *data);  /* get the number of actual fonts within the font file */
    void *(*getTable)(void *data, TsTag tag); /* get a reference to a font table */
    TsResult (*releaseTable)(void *data, void *table); /* release a reference to a font table */
    TsResult (*mapChar)(void *data, TsInt32 charID, TsInt32 *glyphID); /* map character to glyph index */
    TsFontStyle *(*createFontStyle)(void *data, TsFontParam *, TsFont *font); /* construct a font style */
    TsBool (*getBoolMetric)(void *data, TsFontDesignMetric); /* get boolean design metric */
} TsFontFuncs;


/******************************************************************************
 *
 *  Creates and initializes a new TsFont object.
 *
 *  Parameters:
 *      funcs   - [in] a properly initialized TsFontFuncs structure.
 *      data    - [in] the derived class' private data.  This void pointer
 *                     will be passed as the first argument to each of the
 *                     functions defined in TsFontFuncs.
 *
 *  Return value:
 *      Pointer to TsFont object, or NULL on error
 *
 *  <GROUP font>
 */
TS_EXPORT(TsFont *)
TsFont_new(const TsFontFuncs *funcs, void *data);


/******************************************************************************
 *
 *  Creates and initializes a new memory managed TsFont object.
 *
 *  Parameters:
 *      memMgr  - [in] pointer to a memory manager object or NULL
 *      funcs   - [in] a properly initialized TsFontFuncs structure.
 *      data    - [in] the derived class' private data.  This void pointer
 *                     will be passed as the first argument to each of the
 *                     functions defined in TsFontFuncs.
 *
 *  Return value:
 *      Pointer to TsFont object, or NULL on error
 *
 *  <GROUP font>
 */
TS_EXPORT(TsFont *)
TsFont_newex(TsMemMgr *memMgr, const TsFontFuncs *funcs, void *data);


/******************************************************************************
 *
 *  Deletes a TsFont object.
 *
 *  Parameters:
 *      font    - [in] pointer to TsFont object.
 *
 *  Return value:
 *      void
 *
 *  <GROUP font>
 */
TS_EXPORT(void)
TsFont_delete(TsFont *font);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsFont object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsFont_releaseHandle.
 *
 *  Parameters:
 *      font   - [in] pointer to TsFont object
 *
 *  Return value:
 *      Pointer to font object.
 *
 *  <GROUP font>
 */
TS_EXPORT(TsFont *)
TsFont_copyHandle(TsFont *font);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsFont object.
 *
 *  Description:
 *      This function reverses the action of TsFont_copyHandle.
 *
 *  Parameters:
 *      font   - [in] pointer to TsFont object
 *
 *  Return value:
 *      void
 *
 *  <GROUP font>
 */
TS_EXPORT(void)
TsFont_releaseHandle(TsFont *font);


/*****************************************************************************
 *
 *  Get the string length of the name of this font.
 *
 *  Description:
 *      This function returns the size of the buffer needed for the name
 *      of the font.
 *
 *  Parameters:
 *      font     - [in] pointer to TsFont object
 *
 *  Return value:
 *      Length of buffer required for the font name
 *
 *  <GROUP font>
 */
TS_EXPORT(TsSize)
TsFont_getNameLength(TsFont *font);


/*****************************************************************************
 *
 *  Get the name of this font.
 *
 *  Description:
 *      This function takes a buffer and buffer length as parameters and
 *      will return with the buffer set to the font name.
 *
 *  Parameters:
 *      font     - [in] pointer to TsFont object
 *      capacity - [in] length of buffer in bytes
 *      buffer   - [in] pointer to array of TsChar of capacity length
 *
 *  Return value:
 *      Length of buffer filled with name in bytes
 *
 *  <GROUP font>
 */
TS_EXPORT(TsSize)
TsFont_getName(TsFont *font, TsSize capacity, TsChar *buffer);


/*****************************************************************************
 *
 *  Get the string length of the family name of this font.
 *
 *  Description:
 *      This function returns the size of the buffer needed for the family
 *      name of the font.
 *
 *  Parameters:
 *      font     - [in] pointer to TsFont object
 *
 *  Return value:
 *      Length of buffer required for the font family name
 *
 *  <GROUP font>
 */
TS_EXPORT(TsSize)
TsFont_getFamilyNameLength(TsFont *font);


/*****************************************************************************
 *
 *  Get the family name of this font.
 *
 *  Description:
 *      This function takes a buffer and buffer length as parameters and
 *      will return with the buffer set to the font family name.
 *
 *  Parameters:
 *      font     - [in] pointer to TsFont object
 *      capacity - [in] length of buffer in bytes
 *      buffer   - [in] pointer to array of TsChar of capacity length
 *
 *  Return value:
 *      Length of buffer filled with the family name in bytes
 *
 *  <GROUP font>
 */
TS_EXPORT(TsSize)
TsFont_getFamilyName(TsFont *font, TsSize capacity, TsChar *buffer);


/*****************************************************************************
 *
 *  Get the number of fonts embedded within this multi-font font.
 *
 *  Parameters:
 *      font     - [in] pointer to TsFont object
 *
 *  Return value:
 *      Number of embedded fonts
 *
 *  <GROUP font>
 */
TS_EXPORT(TsSize)
TsFont_getNumEmbeddedFonts(TsFont *font);


/*****************************************************************************
 *
 *  Create a font style from this font.
 *
 *  Description:
 *      This function creates a TsFontStyle object from this font using
 *      the parameters in the TsFontParam object provided.
 *
 *  Parameters:
 *      font   - [in] pointer to TsFont object
 *      param  - [in] pointer to TsFontParam object
 *
 *  Return value:
 *      TsFontStyle pointer
 *
 *  <GROUP font>
 */
TS_EXPORT(TsFontStyle *)
TsFont_createFontStyle(TsFont *font, TsFontParam *param);


#ifndef TS_NO_OPENTYPE

/******************************************************************************
 *
 *  Finds and returns the OpenType table layout object associated with
 *  the script and language system, if present, for the TsFont.
 *
 *  Parameters:
 *      font    - [in] pointer to TsFont object
 *      script  - [in] script
 *      langSys - [in] language system
 *
 *  Return value:
 *      TsOtLayout pointer
 *
 *  <GROUP font>
 */
TS_EXPORT(TsOtLayout *)
TsFont_findOtLayout(TsFont *font, TsTag script, TsTag langSys);

#endif /* TS_NO_OPENTYPE */

/******************************************************************************
 *
 *  Gets a pointer to a memory-mapped table.
 *
 *  Description:
 *      The table obtained using this function must be released using
 *      TsFont_releaseTable. This must be done before releasing the handle
 *      to the font.
 *
 *  Parameters:
 *      font    - [in] pointer to TsFont object
 *      tag     - [in] 4-byte table tag
 *
 *  Return value:
 *      Pointer to table.
 *
 *  <GROUP font>
 */
TS_EXPORT(void *)
TsFont_getTable(TsFont *font, TsTag tag);


/******************************************************************************
 *
 *  Releases a pointer to a memory-mapped table.
 *
 *  Parameters:
 *      font    - [in] pointer to TsFont object
 *      table   - [in] pointer to table
 *
 *  Return value:
 *      TS_OK if released OK.  TS_ERR if table could not be found or released.
 *
 *  <GROUP font>
 */
TS_EXPORT(TsResult)
TsFont_releaseTable(TsFont *font, void *table);


/******************************************************************************
 *
 *  Maps a character through the font cmap to produce a glyphID.
 *
 *  Note:
 *      Currently we assume the Unicode cmap.
 *
 *  Parameters:
 *      font    - [in] pointer to TsFont object
 *      charID  - [in] character value.
 *      glyphID - [out] function returns the glyphID here.
 *
 *  Return value:
 *      TS_OK on success, else error code.
 *
 *  <GROUP font>
 */
TS_EXPORT(TsResult)
TsFont_mapChar(TsFont *font, TsInt32 charID, TsInt32 *glyphID);


/******************************************************************************
 *
 *  Returns a boolean metric that is determined based on the design of the font.
 *
 *  Description:
 *      The metric argument may be:
 *
 *      <TABLE>
 *      <B>metric value                             <B>Returns TRUE if
 *      TS_FONT_DESIGN_IS_BOLD                      font was designed emboldened
 *      TS_FONT_DESIGN_IS_ITALIC                    font was designed as italic
 *      TS_FONT_DESIGN_IS_MONOSPACE                 font was designed to be monospace
 *      </TABLE>
 *
 *  Parameters:
 *      font    - [in] pointer to TsFont object
 *      metric  - [in] type of metric being requested
 *
 *  Return value:
 *      TsBool if the font was designed with the requested metric
 *
 *  <GROUP font>
 */
TS_EXPORT(TsBool)
TsFont_getBoolMetric(TsFont *font, TsFontDesignMetric metric);


/*****************************************************************************
 *
 *  Returns a pointer to an object which contains external data managed by
 *  the user.
 *
 *  Description:
 *      This function returns a TsExternalUserData pointer which allows
 *      the user to access external data that was set by a call to
 *      TsExternalUserData_init.
 *
 *      TS_USE_EXTERNAL_USERDATA must be defined in order to use the
 *      private data mechanism. If not defined this function returns NULL.
 *
 *      The TsExternalUserData object that is returned by this function
 *      becomes invalid when this TsFont object is destroyed.
 *      Therefore, you must keep reference to the TsFont object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      font        - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP font>
 */
TS_EXPORT(TsExternalUserData *)
TsFont_getExternalUserdata(TsFont *font);


TS_END_HEADER

#endif /* TSFONT_H */
