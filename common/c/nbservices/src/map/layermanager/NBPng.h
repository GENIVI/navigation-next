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

/*!--------------------------------------------------------------------------
 @file     nbpng.h
 @defgroup nbcommon

 */
/*
 (C) Copyright 2011 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
#ifndef NB_PNG_H
#define NB_PNG_H
#include "paltypes.h"
#include "nberror.h"
#include "datastream.h"

extern "C"
{

/* About 'Flexible array member' and ' Zero-extent array members'

   Usually when we use a dynamic array in C Language, we will define
   a structure like this
     struct Type
     {
        int arraylength;
        char * buffer;
     }
   and use it like this

      struct Type array;
      array.buffer = (char *) malloc(sizeof(char) * array.length);

   when free like this
      free ( array.buffer);

   In memeory , this usage is like this

   stack                  heap
    -----------------      ---------------------------
   |length | buffer  | -> | realbuffer
    -----------------      ---------------------------

   We found that there are two blocks memeory. That will make some problem.
   In particular, when we alloc the structure object in heaps, you need to do like this

        struct Type * array = (struct type *) malloc (sizeof(struct Type));
        array -> buffer = (char *) malloc(sizeof(char) * array->length);
        ...
        free ( array->buffer);
        free ( array );

   There are two malloc and two free. OMG, It is too complex.

   Some great seekers want to resolve the problem, or to be lazy, I think, find a tips, name it 'Flexible array member'
   or 'Zero-extent array members', which now is a C99 feature. Thanks them for it.

   The follow is referance from IBM document about Flexible array members and Zero-extent array members:
   http://publib.boulder.ibm.com/infocenter/comphelp/v8v101/index.jsp?topic=%2Fcom.ibm.xlcpp8a.doc%2Flanguage%2Fref%2Fstrct.htm

   ---------------------------------------------------------------------------------------------------------------------------------
   Flexible array members

   A flexible array member, which is a C99 feature, can be an element of a structure with more than one named member. A flexible array member can be used to access a variable-length object. The flexible array member must be the last element of such a structure, and it has incomplete type. It is declared with an empty index, as follows:

       array_identifier[ ];

   For example, b is a flexible array member of Foo.

      struct Foo{
          int a;
          int b[];
      };

   Since a flexible array member has incomplete type, you cannot apply the sizeof operator to a flexible array.

   Any structure containing a flexible array member cannot be a member of another structure or array.

   For compatibility with GNU C, XL C/C++ extends Standard C and C++, to ease the restrictions on flexible arrays and allow the following:

           Flexible array members can be declared in any part of a structure, not just as the last member.
           Structures containing flexible array members can be members of other structures.
           Flexible array members can be statically initialized.

   In the following example:

     struct Foo{
        int a;
        int b[];
     };

     struct foo foo1 = { 55, {6, 8, 10} };
     struct foo foo2 = { 55, {15, 6, 14, 90} };

   foo1 creates an array b of 3 elements, which are initialized to 6, 8, and 10; while foo2 creates an array of 4 elements, which are initialized to 15, 6, 14, and 90.

   Flexible array members can only be initialized if they are contained in the outermost part of nested structures. Members of inner structures cannot be initialized.

   Zero-extent array members

   A zero-extent array is an array with no dimensions. Like a flexible array member, a zero-extent array can be used as a kind of template for array members whose size are determined dynamically at run time. Like a flexible array member, a zero-extent member must be the last element of a structure or union with more than one member. It must explicitly declared with zero as its dimension:

       array_identifier[0]

   It can only be statically initialized with an empty set. For example:

       struct foo{
           int a;
           char b[0];
       }; bar = { 100, { } };

   Otherwise, it must be initialized as a dyamically-allocated array.

   For example:

       struct foo * bar = (struct foo *) malloc (sizeof(struct foo) + sizeof(char) * length);
       ...
       free (bar);


   Zero-extent array members can only be initialized if they are contained in the outermost part of nested structures. Members of inner structures cannot be initialized.
   -----------------------------------------------------------------------------------------
*/

/*
   We have no idea about how many bytes in a PNG Image, so I use 'Flexible array member' / 'Zero-extent array members' here.
*/

/* The png image data and information needed*/

#ifdef _WIN32

/* in VC , this can cause a waring 4200, we disable it here*/
#pragma warning( disable: 4200 )
#endif
typedef  struct PNG_Data
{
    uint32 m_length;            /*! the length of m_data*/
    uint32 m_width;             /*! image width*/
    uint32 m_height;            /*! image height*/
    uint32 m_bitDepth;          /*! bit depth,3 means 24bit, or 4 means 32bit*/
                                /*! so usually, m_length = m_width * m_bitDepth * m_height / sizeof(m_data[0])   */
    byte   m_data[];            /*! image data */
}PNG_Data;

#ifdef _WIN32
#pragma warning( default: 4200 )
#endif

/*! Decode png from datastream
    @return NE_OK if success
 */
NB_Error DecodePNG(nbcommon::DataStreamPtr dataStream,     /*!< The png datastream */
                   PNG_Data** pngData                      /*!< Return the png image data if
                                                                success,otherwise is NULL.

                                                                Need call DestroyPNG to free.
                                                            */
                   );
/* Destory png buffer which create by DecodePNG*/
void DestroyPNG(PNG_Data * pngData                         /* !< png buffer which create
                                                                 by DecodePNG*/
                );
}

#endif
