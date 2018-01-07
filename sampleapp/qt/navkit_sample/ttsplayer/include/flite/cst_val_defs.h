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

/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                        Copyright (c) 1999                             */
/*                        All Rights Reserved.                           */
/*                                                                       */
/*  Permission is hereby granted, free of charge, to use and distribute  */
/*  this software and its documentation without restriction, including   */
/*  without limitation the rights to use, copy, modify, merge, publish,  */
/*  distribute, sublicense, and/or sell copies of this work, and to      */
/*  permit persons to whom this work is furnished to do so, subject to   */
/*  the following conditions:                                            */
/*   1. The code must retain the above copyright notice, this list of    */
/*      conditions and the following disclaimer.                         */
/*   2. Any modifications must be clearly marked as such.                */
/*   3. Original authors' names are not deleted.                         */
/*   4. The authors' names are not used to endorse or promote products   */
/*      derived from this software without specific prior written        */
/*      permission.                                                      */
/*                                                                       */
/*  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         */
/*  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      */
/*  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    */
/*  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   */
/*  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          */
/*  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       */
/*  THIS SOFTWARE.                                                       */
/*                                                                       */
/*************************************************************************/
/*             Author:  Alan W Black (awb@cs.cmu.edu)                    */
/*               Date:  December 1999                                    */
/*************************************************************************/
/*                                                                       */
/*  Macros for user defined type objects                                 */
/*                                                                       */
/*************************************************************************/
#ifndef _CST_VAL_DEFS_H__
#define _CST_VAL_DEFS_H__

#include <stdlib.h>

/* Macro for defining new user structs as vals  */
#define CST_VAL_USER_TYPE_DCLS(NAME,TYPE)              \
extern const int cst_val_type_##NAME;                  \
TYPE *val_##NAME(const cst_val *v);           \
cst_val *NAME##_val(const TYPE *v);

#define CST_VAL_USER_FUNCPTR_DCLS(NAME,TYPE)           \
extern const int cst_val_type_##NAME;                  \
TYPE val_##NAME(const cst_val *v);            \
cst_val *NAME##_val(const TYPE v);

#define CST_VAL_REGISTER_TYPE(NAME,TYPE)               \
TYPE *val_##NAME(const cst_val *v)                     \
{                                                      \
    return (TYPE *)val_generic(v,cst_val_type_##NAME,#NAME);  \
}                                                      \
void val_delete_##NAME(void *v)                        \
{                                                      \
    delete_##NAME((TYPE *)v);                          \
}                                                      \
                                                       \
cst_val *NAME##_val(const TYPE *v)                     \
{                                                      \
    return val_new_typed(cst_val_type_##NAME,          \
 		         (void *)v);                   \
}                                                      \

#define CST_VAL_REG_TD_TYPE(NAME,TYPE,NUM)             \
extern const int cst_val_type_##NAME;                  \
const int cst_val_type_##NAME=NUM;                     \
void val_delete_##NAME(void *v);                       \

/* When objects of this type can never be owned by vals */
#define CST_VAL_REGISTER_TYPE_NODEL(NAME,TYPE)         \
TYPE *val_##NAME(const cst_val *v)                     \
{                                                      \
    return (TYPE *)val_generic(v,cst_val_type_##NAME,#NAME);  \
}                                                      \
                                                       \
cst_val *NAME##_val(const TYPE *v)                     \
{                                                      \
    return val_new_typed(cst_val_type_##NAME,          \
 		         (void *)v);                   \
}                                                      \

#define CST_VAL_REG_TD_TYPE_NODEL(NAME,TYPE,NUM)       \
extern const int cst_val_type_##NAME;                  \
const int cst_val_type_##NAME=NUM;                     \
void val_delete_##NAME(void *v) { (void)v; }           \

#define CST_VAL_REGISTER_FUNCPTR(NAME,TYPE)            \
TYPE val_##NAME(const cst_val *v)                      \
{                                                      \
    return (TYPE)val_generic(v,cst_val_type_##NAME,#NAME);  \
}                                                      \
                                                       \
cst_val *NAME##_val(const TYPE v)                      \
{                                                      \
    return val_new_typed(cst_val_type_##NAME,          \
 		         (void *)v);                   \
}                                                      \

#define CST_VAL_REG_TD_FUNCPTR(NAME,TYPE,NUM)          \
extern const int cst_val_type_##NAME;                  \
const int cst_val_type_##NAME=NUM;                     \
void val_delete_##NAME(void *v) { (void)v; }           \

#endif
