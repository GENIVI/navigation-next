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

@file     AutoArray.h
@date     03/03/2009
@defgroup MOBIUS_UTILS  Mobius utility classes

Dynamic array utility class.

This dynamic array class

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

#ifndef AUTOARRAY_H
#define AUTOARRAY_H

#include "NcdbTypes.h"
#include <assert.h>

namespace Ncdb {



//! @brief Auto array template class.
//! @details Auto array template class that works exactly like any C style
//! array.
//! @note References to this class are not valid.
//! @note The auto array template is not thread safe.
//! @note The auto array template allocates on read and write.
template <class T>
class AutoArray
{
    typedef AutoArray<T>        self_a;

    //! @brief Create new array by specified data buffer.
    //! @details This constructor for internal use only.
    //! @details Supposed all parameters are valid.
    AutoArray (const T* beg, int len, int desired_capacity, int growth)
    : m_Length(len)
    , m_Capacity(0)
    , m_Growth(growth)
    , m_InitialSize(8)
    , m_Array( 0 )
    {
        assert( beg || m_Length == 0 );
        assert( len >= 0 );
        assert( desired_capacity >= len );

        m_Capacity = h_calc_capacity( desired_capacity );

        m_Array = new T[ m_Capacity ];

        if(beg)
        {
            for (int i = 0; i < m_Length; ++i)
            {
                m_Array[i] = beg[i];
            }
        }
    }

public:
    typedef T               value_type;
    typedef T*              iterator;
    typedef const T*        const_iterator;

    typedef T&              reference;
    typedef const T&        const_reference;

    typedef int             size_type;
    typedef int             difference_type;

	//! @brief Auto array constructor.
	//! @details The array is allocated on first access not on construction.
	//! @param[in] initSize The initial array size allocated on first access.
	//! @param[in] initGrowth The array growth factor.  The array will grow
	//! by the value of initGrowth after passing the initSize.  Growth increases
	//! by a factor of two upon each array overflow.
	//! Example:
	//! first allocation size = initSize
	//! next allocation size = initSize + initGrowth
	//! next allocation size = initSize + initGrowth * 2
	//! next allocation size = initSize + initGrowth * 4
    AutoArray (int const initSize=8, int const initGrowth=4)
    : m_Length (0)
    , m_Capacity (0)
    , m_Growth (initGrowth)
    , m_InitialSize(initSize)
    , m_Array (0)
    {
        if( m_Growth < 1 )
            m_Growth = 4;
    }

	//! Copy constructor.
    AutoArray(const AutoArray<T>& other)
	: m_Length (0)
	, m_Capacity (0)
	, m_Growth (4)
	, m_InitialSize(8)
	, m_Array (0)
	{
		*this = other; // Revert to overload =
	}

    virtual ~AutoArray ()
    {
		if(m_Array)
		{
			delete [] m_Array;
			m_Array = 0;
		}
		m_Length = 0;
		m_Capacity = 0;
    }

	//! Copy operator.
	void operator = (const AutoArray<T>& other)
    {
        if( &other == this ) return;

        m_Length = 0;

        if(other.m_Array)
        {
            add( other.m_Array, other.m_Array + other.m_Length );
        }
    }

	//! Indexing operator.
    inline T & operator [] (int i)
    {
        assert ( i >= 0 && i < m_Length);          // Boundary check in debugging mode.
        return m_Array[i];                       // In release mode cusomer is on his/her own.
    }

    inline T & get(int iIndex)
    {
        assert ( iIndex >= 0 && iIndex < m_Length);          // Boundary check in debugging mode.
        return m_Array[iIndex];                       // In release mode cusomer is on his/her own.
    }

	//! Indexing operator.
    inline const T & operator [] (int i) const
    {
        assert ( i >= 0 && i < m_Length);          // Boundary check in debugging mode.
        return m_Array[i];                       // In release mode cusomer is on his/her own.
    }

    //! @brief Return the number of elements.
	int         size (void) const { return m_Length; }

    //! @brief Return free room inside internal data buffer.
    int         room (void) const { return m_Capacity-m_Length; }

    //! @brief Return true if the array is empty.
    bool        empty(void) const { return m_Length == 0; }

    //! @brief Reset the size to zero.
	void        reset (void) { m_Length = 0; }

    //! @brief Clear the size to zero.
	void        clear (void) { m_Length = 0; }

    //! @brief Return the capacity or internal buffer size of the array.
    int         capacity () const { return m_Capacity; }

    //! @brief Increase the capacity (internal buffer size) of the array.
	//! @param[in] neededCapacity : number of elements that is guaranteed to be allocated in internal buffer.
	void        reserve (int neededCapacity)
    {
        if ( neededCapacity <= m_Capacity )
            return;     // nothing to do

	    int NewSize;
	    if (m_Capacity == 0)
	    {
		    NewSize = m_InitialSize;
	    }
	    else
	    {
		    NewSize = m_Growth + m_Capacity;
	    }
	    if (neededCapacity >= NewSize)
	    {
		    NewSize = neededCapacity + 1;
	    }

	    T * NewArray = new T [NewSize];
	    for (int i = 0; i < m_Capacity; ++i)
	    {
		    NewArray [i] = m_Array [i];
	    }

	    m_Capacity = NewSize;

        delete []m_Array;

	    m_Array = NewArray;
	    m_Growth = m_Growth * 2;
    }

	//! @brief Set grow value, that is used to increase capacity on adding elements.
	//! @param[in] newGrowthValue : number of elements that is used to increase capacity when adding elements.
    void        setGrowthValue (int newGrowthValue)
    {
        m_Growth = newGrowthValue;
        if ( m_Growth < 1 )
        {
            m_Growth = 4;
        }
    }

    //! @brief Decrease the capacity (internal buffer size) of the array to required value, but not less than its current size.
	//! @details If actual capacity is less than parameter, then function does nothing.
	//! @param[in] neededCapacity : new max number of elements in internal buffer.
	void        shrink (int neededCapacity)
    {
	    int newSize = neededCapacity;
	    if (m_Length >= newSize)
	    {
		    newSize = m_Length;
	    }

        if ( newSize >= m_Capacity )
            return;     // nothing to do

	    T * NewArray = new T [newSize];
	    for (int i = 0; i < m_Capacity; ++i)
	    {
		    NewArray [i] = m_Array [i];
	    }

	    m_Capacity = newSize;
	    m_Array = NewArray;
	    m_Growth = ( m_Growth / 2 )  + 1;     // reduce grow value too
    }

    //! @brief Add an element to the end of array.
	//! @details If necessary the capacity of internal array buffer will be increased.
	//! @param[in] el : The object to add into the array.
    //! @return reference to appended element
	T&          add ( const T& other )
    {
        if( 0 == room() ) // need reallocate
        {
            self_a new_arr( m_Array, m_Length, h_calc_capacity(m_Length + 1), m_Growth*2 );

            new_arr.m_Array[ new_arr.m_Length++ ] = other;

            swap( new_arr );
        }
        else
        {
            m_Array[ m_Length++ ] = other;
        }

        return back();
    }


    //! @brief Add an element to the end of array.
	//! @details If necessary the capacity of internal array buffer will be increased.
	//! @param[in] other : The object to add into the array.
	void        add ( const AutoArray<T>& other )
    {
        add( other.begin(), other.end() );
    }

    //! @brief Add elements to the end of array.
	//! @details If necessary the capacity of internal array buffer will be increased.
	//! @param[in] other : The object to add into the array.
	void        add ( const T* beg_, const T* end_ )
    {
        assert( end_ >= beg_ );

        const int other_size = int( end_ - beg_ );

        if( other_size > room() ) // need reallocate
        {
            self_a new_arr( m_Array, m_Length, h_calc_capacity(m_Length + other_size), m_Growth*2);

            new_arr.h_add( beg_, end_ );

            swap( new_arr );
        }
        else
        {
            h_add( beg_, end_ );
        }
    }

    //! @brief Add an element to the end of array.
    //! @details If necessary the capacity of internal array buffer will be increased.
    //! @param[in] el : The object to add into the array.
    //! @return reference to appended element
    void        push_back ( const T& el )    { add(el); }

    //! @brief Resize the array.
    //! @details If new_size > size() default-constructed elements will be added.
    void        resize( int new_size, const T& pattern = T() )
    {
        if( new_size < 0 )
            new_size = 0;

        if( new_size < size() )
        {
            m_Length = new_size;
        }
        else if( new_size > size() )
        {
            const int d_size = new_size - m_Length;

            if( m_Capacity >= new_size )
            {
                T* a = end();
                h_fill( a, a + d_size, pattern );
                m_Length = new_size;
            }
            else
            {
                self_a new_arr( m_Array, m_Length, h_calc_capacity(new_size), m_Growth*2 );
                T* a = new_arr.end();
                h_fill( a, a + d_size, pattern );
                new_arr.m_Length = new_size;
                swap( new_arr );
            }
        }
    }

    //! @brief Swap content of two arrays
    void        swap ( self_a& other )
    {
        {
            int t;

            t = m_Length;
            m_Length = other.m_Length;
            other.m_Length = t;

            t = m_Capacity;
            m_Capacity = other.m_Capacity;
            other.m_Capacity = t;

            t = m_Growth;
            m_Growth = other.m_Growth;
            other.m_Growth = t;

            t = m_InitialSize;
            m_InitialSize = other.m_InitialSize;
            other.m_InitialSize = t;
        }

        {
            T* t = m_Array;
            m_Array = other.m_Array;
            other.m_Array = t;
        }
    }

    //! @brief Erase last element.
    //! @details If array is empty, behavior is undefined
    void        erase_back()
    {
        assert( !empty() );
        --m_Length;
    }

    //! @brief Return the first record in the array.
    T *         begin ()
    {
	    assert(m_Array || m_Length==0);
	    return m_Array;
    }

    //! @brief Return the first record in the array.
    const T*    begin () const\
    {
	    assert(m_Array || m_Length==0);
	    return m_Array;
    }

	//! @brief Return the first record AFTER the array.
    T *         end ()
    {
	    assert(m_Array || m_Length==0);
	    return &m_Array[m_Length];
    }

	//! @brief Return the first record AFTER the array.
    const T *   end () const
    {
	    assert(m_Array || m_Length==0);
	    return &m_Array[m_Length];
    }

    //! @brief Return the last record in the array.
    //  @details If array is empty, behavior is undefined
    T&          back()       { assert(m_Length > 0); return m_Array[m_Length-1]; }

    //! @brief Return the last record in the array.
    //  @details If array is empty, behavior is undefined
    const T&    back() const { assert(m_Length > 0); return m_Array[m_Length-1]; }

    //! @brief Return the first record in the array.
    //  @details If array is empty, behavior is undefined
    T&          front()       { assert(m_Length > 0); return *m_Array; }

    //! @brief Return the first record in the array.
    //  @details If array is empty, behavior is undefined
    const T&    front() const { assert(m_Length > 0); return *m_Array; }



private:
    //! @brief Append content, suppose current capacity is enough.
    void        h_add (const T* beg_, const T* end_ )
    {
        const int other_size = int( end_ - beg_ );

        T* dst = m_Array + m_Length;

        for( int i = 0; i < other_size; ++i )
        {
            *dst++ = beg_[i];
        }

        m_Length += other_size;
    }

    int         h_calc_capacity( int desired_capacity ) const
    {
        assert( m_Growth );

        int capacity = ( (desired_capacity + m_Growth - 1) / m_Growth) * m_Growth;

        return capacity;
    }

    static void h_fill( T* beg, T* end, const T& pattern )
    {
        assert( end >= beg );

        while( beg != end )
        {
            *beg++ = pattern;
        }
    }

	int m_Length;
	int m_Capacity;
	int m_Growth;
	int m_InitialSize;
	T*  m_Array;
};

#ifdef _MSC_VER
template class NCDB_API AutoArray< long >;
template class NCDB_API AutoArray< int >;
template class NCDB_API AutoArray< unsigned int >;
#endif
};
#endif // AUTOARRAY_H
/*! @} */

