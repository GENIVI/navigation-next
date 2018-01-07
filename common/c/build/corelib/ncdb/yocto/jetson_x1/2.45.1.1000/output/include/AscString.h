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

    @file     AscString.h
    @date     02/15/2009
    @defgroup MOBIUS_UTILS  Mobius utility classes 
    
    Simple ASCII byte string class header file.

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
#ifndef ASCSTRING_H_IS_INCLUDED
#define ASCSTRING_H_IS_INCLUDED

#include <limits.h>


#include "NcdbTypes.h"

#include "UBaseString.h"

namespace Ncdb {

/*!    UTF8 string class.
    @note  Regular string operations may result in invalid UTF-8 code string. 
           You can use UnicodeLength() function to check the integrity of UTF-8 string.
 */
class NCDB_API AscString : public UBaseString<char>
{
public:
    //! Default constructor
    AscString()
        { }

    /*! Constructor from Unicode characters with allocation parameters.
        @param[in] chars   - value to initialize the string with.
        @param[in] size     - size of buffer to be allocated in bytes.
        @param[in] sizeIncr - size of buffer allocation increment (in Unicode chars, not in bytes).
     */
    explicit AscString(const char* chars,  int size = 0, int sizeIncr = 0)
        : UBaseString<char>(chars,size,sizeIncr)
        { }

    /*! Constructor attaching the string to a preallocated buffer.
        @param[in] size     - size of buffer to be allocated in bytes.
        @param[in] pBuffer  - character buffer address.
        @param[in] chars   - value to initialize the string with.
     */
    explicit AscString(int size, char* pBuffer,  const char* chars = 0) 
        : UBaseString<char>(size, pBuffer, chars)
        { }

    //! Copy constructor
    AscString(const AscString& str) 
        : UBaseString<char>(str)
        { }

    //! Index operator (unsafe).
    char&   operator[] (int  index) const 
        {
            return UBaseString<char>::operator [](index);
        }

// ---------------  Utility functions ----------------

    /*! Find  a character in the string.
        @param[in]  ch     - character to find.
        @param [in] start  - position (from zero), where to start searching. By default search starts from the beginning.
        @return  the index of found character (in characters from zero).
                Retruns -1 if sub-string is not found. 
    */
    int  Find(char ch, int start = 0)
        {
            return UBaseString<char>::Find(ch, start);
        }

    /*! Find  a sub-string in the string.
        @param[in]  chars  - sub-string characters to find.
        @param [in] start  - position (from zero), where to start searching. By default search starts from the beginning.
        @return  the index of found search string (in characters from zero).
                Retruns -1 if sub-string is not found. 
    */
    int  Find(const char* chars, int start = 0)
        {
            return UBaseString<char>::Find(chars, start);
        }


    /*! Insert a characters into the string before a given position.
        @param [in] start  - position from zero, before which characters will be inserted.
        @param[in]  chars  - characters to insert.
        @param [in] length - length of sub-string in characters. 
                              If pLength < 0, then all characters until the end of the string are inserted.
    */
    void Insert(int start, const char* chars, int length = -1) 
        {
             UBaseString<char>::Insert(start, chars, length);
        }


    /*! Remove sub-string from the string.
        @param [in]  start  - starting index of sub-string to be removed (from zero).
        @param [in]  length - length of sub-string to be removed in characters.  
                              If length < 0, then all characters until the end of the string are removed.
    */
    void Remove(int start, int length = -1) 
        {
             UBaseString<char>::Remove(start, length);
        }


// ---------------  Copy/Concatenation ----------

    //! Concatenate with one character.
    AscString& operator += (char  c);

    //! Concatenate with characters.
    AscString& operator += (const char*  chars);

    //! Concatenate with other string.
    AscString& operator += (const AscString& src);

    //!     Copy operator from characters.
    const  char*  operator = (const char*  chars);

    //!     Copy operator.
    AscString& operator = (const AscString& str);

private:

};

#ifdef _MSC_VER
template class NCDB_API AutoArray < AscString >;
#endif

};

#endif // ASCSTRING_H_IS_INCLUDED
/*! @} */
