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

    @file     AutoPtr.h
    @date     02/15/2009
    @defgroup MOBIUS_UTILS  Mobius utility classes 

    Simple non-counting smart pointer templates.

    These simple non-counting smart pointer templates are intended to be used 
    only as automatic variables or a class data member. 
    Passing them as parameters or returning from functions is forbidden.
    
    You can use AutoArrayPtr template to automatically delete arrays, although 
    the usage of dynamically allocated arrays is not recommended. 
    It's better to use STL 'vector' class or other containers.

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

#ifndef AUTOPTR_H_IS_INCLUDED
#define AUTOPTR_H_IS_INCLUDED


#include "NcdbTypes.h"

namespace Ncdb {

/*!  AutoPtr - Automaticly deleted non-counting pointer. 
                Can be used only as automatic variable or as a class data memeber. 
     @note   1. When copying the original pointer becomes NULL.    
     @note   2. Only explicit copying is allowed, so it can not be passed 
                 as a parameter or returned from functions.   
 */
template<class T>
class AutoPtr
{
public:

    //! Default Constructor prepares a NULL pointer.
    AutoPtr() : m_ptr(0) 
        {}
    
    //! Constructor keeps the pointer parameter.
    AutoPtr(T* ptr) : m_ptr(ptr) 
        {}
    
    //! destructor deletes an object.
    ~AutoPtr() 
        { delete m_ptr; }

    //! Convert AutoPtr to non-const pointer.
    operator  T*  () const 
        { return   m_ptr; }

    //! Non-const member access operator.
    T* operator->() const
        { return m_ptr; }

    //! Convert AutoPtr to non-const reference.
    operator       T &  ()        
        { return * m_ptr; }

    //! Derefernce operator to non-const reference.
    T & operator *  () const       
        { return * m_ptr;}

    ////! Derefernce operator to const reference.
    //const T & operator *  ()  const 
    //    { return * m_ptr; }

    //! Copy pointer to AutoPtr.
    //  It deletes the previous contents of destination object.
    T * operator = (T* ptr) 
        { 
            delete m_ptr; 
            m_ptr = ptr; 
            return *this; 
        }

    //! Copy AutoPtr to another AutoPtr. 
    //  It deletes the previous contents of destination object and empties the source object.
    AutoPtr<T>& operator =( AutoPtr<T> & ptr ) 
        { 
            delete m_ptr; 
            m_ptr = ptr.m_ptr; 
            ptr.m_ptr = 0; 
            return *this; 
        }

private:
    
    //! copy constructor is forbidden to prevent returning it from function or passing as a parameter.
    AutoPtr( AutoPtr<T> & ptr ) 
        { }     

    T*  m_ptr;      // stored pointer to object
};



/*!  AutoArrayPtr - Automaticly deleted non-counting pointer to array.
                    Can be used only as automatic variable or as a class data memeber. 
     Use it instead AutoPtr when array elements has to be normally destructed by operator 'delete []'. 
     @note    1. When copying the original pointer becomes NULL.    
     @note    2. Only explicit copying is allowed, so it can not be passed 
                 as a parameter or returned from functions.   
 */
template<class T>
class AutoArrayPtr
{
public:

    //! Default Constructor prepares a NULL pointer.
    AutoArrayPtr() : m_ptr(0) 
        {}
    
    //! Constructor keeps the pointer parameter.
    AutoArrayPtr(T * arr) : m_ptr(arr) 
        {}
    
    //! destructor deletes an object.
    ~AutoArrayPtr() 
        { delete [] m_ptr; }

    //! Convert AutoArrayPtr to simple non-const pointer.
    operator T* () const
        { return   m_ptr; }

    //! Copy array to AutoArrayPtr.
    //!  @note   It deletes the previous contents of destination object.  
    T * operator =(T* arr) 
        {
            delete[] m_ptr; 
            m_ptr = arr; 
            return m_ptr;
        }

    //! Copy AutoArrayPtr to another AutoArrayPtr. 
    //!  @note It deletes the previous contents of destination object and empties the source object.  
    AutoArrayPtr <T> & operator =( AutoArrayPtr <T> & ptr ) 
        { 
            delete [] m_ptr; 
            m_ptr = ptr.m_ptr; 
            ptr.m_ptr = 0; 
            return *this; 
        }

    //! Index non-const operator. 
    T & operator [] ( int i )
        { 
            return m_ptr[i];
        }

private:
    
    //! copy constructor is forbidden to prevent returning it from function or passing as a parameter.
    AutoArrayPtr( AutoArrayPtr<T> & ptr ) 
        { }     

    T*  m_ptr;      // stored pointer to array
};

};

#endif // AUTOPTR_H_IS_INCLUDED
/*! @} */
