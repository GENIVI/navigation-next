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

    @file     Session.h
    @date     03/09/2009
    @defgroup MOBIUS_COMMON  Mobius Common API 

    Map Session API class.
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
#ifndef NCDB_SESSION_H
#define NCDB_SESSION_H


#include "NcdbTypes.h"
#include "AutoArray.h"
#include "UtfString.h"
#include "LibraryVersion.h"

class EngineConfiguration;
class CountryMgr;
class CoverageMgr;
class ShrlfMgr;


namespace  utility {
class   Logger;
};

namespace Ncdb {

class   DatabaseProperties;
class   EngineProperties;

class   Geocode;
class   PoiSearch;
class   SimpleSearch;

// Option for SetInitOptions (to be caled before any maps are opened)
// Supports defered loading of NBM 3DChunk data
#define SESSION_INIT_DEFER_POI      (0x00000001)
#define SESSION_INIT_DEFER_3DLM     (0x00000002)
#define SESSION_INIT_DEFER_3DUT     (0x00000004)
#define SESSION_INIT_DEFER_B2D      (0x00000008)
#define SESSION_INIT_DEFER_LBLT     (0x00000010)
#define SESSION_INIT_DEFER_LAM      (0x00000020)
#define SESSION_INIT_DEFER_RAST     (0x00000040)
#define SESSION_INIT_DEFER_NBM      (SESSION_INIT_DEFER_POI  | \
                                     SESSION_INIT_DEFER_3DLM | \
                                     SESSION_INIT_DEFER_3DUT | \
                                     SESSION_INIT_DEFER_B2D  | \
                                     SESSION_INIT_DEFER_LAM  | \
                                     SESSION_INIT_DEFER_LBLT | \
                                     SESSION_INIT_DEFER_RAST)
#define SESSION_INIT_DEFER_TRAFFIC  (0x10000000)
#define SESSION_INIT_NO_RESOURCES   (0x20000000)

// Options for LoadNbmChunks, defered loading of NBM 3DChunk data
#define SESSION_LOAD_POI            (0x00000001)
#define SESSION_LOAD_3DLM           (0x00000002)
#define SESSION_LOAD_3DUT           (0x00000004)
#define SESSION_LOAD_B2D            (0x00000008)
#define SESSION_LOAD_LBLT           (0x00000010)
#define SESSION_LOAD_LAM            (0x00000020)
#define SESSION_LOAD_RAST           (0x00000040)
#define SESSION_LOAD_ALL            (0xFFFFFFFF)

//!     Main Database Class
class NCDB_API Session
{
public:
	//! Constructor. 
	Session();      

	//! Destructor
	~Session();

    //! Initialize map session.
    /*! @note Map properties are also loaded. 
        @param[in] mapConfigPath - full path to map config file.
        @return    NCDB_OK      - maps session is opened,
                   NCDB_FAIL    - map config file not found 
    */
    // mapMemOptions parameter will be removed in the future, do not use.
    ReturnCode  Open(const char *  mapConfigPath, MapMemOptions mapMemOptions = MM_Default);
	ReturnCode  IsCompatibleVersion(const char *  mapConfigPath);
    ReturnCode  OpenList(AutoArray<UtfString> & list, MapMemOptions mapMemOptions = MM_Default);
    ReturnCode  OpenList(const char *  mapConfigPathList, MapMemOptions mapMemOptions = MM_Default);

    ReturnCode  Add(const char *  mapConfigPath, MapMemOptions mapMemOptions = MM_Default);
    ReturnCode  AddList(AutoArray<UtfString> & list, MapMemOptions mapMemOptions = MM_Default);
    ReturnCode  AddList(const char *  mapConfigPathList, MapMemOptions mapMemOptions = MM_Default);

    bool CheckConfig () const;

    ReturnCode  OpenTrafficClosures(const char* closureFile);
    ReturnCode  OpenHistoricTraffic();

    // Initialize World map session
    ReturnCode  OpenWorld(const char* worldMapConfigPath);
	// Initialize Outer map session
	ReturnCode  OpenOuter(const char* outerMapConfigPath);
    // Initialize shaded relief map
    ReturnCode  OpenShadedRelief(const char* shadedReliefMapPath);
    // Initialize nbm poi files
    ReturnCode  OpenNbmPois(const char* nbmPoiPath);

    // Initialize NBM 3D Chunk data by type (defered from map open/add).
    ReturnCode  LoadNbmChunks (int chunkTypes);

    //! Terminate map session.
    ReturnCode  Close();

    //! Get engine configuations
    const EngineConfiguration* GetEngineConfig(int mapId) const;
    EngineConfiguration* GetEngineConfig(int mapId);

    //! Get country manager
    // Use default param for backward compatibility
    const CountryMgr* GetCountryMgr(int mapId = 0) const;

    //! Get coverage manager
    CoverageMgr* GetCoverageMgr(int mapId) const;

    //! Get shaded relief map manager
    ShrlfMgr* GetShrlfMgr(int mapId = 0) const;

	Geocode* GetGeocode();
	PoiSearch* GetPoiSearch();
	SimpleSearch* GetSimpleSearch(bool delay = false);

    //! Set The Database Language (mostly for phonem data).
	ReturnCode  SetLanguage(ExonymLanguages language = Exonym_English );

    //! Get The Database Language (mostly for phonem data).
	const ExonymLanguages GetLanguage() const;
    
    // ------------------  Overide ini settings ---------------------

    void SetMapMemOptions (MapMemOptions mapMemOptions) {
        if (!m_sessionHandle) m_MapMemOptions = mapMemOptions;}
    void SetCacheLimit (int cacheLimit) {
        if (!m_sessionHandle) m_CacheLimit = cacheLimit;}
    void SetOnBoardMode (int onBoardMode) {
        if (!m_sessionHandle) m_OnBoardMode = onBoardMode;}

    void SetInitOptions (int initFlags) {
        if (!m_sessionHandle) m_InitOptions = initFlags;}

    // ------------------  Data Access methods ---------------------

    //! Check if map session is succesfully initialized.
    bool IsOpened() const
        { return (m_sessionHandle != 0); }

    //! Get Data Base Properties.
    /*! @return pointer to DatabaseProperties class with map properties or zero if map properties ar not yet loaded. 
    */
	const DatabaseProperties*    GetDatabaseProperties(int mapId = 0) const;

    //! Get internal Session handle.
    void *      GetSessionHandle() 
        { return m_sessionHandle; }

    const void* GetSessionHandle() const;

    //! Get library version as a number. 
    /*! The value consist of three two-digits decimal numbers: Major,Minor,Version.
        @sa GetLibraryVersionString.
     */
    static const int         GetLibraryVersionNumber()
        { return m_LibVersion; }

    //! Get library version as a string. 
    /*! The value consist of three decimal values, separated by dots: "Major,Minor,Version".
        @sa GetLibraryVersionNumber.
     */
    static const char* GetLibraryVersionString();

    //! Get pointer to Logger object.
    utility::Logger*        GetLogger() const;

	//! Set product ID and res supported by this session
	int SwitchMaterials(const char* pMaterialPath);

private:
    ReturnCode OpenInt (const char *  mapConfigPath);
    ReturnCode AddInt (const char *  mapConfigPath);
    ReturnCode InitNBM ();
    ReturnCode InitDefaultMaterials ();

    ReturnCode GetList (
        const char *  mapConfigPath,
        AutoArray<UtfString> & list);

	ReturnCode OpenMainAndMapInfo(const char *  mapConfigPath);

    int                             m_mapId;
    Session *                       m_MainSession;
    Session *                       m_NextSession;
    Session *                       m_SessionByMapId [MAX_MAPS];
    void *                          m_sessionHandle;        // 0 if session is not yet opened
    DatabaseProperties *            m_dbProperties;         // pointer to map properties, 0 if properties are not yet loaded

    EngineProperties *              m_engineProps;          // pointer to engine properties, 0 if not yet loaded

    CountryMgr*                     m_countryMgr;           // Session keep it's own CountryMgr

    CoverageMgr*                    m_coverageMgr;          // Session keep it's own CoverageMgr

    ShrlfMgr*                       m_shrlfMgr;             // Session keep it's own ShrlfMgr

	Geocode*                        m_Geocode;
	PoiSearch*                      m_PoiSearch;
	SimpleSearch*                   m_SimpleSearch;

    // Used by version check
    DatabaseProperties *            m_dbProperties2;
    EngineProperties *              m_engineProps2; 

    // Overridable ini settings. These values are defined in ini files, but can be overriden
    // by user of session.
    MapMemOptions                   m_MapMemOptions;
    int                             m_CacheLimit;
    int                             m_OnBoardMode;

    int                             m_InitOptions;

    static const int   m_LibVersion = NCDB_LIBRARY_VERSION; // library version  decimal number in the form  <Major><Minor><Version>.
};

};

////////////////////////////////////////////////////////////////////////////////////////////////////

inline
const void* Ncdb::Session::GetSessionHandle() const
{
    return m_sessionHandle;
}

#endif // NCDB_SESSION_H
/*! @} */
