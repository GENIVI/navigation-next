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

    @file     DatabaseProperties.h
    @date     03/09/2009
    @defgroup MOBIUS_COMMON  Mobius Common API 

    DatabaseProperties API class.
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
#ifndef NCDB_DATABASEPROPERTIES_H
#define NCDB_DATABASEPROPERTIES_H


#include "NcdbTypes.h"
#include "DisplayStyle.h"
#include <string>

class EngineConfiguration;

namespace Ncdb
{

class DatabasePropertyData;
class MapDisplayProperties;
class RoutingProperties;
//class   DisplayStyleList;

//template class NCDB_API                 AutoArray< DisplayStyle >;
//typedef AutoArray< DisplayStyle >       DisplayStyleList;

// ======================================================================================
//!     Type of map 
// ======================================================================================
typedef enum 
{
    MapType_Undefined = 0,
    MapType_Base,
    MapType_Street,
    MapType_POI,
    MapType_Invalid         // max value mark 
}   NcdbMapType;


// ======================================================================================
//!     Class to access Map Data Base Properties. 
// ======================================================================================
class NCDB_API DatabaseProperties
{
public:
	//! Constructor. 
    //  @param[in] chars - full path to map config file.
	DatabaseProperties(const char *  mapConfigPath);      

	//! Destructor
	virtual ~DatabaseProperties();

    //! Get Engine configuration
    const EngineConfiguration* GetEngineConfig() const;
    EngineConfiguration* GetEngineConfig();

    //! Get World Map path
    std::string GetWorldMapPath() const;

// Functions to obtain additional INI-files properties

    //! Get path to Map Display Properties INI-file.
    const  char*      GetDisplayPropertiesPath() const;

    //! Get path to Routing Properties INI-file.
    const  char*      GetRoutingPropertiesPath() const;

    //! Get path to Engine Properties INI-file.
    const  char*      GetEnginePropertiesPath() const;

    //! Get path to Maneuver Properties INI-file.
    const  char*      GetManeuverPropertiesPath() const;

    //! Get path to Geocode Properties INI-file.
    const  char*      GetGeocodePropertiesPath() const;

    //! Get path to Routing Properties INI-file.
    const  char*      GetResourcePath() const;

    //! Get map data files directory path.
    const char*       DataDirectory() const; 
    
    //! Get map ini files directory path.
    const char*       GetSettingPath() const; 

	//! Get a list of display style ini files.
	const DisplayStyleList& getDisplayStyles(void) const;

// Data Access functions

    //! Check if DatabaseProperties object is valid.
    bool              IsValid() const;

    //! Check if the polygon file is new format.
    bool              IsNewPolygon() const;

  //! Get full path to map config file.
    const char*       MapConfigPath() const; 

// Map Info Data Access functions

    //! Get map copyright information.
    const char*       Copyright() const; 

    //! Get additional comments.
    const char*       Comments() const; 

    //! Get map name.
    const char*       MapName() const; 

    //! Get map type.
    NcdbMapType       MapType() const; 
 
    //! Get vendor .
    const char*       MapVendor() const; 

	//! Get string, identifying map version.
    const char*       MapVersion() const; 

	//! Get string, identifying map region.
    const char*       MapRegion() const; 

	//! Get string, identifying map sub region.
	const char*       MapSubRegion() const; 

	//! Get string, identifying map build ID.
    const char*       MapBuildId() const; 

   //! Get map bounding box.
    const WorldRect*  BoundingBox() const; 

    //! Get map default center point.
    const WorldPoint* DefaultCenter() const; 
 
   //! Get map data creation date in standard ISO-8601 format.
    const char*       MapDate() const; 

    //! Get version of compiler/converter that created this map.
    int               CompilerVersion() const; 

    //! Get minimal version of library supporting this map. 
    /*! The value consist of three two-digits decimal numbers: Major,Minor,Version.
        Function Session::GetLibraryVersionNumber() returns current library version.
    */
    int               MinLibraryVersion() const; 

   //! Get map features.
    const char*       MapFeatures() const; 

 

    //! Get data base language.
    SupportedLanguages Language() const;

    const  char*      GetPronunFileName() const;

    //! Get DatabasePropertyData 
    DatabasePropertyData* GetDatabasePropertyData() const;  
private:

    DatabasePropertyData*   m_dbPropertyData;          // pointer to Map Properties data container
};

} // namespace Ncdb

#endif // NCDB_DATABASEPROPERTIES_H
/*! @} */
