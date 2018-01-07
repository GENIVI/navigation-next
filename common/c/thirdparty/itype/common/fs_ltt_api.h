/******************************************************************************
 *
 *  fs_ltt_api.h - Declares the public read access API for FsLtt structures.
 *
 *  Copyright (C) 2001-2008 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential information of Monotype Imaging Inc.
 *
 *****************************************************************************/


#ifndef FS_LTT_API_H
#define FS_LTT_API_H

#include "fs_err.h"
#include "fs_port.h"
#include "fs_object.h"



#ifndef FS_ERROR
#define FS_ERROR FS_LONG
#endif


#ifdef __cplusplus
extern "C" {
#endif



struct FsPPemAdjust_;

typedef struct FsPPemAdjust_ FsPPemAdjust;

struct FsLttComponent_;

typedef struct FsLttComponent_ FsLttComponent;

struct FsLtt_;

typedef struct FsLtt_ FsLtt;

/*****************************************************************************
 *
 *  Initializes the values of a new component.
 *
 *  Description:
 *      This function initializes the values in a new component.
 *      It should only be called for new components and should 
 *      not be used to reset a component.
 *      
 *  Parameters:
 *      lttc        - [in] pointer to destination component
 *
 *  Return Value:
 *      SUCCESS.
 *
 *  See Also:
 *      FsLtt_component(), FsLttComponent_done()
 *
 *  <GROUP FsLttComponentFunctions>
 */  
ITYPE_API FS_ERROR
FsLttComponent_init(FsLttComponent *lttc, FsLtt *ltt);

/*****************************************************************************
 *
 *  Frees the internal memory of a component.
 *
 *  Description:
 *      This function frees the internal memory used by
 *      a component. It does not free the component itself.
 *      Once this function is called, the component may be 
 *      freed or initialized again.
 *      
 *  Parameters:
 *      lttc        - [in] pointer to destination component
 *
 *  Return Value:
 *      SUCCESS.
 *
 *  See Also:
 *      FsLtt_component(), FsLttComponent_init()
 *
 *  <GROUP FsLttComponentFunctions>
 */  
ITYPE_API FS_ERROR
FsLttComponent_done(_DS_ FsLttComponent *lttc);

/*****************************************************************************
 *
 *  Queries component for pseudo-emboldening.
 *
 *  Description:
 *      This function queries a component to determine if
 *      pseudo-emboldening is enabled for the component and 
 *      if so, the amount of pseudo-emboldening.
 *      
 *  Parameters:
 *      lttc        - [in] pointer to destination component
 *      boldPercent - [out]bold percentage
 *
 *  Return Value:
 *      true (1) if pseudo-emboldening is enabled, or false (0) if
 *      pseudo-emboldening is not enabled. In either case, the bold
 *      percentage for the component is also returned in the argument.
 *
 *  See Also:
 *      FsLttComponent_setBold()
 *
 *  <GROUP FsLttComponentFunctions>
 */  
ITYPE_API FS_BOOLEAN
FsLttComponent_isBold(FsLttComponent *lttc, FS_FIXED *boldPercent);

/*****************************************************************************
 *
 *  Queries component for pseudo-italicizing.
 *
 *  Description:
 *      This function queries a component to determine if
 *      pseudo-italics is enabled for the component and 
 *      if so, the pseudo-italics angle.
 *      
 *  Parameters:
 *      lttc        - [in] pointer to destination component
 *      italicAngle - [out]italics angle in degrees
 *
 *  Return Value:
 *      true (1) if pseudo-italics is enabled, or false (0) if
 *      pseudo-italics is not enabled. In either case, the italics
 *      angle for the component is also returned in the argument.
 *
 *  See Also:
 *      FsLttComponent_setItalic()
 *
 *  <GROUP FsLttComponentFunctions>
 */  
ITYPE_API FS_BOOLEAN
FsLttComponent_isItalic(FsLttComponent *lttc, FS_FIXED *italicAngle);

/*****************************************************************************
 *
 *  Queries a component for number of adjustments.
 *
 *  Description:
 *      This function returns the number of adjustments that apply
 *      for the component.
 *      
 *  Parameters:
 *      lttc        - [in] pointer to destination component
 *
 *  Return Value:
 *      Number of adjustments set for the component.
 *
 *  See Also:
 *      FsLttComponent_getAdjustment(),
 *      FsLtt_autoAdjust()
 *
 *  <GROUP FsLttComponentFunctions>
 */  
ITYPE_API FS_USHORT
FsLttComponent_numAdjustments(FsLttComponent *lttc);

/*****************************************************************************
 *
 *  Retrieves the 'n'th adjustment for a component (via a pointer to FS_CONST).
 *
 *  Description:
 *      This function returns a pointer to the 'n'th FsPPemAdjust data
 *      structure for a component.   The data structure is typed as FS_CONST
 *      and should be treated as such.  Any changes to the structure
 *      are unsupported.  The data structure contains adjustments and
 *      the minimum ppem to which the adjustments apply.  The adjustments
 *      include the scale adjustment (8.8 fixed point) and the 
 *      integer shift amount.  The adjustments apply to all ppem values
 *      up to, but not including, the ppem value specified in the
 *      next ('n+1'th) adjustment.  The adjustments are retrieved in
 *      sorted order based on increasing ppem value.
 *      
 *  Parameters:
 *      lttc        - [in] pointer to destination component
 *      n           - [in] index of adjustment (counting from zero)
 *
 *  Return Value:
 *      Pointer to FS_CONST FsPPemAdjust data. If the adjustment does not
 *      exist, a NULL pointer is returned.  Note, the adjustments will not
 *      be loaded and a NULL will be returned if the corresponding boolean
 *      argument to FsLtt_read() or FsLtt_load() was FALSE when reading
 *      this linked font.
 *
 *  See Also:
 *      FsLttComponent_numAdjustments(), FsLtt_copy(),
 *      FsLttComponent_getAdjustmentAndRange(),
 *      FsLtt_read(), FsLtt_load(),
 *      FsLttComponent_setAdjustmentAndRange()
 *
 *  <GROUP FsLttComponentFunctions>
 */
ITYPE_API FS_CONST FsPPemAdjust *
FsLttComponent_getAdjustment(FsLttComponent *lttc, FS_USHORT n);


/*****************************************************************************
 *
 *  Gets adjustment values for a range of ppem sizes for a component.
 *
 *  Description:
 *      This function retrieves the scale and shift adjustments
 *      for a range of ppem sizes for a component.  The target ppem for which
 *      adjustments are retrieved is specified in the FsPPemAdjust structure
 *      when the function is called.  The ppem value in this structure may
 *      be changed to the range minimum upon return.  Scale adjustments
 *      are in 8.8 fixed point and shift adjustments are integer.
 *      The returned range is specified by the minimum ppem as indicated in the
 *      FsPPemAdjust structure and the maximum ppem as returned in the
 *      variable pointed to by the "maxppem" argument.  The maximum ppem
 *      is inclusive, that is, it indicates the maximum ppem to which the
 *      adjustments apply.  For example, the adjustments for a single ppem
 *      size will return with the maximum ppem equal to the (minimum) ppem
 *      as indicated in the FsPPemAdjust structure.  The maximum ppem will
 *      be equal to 0xFFFF if the adjustments for all ppem sizes
 *      equal to or greater than the indicated minimum ppem are the same.
 *      
 *  Parameters:
 *      lttc      - [in] pointer to component
 *      adj       - [in/out] target ppem/minimum ppem for range and adjustments
 *      maxppem   - [out] maximum ppem to which adjustments apply
 *
 *  Return Value:
 *      SUCCESS, or error code if the adjustments are not loaded.
 *      Note, the adjustments will not be loaded and an error code will
 *      be returned if the corresponding boolean
 *      argument to FsLtt_read() or FsLtt_load() was FALSE when reading
 *      this linked font.
 *
 *      <TABLE>
 *      Return values               Reason
 *      ------------------          --------------------------------
 *      ERR_INVALID_ADDRESS         Adjustments were not loaded.
 *      </TABLE>
 *
 *  See Also:
 *      FsLttComponent_numAdjustments(), FsLtt_copy(),
 *      FsLttComponent_getAdjustment(),
 *      FsLtt_read(), FsLtt_load(),
 *      FsLttComponent_setAdjustmentAndRange()
 *
 *  <GROUP FsLttComponentFunctions>
 */  
ITYPE_API FS_ERROR
FsLttComponent_getAdjustmentAndRange(FsLttComponent *lttc,
                                     FsPPemAdjust *adj, FS_USHORT *maxppem);


/*****************************************************************************
 *
 *  Creates a new linked font.
 *
 *  Description:
 *      This function allocates a new FsLtt and initializes it using FsLtt_init().
 *      System heap is used for the linked font. An iType state is required during
 *      the creation process and its properties are added to the FsLtt.
 *      
 *  Parameters:
 *      FS_state_ptr  - [in] iType state pointer
 *
 *  Return Value:
 *      Pointer to FsLtt structure, or NULL if the linked font cannot be created.
 *
 *  See Also:
 *      FsLtt_init(), FsLtt_done(), FsLtt_delete()
 *
 *  <GROUP FsLttFunctions>
 */  
ITYPE_API FsLtt *
FsLtt_new(_DS0_);

/*****************************************************************************
 *
 *  Initializes a linked font.
 *
 *  Description:
 *      This function is used during linked font creation to initialize
 *      the values within a linked font data structure (FsLtt). It should 
 *      only be called for empty FsLtt, otherwise a memory leak may occur.
 *      The function FsLtt_done() may be used to free internal memory 
 *      within a FsLtt. Following that operation, the linked font may
 *      be re-initialized using FsLtt_init().
 *      
 *  Parameters:
 *      FS_state_ptr  - [in] iType state pointer
 *      ltt           - [in] pointer to linked font data structure
 *
 *  Return Value:
 *      SUCCESS.
 *
 *  See Also:
 *      FsLtt_new(), FsLtt_done(), FsLtt_delete()
 *
 *  <GROUP FsLttFunctions>
 */  
ITYPE_API FS_ERROR
FsLtt_init(_DS_ FsLtt *ltt);

/*****************************************************************************
 *
 *  Frees the internal memory of a linked font.
 *
 *  Description:
 *      This function frees the internal memory of a linked font (FsLtt), 
 *      including all components and their internal memory. The linked
 *      font is not re-initialized.
 *      
 *      
 *  Parameters:
 *      ltt           - [in] pointer to linked font data structure
 *
 *  Return Value:
 *      None
 *
 *  See Also:
 *      FsLtt_new(), FsLtt_init(), FsLtt_delete()
 *
 *  <GROUP FsLttFunctions>
 */  
ITYPE_API FS_VOID
FsLtt_done(FsLtt *ltt);

/*****************************************************************************
 *
 *  Frees a linked font.
 *
 *  Description:
 *      This function calls FsLtt_done() to free all internal memory
 *      in a linked font and then frees the linked font itself.
 *      
 *  Parameters:
 *      ltt           - [in] pointer to linked font data structure
 *
 *  Return Value:
 *      None
 *
 *  See Also:
 *      FsLtt_new(), FsLtt_init(), FsLtt_done()
 *
 *  <GROUP FsLttFunctions>
 */  
ITYPE_API FS_VOID
FsLtt_delete(FsLtt *ltt);

/*****************************************************************************
 *
 *  Retrieve the family name from a linked font.
 *
 *  Description:
 *      This function returns a string pointer to the family name of
 *      a linked font. This string should not be freed by the caller
 *      since it points to the internal family name of the linked font.
 *      
 *  Parameters:
 *      ltt           - [in] pointer to linked font data structure
 *
 *  Return Value:
 *      FILECHAR pointer to FsLtt family name. A NULL pointer indicates
 *      that the name has not been set.
 *
 *  See Also:
 *      FsLtt_setName()
 *
 *  <GROUP FsLttFunctions>
 */  
ITYPE_API FS_CONST FILECHAR *
FsLtt_getName(FsLtt *ltt);

/*****************************************************************************
 *
 *  Retrieve the subfamily name from a linked font.
 *
 *  Description:
 *      This function returns a string pointer to the subfamily name of
 *      a linked font. This string should not be freed by the caller
 *      since it points to the internal family name of the linked font.
 *      
 *  Parameters:
 *      ltt           - [in] pointer to linked font data structure
 *
 *  Return Value:
 *      FILECHAR pointer to FsLtt subfamily name. The string "Regular" is
 *      returned if the subfamily name has not been set.
 *
 *  See Also:
 *      FsLtt_setSubfamilyName()
 *
 *  <GROUP FsLttFunctions>
 */  
ITYPE_API FS_CONST FILECHAR *
FsLtt_getSubfamilyName(FsLtt *ltt);

/*****************************************************************************
 *
 *  Retrieve the user file version string from a linked font.
 *
 *  Description:
 *      This function returns a string pointer to the user's file version in
 *      a linked font. This string should not be freed by the caller
 *      since it points to the internal family name of the linked font.
 *      
 *  Parameters:
 *      ltt           - [in] pointer to linked font data structure
 *
 *  Return Value:
 *      FILECHAR pointer to FsLtt subfamily name. NULL is
 *      returned if the file version string has not been set.
 *
 *  See Also:
 *      FsLtt_setFileVersion()
 *
 *  <GROUP FsLttFunctions>
 */  
ITYPE_API FS_CONST FILECHAR *
FsLtt_getFileVersion(FsLtt *ltt);

/*****************************************************************************
 *
 *  Retrieve the number of components in a linked font.
 *
 *  Description:
 *      This function returns the number of components that have been 
 *      added to a linked font.
 *      
 *  Parameters:
 *      ltt           - [in] pointer to linked font data structure
 *
 *  Return Value:
 *      Number of components in the linked font.
 *
 *  See Also:
 *      FsLtt_addFont(), FsLtt_addFontMem(), FsLtt_component()
 *
 *  <GROUP FsLttFunctions>
 */  
ITYPE_API FS_USHORT
FsLtt_numComponents(FsLtt *);

/*****************************************************************************
 *
 *  Retrieves the specified component in a linked font.
 *
 *  Description:
 *      This function returns a pointer to the specified component within
 *      a linked font. This pointer may be used to retrieve the properties
 *      of the component. The component should not be deleted using 
 *      FsLttComponent_done(). Instead, use FsLtt_removeFont() to 
 *      remove the component.
 *      
 *  Parameters:
 *      ltt         - [in] pointer to linked font data structure
 *      ind         - [in] index of component (counting from zero)
 *
 *  Return Value:
 *      Pointer to component, or NULL if component index not found.
 *
 *  See Also:
 *      FsLtt_numComponents(), FsLtt_removeFont()
 *
 *  <GROUP FsLttFunctions>
 */  
ITYPE_API FsLttComponent *
FsLtt_component(FsLtt *ltt, FS_USHORT ind);

/*****************************************************************************
 *
 *  Retrieves the metric font index of a linked font.
 *
 *  Description:
 *      This function returns the index of the metric font within a 
 *      linked font. A value of 0xFFFF indicates that the metric font
 *      has not been set.
 *      
 *  Parameters:
 *      ltt         - [in] pointer to linked font data structure
 *
 *  Return Value:
 *      Metric font index (counting from zero), or 0xFFFF if metric font has 
 *      not been set.
 *
 *  See Also:
 *      FsLtt_getMetric()
 *
 *  <GROUP FsLttFunctions>
 */  
ITYPE_API FS_USHORT
FsLtt_getMetric(FsLtt *ltt);


/*****************************************************************************
 *
 *  Group type bitfield masks.
 *
 *  Description:
 *      These are bitfield masks for particular group type features.
 *
 *  <GROUP FsLttFunctions>
 */
#define FsLTT_GROUPTYPE_BASELINESHIFT 0x1   /* bit 0 */
#define FsLTT_GROUPTYPE_INCREASESCALE 0x2   /* bit 1 */
#define FsLTT_GROUPTYPE_DECREASESCALE 0x4   /* bit 2 */


/*****************************************************************************
 *
 *  Retrieves the defined type for a particular component group.
 *
 *  Description:
 *      This function returns the type of the group within a
 *      linked font.  Each of the three low-order bits of the type are
 *      booleans indicating an auto-adjust feature.  The bits are defined
 *      as:
 *          <TABLE>
 *          #define FsLTT_GROUPTYPE_BASELINESHIFT 0x1    bit 0
 *          #define FsLTT_GROUPTYPE_INCREASESCALE 0x2    bit 1
 *          #define FsLTT_GROUPTYPE_DECREASESCALE 0x4    bit 2
 *          </TABLE>
 *
 *      Group 0 will always be returned as type 0.
 *      
 *  Parameters:
 *      ltt         - [in] pointer to linked font data structure
 *      group       - [in] group number
 *
 *  Return Value:
 *      Type value.
 *
 *  See Also:
 *      FsLtt_setGroupType(), 
 *      FsLTT_GROUPTYPE_BASELINESHIFT,
 *      FsLTT_GROUPTYPE_INCREASESCALE,
 *      FsLTT_GROUPTYPE_DECREASESCALE
 *
 *  <GROUP FsLttFunctions>
 */  
ITYPE_API FS_USHORT
FsLtt_getGroupType(FsLtt *ltt, FS_USHORT group);


typedef FS_SHORT FS_FIX88;

#define FIX88_ONE (1 << 8)

/*****************************************************************************
 *
 *  Macro that converts a 8.8 fixed-point number to 16.16.
 *      
 *  Parameters:
 *      val           - [in] 8.8 fixed-point number
 *
 *  Return Value:
 *      16.16 FS_FIXED value
 *
 *  NOTES:
 *
 *      From: ISO/IEC 9899:1999 (ANSI C),
 *            6.5.7 Bitwise shift operators, paragraph 3:
 *            If the value of the right operand is negative or is
 *            greater than or equal to the width of the promoted
 *            left operand, the behavior is undefined.
 *
 *  <GROUP fsfixedpoint>
 */
#define FS_FIX88_TO_FS_FIXED(val)  ((val < 0) ? (-((FS_FIXED)(-val) << 8)) : ((FS_FIXED)val << 8))

/*****************************************************************************
 *
 *  Structure that describes a component adjustment.
 *
 *  Description:
 *      This data structure represents a single component adjustment.
 *      Each component has an array of FsPPemAdjust structures that are
 *      stored in sorted order based on increasing ppem value.
 *      The ppem value of this structure is the smallest ppem to which
 *      this adjustment applies, that is, this adjustment applies to all
 *      ppem values equal to or greater than the specified ppem and less
 *      than the ppem value specified in the next ((n+1)th) FsPPemAdjust
 *      structure in the array.
 *
 *  <GROUP lttstructs>
 */
struct FsPPemAdjust_
{
    FS_USHORT ppem;  /* size range minimum */
    FS_FIX88  scale; /* scale adjustment   */
    FS_SHORT  shift; /* shift adjustment   */
};


#ifdef __cplusplus
}
#endif

#endif /* FS_LTT_API_H */
