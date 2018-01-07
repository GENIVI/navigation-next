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

    @file     AutoSharedPtr.h
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

#ifndef AUTOREFPTR_H_IS_INCLUDED
#define AUTOREFPTR_H_IS_INCLUDED


#include "NcdbTypes.h"

namespace Ncdb {

// ****************************************************************************
/*!    Reference counting smart pointer to shared objects.
       It automatically deletes an object, when all AutoSharedPtr instances, pointing to it, are deleted. 
       @note   1. It should not be used for static or global variables.
       @note   2. This version is not thread safe.
 */
template <class T> 
class AutoSharedPtr
{
public:
    //! Default constructor
    AutoSharedPtr() : m_Pointer(0), m_pCount(0)
        { }

    //! Constructor from pointer
    AutoSharedPtr(T* ptr) : m_Pointer(ptr), m_pCount(0)
        {
            if( ptr )
            {   m_pCount = new unsigned;
                *m_pCount = 1;
            }
        }

    //! Copy constructor
    inline AutoSharedPtr(const AutoSharedPtr<T>& ptr)
    {
        Copy( ptr.m_Pointer, ptr.m_pCount );
    }


    template< typename SOURCE_TYPE >
    inline AutoSharedPtr(const AutoSharedPtr< SOURCE_TYPE >& source)
    {
        Copy( source.m_Pointer, source.m_pCount );
    }

    //! Destructor
    ~AutoSharedPtr()
        {
            Unlink();
        }

    //! Convert AutoSharedPtr to non-const pointer.
    operator T* () const
        {
            return ( (m_pCount == 0) ? (0) : (m_Pointer) );
        }

    //! Copy operator:  AutoSharedPtr to AutoSharedPtr.
    AutoSharedPtr<T>& operator = (const AutoSharedPtr<T>& ptr)
    {
        Unlink();

        m_pCount = ptr.m_pCount;
        m_Pointer = ptr.m_Pointer;
        if( m_pCount != 0 && *m_pCount > 0 )
            (*m_pCount)++;
        return *this;
    }

    //! Non-const member access operator.
    T* operator->() const
        {
            return ( (m_pCount == 0) ? (0) : (m_Pointer) );
        }

	unsigned RefCount(void) const
		{
			return ( (m_pCount == 0) ? 0 : *m_pCount );
		}

private:

    void Unlink()              // remove pointer to object from this object
    {
        if( m_pCount != 0 )
        {
            if (*m_pCount > 0) 
            {
                --(*m_pCount);
                if ( *m_pCount == 0 )
                {
                    delete m_pCount;
                    delete m_Pointer;
                }
            }

            m_pCount = 0;
        }
        m_Pointer = 0;
    }

    inline void Copy(T* pointer, unsigned* count)
    {
        m_Pointer = pointer;
        m_pCount  = count;
        if( m_pCount != 0 && *m_pCount > 0 )
            (*m_pCount)++;
    }

// DATA
    T*          m_Pointer;      // pointer to object
    unsigned*   m_pCount;       // pinter to reference count

    template< typename SOURCE_TYPE > friend class AutoSharedPtr;
};

};
#endif //AUTOREFPTR_H_IS_INCLUDED
/*! @} */
