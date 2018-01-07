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

/*-
 * datautil.h: created 2004/12/09 by Mark Goddard.
 */

#ifndef DATA_UTIL_
#define DATA_UTIL_

#include "nberror.h"
#include "tpselt.h"
#include "fsalloc.h"
#include "paldebuglog.h"
#include "nbexp.h"
#include "vec.h"

#define VEC_ALLOC_FSA(h,s, p)                                               \
    do {                                                                    \
        p = CSL_VectorAllocPlace(s, fsaheap_alloc(h));                      \
        DEBUGLOG(LOG_SS_MEMORY, LOG_SEV_DEBUG,                              \
        ("VEC_ALLOC_FSA: [%s|%u|%p|%u]", __FILE__, __LINE__, p, s));        \
    } while (0)

#define VEC_DEALLOC_FSA(h,v)                                    \
    do {                                                        \
        DEBUGLOG(LOG_SS_MEMORY, LOG_SEV_DEBUG,                  \
        ("VEC_DEALLOC_FSA: [%s|%u|%p|]", __FILE__, __LINE__, v));\
        CSL_VectorDeallocPlace(v);                                  \
        fsaheap_free(h, v);                                     \
        v=NULL;                                                 \
    } while (0)


#define DATA_MEM_ZERO(PTR, DATA_TYPE)                           \
    nsl_memset(PTR, 0, sizeof(DATA_TYPE))


#define DATA_INIT(PDS, ERR, PTR, DATA_TYPE)                     \
    do {                                                        \
        if (ERR == NE_OK)                                       \
            ERR = DATA_TYPE##_init(PDS, PTR);                   \
    } while(0)

#define DATA_FREE(PDS, PTR, DATA_TYPE)                          \
    DATA_TYPE##_free(PDS, PTR)

#define DATA_REINIT(PDS, ERR, PTR, DATA_TYPE)                   \
    do {                                                        \
        if (ERR == NE_OK) {                                     \
            DATA_FREE(PDS, PTR, DATA_TYPE);                     \
            DATA_INIT(PDS, ERR, PTR, DATA_TYPE);                \
        }                                                       \
    } while (0)


#define DATA_EQUAL(PDS, RET, LHS, RHS, DATA_TYPE)               \
    do {                                                        \
       if (RET)                                                 \
           RET = DATA_TYPE##_equal(PDS, LHS, RHS);              \
    } while(0)


#define DATA_COPY(PDS, ERR, DST, SRC, DATA_TYPE)                \
    do {                                                        \
        if (ERR == NE_OK)                                       \
            ERR = DATA_TYPE##_copy(PDS, DST, SRC);              \
    } while (0)


#define DATA_VEC_ALLOC(ERR, VECPTR, DATA_TYPE)                              \
    do {                                                                    \
        if (ERR == NE_OK) {                                                 \
            VECPTR = CSL_VectorAlloc(sizeof(DATA_TYPE));                    \
            if (VECPTR == NULL)                                             \
                ERR = NE_NOMEM;                                             \
        }                                                                   \
    } while(0)

#define DATA_VEC_FREE(PDS, VECPTR, DATA_TYPE)                               \
    do {                                                                    \
        if(VECPTR) {                                                        \
            int n, l;                                                       \
            l = CSL_VectorGetLength(VECPTR);                                \
            for (n=0; n<l; n++)                                             \
               DATA_TYPE##_free(PDS, (DATA_TYPE*) CSL_VectorGetPointer(VECPTR, n));    \
            CSL_VectorDealloc(VECPTR);                                      \
            VECPTR = NULL;                                                  \
        }                                                                   \
    } while (0)

#define DATA_VEC_COPY(PDS, ERR, VECDST, VECSRC, DATA_TYPE)                  \
    do {                                                                    \
        if (ERR == NE_OK) {                                                 \
           int n, l;                                                        \
           DATA_TYPE tmp;                                                   \
           l = CSL_VectorGetLength(VECSRC);                                 \
           for (n=0; n<l && ERR == NE_OK; n++) {                            \
               ERR = ERR ? ERR : DATA_TYPE##_init(PDS, &tmp);               \
               ERR = ERR ? ERR : DATA_TYPE##_copy(PDS, &tmp, CSL_VectorGetPointer(VECSRC, n)); \
               ERR = ERR ? ERR : CSL_VectorAppend(VECDST, &tmp) ? NE_OK : NE_NOMEM;\
               if (ERR)                                                     \
                   DATA_TYPE##_free(PDS, &tmp);                             \
            }                                                               \
        }                                                                   \
    } while (0)

#define DATA_VEC_COPY_EX(PDS, ERR, VECDST, VECSRC, DATA_TYPE, ADDITIONAL_PARAM) \
    do {                                                                    \
        if (ERR == NE_OK) {                                                 \
           int n, l;                                                        \
           DATA_TYPE tmp;                                                   \
           l = CSL_VectorGetLength(VECSRC);                                 \
           for (n=0; n<l && ERR == NE_OK; n++) {                            \
               ERR = ERR ? ERR : DATA_TYPE##_init(PDS, &tmp);               \
               ERR = ERR ? ERR : DATA_TYPE##_copy(PDS, &tmp, CSL_VectorGetPointer(VECSRC, n), ADDITIONAL_PARAM); \
               ERR = ERR ? ERR : CSL_VectorAppend(VECDST, &tmp) ? NE_OK : NE_NOMEM;\
               if (ERR)                                                     \
                   DATA_TYPE##_free(PDS, &tmp);                             \
            }                                                               \
        }                                                                   \
    } while (0)

#define DATA_VEC_EQUAL(PDS, RET, VECPTR1, VECPTR2, DATA_TYPE)               \
    do {                                                                    \
        int n, l = 0;                                                       \
        if (RET) {                                                          \
            RET = (VECPTR1 != NULL) && (VECPTR2 != NULL);                   \
            RET = RET && ((l = CSL_VectorGetLength(VECPTR1)) == CSL_VectorGetLength(VECPTR2));        \
            for (n=0; RET && n<l; n++)                                      \
                RET = RET && DATA_TYPE##_equal(PDS, (DATA_TYPE*)CSL_VectorGetPointer(VECPTR1,n), (DATA_TYPE*)CSL_VectorGetPointer(VECPTR2,n)); \
        }                                                                   \
    } while (0)

#define DATA_ALLOC_TPSELT(ERR_LABEL, ELT, ELT_NAME)                         \
    do {                                                                    \
        ELT = te_new(ELT_NAME);                                             \
        if (!ELT) goto ERR_LABEL;                                           \
    } while(0)


#define DATA_STR_SETATTR(PDS, ERR_LABEL, ELEM, NAME, PTR)                   \
    do {                                                                    \
       if (!te_setattrc(ELEM, NAME, data_string_get(PDS, PTR))) {           \
           goto ERR_LABEL;                                                  \
       }                                                                    \
    } while (0)

#define DATA_BLOB_SETATTR(PDS, ERR_LABEL, ELEM, NAME, PTR)                  \
    do {                                                                    \
        if (!te_setattr(ELEM, NAME, (char*)((PTR)->data), (PTR)->size)) {   \
            goto ERR_LABEL;                                                 \
        }                                                                   \
    } while (0)

#define DATA_STR_SETATTR_OPTIONAL(PDS, ERR_LABEL, ELEM, NAME, PTR)          \
    do {                                                                    \
        if (nsl_strcmp(data_string_get(PDS, PTR), "") != 0) {               \
            DATA_STR_SETATTR(PDS, ERR_LABEL, ELEM, NAME, PTR);              \
        }                                                                   \
    } while (0)

#define DATA_TO_TPS(PDS, ERR_LABEL, PARENT_ELEM, PTR, DATA_TYPE)            \
    do {                                                                    \
        tpselt ce = 0;                                                      \
        ce = DATA_TYPE##_to_tps(PDS, PTR);                                  \
        if (!ce || !te_attach(PARENT_ELEM, ce)){                            \
           if (ce) te_dealloc(ce);                                          \
           goto ERR_LABEL;                                                  \
        }                                                                   \
    } while (0)

#define EMPTY_ELEMENT_TO_TPS(PDS, ERR_LABEL, PARENT_ELEM, TPS_NAME)         \
    do {                                                                    \
        tpselt ce = 0;                                                      \
        DATA_ALLOC_TPSELT(ERR_LABEL, ce, TPS_NAME);                         \
        if (!ce || !te_attach(PARENT_ELEM, ce)){                            \
           if (ce) te_dealloc(ce);                                          \
           goto ERR_LABEL;                                                  \
        }                                                                   \
    } while (0)

#define DATA_VEC_TO_TPS(PDS, ERR_LABEL, PARENT_ELEM, VEC_PTR, DATA_TYPE)    \
    do {                                                                    \
        int n = 0;                                                          \
        int l = CSL_VectorGetLength(VEC_PTR);                               \
        for (n = 0; n < l; n++) {                                           \
            DATA_TYPE* p = CSL_VectorGetPointer(VEC_PTR, n);                \
            DATA_TO_TPS(PDS, ERR_LABEL, PARENT_ELEM, p, DATA_TYPE);         \
        }                                                                   \
    } while (0)

#define DATA_VEC_GET_TPS_SIZE(PDS, SIZE, VEC_PTR, DATA_TYPE)                 \
    do {                                                                     \
        int n = 0, length = 0;                                               \
        length = CSL_VectorGetLength(VEC_PTR);                               \
        for(n = 0; n < length; ++n)                                          \
        {                                                                    \
            SIZE += DATA_TYPE##_get_tps_size(PDS, CSL_VectorGetPointer(VEC_PTR, n)); \
        }                                                                    \
    } while (0)

#define DATA_VEC_TO_BUF(PDS, BUF, VEC_PTR, DATA_TYPE)                        \
    do {                                                                     \
        uint32 n = 0, length = 0;                                            \
        length = ( uint32 )CSL_VectorGetLength(VEC_PTR);                     \
        dbufcat(BUF, (const byte*)&length, sizeof(length));                  \
        for(n = 0; n < length; ++n)                                          \
        {                                                                    \
            DATA_TYPE##_to_buf(PDS, CSL_VectorGetPointer(VEC_PTR, n), BUF);  \
        }                                                                    \
    } while (0)

#define DATA_VEC_FROM_BINARY(PDS, ERR, PDATA, PDATALEN, VEC_PTR, DATA_TYPE)                       \
    if (ERR == NE_OK) {                                                                           \
        uint32 length = 0;                                                                        \
        ERR = ERR ? ERR : data_uint32_from_binary(PDS, &length, PDATA, PDATALEN);                 \
        if (ERR == NE_OK && length > 0)                                                           \
        {                                                                                         \
            uint32 i = 0;                                                                         \
            if ( NULL == VEC_PTR )                                                                \
            {                                                                                     \
                DATA_VEC_ALLOC(ERR, VEC_PTR, DATA_TYPE);                                          \
            }                                                                                     \
            for (i = 0; i < length ; i++)                                                         \
            {                                                                                     \
                DATA_TYPE tmp_data;                                                               \
                if (*PDATALEN >= sizeof(tmp_data))                                                \
                {                                                                                 \
                    ERR = ERR ? ERR : DATA_TYPE##_init(PDS, &tmp_data);                           \
                    ERR = ERR ? ERR : DATA_TYPE##_from_binary(PDS, &tmp_data, PDATA, PDATALEN);   \
                    if(ERR)                                                                       \
                    {                                                                             \
                        break;                                                                    \
                    }                                                                             \
                    if (!CSL_VectorAppend(VEC_PTR, &tmp_data))                                    \
                    {                                                                             \
                        ERR = NE_NOMEM;                                                           \
                        break;                                                                    \
                    }                                                                             \
                }                                                                                 \
                else                                                                              \
                {                                                                                 \
                    ERR = NE_BADDATA;                                                             \
                    break;                                                                        \
                }                                                                                 \
            }                                                                                     \
        }                                                                                         \
    }

#define DATA_FROM_TPS(PDS, ERR, ELEM, PTR, DATA_TYPE)                       \
    ERR = ERR ? ERR : DATA_TYPE##_from_tps(PDS, PTR, ELEM)


#define DATA_FROM_TPS_ADD_TO_VEC(PDS, ERR, ELEM, VEC, DATA_TYPE)            \
    do {                                                                    \
        if (ERR == NE_OK) {                                                 \
            DATA_TYPE tmp;                                                  \
            ERR = DATA_TYPE##_init(PDS, &tmp);                              \
            ERR = ERR ? ERR : DATA_TYPE##_from_tps(PDS, &tmp, ELEM);        \
            if (ERR == NE_OK) {                                             \
                if (!CSL_VectorAppend(VEC, &tmp))                           \
                    ERR = NE_NOMEM;                                         \
            }                                                               \
            if (ERR) DATA_TYPE##_free(PDS, &tmp);                           \
        }                                                                   \
    } while (0)

#define DATA_ELEMENT_TO_TPS(PDS, ERR_LABEL, PARENT_ELEM, ELEM_NAME)         \
    do {                                                                    \
        tpselt ce = 0;                                                      \
        ce = te_new(ELEM_NAME);                                             \
        if (!ce || !te_attach(PARENT_ELEM, ce)){                            \
           if (ce) te_dealloc(ce);                                          \
           goto ERR_LABEL;                                                  \
        }                                                                   \
    } while (0)

#define DATA_PTR_FREEIF(STATE, PTR, DATA_TYPE)                              \
    do {                                                                    \
        if (PTR != NULL) {                                                  \
            DATA_TYPE##_free(STATE, PTR);                                   \
            nsl_free(PTR);                                                  \
            PTR = NULL;                                                     \
        }                                                                   \
    } while (0)


typedef struct data_util_state {

    struct fsaheap vec_heap;

} data_util_state;


NB_Error    data_util_state_init(data_util_state* pds);
NB_Error    data_util_state_free(data_util_state* pds);

NB_DEC NB_Error data_double_from_binary(data_util_state* pds, double* pdbl,
                byte** ppdata, size_t* pdatalen);
NB_DEC NB_Error data_uint32_from_binary(data_util_state* pds, uint32* pui,
                byte** ppdata, size_t* pdatalen);
NB_DEC NB_Error data_uint64_from_binary(data_util_state* pds, uint64* pui,
                byte** ppdata, size_t* pdatalen);
NB_DEC NB_Error data_boolean_from_binary(data_util_state* pds, boolean* pb,
                byte** ppdata, size_t* pdatalen);

#endif
