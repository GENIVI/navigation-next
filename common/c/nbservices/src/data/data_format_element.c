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
 * data_format_element.c: created 2006/10/11 by Chetan Nagaraj.
 */

#include "data_format_element.h"
#include "csltypes.h"

NB_Error
data_format_element_init(data_util_state* pds, data_format_element* pfe)
{
    NB_Error err = NE_OK;
    pfe->type = NB_FormatElement_None;
    err = data_string_init(pds, &pfe->text);
    pfe->color = 0;
    err = err ? err : data_string_init(pds, &pfe->font);
    pfe->newline = FALSE;
    err = err ? err : data_link_init(pds, &pfe->link);

    return err;
}

void
data_format_element_free(data_util_state* pds, data_format_element* pfe)
{
    data_string_free(pds, &pfe->text);
    data_string_free(pds, &pfe->font);
    data_link_free(pds, &pfe->link);
}

NB_Error
data_format_element_from_tps(data_util_state* pds, data_format_element* pfe, tpselt te)
{
    NB_Error err = NE_OK;
    const char* name;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_format_element_free(pds, pfe);

    err = data_format_element_init(pds, pfe);

    if (err != NE_OK)
        goto errexit;

    name = te_getname(te);

    if (nsl_strcmp(name, "text") == 0) {
        pfe->type = NB_FormatElement_Text;
        err = err ? err : data_string_from_tps_attr(pds, &pfe->text, te, "data");
    }
    else if (nsl_strcmp(name, "color") == 0) {
        pfe->type = NB_FormatElement_Color;
        pfe->color = MAKE_NB_COLOR(te_getattru(te, "red"), te_getattru(te, "green"), te_getattru(te, "blue"));
    }
    else if (nsl_strcmp(name, "font") == 0) {
        pfe->type = NB_FormatElement_Font;
        err = err ? err : data_string_from_tps_attr(pds, &pfe->font, te, "name");
    }
    else if (nsl_strcmp(name, "newline") == 0) {
        pfe->type = NB_FormatElement_NewLine;
        pfe->newline = TRUE;
    }
    else if (nsl_strcmp(name, "link") == 0) {
        pfe->type = NB_FormatElement_Link;
        err = err ? err : data_link_from_tps(pds, &pfe->link, te);
    }

errexit:
    if (err != NE_OK)
        data_format_element_free(pds, pfe);

    return err;

}

boolean
data_format_element_equal(data_util_state* pds, data_format_element* pfe1, data_format_element* pfe2)
{
    return (boolean) (pfe1->type == pfe2->type &&
                        data_string_equal(pds, &pfe1->text, &pfe2->text) &&
                        pfe1->color == pfe2->color &&
                        data_string_equal(pds, &pfe1->font, &pfe2->font) &&
                        pfe1->newline == pfe2->newline &&
                        data_link_equal(pds, &pfe1->link, &pfe2->link));
}

NB_Error
data_format_element_copy(data_util_state* pds, data_format_element* pfe_dest, data_format_element* pfe_src)
{
    NB_Error err = NE_OK;

    data_format_element_free(pds, pfe_dest);

    err = data_format_element_init(pds, pfe_dest);

    pfe_dest->type = pfe_src->type;
    data_string_copy(pds, &pfe_dest->text, &pfe_src->text);
    pfe_dest->color = pfe_src->color;
    data_string_copy(pds, &pfe_dest->font, &pfe_src->font);
    pfe_dest->newline = pfe_src->newline;
    data_link_copy(pds, &pfe_dest->link, &pfe_src->link);

    return err;
}

void
data_format_element_to_buf(data_util_state* pds, data_format_element* pfe, struct dynbuf* pdb)
{
    uint32 i = 0;

    i = pfe->type;
    dbufcat(pdb, (const byte*) &i, sizeof(uint32));

    switch(pfe->type) {
        case NB_FormatElement_Text:
            data_string_to_buf(pds, &pfe->text, pdb);
            break;
        case NB_FormatElement_Color:
            dbufcat(pdb, (const byte*) &pfe->color, sizeof(pfe->color));
            break;
        case NB_FormatElement_Font:
            data_string_to_buf(pds, &pfe->font, pdb);
            break;
        case NB_FormatElement_NewLine:
            if (pfe->newline)
                i = 1;
            else
                i = 0;
            dbufcat(pdb, (const byte*)&i, sizeof(uint32));
            break;
        case NB_FormatElement_Link:
            data_string_to_buf(pds, &pfe->link.href, pdb);
            data_string_to_buf(pds, &pfe->link.text, pdb);
            break;
        default:
            break;
    }
}

NB_Error
data_format_element_from_binary(data_util_state* pds, data_format_element* pfe, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    uint32 i = 0;
    uint32 type = 0;

    err = err ? err : data_uint32_from_binary(pds, &type, pdata, pdatalen);
    pfe->type = (NB_FormatElement) type;

    switch (pfe->type) {

        case NB_FormatElement_Text:
            err = err ? err : data_string_from_binary(pds, &pfe->text, pdata, pdatalen);    
            break;
        case NB_FormatElement_Color:
            err = err ? err : data_uint32_from_binary(pds, &pfe->color, pdata, pdatalen);
            break;
        case NB_FormatElement_Font:
            err = err ? err : data_string_from_binary(pds, &pfe->font, pdata, pdatalen);
            break;
        case NB_FormatElement_NewLine:
            err = err ? err : data_uint32_from_binary(pds, &i, pdata, pdatalen);
            if (i == 0)
                pfe->newline = FALSE;
            else
                pfe->newline = TRUE;
            break;
        case NB_FormatElement_Link:
            err = err ? err : data_string_from_binary(pds, &pfe->link.href, pdata, pdatalen);
            err = err ? err : data_string_from_binary(pds, &pfe->link.text, pdata, pdatalen);
            break;
        default:
            break;
    }

    return err;

}

uint32 data_format_element_get_tps_size(data_util_state* pds, data_format_element* pfe)
{
    uint32 size = 0;

    size += sizeof(uint32); /*NB_FormatElement type*/
    size += data_string_get_tps_size(pds, &pfe->text);
    size += sizeof(pfe->color);
    size += data_string_get_tps_size(pds, &pfe->font);
    size += sizeof(pfe->newline);
    size += data_link_get_tps_size(pds, &pfe->link);

    return size;
}

NB_DEC NB_Error data_format_element_get_font(data_format_element* pfe, NB_Font *font)
{
    if (pfe != NULL && font != NULL && pfe->type == NB_FormatElement_Font)
    {
        if (data_string_compare_cstr(&pfe->font, "normal") == TRUE)
        {
            *font = NB_Font_Normal;
        }
        else if (data_string_compare_cstr(&pfe->font, "bold") == TRUE)
        {
            *font = NB_Font_Bold;
        }
        else if (data_string_compare_cstr(&pfe->font, "large") == TRUE)
        {
            *font = NB_Font_Large;
        }
        else if (data_string_compare_cstr(&pfe->font, "large-bold") == TRUE)
        {
            *font = NB_Font_Large_Bold;
        }
        else if (data_string_compare_cstr(&pfe->font, "custom-1") == TRUE)
        {
            *font = NB_Font_Custom1;
        }
        else if (data_string_compare_cstr(&pfe->font, "custom-2") == TRUE)
        {
            *font = NB_Font_Custom2;
        }
        else if (data_string_compare_cstr(&pfe->font, "custom-3") == TRUE)
        {
            *font = NB_Font_Custom3;
        }
        else if (data_string_compare_cstr(&pfe->font, "custom-4") == TRUE)
        {
            *font = NB_Font_Custom4;
        }
        else
        {
            *font = NB_Font_Normal;
        }
    }
    return NE_OK;
}