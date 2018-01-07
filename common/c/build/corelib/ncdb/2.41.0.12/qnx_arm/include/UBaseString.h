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

    @file     UBaseString.h
    @date     02/15/2009
    @defgroup MOBIUS_UTILS  Mobius utility classes 

    Counting smart pointer template.

    This counting smart pointer template should be used when the same object pointer is owned
    by several modules. It guarantees, that the pointed object will be deleted after
    all its owners do not need it anymore.

    Objects of this class can be passed as parameters by value and returned from functions.
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

#ifndef UBASESTRING_H_IS_INCLUDED
#define UBASESTRING_H_IS_INCLUDED


#include "NcdbTypes.h"
#include <assert.h>

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#define USTRING_HEAPCHECK_ON
#if defined(_MSC_VER) & defined(_DEBUG) & defined(USTRING_HEAPCHECK_ON)
    #include <crtdbg.h>
    #define USTRING_HEAPCHECK    assert( _CrtCheckMemory() ) 
#else
    #define USTRING_HEAPCHECK
#endif 
 

namespace Ncdb {

// -------------------------------------------------------------------------------------
/*!   Universal Base Bemplate class for strings.
      @note  No virtual functions are used to avoid memory overhead.
 */
template<class TChar>
class NCDB_API UBaseString
{
public:

    enum MemoryControlConstants {   // -------- Internal memory related constants ------------
        NoAllocation  = INT_MAX,    // special value for 'SizeIncr' parameter, telling it not to use memory.
        DefaultSize   = 16,         // default string allocation size.
        DefaultIncrement = 0        // default Allocation Increment.
    } ;       

    //! destructor has to be public.
    ~UBaseString()
        {
            if ( !IsExternalBuffer() )
            {
                Clear(true);            // release memory, if needed.
            }
        }


// ---------------  Copy/Concatenation ----------

    //! Copy constructor
	UBaseString(const UBaseString& src) 
		: m_Data(0), m_Size(0), m_Length(0), m_AllocIncrement(DefaultIncrement)
	{
		if ( src.IsEmpty() )
			return;

		m_Data = Allocate(src.m_Length + 1);
		if ( m_Data == 0 )
			return;         // no memory left 

		m_Size = src.m_Length + 1;
		m_Length = src.m_Length;
		memcpy(m_Data, src.m_Data, m_Size * sizeof(TChar) );
		// PutEOS();    -- not required, if initial string had EOS at end.
	}

    /*! Add one or more characters (not bytes) to the string.
        This function may reallocate the buffer memory.
        @param[in] chars   - pointer to charecters to add.
        @param[in] size     - number of characters (not bytes) to add. If it is < 0 , then all characters until zero are added.
      */
    void    Append(const TChar* chars, int size = -1);

// ---------------  Utility functions ----------------

    //! Check if string does not contain any characters.
    bool    IsEmpty() const
        {  return (m_Data == 0 || m_Length == 0); }

    //! Clear string with optional releasing of character buffer.
    //! @param [in]  freeMemory  - if 'true', then release internal buffer memory. Default is 'false'.
    void    Clear(bool  freeMemory = false);

    /*! Attach string object to a preallocated buffer.
        @param[in] size     - size of buffer to be used by string object (in TChar's not in bytes).
        @param[in] pBuffer  - character buffer address.
        @param[in] chars   - value to initialize the string with.
     */
    void   Attach(int size, TChar* pBuffer,  const TChar* chars = 0)
    {
        assert( size > 0 && pBuffer != 0 );

        // Release previous string if required.
        if ( m_Size > 0 && !IsExternalBuffer() )
        {
            delete [] m_Data;          // release string buffer
        }

        m_AllocIncrement = NoAllocation;

        if ( pBuffer == 0 || size == 0)
        {
            return;
        }

        m_Size  = size;
        m_Data  = pBuffer;

        // Set Initial value, if provided.
        if ( chars != 0 )
        {
            m_Length = 0;
            Append(chars);          // initialize buffer with a given char string
        }
        else
        {
            m_Length = CalcLength(m_Data);  // use the previous data length
        }
    }

    /*! Make Sub-string of string.
        @note This is a fast variant without additional memory allocations.
        @param [out] result - the string with resulting characters.
        @param [in]  start  - starting index of sub-string  from zero.
        @param [in]  length - length of sub-string in characters. 
                              If pLength < 0, then all characters until the end of the string are selected.
        @return  the length of resulting string in characters.
    */
    int Substr(UBaseString<TChar>& result, int start, int length = -1) const
    {
        result.Clear();
        if ( start < 0 || start >= m_Length || length == 0 )
        {
            return 0;     // result is empty
        }

        int resLength = length;
        if ( start + resLength > m_Length || length < 0 )
        {
            resLength =  m_Length - start;
        }

        result.Append(m_Data + start, resLength );    // copy characters to a place
        return resLength;
    }

    /*! Make Sub-string in a new string.
        @note This variant is handy, but requires additional memory allocations.
        @param [in]  start  - starting index of sub-string  from zero in characters.
        @param [in]  length - length of sub-string in characters. 
                                If pLength < 0, then all characters until the end of the string are selected.
        @return  the resulting sub-string.
    */
    UBaseString<TChar> Substr( int start, int length = -1) const
    {
        UBaseString<TChar>  strTemp;
        Substr(strTemp, start, length);
        return strTemp;
    }

    /*! Remove leading and last characters contained in parameter.
        @param [in]  charsToTrim - character set to trim off.
        @note For UtfString only regular ASCII characters should be used in parameter.
    */
    void Trim(const TChar* charsToTrim)
    {
        TChar  c;                       // current analyzed character from string

        if ( IsEmpty() )
            return;

        // ------------------------------
        //   find first non-space char 
        // ------------------------------
        int kFirst  = 0;                // postion from start of string
        while ( (c = m_Data[kFirst]) )
        {
            if ( SearchCharacter(charsToTrim, c) == 0 )
            {
                break;  // character does not belong to trimming set
            }
            kFirst++;
        }

        if ( m_Data[kFirst] == 0 || kFirst >= m_Length )  
        {
            // Resulting string becomes empty
            Clear(); 
            return;
        }

        // ----------------------------------
        //    truncate end of string 
        // ----------------------------------
        int kLast = m_Length - 1;       // postion from end of string
        while ( kLast > 0 )
        {
            c = m_Data[kLast];
            if ( SearchCharacter(charsToTrim, c) == 0 )
            {
                break;  // character does not belong to trimming set
            }
            --kLast;
        }
        m_Length = kLast + 1;
        m_Data[m_Length] = 0;     // put EOS

        // ----------------------------------------------------------------------
        //    shift remaining string left to eliminate leading trimmed chars
        // ----------------------------------------------------------------------
        int k2 = 0 ;         
        int k1 = kFirst;   
        if ( kFirst > 0 )
        {
            while ( k1 < m_Length )
            {
                m_Data[k2++] = m_Data[k1++];
            }
            m_Length -= kFirst;
            PutEOS();
        }
    }

// ---------------  Conversions ----------------

    //! Conversion to const Character pointer. NULL pointer is never returned.
    operator const TChar* () const;

    //! Get string character buffer as it is. 
    //! @note This function can return NULL pointer and is potentially dangerous. 
    TChar* GetBuffer()
        {
            return ( m_Data );
        }

    const TChar* GetBuffer() const
    {
        return m_Data;
    }
// --------------- Static Utility functions ---------------

    //! Calculate string length in char (a 'strlen' replacement). For UTF-8 strings it gives a number of bytes.
    static int CalcLength(const TChar* chars);

    /*! Allocate memory for 'Size' symbols.
        @param[in] size of allocated memory in TChar's (not in bytes).
     */
    static TChar* Allocate(int size);

    //! Search for a character 'ch' within 'strSource' string.
    static const TChar*  SearchCharacter(const TChar* strSource, TChar ch);

    //! Search for sub-string 'chars' within 'strSource' string.
    static const TChar*  SearchSubstring(const TChar* strSource, const TChar* chars);

    //!    Compare null-terminated C-strings
    static int   CompareChars(const TChar* chars1, const TChar* chars2);

    //! Test if character is a number.
    static bool isDigit(const TChar c)
    {
        return (c >= '0' && c <= '9');
    }


// ---------------  Comparisons ----------------

    //! Equal Comparison Operator.
    bool        operator== (const TChar*  chars) const
    {
        int rc = 0;
        if ( IsEmpty() || chars == 0 )
        {
            rc = CompareEmpty(chars);
        }
        else
        {
            rc = CompareChars(m_Data,chars);   // calls Standard C Run-time Function
        }
        return rc == 0;
    }

    //! Not-Equal Comparison Operator.
    bool        operator!= (const TChar*  chars) const
    {
        if ( IsEmpty() || chars == 0 )
            return ( CompareEmpty(chars) != 0 );
        return CompareChars(m_Data,chars) != 0;   // calls Standard C Run-time Function
    }

    //! Less Comparison Operator.
    bool        operator<  (const TChar*  chars) const
    {
        if ( IsEmpty() || chars == 0 )
            return ( CompareEmpty(chars) < 0 );
        return CompareChars(m_Data,chars) < 0;   // calls Standard C Run-time Function
    }

    //! Greater Comparison Operator.
    bool        operator>  (const TChar*  chars) const
    {
        if ( IsEmpty() || chars == 0 )
            return ( CompareEmpty(chars) > 0 );
        return CompareChars(m_Data,chars) > 0;   // calls Standard C Run-time Function
    }

    //! Less or Equal Comparison Operator.
    bool        operator<=  (const TChar*  chars) const
    {
        if ( IsEmpty() || chars == 0 )
            return ( CompareEmpty(chars) <= 0 );
        return CompareChars(m_Data,chars) <= 0;   // calls Standard C Run-time Function
    }

    //! Greater or Equal Comparison Operator.
    bool        operator>=  (const TChar*  chars) const
    {
        if ( IsEmpty() || chars == 0 )
            return ( CompareEmpty(chars) >= 0 );
        return CompareChars(m_Data,chars) >= 0;   // calls Standard C Run-time Function
    }


// ---------------  Accessors --------------

    //! Get current string length in characters. It should be used in most cases.
    int Length()   const
        { return m_Length; }

    //! Get current buffer size in characters (not in bytes).
    int Size()  const
        { return m_Size; }

    //! Get buffer size allocation increment in characters, not in bytes.
    int AllocIncrement() const
        { return m_AllocIncrement; }

    //! Check if String class uses external buffer. Such buffer can not be reallocated/deleted. 
    bool    IsExternalBuffer()  const
        { return (m_AllocIncrement == NoAllocation); }

    /*! Find  a character in the string.
        @param[in]  ch     - character to find.
        @param [in] start  - position (from zero), where to start searching. By default search starts from the beginning.
        @return  the index of found search string (in characters from zero).
                Retruns -1 if sub-string is not found. 
    */
    int  Find(TChar ch, int start = 0) const
    {
        if ( IsEmpty() || start < 0  || start >= m_Length )
            return -1;

        const TChar* pc = SearchCharacter(m_Data + start, ch);
        if ( pc == 0 )
            return -1;

        return (int)(pc - m_Data);
    }

    /*! Find  a sub-string in the string.
        @param[in]  chars  - sub-string characters to find.
        @param [in] start  - position (from zero), where to start searching. By default search starts from the beginning.
        @return  the index of found search string (in characters from zero).
                Retruns -1 if sub-string is not found. 
    */
    int  Find(const TChar* chars, int start = 0) const
    {
        if ( IsEmpty() || chars == 0 || start < 0  || start >= m_Length || chars[0] == 0 )
            return -1;

        const TChar* pc = SearchSubstring(m_Data + start, chars);
        if ( pc == 0 )
            return -1;

        return (int)(pc - m_Data);
    }


    /*! Insert a characters into the string before a given position.
        @param [in] start  - position from zero, before which characters will be inserted.
        @param[in]  chars  - characters to insert.
        @param [in] length - length of sub-string in characters. 
                              If pLength < 0, then all characters until the end of the string are inserted.
    */
    void Insert(int start, const TChar* chars, int length = -1)
    {
        if (chars == 0 || start < 0 || start > m_Length)
            return;     // nothing to add

        int copyLength = length;                  // # of characters to be actually copied.
        int realLength = CalcLength(chars);
        
        if ( copyLength < 0 || copyLength > realLength )
        {
            copyLength = realLength;    // correct length in order to keep consistent m_Length
        }

        if ( copyLength == 0 )
            return;     // nothing to add

        int newLength = m_Length + copyLength;      // the resulting string length.

        if ( IsExternalBuffer() )
        {
            // --------------------------------------------------------
            //   Truncate result if necessary for a fixed buffer.
            // --------------------------------------------------------
            assert( m_Size > 1 );

            if (m_Size < 2 )
                return;   // buffer is too small => do nothing

            if ( start + copyLength > m_Size - 1 )
            {
                // String to Insert goes beyound size, truncate it. 
                copyLength = m_Size - start - 1;
                if ( copyLength <= 0 )
                    return;

                newLength = m_Length + copyLength;
            }
            
            // Truncate result if it goes beyond. 
            //newLength = min(m_Size - 1, newLength);
            if((m_Size - 1) < newLength)
            {
            	newLength = m_Size - 1;
            }
        }
        else
        {
            // --------------------------------------------------------
            //   Reallocate buffer, if required.
            // --------------------------------------------------------
            if ( m_Data == 0 || m_Size < newLength + 1 )
            {
                //int newSize = max(newLength + 1, (int)DefaultSize);
            	int newSize = newLength + 1;
            	if(newSize < (int)DefaultSize)
            	{
            		newSize = (int)DefaultSize;
            	}

                if ( !Reallocate(newSize) )
                {
                    return;             // Rellocation failed.
                }

            }
        }

        // Move 2nd part of string to the end to free space for insertion.
        //   Preconditions: start + copyLength < newLength && copyLength > 0 && m_Size > newLength (i.e. no reallocation required)
        for( int j = newLength; j >= (start + copyLength) ; j-- ) 
        {
            m_Data[j] = m_Data[j - copyLength];
        }

        // put new characters on place.
        memmove( m_Data + start, chars, copyLength * sizeof(TChar) );

        m_Length = newLength;
        PutEOS();
    }


    /*! Remove sub-string from the string.
        @param [in]  start  - starting index of sub-string to be removed (from zero).
        @param [in]  length - length of sub-string to be removed in characters.  
                              If length < 0, then all characters until the end of the string are removed.
    */
    void Remove(int start, int length = -1)
    {
        if ( start < 0 || start >= m_Length || length == 0 )
            return;

        if ( start + length >=  m_Length || length < 0 )
        {
            // A simple string truncation.
            m_Length = start;
        }
        else
        {
            int copyLength = m_Length - (start + length);
            memmove( m_Data + start, m_Data + (start + length), copyLength * sizeof(TChar) );
            m_Length = start + copyLength;
        }
        PutEOS();
    }

protected:

// --------------- Protected methods ---------------
    //! Index operator (very unsafe).
    TChar&   operator[] (int  index) const 
        {
            assert( m_Data == 0 || (index >= 0 && index < m_Length) );   // check boundaries for debugging
            return ( m_Data[index] );                                    // m_Data should be non-zero here.
        }

// --------------- Protected implementation functions ---------------

    /*! Allocate a new buffer and copy existing content to it.
        @param[in] newSize  - size of buffer to be allocated (in TChar's not in bytes).
        @return   'false' when buffer with new size can not be allocated.
     */
    bool    Reallocate(int newSize);

    //! Compare empty string with TChar characters. 
    int     CompareEmpty(const TChar* chars) const
    {
        if ( IsEmpty() )
        {
            if ( chars == 0 || chars[0] == 0 )
                return 0;       // assume an equality with empty string 
            else 
                return -1;
        }
    
        // Not an empty string.
        if ( chars == 0 )
            return 1;       // assume string is larger
    
        return 999;         // undefined, a real comparison is required
    }

    //! Put Zero character into string buffer after current length.
    void    PutEOS()
    {
        if (m_Data != 0 && m_Size > 0 )
        {
            assert(m_Length < m_Size);
            m_Data[m_Length] = 0;
        }
    }

protected:

    //! Default constructor
    UBaseString() 
        : m_Data(0), m_Size(0), m_Length(0), m_AllocIncrement(DefaultIncrement)
        { }

    /*! Constructor from characters with allocation parameters.
        @param[in] chars   - value to initialize the string with.
        @param[in] size     - size of buffer to be allocated (in TChar's not in bytes).
        @param[in] sizeIncr - size of buffer allocation increment (in TChar's not in bytes).
     */
    explicit UBaseString(const TChar* chars,  int size = 0, int sizeIncr = 0 ): m_Data(0), m_Size(size), m_Length(0), m_AllocIncrement(sizeIncr)
    {   
        int allocSize = size;
        int charsLength = 0;            // length of initial value

        if ( chars != 0 )
        {
            // Initial string is not empty.
            charsLength = CalcLength(chars);
            if ( size == 0 )
            {
                allocSize = charsLength + 1;
            }
            else if ( size <= charsLength )
            {
                charsLength = size;
                allocSize   = size + 1; 
            }
        }

        if ( allocSize == 0 )
        {
            allocSize = DefaultSize;         // default 
        }
        
        m_Data = Allocate(allocSize);

        if ( m_Data == 0 )
        {
            m_Size = 0;
        }
        else 
        {
            m_Size = allocSize;

            if ( chars != 0 )
            {
                memcpy(m_Data, chars, charsLength * sizeof(TChar) );
                m_Length = charsLength;
                PutEOS();
            }
        }
    }

    /*! Constructor attaching the string to a preallocated buffer.
        @param[in] size     - size of buffer to be used by string object (in TChar's not in bytes).
        @param[in] pBuffer  - character buffer address.
        @param[in] chars   - value to initialize the string with.
     */
    explicit UBaseString(int size, TChar* pBuffer,  const TChar* chars = 0)
        : m_Data(pBuffer), m_Size(size), m_Length(0), m_AllocIncrement(NoAllocation)
    {   
        assert( m_Size > 0 && pBuffer != 0 );
        if ( pBuffer == 0 || m_Size == 0)
        {
            return;
        }

        if ( chars != 0 )
        {
            m_Length = 0;
            Append(chars);          // initialize buffer with a given char string
        }
        else
        {
            m_Length = CalcLength(m_Data);  // remeber the length of previous data
        }
    }

    // Dereferencing operator is forbidden.
    TChar operator* () { assert(0); return 0; }    

// DATA
    TChar*      m_Data;             // Pointer to character data buffer
    int         m_Size;             // Size of allocated buffer
    int         m_Length;           // Current string length
    int         m_AllocIncrement;   // Buffer size increment if buffer has to be extended: 
                                    //      if 0, then size is fixed and data memory is not freed;
                                    //      if INT_MAX then on extension m_Size is multiplied by two. 
};


// --------------------------------------------------------------------------------
//              Template class methods specializations.
// --------------------------------------------------------------------------------

//!    Compare char null-terminated C-strings.
template<> inline 
int   
UBaseString<char>::CompareChars(const char* chars1, const char* chars2)
{
    return strcmp( chars1, chars2);       // for 'char' type.
}


//!    Compare wchar_t null-terminated C-strings.
template<> inline 
int   
UBaseString<wchar_t>::CompareChars(const wchar_t* chars1, const wchar_t* chars2)
{
    return wcscmp( chars1, chars2 );    // for 'wchar_t' type.
}


//! Conversion to const char pointer.  NULL pointer is never returned.
template<> inline 
UBaseString<char>::operator const char* () const
{
    return ( (m_Data != 0 ) ? m_Data : "" );
}

//! Conversion to const wchar_t pointer.  NULL pointer is never returned.
template<> inline 
UBaseString<wchar_t>::operator const wchar_t* () const
{
    return ( (m_Data != 0 ) ? m_Data : L"" );
}

};

#endif // UBASESTRING_H_IS_INCLUDED
/*! @} */
