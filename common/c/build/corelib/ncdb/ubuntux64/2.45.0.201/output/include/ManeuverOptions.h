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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2014 by TeleCommunication Systems, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to TeleCommunication Systems, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of TeleCommunication Systems is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/
#ifndef NCDB_MANEUVER_OPTIONS_H
#define NCDB_MANEUVER_OPTIONS_H

#include "NcdbTypes.h" 

#define NCDB_MANEUVER_CMD_VERSION 101 //define the newest version number

namespace Ncdb {
class NCDB_API ManeuverOptions
{
public:

    ManeuverOptions();
    ~ManeuverOptions(){};

    /*! Sets the various options
    @param[in] unPavedRdInfo - want unpaved road attribute
    @param[in] simpleMan - terse/web style Maneuver
    @param[in] bridge - want bridge maneuvers if present in route
    @param[in] ferry - want ferry maneuvers if present in route
    @param[in] tunnel - want tunnel maneuvers if present in route
    @param[in] countryMan - want country crossing info 
    @param[in] crossstreet 
    @param[in] roundabout -want enter roundabout advisory
    */
    void SetManeuverOptions ( /* Deprecated - will be removed */
        bool unused1,
        bool unPavedRdInfo,
        bool simpleMan,
        bool bridge,
        bool ferry,
        bool tunnel,
        bool countryMan,
        bool unused2,
        bool crossstreet,
        bool unused3,
        bool roundabout);

    void SetBasicOptions (
        bool unPavedRdInfo,
        bool simpleMan,
        bool bridge,
        bool ferry,
        bool tunnel,
        bool countryMan,
        bool crossStreet,
        bool roundabout);

    void SetAdvancedOptions (
        bool naturalGuidance,
        bool trafficLightStopSign,
        bool laneGuidance,
        bool exitNumber,
        bool speedLimited,
        bool specialRegion,
        bool firstManCrossroad,
        bool routeFeatures,
        bool trafficDelay);

    void SetWndOptions (
        int routePolyWnd,
        int pronunWnd);

	/*! Sets the VX options
	@param[in] returnSAR -Client want SAR
	@param[in] returnMJO -Client want MJO
	@param[in] returnECM -Client want ECM
	*/
	void SetEnNVOptions(bool returnSAR, bool returnMJO, bool returnECM);

	/*! @brief Return if SAR is wanted
	@return m_WantSAR
	*/
	bool GetWantSAR()const;

	/*! @brief Return if MJO is wanted
	@return m_WantMJO
	*/
	bool GetWantMJO()const;

	/*! @brief Return if ECM is wanted
	@return m_WantECM
	*/
	bool GetWantECM()const;

	/*! Sets the Natural Guidance Option for bridge and rail
	@param[in] wantNatGuidance
	*/
	void SetWantNaturalGuidance(bool wantNatGuidance);

	/*! Gets the Natural Guidance Option for bridge and rail
	@param[out] wantNatGuidance
	*/
	bool GetWantNaturalGuidance() const;

    /*! Sets the Natural Guidance Option for traffic light and stop sign
	@param[in] wantNatGuidance
	*/
    void SetWantTrafficLights(bool wantTraffic);

    /*! Gets the Natural Guidance Option for traffic light and stop sign
	@param[out] wantNatGuidance
	*/
    bool GetWantTrafficLights() const;

	/*! Sets the Natural Guidance Option
	@param[in] wantlaneguidance
	*/
	void SetWantLaneGuidance(bool wantlaneguidance);

	/*! Gets the Natural Guidance Option
	@param[out] wantlaneguidance
	*/
	bool GetWantLaneGuidance() const;

    /*! Sets the Exit Number for high way Option
	@param[in] wantExitNumber
	*/
    void SetWantExitNumber(bool wantExitNumber);

    /*! Gets the Exit Number Option
	@param[out] wantExitNumber
	*/
    bool GetWantExitNumber() const;

	/*! Sets the VX options
	@param[in] returnSpeedLimited -Client want SpeedLimited
	*/
	void SetSpeedLimitedOption(bool returnSpeedLimited);

	/*! @brief Return if SpeedLimited is wanted
	@return m_WantSpeedLimited
	*/
	bool GetWantSpeedLimited()const;

    /*! Sets the value of "cmd set version" option
    @param[in] newValue
    */
    void SetCmdSetVersion( int newValue );

    /*! Returns the value of "cmd set version" option
    @return m_iCmdSetVersion
    */
    int GetCmdSetVersion() const;

    /*! Sets the value of "route polyline window" option
    @param[in] newValue
    */
    void SetRoutePolyWnd( int newValue );

    /*! Returns the value of "route polyline window" option
    @return m_iRoutePolyWnd
    */
    int GetRoutePolyWnd() const;

    /*! Sets the value of "pronun window" options
    @param[in] newValue
    */
    void SetPronunWnd( int newValue );

    /*! Returns the value of "pronun window" options
    @return m_iPronunWnd
    */
    int GetPronunWnd() const;

    /*! Sets the unpaved road info wanted
	@param[in] wantUnpavedRdInfo
	*/
    void SetUnPavedRdInfo(bool wantUnpavedRdInfo);

    /*!Return if UnPavedRdInfo is wanted
    @return m_UnPavedRdInfo
    */
    bool GetUnPavedRdInfo()const;

    /*! Sets simple maneuvers wanted
	@param[in] wantSimpleManeuvers
	*/
    void SetSimpleManeuvers(bool wantSimpleManeuvers);

    /*!Return if SimpleManeuvers is wanted
    @return m_SimpleManeuvers
    */
    bool GetSimpleManeuvers()const;

    /*! Sets bridge maneuvers wanted
	@param[in] wantBridgeManeuvers
	*/
    void SetBridgeManeuvers(bool wantBridgeManeuvers);

    /*!Return if BridgeManeuvers is wanted
    @return m_BridgeManeuvers
    */
    bool GetBridgeManeuers()const;

    /*! Sets tunnel maneuvers wanted
	@param[in] wantTunnelManeuvers
	*/
    void SetTunnelManeuvers(bool wantTunnelManeuvers);

    /*!Return if TunnelManeuvers is wanted
    @return m_TunnelManeuvers
    */
    bool GetTunnelManeuvers()const;

    /*! Sets ferry maneuvers wanted
	@param[in] wantFerryManeuvers
	*/
    void SetFerryManeuvers(bool wantFerryManeuvers);

    /*!Return if FerryManeuvers is wanted
    @return m_FerryManeuvers
    */
    bool GetFerryManeuvers()const;

    /*! Sets country maneuvers wanted
	@param[in] wantCountryManeuvers
	*/
    void SetCountryManeuvers(bool wantCountryManeuvers);

    /*!Return if CountryManeuver is wanted
    @return m_CountryManeuver
    */
    bool GetCountryManeuver()const;

    /*! Sets cross street wanted
	@param[in] wantCrossStreet
	*/
    void SetCrossStreet(bool wantCrossStreet);

    /*!Return if CrossStreet is wanted
    @return m_CrossStreet
    */
    bool GetCrossStreet()const;

    /*! Sets roundabout wanted
	@param[in] wantRoundAbout
	*/
    void SetRoundAbout(bool wantRoundAbout);

    /*!Return if RoundAbout is wanted
    @return m_RoundAbout
    */
    bool GetRoundAbout()const;

    /*! Sets the value of "want-special-regions" option
    @param[in] wantSpecialRegion
    */
    void SetWantSpecialRegion( bool wantSpecialRegion );

    /*! Returns the value of "want-special-regions" options
    @return m_WantSpecialRegion
    */
    bool GetWantSpecialRegion() const;

    /*! Sets the value of "want-first-maneuver-crossroad" option
    @param[in] m_wantFirstManeuverCrossroad
    */
    void SetWantFirstManeuverCrossroad( bool wantFirstManeuverCrossroad );

    /*! Returns the value of "want-first-maneuver-crossroad" options
    @return m_wantFirstManeuverCrossroad
    */
    bool GetWantFirstManeuverCrossroad() const;

    /*! Sets the value of "want-route-features" option
    @param[in] wantRouteFeature
    */
    void SetWantRouteFeatures( bool wantRouteFeature );

    /*! Returns the value of "want-route-features" options
    @return m_wantRouteFeature
    */
    bool GetWantRouteFeatures() const;

    /*! Sets the value of "want-traffic-delay" option
    @param[in] wantTrafficDelay
    */
    void SetWantTrafficDelay( bool wantRouteFeature );

    /*! Returns the value of "want-traffic-delay" options
    @return m_wantTrafficDelay
    */
    bool GetWantTrafficDelay() const;

private:

    //!If True Client wants WebStyle or Terse Maneuvers
    bool m_SimpleManeuvers;
    //! If True 
    bool m_UnPavedRdInfo;
    //!If True Client wants Bridge Maneuvers
    bool m_BridgeManeuers;
    //!If True Client wants Tunnel Maneuvers
    bool m_TunnelManeuvers;
    //!If True Client wants Ferry Maneuvers
    bool m_FerryManeuvers;
    //!If True Client wants Country crossing prompts 
    bool m_CountryManeuver;
    //!If True Client wants a list of all cross streets  in between 
    bool m_CrossStreet;
    //! If True Client wants enter RoundAbout advisory
    bool m_RoundAbout;
    //! Version of cmd set
    int  m_iCmdSetVersion;
    //! Size of route polyline window. Valid range [1,N]. -1 indicates do not consider this value
    int  m_iRoutePolyWnd;
    //! Size of pronun window. Valid range [1,N]. -1 indicates do not consider this value
    int  m_iPronunWnd;
	//! for VX
	//! If True Client wants SAR
	bool m_wantSAR;
	//! If True Client wants MJO
	bool m_wantMJO;
	//! If True Client wants ECM
	bool m_wantECM;
	//! Natural Guidance
	bool m_wantNaturalGuidance;
    //! Natural Guidance -- want traffic light and stop sign
    bool m_wantTrafficLightAndStopSign;
	//!Lane Guidance
	bool m_wantLaneGuidance;
    //!Exit Number for High way;
    bool m_wantExitNumber;

	//! If True Client wants SpeedLimited
	bool m_wantSpeedLimited;

    //! If True Client wants special-region
    bool m_wantSpecialRegion;

    //! If True Client wants first maneuver cross road
    bool m_wantFirstManeuverCrossroad;

    //! If True Client wants Route features, Traffic delay
    bool m_wantRouteFeatures;
    bool m_wantTrafficDelay;
};
};
#endif
