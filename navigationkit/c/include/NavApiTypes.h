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
    @file     NavApiTypes.h
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __NAVAPINAVIGATIONTYPES_H__
#define __NAVAPINAVIGATIONTYPES_H__

extern "C"
{
#include "nberror.h"
#include "nbcontext.h"
}
#include "smartpointer.h"
#include <string>
#include <vector>

namespace nbnav
{
/*! Non thread safe classes  */

 /*! Navigate Route Error */
typedef enum
{
    NRE_None = 3000,
    NRE_TimedOut = 3001,
    NRE_BadDestination = 3002,
    NRE_BadOrigin = 3003,
    NRE_CannotRoute = 3004,
    NRE_EmptyRoute = 3005,
    NRE_NetError = 3006,
    NRE_UnknownError = 3007,
    NRE_NoMatch = 3008,
    NRE_ServerError = 3009,
    NRE_NoDetour = 3010,
    NRE_PedRouteTooLong = 3011,
    NRE_OriginCountryUnsuppoted = 3013,
    NRE_DestinationCountryUnsupported = 3014,
    NRE_Unauthorized = 4299,
    NRE_BicycleRouteTooLong = 23500,
    NRE_RouteTooLong = 23501,
    NRE_GPSTimeout = 23502,
    NRE_Cancelled = 23503,
    NRE_Internal  = 23504
} NavigateRouteError;

/*! Valid flags set when a location fix is received.

    Used to indicate which fields in the class {@link Location} contain valid data.
    These values are ORed together, so multiple fields can be valid.
*/
typedef enum
{
    None                     = 0,
    Latitude                 = (1<<0),
    Longitude                = (1<<1),
    Altitude                 = (1<<2),
    Heading                  = (1<<3),
    HorizontalVelocity       = (1<<4),
    VerticalVelocity         = (1<<5),
    HorizontalUncertainty    = (1<<6),
    AxisUncertainty          = (1<<7),
    PerpendicularUncertainty = (1<<8),
    VerticalUncertainty      = (1<<9),
    UTCOffset                = (1<<10),
    SatelliteCount           = (1<<11)
} LocationValid;

/*! Route property.
 
 Used for getting the properties of route information.
 */
typedef enum
{
    HOV = 0,      /*!< The route contains at least one HOV segment. */
    HIGHWAY,      /*!< The route contains at least one highway segment. */
    TOLL_ROAD,    /*!< The route contains at least one toll road segment. */
    FERRY,        /*!< The route includes taking a ferry. */
    UNPAVED_ROAD, /*!< The route includes at least one unpaved road segment. */
    ENTER_COUNTRY,/*!< The route contains at lest one enter country segment. */
    PRIVATE_ROAD, /*!< The route contains at lest one private road. */
    GATED_ACCESS_ROAD, /*!< The route contains at lest one gated access road. */
    CONGESTION,   /*!< The route includes at least one segment where there is a traffic congestion. */
    ACCIDENT,     /*!< The route includes at least one segment where there is a traffic accident. */
    CONSTRUCTION  /*!< The route includes at least one segment where there is construction.. */
} RouteProperty;

/*! The destination position enum. */
typedef enum
{
    Center = 0,
    Left,
    Right
} DestinationStreetSide;
    
typedef struct
{
    /*!
     * language locale for the audio clip
     */
    std::string AudioLocale;
    /*!
     * Provides phonetic format for the audio clip
     */
    std::string PhoneticFormat;
    /*!
     * Provides phonetic notation for the audio clip
     */
    std::string PhoneticNotation;
    /*!
     * Provides audio text for the audio clip
     */
    std::string AudioText;
    /*!
     * Provides audio name for the audio clip
     */
    std::string AudioName;
    /*!
     * Provides transliterated text for the audio clip
     */
    std::string TransliteratedText;
} AudioClip;

/*! Location

    The location information interface.
*/
class Location
{
public:
    /*! Gets location altitude.

        @return Altitude in meters
    */
    virtual double Altitude() const = 0;

    /*! Gets location GPS time.

        @return Time in seconds since the GPS Epoch: Midnight, January 6, 1980
    */
    virtual unsigned int GpsTime() const = 0;

    /*! Gets location heading.

        @return Heading in degrees
    */
    virtual double Heading() const = 0;

    /*! Gets standard deviation of horizontal uncertainty along the axis of uncertainty.

        @return Horizontal uncertainty in meters
    */
    virtual double HorizontalUncertaintyAlongAxis() const = 0;

    /*! Gets Standard deviation of horizontal uncertainty perpendicular to the axis of uncertainty.

        @return Horizontal uncertainty in meters
    */
    virtual double HorizontalUncertaintyAlongPerpendicular() const = 0;

    /*! Gets heading in degrees of the axis of uncertainty.

        @return Heading in degrees
    */
    virtual double HorizontalUncertaintyAngleOfAxis() const = 0;

    /*! Gets horizontal velocity in meters/second.

        @return velocity in meters/second
    */
    virtual double HorizontalVelocity() const = 0;

    /*! Gets location latitude in degrees.

        @return Latitude in degrees
    */
    virtual double Latitude() const = 0;

    /*! Gets location longitude in degrees.

        @return Longitude in degrees
   */
    virtual double Longitude() const = 0;

    /*! Gets number of satellites acquired.

        @return Number of satellites
    */
    virtual int NumberOfSatellites() const = 0;

    /*! Gets difference in seconds between GPS time and UTC time (leap seconds).

        @return Difference in seconds
    */
    virtual int UtcOffset() const = 0;

    /*! Gets {@link LocationValid} flags indicating which fields in the class have valid values.

        @return Valid flags
    */
    virtual unsigned int Valid() const = 0;

    /*! Gets standard deviation of vertical uncertainty.

        @return Vertical uncertainty in meters
    */
    virtual double VerticalUncertainty() const = 0;

    /*! Gets vertical velocity in meters/seconds.

        @return Vertical velocity in meters/seconds
    */
    virtual double VerticalVelocity() const = 0;

    /*! Gets is this location is gps fix.
     
        @return {@code true} is gps fix
     */
    virtual bool IsGpsFix() const = 0;
};


/*! struct Coordinates

    Defines a latitude/longitude point on the globe.
*/
typedef struct
{
    double  latitude;      /*!< Latitude in degrees */
    double  longitude;     /*!< Longitude in degrees */
    double  accuracy;      /*!< horizontal uncertainty along axis */
} Coordinates;


/*! struct BoundingBox

    Bounding box in Coordinates that defines a region
*/
typedef struct
{
    Coordinates  point1;   /*!< One corner */
    Coordinates  point2;   /*!< The other corner */
} BoundingBox;


/*! struct MapLocation

    Information about a location. The type, latitude and longitude are required;
    all other values are optional and may be the empty string.
*/
typedef struct
{
    std::string airport;      /*!< airport code, optional */
    std::string freeform;     /*!< freeform address query, optional */
    std::string number;       /*!< street number, optional */
    std::string street;       /*!< street name, optional */
    std::string street2;      /*!< cross-street name, optional */
    std::string city;         /*!< city name, optional */
    std::string county;       /*!< county name, optional */
    std::string state;        /*!< state name, optional */
    std::string postal;       /*!< postal code, optional */
    std::string country;      /*!< country code (ISO 3 letter country code), optional */
    std::string areaname;     /*!< area name or airport name, optional */
    Coordinates center;  /*!< coordinates of the location */
    std::string addressLine1;
    std::string addressLine2;
} MapLocation;


/*! struct Category

    Information about a search category
*/
typedef struct
{
    std::string    code;     /*!< Category code */
    std::string    name;     /*!< Category name */
} Category;


/*! Type of phone number
*/
typedef enum
{
    Primary = 0,  /*!< Primary phone. */
    Secondary,    /*!< Secondary phone. */
    National,     /*!< National phone. */
    Mobile,       /*!< Mobile phone. */
    Fax           /*!< Fax. */
} PhoneType;


/*! struct Phone

    Phone number information
*/
typedef struct
{
    PhoneType  type;      /*!< Kind of phone contact */
    std::string     country;   /*!< Country code */
    std::string     area;      /*!< Area code */
    std::string     number;    /*!< Actual phone number */
} Phone;


/*! Font for formatted text
*/
typedef enum
{
    Normal = 0,  /*!< Normal font. */
    Bold,        /*!< Bold font. */
    Large,       /*!< Large font. */
    LargeBold    /*!< Large bold font. */
} Font;


/*! struct FormattedText

    Formatted text.
*/
typedef struct
{
    Font font;            /*!< Font to display the text in. */
    unsigned int color;   /*!< Color to display the text in. */
    std::string text;          /*!< Text to be displayed. */
    bool newline;         /*!< if {@code true} a newline should follow the text. */
} FormattedText;


/*! Image types
*/
typedef enum
{
    PNG,     /*!< PNG */
    JPEG,    /*!< JPEG */
    BMP      /*!< BMP */
} ImageType;

/*! FormattedTextBlock

    Represents a formatted text block - a collection of formatted texts
    @see FormattedText
*/
class FormattedTextBlock
{
public:
    /*! Returns the number of formatted blocks present.  */
    int GetFormattedTextCount();

    /*! Returns the formattedblock at this index.

        @param index Index
        @returns NULL formattedText for invalid index.
    */
    FormattedText GetFormattedText(int index);

    /*! Adds a formatted block to this instance.

        @param formattedBlock Formatted block to add.
        @return None
    */
    void AddFormattedText(FormattedText formattedBlock);

private:
    std::vector<FormattedText> m_texts;
};

/*! class BinaryBuffer

    A type describing an binary buffer
*/
class BinaryBuffer
{
public:
    /*! Constructor. For internal use only. */
    BinaryBuffer(unsigned char* data, size_t length);

    /*! Gets data as bytes array.

        @return binary data.
    */
    const std::vector<unsigned char>& GetData() const;

private:
    std::vector<unsigned char> m_data;
};


/*! class Image

    A type describing an image
*/
class Image : public BinaryBuffer
{
public:
    /*! Constructor. For internal use only.   */
    Image(unsigned char* data, size_t length, ImageType type);

    /*! Gets image type.

        @return image type.
    */
    ImageType GetType() const;

private:
    ImageType m_type;   /*!< Type of image */
};


/*! class Place

    Represents a basic place.
*/
class Place
{
public:
    /*! Creates an empty place. */
    Place();

    /*! Creates a place with the given location.

        @param location The location for which the user wants to create a place.
    */
    Place(const MapLocation& location);

    /*! Creates a place with the given name and map location.

        @param name The name of the place.
        @param mapLocation The map location from which the user wants to create a place.
    */
    Place(const std::string& name, const MapLocation& mapLocation);

    /*! Gets place location

        @return place location.
    */
    MapLocation GetLocation() const;

    /*! Sets place location.

        @param mapLocation {@link MapLocation} object to set.
        @return None
    */
    void SetLocation(const MapLocation& mapLocation);

    /*! Gets number of phone numbers available for this place.

        @return phone numbers count
    */
    int GetPhoneNumberCount() const;

    /*! Clears all the phone numbers associated with this place.

        @return None
    */
    void ClearPhoneNumbers();

    /*! Gets the phone number at the given index.

        @param index The 0 based index of the phone number.
        @param phoneNumber Phone structure. Result will be stored here.
        @return {@link NBI_Error} error code.
    */
    NB_Error GetPhoneNumber(int index, Phone& phoneNumber) const;

    /*! Adds a new phone number to this place.

        @param phoneNumber Phone number to be added.
        @return None
    */
    void AddPhoneNumber(const Phone& phoneNumber);

    /*! Gets the number of categories associated for this place.

        @return number of categories
    */
    int GetCategoriesCount() const;

    /*! Clears all the Categories

        @return None
    */
    void ClearCategories();

    /*! Returns the Category at the given index.

        @param index The 0 based index of the Category
        @param category Category structure. Result will be stored here.
        @return {@link NBI_Error} error code.
    */
    NB_Error GetCategory(int index, Category& category) const;

    /*! Adds a new category to this place.

        @param category Category to add
        @return None
    */
    void AddCategory(const Category& category);

    /*! Returns place name

        @return place name
    */
    std::string GetName() const;

    /*! Sets place name

        @param name the name to set
        @return None
    */
    void SetName(const std::string& name);

private:
    std::string           m_name;
    MapLocation           m_location;
    std::vector<Category> m_categories;
    std::vector<Phone>    m_phones;

};

class NavigationImpl;
class ManeuverImpl;

/*! Represents a navigation maneuver. */
class Maneuver
{
public:

    /*! Gets unique ID identifying the maneuver.

        @return the maneuverId
    */
    int GetManeuverID() const;

    /*! Returns the coordinates on route polyline for this maneuver

        @return vector of polyline
    */
    const std::vector<Coordinates>& GetPolyline() const;

    /*! Gets the string to be used to draw the routing icon using the TT guidance font.

        @return string
    */
    std::string GetRoutingTTF() const;

    /*! Returns distance.

        @return distance value in meters
    */
    double GetDistance() const;

    /*! Returns primary street.

        @return primary street string
    */
    std::string GetPrimaryStreet() const;

    /*! Returns secondary description.

        @return the second street name
    */
    std::string GetSecondaryStreet() const;

    /*! Returns the maneuver actual time

        @return the actual maneuver time
    */
    double GetTime() const;

    /*! Gets commmand

        @return commang name
    */
    std::string GetCommand() const;

    /*! Gets point coordinates

        @return point coordinates
    */
    Coordinates GetPoint() const;

    /*! Gets formatted maneuver text

        @param isMetric set to {@code true} to indicate that metric system is used
        @return {@link FormattedText} object
    */
    FormattedTextBlock GetManeuverText(bool isMetric) const;

    /*! Returns maneuver description.

        @param isMetric
        @param isFormatted
        @return maneuver description string
    */
    std::string GetDescription(bool isMetric, bool isFormatted) const;

    /*! Returns the maneuver delay time

        @return the total delay
    */
    double GetTrafficDelay() const;


    /*! Returns true if the next maneuver should be stacked

        @return the stack advise
    */
    bool GetStackAdvise() const;

    /*! Gets exit number

        @return exit number text
    */
    std::string GetExitNumber() const;

    /*! Get if this maneuver is destination or not

        @return true if it is a destination maneuver
     */
    bool IsDestination() const;
    
    /*! Get maneuver properties.
     
     Get maneuver properties which this maneuver contains.
     
     @return maneuverProperty array of this route.
     */
    std::vector<RouteProperty> GetManeuverProperties() const;

    /*! Returns the maneuver limited speed

     @return the maneuver limited speed
     */
    double GetSpeed() const;
    
    double GetHeading() const;

    /*! This constructor is for internal use only.

        @param impl
    */
    Maneuver(shared_ptr<ManeuverImpl> impl);

private:
    shared_ptr<ManeuverImpl> m_maneuver;
};

/*! Represents a list of navigation maneuvers. */
class ManeuverList
{
public:
    /*! Gets route origin place.

        @return route origin {@link Place} object.
    */
    Place GetOrigin() const;

    /*! Gets route destination place.

        @return rout destination {@link Place} object.
    */
    Place GetDestination() const;

    /* Returns the maneuver requested from the list

       @param index Index of maneuver in the list (0-based).
       @return Pointer to new {@link Maneuver} object. NULL if error happens.
    */
    const Maneuver* GetManeuver(unsigned int index) const;

    /*! Returns the number of maneuvers available in the route.

        @return number of maneuvers
    */
    unsigned int GetNumberOfManeuvers() const;

    /*! Returns the total trip delay

        @return total trip delay
     */
    double GetTotalDelay() const;

    /*! Returns the total trip distance

        @return total trip distance
    */
    double GetTotalDistance() const;

    /*! Returns the total trip time

        @return total trip time
    */
    double GetTotalTripTime() const;

     /*!Returns a bounding box that encloses the polyline for all the maneuvers in the list

        @return bounding box
    */
    BoundingBox GetRouteBoundingBox() const;

    /*! This constructor is for internal use only.   */
    ManeuverList(NavigationImpl* impl, unsigned int routeIndex, bool isOnlyUpComming = false, bool isCollapManeuver = true);
    
    void Update(NavigationImpl* impl, unsigned int routeIndex, bool isOnlyUpComming, int maneuverIndex);

private:
    double                m_routeDistance;
    unsigned long         m_routeTime;
    unsigned long         m_routeDelay;
    Place                 m_origin;
    Place                 m_destination;
    BoundingBox           m_boundingBox;
    std::vector<Maneuver> m_maneuvers;
};

class SpeechSynthesis
{
public:
    void init(std::string local, vector<AudioClip>& audioClip);
    std::string GetLocal();
    std::string GetMarkup();
private:
    std::string GetFormatedString(std::string str);
    void appendText(std::string text, std::string local, std::string& result);
    std::string m_local;
    std::string m_markup;
};
/*! Provides audio data to be played, both audio files as well as the audio text that can be fed to TTS engine. */
class Announcement
{
public:
    /*! Provides the output stream for the announcement audio data to be played.

        @return audio buffer
     */
    virtual BinaryBuffer GetAudio() const = 0;

    /*! Client must call this method to let NavKit know that this announcement started.

        @return None
    */
    virtual void OnAnnouncementStart() const = 0;

    /*! Client must call this method to let NavKit know that this announcement has been played or
        client does it before client destroys Navigation

        @return None
    */
    virtual void OnAnnouncementComplete() = 0;

    /*! Audio text that can be used for playing the announcement using TTS engine.
        The text maybe contains '{number}' which means the TTS audio engine should break the long announcemt
        at the place of '{number}' in 'number' seconds in order to improve the customer feeling.

        @return text
    */
    virtual std::string GetText() const = 0;

    /*! Provides the mime type for announcement being provided

       @return audioMimetype
    */
    virtual std::string GetAudioMimeType() const = 0;
    
    /*! Get Audio clips. */
    virtual const std::vector<AudioClip> GetAudioClip() const = 0;

    /*! Get speech synthesis. */
    virtual SpeechSynthesis& GetSpeechSynthesis() = 0;

    /*! Destructor. */
    virtual ~Announcement() {}
};
    
class AnnouncementImpl
    : public Announcement
{
public:
    virtual BinaryBuffer GetAudio() const;
    virtual void OnAnnouncementStart() const;
    virtual void OnAnnouncementComplete();
    virtual std::string GetText() const;
    virtual std::string GetAudioMimeType() const;
    virtual const std::vector<AudioClip> GetAudioClip() const;
    virtual SpeechSynthesis& GetSpeechSynthesis();

    virtual ~AnnouncementImpl();

    /*! Costructor.

        For internal use only.

        @param impl
        @param _data
     */
    AnnouncementImpl(const NavigationImpl& impl, void *data);

    AnnouncementImpl(const Announcement* announcement);
private:
    void*            m_data;
    BinaryBuffer     m_audioData;
    std::string      m_text;
    std::string      m_phonetic;
    std::string      m_audioMimeType;
    PAL_Instance*    m_pal;
    shared_ptr<bool> m_isValid;
    std::vector<AudioClip> m_audioClip;
    SpeechSynthesis  m_speechSynth;
};

/*! Provides information for rendering the road sign */
class RoadSign
{
public:
    /*! Provides sign ID

        @return sign ID
    */
    std::string SignInformation() const;

    /*! Provides Road Sign image Bitmap

        @return Image data structure.
    */
    const Image& GetImageData() const;

    /*! Provides portrait sign ID

        @return sign ID
     */
    std::string PortraitSignInformation() const;

    /*! Provides portrait road sign image bitmap

        @return Image data structure.
     */
    Image GetPortraitImageData() const;

    /*! Costructor.

        For internal use only.

        @param impl
        @param data
    */
    RoadSign(const NavigationImpl& impl, const void* data);
    RoadSign();

private:
    std::string m_signInformation;
    Image       m_imageData;
    std::string m_portraitInformation;
    Image       m_portraitImageData;
};
    
/*! Waypoint. */
typedef struct
{
    Place location;
    bool isStopPoint;
} WayPoint;

}

#endif

/*! @} */
