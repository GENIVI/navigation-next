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
   @file        poiimageidentifier.h

   Class PoiImageImageIdentifier is an object which identifies an image, and
   is provided to PoiImageListener.
*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of Networks In
   Motion is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/


#ifndef _POIIMAGEIDENTIFIER_H_
#define _POIIMAGEIDENTIFIER_H_

extern "C"
{
#include "nbexp.h"
#include "nberror.h"
#include "nbsinglesearchinformation.h"
#include "nbsearchinformation.h"
}

#include "smartpointer.h"
#include <string>

// Constants ....................................................................................


// Types ........................................................................................
namespace nbsearch
{

class NB_DLL PoiImageIdentifier {

public:

    /*! Default constructor.   */
    PoiImageIdentifier()
    {
        // Nothing to do here.
    }

    /*! Default deconstructor. */
    virtual ~PoiImageIdentifier()
    {
        // Nothing to do here.
    }

   /*! Return full url of this Identifier.

       @return full url.
   */
    virtual shared_ptr<std::string>
    GetUrl() = 0;

    /*! Indicate whether this is a thumbnail or not.

        @return true if it's thumbnail, or false otherwise.
    */
    virtual bool
    IsThumbnail() = 0;

    /*! Judge whether image specified by index and identifier is the same one.

        This function will check the image specified by the index in NB_SingleSearchInformation
        to see whether it points to the same image identified by this class.

        @return true if they are identical, or false otherwise.
    */
    virtual bool
    IsIdentical(NB_SingleSearchInformation* information, /*!< Single search information. */
                int index,                               /*!< Index of image to be checked. */
                bool isThumbnail = true                  /*!< Flag to indicate type of image. */
                ) = 0;

    /*! Judge whether image specified by index and identifier is the same one.

        This function will check the image specified by the index in NB_SearchInformation
        to see whether it points to the same image identified by this class.

        @return TRUE if they are identical, or FALSE otherwise.
    */
    virtual bool
    IsIdentical(NB_SearchInformation* information,       /*!< Search information instance. */
                int index,                               /*!< Index of image to be checked. */
                bool isThumbnail = true                  /*!< Flag to indicate type of image. */
                ) = 0;

private:
    // Private functions .......................................................................
    // Copy constructor and assignment operator are not supported.
    PoiImageIdentifier(const PoiImageIdentifier& requst);
    PoiImageIdentifier& operator=(const PoiImageIdentifier& request);
};

typedef shared_ptr<PoiImageIdentifier>  PoiImageIdentifierPtr;

};  // namespace navbuilder

#endif /* _POIIMAGEIDENTIFIER_H_ */
