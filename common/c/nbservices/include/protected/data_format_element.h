/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*
 * data_format_element.h: created 2006/10/10 by Chetan Nagaraj.
 */
#ifndef DATA_FORMAT_ELEMENT_H
#define DATA_FORMAT_ELEMENT_H

#include "datautil.h"
#include "data_string.h"
#include "nbexp.h"
#include "data_link.h"
#include "nbplace.h"

typedef enum
{
    NB_FormatElement_None	= 0,
    NB_FormatElement_Text,
    NB_FormatElement_Color,
    NB_FormatElement_Font,
    NB_FormatElement_NewLine,
    NB_FormatElement_Link
} NB_FormatElement;

typedef struct
{
    NB_FormatElement    type;
    data_string         text;
    uint32              color;
    data_string         font;
    boolean             newline;
    data_link			link;

} data_format_element;

NB_Error    data_format_element_init(data_util_state* pds, data_format_element* pfe);
void        data_format_element_free(data_util_state* pds, data_format_element* pfe);
NB_Error    data_format_element_from_tps(data_util_state* pds, data_format_element* pfe, tpselt te);
boolean	    data_format_element_equal(data_util_state* pds, data_format_element* pfe1, data_format_element* pfe2);
NB_Error    data_format_element_copy(data_util_state* pds, data_format_element* pfe_dest, data_format_element* pfe_src);
void        data_format_element_to_buf(data_util_state* pds, data_format_element* pfe, struct dynbuf *pdb);
NB_Error    data_format_element_from_binary(data_util_state* pds, data_format_element* pfe, byte** ppdata, size_t* pdatalen);
uint32      data_format_element_get_tps_size(data_util_state* pds, data_format_element* pfe);

NB_DEC NB_Error data_format_element_get_font(data_format_element* pfe, NB_Font *font);

#endif