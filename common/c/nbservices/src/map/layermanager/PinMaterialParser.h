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
   @file         PinMaterialParser.h
   @defgroup     nbmap
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */
#ifndef _PINMATERIALPARSER_H_
#define _PINMATERIALPARSER_H_

#include "PinLayer.h"
#include <map>

class TiXmlElement;

namespace nbmap
{

/*! Representation of a Pin Material. */
class PinMaterial
{
public:
    PinMaterial(const TiXmlElement* element = NULL);
    PinMaterial(const PinMaterial& material);
    virtual ~PinMaterial();
    string m_name;
    string m_id;
    uint16 m_index;
};

class PinMaterialParser
{
public:
    PinMaterialParser(const char* path=NULL);
    virtual ~PinMaterialParser();

    /*! Load configuration from file specified by path.

        @return None.
    */
    void LoadFromFile(const char* path);

    /*! Check if this parser is valid.

        @return true if so.
    */
    bool IsValid();

    /*! Looks for selected and unselected index the given PinType.

        @return NE_OK if succeeded.
    */
    NB_Error GetPinMaterialIndex(PinType type,    /*!< Type of Pin*/
                                 uint16& pinIndex /*!< Index of unselected material */
                                 );
private:

    /*! Resets member fields of this parser.

        @return None.
    */
    void ResetCachedMaterials();

    /*! Load mapping of pintype and id of material file.

        @return void
    */
    void LoadPinTypeMapping();

    bool                     m_valid;           /*!< Flag to indicate if this parser is valid. */
    map<PinType, string>     m_typeMapping;     /*!< Mapping of PinType and id in material
                                                     configuration file. */
    map<string, PinMaterial> m_cachedMaterials; /*!< Cached Materials, Material Configuration
                                                     will be parsed once, and its content
                                                     will be cached here.  */
};

}

#endif /* _PINMATERIALPARSER_H_ */
/*! @} */
