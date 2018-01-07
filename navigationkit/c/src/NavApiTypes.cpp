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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "NavApiTypes.h"
#include "NavigationImpl.h"
#include "NavApiNavUtils.h"
#include "ManeuverImpl.h"
extern "C"
{
#include "nberror.h"
#include "nbcontext.h"
#include "nbrouteinformation.h"
#include "nbnavigation.h"
#include "nbspatial.h"
#include "nbcontextaccess.h"
#include "abpalaudio.h"
#include "cslcache.h"
#include "palfile.h"
}

namespace nbnav
{
const int bufferSize = 200;
const int bufferSizeLarge = 512;
const char* toneFlag = "-tone";



//////////////////////////////////////////////////////
//  class Maneuver
//////////////////////////////////////////////////////

Maneuver::Maneuver(shared_ptr<ManeuverImpl> impl)
    : m_maneuver(impl)
{
}

int Maneuver::GetManeuverID() const
{
    return m_maneuver->GetManeuverID();
}

const vector<Coordinates>& Maneuver::GetPolyline() const
{
    return m_maneuver->GetPolyline();
}

string Maneuver::GetRoutingTTF() const
{
   return m_maneuver->GetRoutingTTF();
}

double Maneuver::GetDistance() const
{
    return m_maneuver->GetDistance();
}

string Maneuver::GetPrimaryStreet() const
{
    return m_maneuver->GetPrimaryStreet();
}

string Maneuver::GetSecondaryStreet() const
{
    return m_maneuver->GetSecondaryStreet();
}

double Maneuver::GetTime() const
{
    return m_maneuver->GetTime();
}

string Maneuver::GetCommand() const
{
    return m_maneuver->GetCommand();
}

Coordinates Maneuver::GetPoint() const
{
    return m_maneuver->GetPoint();
}

string Maneuver::GetDescription(bool isMetric, bool isFormatted) const
{
    return m_maneuver->GetDescription(isMetric, isFormatted);
}

FormattedTextBlock Maneuver::GetManeuverText(bool isMetric) const
{
    return m_maneuver->GetManeuverText(isMetric);
}

double Maneuver::GetTrafficDelay() const
{
    return m_maneuver->GetTrafficDelay();
}

bool Maneuver::GetStackAdvise() const
{
    return m_maneuver->GetStackAdvise();
}

string Maneuver::GetExitNumber() const
{
    return m_maneuver->GetExitNumber();
}

bool Maneuver::IsDestination() const
{
    return m_maneuver->IsDestination();
}
    
std::vector<RouteProperty> Maneuver::GetManeuverProperties() const
{
    return m_maneuver->GetManeuverProperties();
}

double Maneuver::GetSpeed() const
{
    return m_maneuver->GetSpeed();
}
    
double Maneuver::GetHeading() const
{
    const vector<Coordinates>& polyline = m_maneuver->GetPolyline();
    if(polyline.size() > 1)
    {
        Coordinates point1 = polyline[polyline.size()-1];
        Coordinates point2 = polyline[polyline.size()-2];
        double heading;
        NB_SpatialGetLineOfSightDistance(point2.latitude, point2.longitude, point1.latitude, point1.longitude, &heading);
        return heading;
    }
    else
    {
        return 0;
    }
}

//////////////////////////////////////////////////////
//  class ManeuverList
//////////////////////////////////////////////////////

ManeuverList::ManeuverList(NavigationImpl* impl, unsigned int routeIndex, bool isOnlyUpComming, bool isCollapManeuver)
{
    NB_RouteInformation* routeInfo = impl->GetNbRoute(routeIndex);
    
    uint32 count = NB_RouteInformationGetManeuverCount(routeInfo);
    uint32 startManeuver = (isOnlyUpComming && impl->GetClosestManeuver() != NAV_MANEUVER_NONE &&
                            impl->GetClosestManeuver() != NAV_MANEUVER_START) ? impl->GetClosestManeuver() : 0;
    uint32 nextStartIndex = startManeuver;
    double firstManeuverDistance = 0;
    if(isCollapManeuver)
    {
        for (uint32 i = startManeuver; i < count; ++i)
        {
            ManeuverImplPtr tmp = ManeuverImplPtr(new ManeuverImpl(impl, i, i, routeIndex, 0));
            if (m_maneuvers.size() == 0)
            {
                ManeuverImplPtr maneuverImpl = ManeuverImplPtr(new ManeuverImpl(impl, nextStartIndex, i, routeIndex, -firstManeuverDistance, FALSE));
                //skip the first NC. maneuver when NC maneuver is disabled
                if (maneuverImpl->GetCommand().compare("NC.") == 0 && !impl->IsNCEnabled())
                {
                    firstManeuverDistance += maneuverImpl->GetDistance();
                    continue;
                }
                m_maneuvers.push_back(maneuverImpl);
                nextStartIndex = i+1;
                firstManeuverDistance = 0;
            }
            else if (NB_RouteInformationIsManeuverSignificantForTurnLists(routeInfo, i, i == startManeuver))
            {
                // need to subtract the first maneuver's distance from the second maneuver
                // when first maneuver is not a significant
                ManeuverImplPtr maneuverImpl = ManeuverImplPtr(new ManeuverImpl(impl, nextStartIndex,i, routeIndex, -firstManeuverDistance, FALSE));
                nextStartIndex = i+1;
                m_maneuvers.push_back(maneuverImpl);
                firstManeuverDistance = 0;
            }
            else
            {
                ManeuverImplPtr tmp = ManeuverImplPtr(new ManeuverImpl(impl, i, i, routeIndex, 0, FALSE));
                firstManeuverDistance += tmp->GetDistance();
            }
        }
    }
    else
    {
        for(uint32 i = 0; i < count; i++)
        {
            ManeuverImplPtr maneuverImpl = ManeuverImplPtr(new ManeuverImpl(impl, i, i, routeIndex, 0, FALSE));
            m_maneuvers.push_back(maneuverImpl);
        }
    }

    if (count > 0)
    {
        NB_LatitudeLongitude min, max;
        if (NB_RouteInformationGetRouteExtent(routeInfo, 0, count, &min, &max) == NE_OK)
        {
            m_boundingBox.point1.latitude = min.latitude;
            m_boundingBox.point1.longitude = min.longitude;
            m_boundingBox.point2.latitude = max.latitude;
            m_boundingBox.point2.longitude = max.longitude;
        }
    }

    NB_Place nbPlaceDestination;
    nsl_memset(&nbPlaceDestination, 0, sizeof(nbPlaceDestination));
    NB_RouteInformationGetDestination(routeInfo, &nbPlaceDestination);
    m_destination = NbPlaceToPlace(nbPlaceDestination);

    NB_Place nbPlaceOrigin;
    nsl_memset(&nbPlaceOrigin, 0, sizeof(nbPlaceOrigin));
    NB_RouteInformationGetOrigin(routeInfo, &nbPlaceOrigin);
    m_origin = NbPlaceToPlace(nbPlaceOrigin);

    NB_RouteInformationGetSummaryInformation(routeInfo, reinterpret_cast<uint32*>(&m_routeTime),
                                             reinterpret_cast<uint32*>(&m_routeDelay), &m_routeDistance);
}
    
void ManeuverList::Update(NavigationImpl* impl, unsigned int routeIndex, bool isOnlyUpComming, int maneuverIndex)
{
    impl->Lock();
    NB_RouteInformation* routeInfo = impl->GetNbNavRouteInfo();
    m_maneuvers.clear();
    uint32 count = NB_RouteInformationGetManeuverCount(routeInfo);
    uint32 startManeuver = maneuverIndex;
    uint32 nextStartIndex = startManeuver;
    double firstManeuverDistance = 0;
    for (uint32 i = startManeuver; i < count; ++i)
    {
        ManeuverImplPtr tmp = ManeuverImplPtr(new ManeuverImpl(impl, i, i, routeIndex, 0));
        if (m_maneuvers.size() == 0)
        {
            ManeuverImplPtr maneuverImpl = ManeuverImplPtr(new ManeuverImpl(impl, nextStartIndex, i, routeIndex, 0, FALSE));
            
            //skip the first NC. maneuver when NC maneuver is disabled
            if (maneuverImpl->GetCommand().compare("NC.") == 0 && !impl->IsNCEnabled())
            {
                firstManeuverDistance += maneuverImpl->GetDistance();
                continue;
            }
            m_maneuvers.push_back(maneuverImpl);
            nextStartIndex = i+1;
            if (!NB_RouteInformationIsManeuverSignificantForTurnLists(routeInfo, i,i == startManeuver))
            {
                firstManeuverDistance += maneuverImpl->GetDistance();
            }
        }
        else if (NB_RouteInformationIsManeuverSignificantForTurnLists(routeInfo, i,i == startManeuver))
        {
            // need to subtract the first maneuver's distance from the second maneuver
            // when first maneuver is not a significant
            ManeuverImplPtr maneuverImpl = ManeuverImplPtr(new ManeuverImpl(impl, nextStartIndex,i, routeIndex, firstManeuverDistance));
            nextStartIndex = i+1;
            m_maneuvers.push_back(maneuverImpl);
            firstManeuverDistance = 0;
        }
    }
    
    if (count > 0)
    {
        NB_LatitudeLongitude min, max;
        if (NB_RouteInformationGetRouteExtent(routeInfo, 0, count, &min, &max) == NE_OK)
        {
            m_boundingBox.point1.latitude = min.latitude;
            m_boundingBox.point1.longitude = min.longitude;
            m_boundingBox.point2.latitude = max.latitude;
            m_boundingBox.point2.longitude = max.longitude;
        }
    }
    
    NB_RouteInformationGetSummaryInformation(routeInfo, reinterpret_cast<uint32*>(&m_routeTime),
                                             reinterpret_cast<uint32*>(&m_routeDelay), &m_routeDistance);
    impl->UnLock();
}

const Maneuver* ManeuverList::GetManeuver(unsigned int index) const
{
    if (index >= m_maneuvers.size())
    {
        return NULL;
    }

    return &m_maneuvers[index];
}

unsigned int ManeuverList::GetNumberOfManeuvers() const
{
    return (unsigned int)m_maneuvers.size();
}

Place ManeuverList::GetOrigin() const
{
    return m_origin;
}

Place ManeuverList::GetDestination() const
{
    return m_destination;
}

double ManeuverList::GetTotalDelay() const
{
   return m_routeDelay;
}

double ManeuverList::GetTotalDistance() const
{
    return m_routeDistance;
}

double ManeuverList::GetTotalTripTime() const
{
    return m_routeTime;
}

BoundingBox ManeuverList::GetRouteBoundingBox() const
{
    return m_boundingBox;
}

//////////////////////////////////////////////////////
//  class FormatTextBlock
//////////////////////////////////////////////////////

int FormattedTextBlock::GetFormattedTextCount()
{
    return (int)m_texts.size();
}

FormattedText FormattedTextBlock::GetFormattedText(int index)
{
    return m_texts.at(index);
}

void FormattedTextBlock::AddFormattedText(FormattedText formattedBlock)
{
    m_texts.push_back(formattedBlock);
}

//////////////////////////////////////////////////////
//  class BinaryBuffer
//////////////////////////////////////////////////////

BinaryBuffer::BinaryBuffer(unsigned char* data, size_t length)
{
    if (data != NULL && length > 0)
    {
        m_data.insert(m_data.begin(), data, data + length);
    }
}

const vector<unsigned char>& BinaryBuffer::GetData() const
{
    return m_data;
}

//////////////////////////////////////////////////////
//  class Image
//////////////////////////////////////////////////////

Image::Image(unsigned char* data, size_t length, ImageType type)
    : BinaryBuffer(data, length)
{
}

ImageType Image::GetType() const
{
    return m_type;
}

class AnnouncementReleaseData
{
public:
    NB_GuidanceMessage* message;
    shared_ptr<bool>    isValid;
};

//////////////////////////////////////////////////////
//  class Announcement
//////////////////////////////////////////////////////
AnnouncementImpl::AnnouncementImpl(const NavigationImpl& impl, void *data)
    : m_data(data),
      m_audioData(0,0),
      m_text(),
      m_phonetic(),
      m_audioMimeType()
{
    if (!data)
    {
        return;
    }
    NB_GuidanceMessage* message = static_cast<NB_GuidanceMessage*>(m_data);
    m_pal = impl.GetPal();
    m_isValid = impl.GetValidPointer();

    int codeCount = NB_GuidanceMessageGetCodeCount(message);
    const char* code = NULL;
    char textBuffer[bufferSize] = {0};
    uint32 textBufferSize = bufferSize;
    byte* phoneticsData = static_cast<byte*>(nsl_malloc(bufferSize));
    uint32 phoneticsDataSize = bufferSize;
    char combineCode[bufferSize] = {0};
    char language[bufferSize] = {0};
    uint32 languageSize = bufferSize;
    char translatedText[bufferSize] = {0};
    uint32 translatedTextSize = bufferSize;
    int combinedCount = 0; // count of code which has been combined from beginning.
    int uncombinedAt = 0; // current index of uncombined code from ending.
    CSL_Cache* pVoiceCache = NB_ContextGetVoiceCache(impl.GetNbContext());
    bool isDataFound = false;
    ABPAL_AudioCombiner* pCombiner = NULL;
    if (!impl.IsTtsEnabled())
    {
        ABPAL_AudioCombinerCreate(impl.GetPal(), ABPAL_AudioFormat_AAC, &pCombiner);
    }
    for (int i = 0; i < codeCount && combinedCount < codeCount; ++i)
    {
        nsl_memset(combineCode, 0, bufferSize);
        uncombinedAt = 0;
        for (int j = combinedCount; i < codeCount && j < codeCount; ++j)
        {
            if (combineCode[0] != 0)
            {
                nsl_strlcat(combineCode, "+", sizeof(combineCode));
            }
            code = NB_GuidanceMessageGetCode(message, j);
            nsl_strlcat(combineCode, code, sizeof(combineCode));
        }

        for (int j = codeCount; j > i; --j)
        {
            isDataFound = false;
            nb_boolean isBaseAudio = false;
            textBufferSize = bufferSize;
            phoneticsDataSize = bufferSize;
            nsl_memset(&textBuffer, 0, sizeof(textBuffer));
            nsl_memset(phoneticsData, 0, bufferSize);
            nsl_memset(&language, 0, bufferSize);
            nsl_memset(&translatedText, 0, bufferSize);

            if (impl.IsTtsEnabled())
            {
                if (NE_OK == NB_NavigationGetPronunInformation(impl.GetNbNavigation(), combineCode,
                                                               textBuffer, &textBufferSize,
                                                               phoneticsData, &phoneticsDataSize,
                                                               language, &languageSize,
                                                               translatedText, &translatedTextSize,
                                                               NULL,&isBaseAudio))
                {
                    isDataFound = true;
                    combinedCount = codeCount - uncombinedAt;
                    std::string temp = NavUtils::ConvertFormatStringToPlainString(textBuffer);
                    if(temp.length() == 0)
                    {
                        break;
                    }
                    AudioClip clip;
                    if(isBaseAudio)
                    {
                        clip.AudioText = temp;
                    }
                    else
                    {
                        clip.AudioText = textBuffer;
                    }
                    clip.AudioLocale = language;
                    if(clip.AudioLocale.empty())
                    {
                        clip.AudioLocale = impl.GetLocal();
                    }
                    clip.AudioName = combineCode;
                    clip.PhoneticFormat = "ipa";
                    clip.PhoneticNotation.assign((char*)phoneticsData, phoneticsDataSize);
                    clip.TransliteratedText = translatedText;
                    m_audioClip.push_back(clip);

                    if (i > 0)
                    {
                        m_text += " ";
                        m_phonetic += " ";
                    }
                    i = j - 1;

                    if (phoneticsDataSize)
                    {
                        m_phonetic.append((char*)phoneticsData, phoneticsDataSize);
                    }
                    else if (textBufferSize)
                    {
                        m_phonetic.append(textBuffer);
                    }
                    if (textBufferSize)
                    {
                        m_text.append(textBuffer);
                    }
                    break;
                }
            }
            else
            {
                void* pData = NULL;
                size_t dataSize = 0;
                if(CSL_CacheFind(pVoiceCache, (byte*)combineCode, nsl_strlen(combineCode),
                                 (byte**)&pData, &dataSize, TRUE, TRUE, FALSE) != NE_OK)
                {
                    string voiceFilePath(impl.GetBaseVoicesPath() + combineCode + ".aac");
                    if (voiceFilePath.length() < PAL_FILE_MAX_NAME_LENGTH / 2)
                    {
                        PAL_FileLoadFile(impl.GetPal(), voiceFilePath.c_str(), (unsigned char**)&pData, (uint32*)&dataSize);
                    }
                }
                if (pData && dataSize > 0)
                {
                    isDataFound = true;
                    combinedCount = codeCount - uncombinedAt;
                    if (pCombiner)
                    {
                        (void)ABPAL_AudioCombinerAddBuffer(pCombiner, (byte*)pData, (int)dataSize, true);
                    }
                    nsl_free(pData);
                    pData = NULL;
                    break;
                }
            }

            if (!isDataFound)
            {
                size_t len = nsl_strlen(combineCode);
                while (combineCode[len] != '+' && len > 0)
                {
                    --len;
                    if (combineCode[len] == '+')
                    {
                        uncombinedAt++;
                        combineCode[len] = '\0';
                        break;
                    }
                }
            }
        }
    }
    if (pCombiner && !impl.IsTtsEnabled())
    {
        byte* buffer = 0;
        int bufferSize = 0;

        (void) ABPAL_AudioCombinerGetData(pCombiner, &buffer, &bufferSize);
        if (buffer)
        {
            BinaryBuffer tempBuffer(buffer, bufferSize);
            m_audioData = tempBuffer;
            nsl_free(buffer);
        }
        (void) ABPAL_AudioCombinerDestroy(pCombiner);
        m_audioMimeType = "audio/aac";
    }
    if (impl.IsTtsEnabled())
    {
        // TODO Check audio mime type for TTS
        m_audioMimeType = "ipa";
    }

    m_text = NavUtils::ConvertFormatStringToPlainString(m_text);
    // check if guidance message is tone.
    if (codeCount == 1 &&
        m_text.empty() &&
        m_phonetic.empty())
    {
        code = NB_GuidanceMessageGetCode(message, 0);
        if (nsl_stristr(code, toneFlag) != NULL)
        {
            if (m_audioData.GetData().empty())
            {
                byte* pData = NULL;
                size_t dataSize = 0;
                if(CSL_CacheFind(pVoiceCache, (byte*)code, nsl_strlen(code),
                                 &pData, &dataSize, TRUE, FALSE, FALSE) == NE_OK)
                {
                    BinaryBuffer tempBuffer(pData, dataSize);
                    m_audioData = tempBuffer;

                    // don't need free pData here for makeCopy of CSL_CacheFind is FALSE
                    pData = NULL;
                }
            }
            m_audioMimeType = "tone/aac";
        }
    }
    m_speechSynth.init(impl.GetLocal(), m_audioClip);
    nsl_free(phoneticsData);
}

SpeechSynthesis& AnnouncementImpl::GetSpeechSynthesis()
{
    return m_speechSynth;
}

void SpeechSynthesis::init(std::string local, vector<AudioClip>& audioClip)
{
    char tmpBuf[bufferSizeLarge] = {0};
    std::string tmpMarkup;
    m_local = local;
    for(size_t i = 0; i < audioClip.size(); i++)
    {
        AudioClip& clip = audioClip[i];
        //pause
        if(clip.AudioName == "pause")
        {
            tmpMarkup.append("<break/>");
        }
        else if(!clip.PhoneticNotation.empty()) //contain phonetic
        {
            sprintf(tmpBuf, "<phoneme alphabet=\"%s\" ph=\"%s\"/>",clip.PhoneticFormat.c_str(), clip.PhoneticNotation.c_str());
            tmpMarkup.append(tmpBuf);
        }
        else if(clip.AudioText.find("<speak xml") != string::npos) //markup string
        {
            std::string format = GetFormatedString(clip.AudioText);
            if(i > 0 && tmpMarkup.size()>0 && tmpMarkup[tmpMarkup.size()-1] != '>' && !format.empty() && format[format.size()] != '<')
            {
                tmpMarkup.append(" ");
            }
            tmpMarkup.append(format);
        }
        else
        {
            if(i > 0 && tmpMarkup[tmpMarkup.size()-1] != '>')
            {
               tmpMarkup.append(" ");
            }
            tmpMarkup.append(clip.AudioText);
        }
    }
    sprintf(tmpBuf, "<speak xml:lang=\"%s\">%s</speak>", m_local.c_str(), tmpMarkup.c_str());
    m_markup = tmpBuf;
}

std::string SpeechSynthesis::GetFormatedString(std::string str)
{
    const std::string speakStart = "<speak xml:lang=\"";
    const std::string speakEnd = "</speak>";
    const std::string languageEnd = "\"";
    const std::string langStart = "<lang xml:lang=\"";
    const std::string langEnd = "</lang>";
    std::string tmpString = str;
    std::string text;
    std::string currentLocal;
    std::string result;
    //find first markup language
    string::size_type currentPos = 0;
    while(currentPos < tmpString.size())
    {
        if(tmpString.find(speakStart, currentPos) == 0)  //speak
        {
            //move current pos to language start
            currentPos = currentPos + speakStart.size();
            string::size_type end = tmpString.find(languageEnd, currentPos);
            if(end != string::npos)
            {
                //get language
                currentLocal = tmpString.substr(currentPos, end - currentPos);
                currentPos = end + languageEnd.size() + 1;
                end = tmpString.find(speakEnd, currentPos);
                if(end != string::npos)
                {
                    tmpString = tmpString.substr(0, end); //remove </speak>
                }
            }
            else //format error
            {
                return "";
            }
        }
        else if(tmpString.find(langStart, currentPos) == currentPos)  //lang
        {
            std::string langLocal;
            currentPos = currentPos + langStart.size();
            string::size_type end = tmpString.find(languageEnd, currentPos);
            if(end != string::npos)
            {
                langLocal = tmpString.substr(currentPos, end - currentPos);
                currentPos = end + languageEnd.size() + 1; //move to text start
                end = tmpString.find(langEnd, currentPos);
                if(end != string::npos)
                {
                    std::string langText = tmpString.substr(currentPos, end - currentPos);
                    currentPos = end + langEnd.size();
                    if(langLocal == currentLocal)
                    {
                        text.append(" ");
                        text.append(langText);
                    }
                    else
                    {
                        appendText(text, currentLocal, result);
                        text = "";
                        appendText(langText, langLocal, result);
                    }
                }
                else
                {
                    return "";
                }
            }
            else
            {
                return "";
            }
        }
        else
        {
            if(!currentLocal.empty())
            {
                text.append(tmpString.substr(currentPos, 1));
            }
            currentPos++;
        }
    }
    if(!currentLocal.empty() && !text.empty())
    {
        appendText(text, currentLocal, result);
    }
    return result;
}

void SpeechSynthesis::appendText(std::string text, std::string local, std::string& result)
{
    char formatBuffer[bufferSizeLarge];
    if(text.empty() || local.empty())
    {
        return;
    }
    if(local != m_local)
    {
        sprintf(formatBuffer, "<lang xml:lang=\"%s\">%s</lang>", local.c_str(), text.c_str());
        result.append(formatBuffer);
    }
    else
    {
        result.append(text);
    }
}

std::string SpeechSynthesis::GetLocal()
{
    return m_local;
}

std::string SpeechSynthesis::GetMarkup()
{
    return m_markup;
}

AnnouncementImpl::~AnnouncementImpl()
{
    NB_GuidanceMessage* message = static_cast<NB_GuidanceMessage*>(m_data);

    if (message)
    {
        NB_GuidanceMessageDestroy(message);
        message = NULL;
        m_data = NULL;
    }
}
    
const std::vector<AudioClip> AnnouncementImpl::GetAudioClip() const
{
    return m_audioClip;
}

BinaryBuffer AnnouncementImpl::GetAudio() const
{
    return m_audioData;
}

static void ReleaseGuidanceMessage(PAL_Instance* pal, void* userData)
{
    AnnouncementReleaseData* data = static_cast<AnnouncementReleaseData*>(userData);
    if (data)
    {
        if (data->isValid && (*data->isValid) && data->message)
        {
            NB_GuidanceMessagePlayed(data->message);
        }
        if(data->message)
        {
            NB_GuidanceMessageDestroy(data->message);
        }
        delete data;
    }
}

void AnnouncementImpl::OnAnnouncementStart() const
{
}

void AnnouncementImpl::OnAnnouncementComplete()
{
    NB_GuidanceMessage* message = static_cast<NB_GuidanceMessage*>(m_data);

    if (message)
    {
        AnnouncementReleaseData* releaseData = new AnnouncementReleaseData();
        releaseData->message = message;
        releaseData->isValid = m_isValid;
        message = NULL;
        m_data = NULL;

        TaskId taskId = 0;
        PAL_EventTaskQueueAdd(m_pal, ReleaseGuidanceMessage, releaseData, &taskId);
    }
}

string AnnouncementImpl::GetText() const
{
    return m_text;
}

string AnnouncementImpl::GetAudioMimeType() const
{
    return m_audioMimeType;
}

//////////////////////////////////////////////////////
//  class RoadSign
//////////////////////////////////////////////////////
string RoadSign::SignInformation() const
{
    return m_signInformation;
}

const Image& RoadSign::GetImageData() const
{
    return m_imageData;
}

string RoadSign::PortraitSignInformation() const
{
    return m_portraitInformation;
}

Image RoadSign::GetPortraitImageData() const
{
    return m_portraitImageData;
}

RoadSign::RoadSign()
    : m_imageData(Image(0, 0, PNG)),
      m_portraitImageData(Image(0, 0, PNG))
{
}

RoadSign::RoadSign(const NavigationImpl& impl, const void* data)
    : m_imageData(Image(0, 0, PNG)),
      m_portraitImageData(Image(0, 0, PNG))
{
    const NB_EnhancedContentState* state = static_cast<const NB_EnhancedContentState*>(data);
    if (state)
    {
        NB_RealisticSign realisticSign;
        memset(&realisticSign, 0, sizeof(realisticSign));
        NB_Error err = NB_EnhancedContentStateGetRealisticSign(const_cast<NB_EnhancedContentState*>(state),
                                                               &realisticSign);

        if (err == NE_OK)
        {
            Image tmpImageData(static_cast<unsigned char*>(realisticSign.data),
                               static_cast<size_t>(realisticSign.dataSize), PNG);
            m_imageData = tmpImageData;
            m_signInformation = realisticSign.signId;

            Image tmpPortraitImageData(static_cast<unsigned char*>(realisticSign.portraitData),
                                       static_cast<size_t>(realisticSign.portraitDataSize), PNG);
            m_portraitImageData = tmpPortraitImageData;
            m_portraitInformation = realisticSign.portraitSignId;

            NB_EnhancedContentStateFreeRealisticSign(const_cast<NB_EnhancedContentState*>(state), &realisticSign);
        }
    }
}
}
