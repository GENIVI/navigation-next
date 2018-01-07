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

    @file     UniString.h
    @date     02/15/2009
    @defgroup MOBIUS_UTILS  Mobius utility classes 

    Unicode character string class header file.

*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.                
                                                                  
    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
/*! @{ */

#pragma once
#ifndef UNISTRING_H_IS_INCLUDED
#define UNISTRING_H_IS_INCLUDED

#include <limits.h>


#include "NcdbTypes.h"

#include "UBaseString.h"

namespace Ncdb {

class   UtfString;   // UTF-8 strings
class   AscString;

/*!    Unicode string class.

 */
class NCDB_API UniString : public UBaseString<wchar_t>
{
public:
    //! Default constructor
    UniString()
        { }

    /*! Constructor from Unicode characters with allocation parameters.
        @param[in] chars   - value to initialize the string with.
        @param[in] size     - size of buffer to be allocated (in Unicode chars, not in bytes).
        @param[in] sizeIncr - size of buffer allocation increment (in Unicode chars, not in bytes).
     */
    UniString(const wchar_t* chars,  int size = 0, int sizeIncr = 0)
        : UBaseString<wchar_t>(chars,size,sizeIncr)
        { }

    /*! Constructor attaching the string to a preallocated buffer.
        @param[in] size     - size of buffer to be allocated (in Unicode chars, not in bytes).
        @param[in] pBuffer  - Unicode character buffer address.
        @param[in] chars   - value to initialize the string with.
     */
    UniString(int size, wchar_t* pBuffer,  const wchar_t* chars = 0) 
        : UBaseString<wchar_t>(size, pBuffer, chars)
        { }

    //! Copy constructor    
    UniString(const UniString& str)        
       : UBaseString<wchar_t>(str)        
        { }

    explicit UniString(const UtfString& str);

    //! Index operator (unsafe).
    wchar_t&   operator[] (int  index) const 
        {
            return UBaseString<wchar_t>::operator[] (index);
        }


// ---------------  Copy/Concatenation ----------

    //! Concatenate with one character.
    UniString& operator += (wchar_t  wc);

    //! Concatenate with characters.
    UniString& operator += (const wchar_t*  chars);

    //! Concatenate with other string.
    UniString& operator += (const UniString& src);

    //!     Copy operator from characters.
    UniString& operator = (const wchar_t*  chars);

	//!     Copy operator.
    UniString& operator = (const wchar_t wc);

    //!     Copy operator.
    UniString& operator = (const UniString& str);

    //!     Conversion copy operator from UTF8 in  UtfString.
    UniString& operator = (const UtfString& str);

    //!     Conversion copy operator from UTF8 in AscString.
    UniString& operator = (const AscString& str);

// ---------------  Conversions ----------------

    //! Convert UTF-8 string to Unicode.
    //! @return  'false' if source UTF string is not valid.
    bool  ConvertFromUtf( const char* pc, bool bLangCode = false);


// ---------------  Utility functions ----------------

    /*! Find  a character in the string.
        @param[in]  ch     - character to find.
        @param [in] start  - position (from zero), where to start searching. By default search starts from the beginning.
        @return  the index of found character (in characters from zero).
                Retruns -1 if sub-string is not found. 
    */
    int  Find(wchar_t ch, int start = 0)
        {
            return UBaseString<wchar_t>::Find(ch, start);
        }

    /*! Find  a sub-string in the string.
        @param[in]  chars  - sub-string characters to find.
        @param [in] start  - position (from zero), where to start searching. By default search starts from the beginning.
        @return  the index of found search string (in characters from zero).
                Retruns -1 if sub-string is not found. 
    */
    int  Find(const wchar_t* chars, int start = 0)
        {
            return UBaseString<wchar_t>::Find(chars, start);
        }


    /*! Insert a characters into the string before a given position.
        @param [in] start  - position from zero, before which characters will be inserted.
        @param[in]  chars  - characters to insert.
        @param [in] length - length of sub-string in characters. 
                              If pLength < 0, then all characters until the end of the string are inserted.
    */
    void Insert(int start, const wchar_t* chars, int length = -1)
        {
            UBaseString<wchar_t>::Insert(start, chars, length);
        }



    /*! Remove sub-string from the string.
        @param [in]  start  - starting index of sub-string to be removed (from zero).
        @param [in]  length - length of sub-string to be removed in characters.  
                              If length < 0, then all characters until the end of the string are removed.
    */
    void Remove(int start, int length = -1) 
        {
            UBaseString<wchar_t>::Remove(start, length);
        }



private:
    static void   Dummy_Expansion();    // never called, only generates template functions body for Linker
};

};

#endif // UNISTRING_H_IS_INCLUDED
/*! @} */
