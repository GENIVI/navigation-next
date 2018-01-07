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

@file     callbackhttpresponsestatus.cpp
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

#include "palnet.h"
#include "palstdlib.h"
#include "callbackdnsresult.h"
#include "palmath.h"

namespace nimpal
{
    namespace network
    {
        CallbackDnsResult::CallbackDnsResult(void* function, void* userData,
                PAL_Error errorCode, const char* hostName,
                const char** addressList, short /*addressLength*/)
            : Callback(function, userData),
            m_errorCode(errorCode), m_hostName(NULL), m_addressCount(0)
        {
            uint32 count = 0;
            uint8 size = 0;

            if (hostName)
            {
                m_hostName = nsl_strdup(hostName);
            }

            nsl_memset(m_addresses, 0, sizeof(m_addresses));

            // determine address count
            while ( addressList && addressList[count] && *addressList[count] )
            {
                count++;
            }

            // translate and populate addresses
            m_addressCount = MIN(count, MAX_DNS_RESULT_ADDRESSES);
            for ( count = 0; count < m_addressCount; count++ )
            {
                if ( m_addresses[count] )
                {
                    nsl_free( m_addresses[count] );
                    m_addresses[count] = NULL;
                }
                // decide how much memory we need - it depends on IP version, so check address length
                size = ( nsl_strlen( addressList[count] ) == sizeof( uint32 ) ) ? sizeof( uint32 ) : 4 * sizeof( uint32 );
                m_addresses[count] = (uint32*) malloc( size + sizeof( uint32 ) );   // + sizeof( uint32 ) is for end NULL - idea is the same as for char* and '\0'

                if ( m_addresses[count] )
                {
                    *m_addresses[count] = nsl_htonl(*(uint32*)addressList[count]);
                    if ( size == ( 4 * sizeof( uint32 ) ) )                         // IPv6 case
                    {
                        if ( *m_addresses[count] == *(uint32*)addressList[count] )  // what byte order is used by this system
                        {
                            // no convert need
                            memset( m_addresses[count], 0, sizeof( m_addresses[count] ) );
                            memcpy( m_addresses[count], addressList[count], sizeof( m_addresses[count] ) );
                        }
                        else
                        {
                            m_addresses[count][0] = nsl_htonl( ( (uint32*)addressList[count] )[3] );
                            m_addresses[count][1] = nsl_htonl( ( (uint32*)addressList[count] )[2] );
                            m_addresses[count][2] = nsl_htonl( ( (uint32*)addressList[count] )[1] );
                            m_addresses[count][3] = nsl_htonl( ( (uint32*)addressList[count] )[0] );
                        }
                    }
                }
                m_addresses[count][size/sizeof( uint32 )] = NULL;
            }

        }

        CallbackDnsResult::~CallbackDnsResult()
        {
            for ( uint32 count = 0; count < MAX_DNS_RESULT_ADDRESSES; count++ )
            {
                if ( m_addresses[count] )
                {
                    nsl_free( m_addresses[count] );
                    m_addresses[count] = NULL;
                }
            }
            if (m_hostName)
            {
                nsl_free(m_hostName);
            }
        }

        void CallbackDnsResult::InvokeFunction()
        {
            if (m_function)
            {
                ((PAL_NetDnsResultCallback)m_function)(m_userData, m_errorCode,
                    m_hostName, m_addresses, m_addressCount);
            }
        }
    }
}

