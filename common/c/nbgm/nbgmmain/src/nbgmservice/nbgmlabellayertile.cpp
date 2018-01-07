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

/*--------------------------------------------------------------------------

(C) Copyright 2012 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbgmlabellayertile.h"
#include "nbgmpointlabel.h"
#include "nbgmlabellayer.h"
#include "nbgmcommon.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmconst.h"
#include "nbgmroadlabel.h"
#include "nbgmmaproadlabel.h"
#include "nbgmshield.h"
#include "nbgmstaticpoi.h"
#include "nbretypeconvert.h"
#include "nbgmvectortiledata.h"
#include "nbgmlayoutmanager.h"
#include "nbgmstaticlabeltile.h"
#include "nbgmlabeldebugutil.h"
#include "nbgmcustompin.h"
#include "nbgmlabellayer.h"

//true or false stands for this language is or is not Latin-character language, the index of this array is same as g_Langs[]'s index
static bool g_bLatinLangs[] = {
    /* 0 */ false, // UNKNOWN
    /* 1 */ true, // Albanian
    /* 2 */ false, // Arabic
    /* 3 */ true, // Basque
    /* 4 */ false, // Belarusian
    /* 5 */ true, // Belarusian Transliterated
    /* 6 */ true, // Bosnian
    /* 7 */ true, // Bosnian Transliterated
    /* 8 */ false, // Bulgarian
    /* 9 */ true, // Bulgarian Transcribed
    /* 10*/ true, // Bulgarian Transliterated
    /* 11*/ true, // Catalan
    /* 12*/ false, // Chinese (Modern)
    /* 13*/ false, // Chinese (Traditional)
    /* 14*/ true, // Croatian
    /* 15*/ true, // Croatian Transliterated
    /* 16*/ true, // Czech
    /* 17*/ true, // Czech Transliterated
    /* 18*/ true, // Danish
    /* 19*/ true, // Dutch
    /* 20*/ true, // English
    /* 21*/ true, // Estonian
    /* 22*/ true, // Estonian Transliterated
    /* 23*/ true, // Finnish
    /* 24*/ true, // French
    /* 25*/ true, // German
    /* 26*/ true, // Irish Gaelic
    /* 27*/ true, // Galician
    /* 28*/ false, // Greek
    /* 29*/ false, // Modern Greek
    /* 30*/ true, // Greek Transliterated
    /* 31*/ true, // Hungarian
    /* 32*/ true, // Hungarian Transliterated
    /* 33*/ true, // Icelandic
    /* 34*/ true, // Bahasa Indonesia
    /* 35*/ true, // Italian
    /* 36*/ true, // Latvian
    /* 37*/ true, // Latvian Transliterated
    /* 38*/ true, // Lithuanian
    /* 39*/ true, // Lithuanian Transliterated
    /* 40*/ false, // Macedonian
    /* 41*/ true, // Macedonian Transcribed
    /* 42*/ true, // Malaysian
    /* 43*/ true, // Moldovan
    /* 44*/ true, // Moldovan Transliterated
    /* 45*/ false, // Montenegrin
    /* 46*/ true, // Montenegrin Transliterated
    /* 47*/ true, // Norwegian
    /* 48*/ true, // Polish
    /* 49*/ true, // Polish Transliterated
    /* 50*/ true, // Portuguese
    /* 51*/ true, // Pinyin
    /* 52*/ true, // Romanian Transliterated
    /* 53*/ true, // Romanian
    /* 54*/ false, // Russian
    /* 55*/ true, // Russian Transcribed
    /* 56*/ true, // Russian Transliterated
    /* 57*/ false, // Serbian
    /* 58*/ true, // Serbian Transcribed
    /* 59*/ true, // Serbian Transliterated
    /* 60*/ true, // Slovak
    /* 61*/ true, // Slovak Transliterated
    /* 62*/ true, // Slovenian
    /* 63*/ true, // Slovenian Transliterated
    /* 64*/ true, // Spanish
    /* 65*/ true, // Swedish
    /* 66*/ false, // Thai
    /* 67*/ true, // Thai English
    /* 68*/ true, // Turkish Transcribed
    /* 69*/ true, // Turkish
    /* 70*/ true, // Turkish Transliterated
    /* 71*/ false, // Ukrainian
    /* 72*/ true, // Ukrainian Transliterated
    /* 73*/ true, // Welsh
    /* 74*/ true, // ?? Malta in IT5
    /* 75*/ true, // Maltese
    /* 76*/ false, // Kazakh
    /* 77*/ true, // "SCR"
    /* 78*/ false, // "HIN"
    /* 79*/ false, // "JPN"
    /* 80*/ false, // "KOR"
    /* 81*/ false, // "UKT"
    /* 82*/ false, // "TRA"
    /* 83*/ false, // "UND"
    /* 84*/ true, // Western Frisian
    /* 85*/ false, // Armenian
    /* 86*/ false, // Australian languages
    /* 87*/ false,  // AZERBAIJAN
    /* 88*/ true,  // Baltic (Other)
    /* 89*/ true,  // Belarusian Latin
    /* 90*/ true, // Breton
    /* 91*/ true, // Bulgarian (Latin)
    /* 92*/ false, // Burmese
    /* 93*/ true,  // Celtic (Other)
    /* 94*/ true, // Pinyin (Chinese - Latin Character representation)
    /* 95*/ true, // Esperanto
    /* 96*/ true, // Faroese
    /* 97*/ true, // Germanic (Other)
    /* 98*/ false, // Georgian
    /* 99*/ true,  // Gaelic (Scots)
    /*100*/ false, // Greek, Ancient
    /*101*/ true, // Greek (modern Greek - Latin syntax)
    /*102*/ true, // Swiss German
    /*103*/ true, // Hawaiian
    /*104*/ false, // Hebrew
    /*105*/ true, // Croatian
    /*106*/ false, // Khmer
    /*107*/ true, // KOREAN TRANSLITERATED
    /*108*/ false, // Lao
    /*109*/ true, // Latin
    /*110*/ true, // Letzeburgesch
    /*111*/ true, // Maori
    /*112*/ false, // Austronesian (Other)
    /*113*/ true, // MALTESE TRANSLITERATED
    /*114*/ false, // Mongolian
    /*115*/ true, // Mayan Languages
    /*116*/ true, // North American Indian
    /*117*/ true, // Norwegian Nynorsk
    /*118*/ true, // Norwegian Bokm
    /*119*/ true, // Occitan
    /*120*/ false, // Papuan-Australian (Other)
    /*121*/ false, // Persian
    /*122*/ false, // Pushto
    /*123*/ true, // Romance (Other)
    /*124*/ true, // Raeto-Romance
    /*125*/ true, // Russian (Latin Transcription)
    /*126*/ true, // Serbian Latin
    /*127*/ true, // Scots
    /*128*/ true, // Serbian (Cyrillic)
    /*129*/ false, // Slavic
    /*130*/ true, // Lapp (Sami)
    /*131*/ true, // Inari Sami
    /*132*/ true, // Sardinian
    /*133*/ false, // Serbian
    /*134*/ false, // Swahili
    /*135*/ false, // Tamil
    /*136*/ true, // Tagalog
    /*137*/ true, // Thai Latin
    /*138*/ false, // Tibetan
    /*139*/ true, // Ukranian (Latin)
    /*140*/ false, // Urdu
    /*141*/ false, // Uzbek
    /*142*/ true, // Valencian
    /*143*/ true, // Vietnamese
    /*144*/ true, // Sorbian languages
    /*145*/ false, // Yiddish
    /*146*/ true, // Zulu
    /*147*/ false //Invalid
};


static bool IsEnglishLikeCode(uint32 code)
{
    return (code >= sizeof(g_bLatinLangs)/sizeof(bool))?false:g_bLatinLangs[code];
}

static bool IsEnglishCode(uint32 code)
{
    return code == 20;
}

static uint32 ConvertMaterialFontId(uint32 id)
{
    return id & 0xffff7fff;
}

NBGM_LabelLayerTile::NBGM_LabelLayerTile(NBGM_Context* nbgmContext, const NBGM_LabelLayer* layer, const NBRE_String& id, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, NBRE_Overlay* /*textOverlay*/,
                                         const NBGM_LabelLayerPriority* priority, const NBRE_Point3d& tileCenter, nb_boolean useNavRoadLabel, uint8 labelDrawOrder, uint8 baseDrawOrder, int32 subDrawOrder, const NBRE_String& materialCategoryName)
    :mNBGMContext(nbgmContext)
    ,mLayer(layer)
    ,mSubView(subView)
    ,mOverlayManager(overlayManager)
    ,mNode(NBRE_NodePtr(NBRE_NEW NBRE_Node()))
    ,mSubNode(NBRE_NodePtr(NBRE_NEW NBRE_Node()))
    ,mId(id)
    ,mStaticLabelTile(NULL)
    ,mLoadedLayerFlag(0)
    ,mPriority(priority)
    ,mIsNav(useNavRoadLabel)
    ,mLabelDrawOrder(labelDrawOrder)
    ,mBaseDrawOrder(baseDrawOrder)
    ,mSubDrawOrder(subDrawOrder)
    ,mExtendType(NBGM_TMBET_NONE)
    ,mTileCenter(tileCenter)
    ,mActive(TRUE)
    ,mHaveStaticPoi(FALSE)
    ,mMaterialCategoryName(materialCategoryName)
{
    mNode->AddChild(mSubNode);
    mSubNode->SetPosition(tileCenter);
}

NBGM_LabelLayerTile::~NBGM_LabelLayerTile()
{
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        (*i)->RemoveFromVisibleSet();
        NBRE_DELETE *i;
    }

    if (mStaticLabelTile)
    {
        NBRE_DELETE mStaticLabelTile;
    }

    if (mSubNode)
    {
        mNode->RemoveChild(mSubNode.get());
    }

    for (uint32 i = 0; i < 3; ++i)
    {
        if (mRepeatSubNode[i].get() != NULL)
        {
            mNode->RemoveChild(mRepeatSubNode[i].get());
        }
    }
}

void
NBGM_LabelLayerTile::AddTileNameLabel()
{
    NBRE_FontMaterial fm;
    fm.foreColor.r = 0.0f;
    fm.foreColor.g = 0.0f;
    fm.foreColor.b = 1.0f;
    fm.minFontHeightPixels = 21;
    fm.maxFontHeightPixels = 21;
    NBGM_PointLabel* label = NBRE_NEW NBGM_PointLabel(*mNBGMContext, mSubNode.get(), mSubView, 0
                                                      , NBRE_Font::ToUnicode(mId.c_str(), 0), fm, NBRE_INVALID_SHORT_INDEX, METER_TO_MERCATOR(10.0f), NBRE_Vector3d(0,0,0), TRUE);
    label->SetLayoutPriority(0);
    mElements.push_back(label);

    ElementAssistInfo elementAssistInfo;
    elementAssistInfo.mNearVisibility = 0.0f;
    elementAssistInfo.mFarVisibility = NBRE_Math::NaN<float>();
    elementAssistInfo.mType = NBRE_LDT_PNTS;
    elementAssistInfo.mLayoutElementList.push_back(label);

    mElementAssistInfoList.push_back(elementAssistInfo);
}

void
NBGM_LabelLayerTile::CreateTileAreaLabels(NBGM_NBMLayerData* layer, const NBRE_Point3d& /*refCenter*/, uint8 /*drawOrder*/, int32 /*subDrawOrder*/, uint8 labelDrawOrder, const NBRE_String& materialCategoryName)
{
    ElementAssistInfo elementAssistInfo;
    elementAssistInfo.mNearVisibility = layer->GetLayerInfo().nearVisibility;
    elementAssistInfo.mFarVisibility = layer->GetLayerInfo().farVisibility;
    elementAssistInfo.mType = NBRE_LDT_AREA;

    NBGM_NBMAREALayerData* areaLayer = static_cast<NBGM_NBMAREALayerData*> (layer);
    for (NBRE_Vector<NBGM_PointLabelData*>::const_iterator iter = areaLayer->GetPointLabels().begin(); iter != areaLayer->GetPointLabels().end(); ++iter)
    {
        NBGM_PointLabelData* pointLabelData = *iter;
        if(pointLabelData == NULL || pointLabelData->text == NULL)
        {
            continue;
        }
        uint32 langCode = 0;
        const char* str = GetLabel(*pointLabelData->text, langCode);
        if (str == NULL)
        {
            continue;
        }

        const NBRE_FontMaterial* fontMaterial = mLayer->GetFontMaterial(materialCategoryName, pointLabelData->text->fontMaterialId);
        if (fontMaterial == NULL)
        {
            continue;
        }

        NBRE_Vector3d position = pointLabelData->position;
        NBGM_PointLabel* label = NBRE_NEW NBGM_PointLabel(*mNBGMContext,
            mSubNode.get(),
            mSubView,
            mOverlayManager.AssignOverlayId(DrawOrderStruct(labelDrawOrder, 0, 0)),
            NBRE_Font::ToUnicode(str, langCode),
            *fontMaterial,
            ConvertMaterialFontId(pointLabelData->text->fontMaterialId),
            METER_TO_MERCATOR(10.0f),
            position,
            FALSE);
        label->SetLayoutPriority(pointLabelData->text->GetPriority());
        label->SetTypeId(NBGM_LET_AREA_LABEL);
        mElements.push_back(label);

#ifdef SHOW_LABEL_DEBUG_INFO
        NBRE_Vector3d refCenter = mSubNode->WorldPosition();
        float pixelToMercator = mNBGMContext->WorldToModel(METER_TO_MERCATOR(CalcPixelResolution(CalcZoomLevel((layer->GetLayerInfo().nearVisibility * 0.9f + layer->GetLayerInfo().farVisibility * 0.1f)))));
        float rcSize = fontMaterial->minFontHeightPixels * pixelToMercator;

        NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreateDebugRectEntity(mNBGMContext->renderingEngine->Context()
            , (float)(position.x)
            , (float)(position.y)
            , rcSize
            , rcSize
            , NBRE_Color(0, 0, 0, 1)
            )));
        NBRE_EntityPtr ent(NBRE_NEW NBRE_Entity(mNBGMContext->renderingEngine->Context(),NBRE_ModelPtr(model), mLabelDrawOrder));
        ent->SetPriority(1);
        mSubNode->AttachObject(ent);
#endif

        elementAssistInfo.mLayoutElementList.push_back(label);
    }

    mElementAssistInfoList.push_back(elementAssistInfo);
}

static nb_boolean
FindPolylineAcutePoints(NBRE_Polyline2f::Point2List& vs, const NBRE_Point2f* pt)
{
    if (vs.size() > 1)
    {
        NBRE_Vector2f initDir(vs[1] - vs[0]);
        initDir.Normalise();

        NBRE_Vector2f curDir(*pt - vs[vs.size() - 1]);
        curDir.Normalise();

        if (curDir.DotProduct(initDir) < LABEL_MAX_DIRECTION_CHANGE_COS)
        {
            return TRUE;
        }

        if (vs.size() > 2)
        {
            NBRE_Vector2f lastDir(vs[vs.size() - 1] - vs[vs.size() - 2]);
            lastDir.Normalise();
            if (curDir.DotProduct(lastDir) < LABEL_MAX_DIRECTION_CHANGE_COS)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

nb_boolean
NBGM_LabelLayerTile::TryMergeRoadTracks(RoadTrackInfo* dst, RoadTrackInfo* src)
{
    if (dst->materialId != src->materialId)
    {
        return FALSE;
    }

    if (dst->polyline.back() == src->polyline.front())
    {
        for (uint32 i = 1; i < src->polyline.size(); ++i)
        {
            dst->polyline.push_back(src->polyline[i]);
        }
        return TRUE;
    }
    else if (dst->polyline.front() == src->polyline.back())
    {
        uint32 srcSize = src->polyline.size();
        for (uint32 i = 1; i < srcSize; ++i)
        {
            dst->polyline.insert(dst->polyline.begin(), src->polyline[srcSize - 1 - i]);
        }
        return TRUE;
    }
    else if (dst->polyline.back() == src->polyline.back())
    {
        uint32 srcSize = src->polyline.size();
        for (uint32 i = 1; i < srcSize; ++i)
        {
            dst->polyline.push_back(src->polyline[srcSize - 1 - i]);
        }
        return TRUE;
    }
    else if (dst->polyline.front() == src->polyline.front())
    {
        uint32 srcSize = src->polyline.size();
        for (uint32 i = 1; i < srcSize; ++i)
        {
            dst->polyline.insert(dst->polyline.begin(), src->polyline[i]);
        }
        return TRUE;
    }

    return FALSE;
}

void
NBGM_LabelLayerTile::MergeRoadTracks(NBGM_NBMLayerData* layer, RoadTrackList& result)
{
    NBGM_NBMTPTHLayerData* tpthLayer = static_cast<NBGM_NBMTPTHLayerData*> (layer);
    for (NBRE_Vector<NBGM_RoadLabelData*>::const_iterator iter = tpthLayer->GetRoadLabels().begin(); iter != tpthLayer->GetRoadLabels().end(); ++iter)
    {
        const NBGM_RoadLabelData* roadLabelData = *iter;
        if (roadLabelData == NULL || roadLabelData->text == NULL || roadLabelData->text->fontMaterial == NULL)
        {
            continue;
        }
        uint32 vertexCount = roadLabelData->track.size();
        if (vertexCount < 2)
        {
            continue;
        }

        uint32 langCode = 0;
        const char* str = GetLabel(*roadLabelData->text, langCode);
        if (str == NULL)
        {
            continue;
        }
        NBRE_WString text(str);
        RoadTrackInfo* r = NBRE_NEW RoadTrackInfo;
        r->material = roadLabelData->text->fontMaterial;
        r->materialId = roadLabelData->text->fontMaterialId;
        r->priority = roadLabelData->GetPriority();
        r->width = roadLabelData->width;
        r->text = text;
        for (uint32 j = 0; j < vertexCount; ++j)
        {
            NBRE_Point2f pt = roadLabelData->track[j];
            r->polyline.push_back(pt);
        }

        for (uint32 j = 0; j < result.size(); ++j)
        {
            RoadTrackInfo* ti = result[j];
            if (text == ti->text)
            {
                if(TryMergeRoadTracks(ti, r))
                {
                    NBRE_DELETE r;
                    r = NULL;
                    break;
                }
            }
        }

        if(r != NULL)
        {
            result.push_back(r);
        }
    }
}

void
NBGM_LabelLayerTile::CreateTileRoadLabels(NBGM_NBMLayerData* layer, const NBRE_Point3d& /*refCenter*/, uint8 /*drawOrder*/, int32 /*subDrawOrder*/, uint8 labelDrawOrder, const NBRE_String& materialCategoryName)
{
    ElementAssistInfo elementAssistInfo;
    elementAssistInfo.mNearVisibility = layer->GetLayerInfo().nearVisibility;
    elementAssistInfo.mFarVisibility = layer->GetLayerInfo().farVisibility;
    elementAssistInfo.mType = NBRE_LDT_TPTH;

    NBGM_NBMTPTHLayerData* tpthLayer = static_cast<NBGM_NBMTPTHLayerData*> (layer);
    if (!mIsNav && tpthLayer->GetRoadLabels().size() > 0 && mStaticLabelTile == NULL)
    {
        mStaticLabelTile = NBRE_NEW NBGM_StaticLabelTile(*mNBGMContext, mSubNode.get());
    }

    RoadTrackList mergedTracks;
    MergeRoadTracks(tpthLayer, mergedTracks);

    for (uint32 i = 0; i < mergedTracks.size(); ++i)
    {
        RoadTrackInfo* track = mergedTracks[i];
#ifdef SHOW_LABEL_DEBUG_INFO
        {
            NBRE_Polyline2f pl(track->polyline);
            pl.SetWidth(track->width);
            NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(
                CreateDebugPolylineEntity(mNBGMContext->renderingEngine->Context(), pl, NBRE_Color(0, 0, 1, 1))
                ));
            NBRE_EntityPtr ent(NBRE_NEW NBRE_Entity(mNBGMContext->renderingEngine->Context(),NBRE_ModelPtr(model), 13));
            mSubNode->AttachObject(ent);
        }
#endif
        const NBRE_FontMaterial* fontMaterial = mLayer->GetFontMaterial(materialCategoryName, track->materialId);
        if (fontMaterial == NULL)
        {
            continue;
        }

        NBRE_Polyline2f::Point2List vs;
        for (uint32 j = 0; j < track->polyline.size(); ++j)
        {
            NBRE_Point2f* pt = &track->polyline[j];

            if (FindPolylineAcutePoints(vs, pt))
            {
                nb_boolean isLastPtExisting = FALSE;
                NBRE_Vector2f lastPt;

                if (vs.size() > 0)
                {
                    isLastPtExisting = TRUE;
                    lastPt = vs.back();
                }

                if (vs.size() >= 2)
                {
                    NBRE_Polyline2f pl(vs);
                    if (mIsNav)
                    {
                        NBGM_RoadLabel* label = NBRE_NEW NBGM_RoadLabel(*mNBGMContext,
                            mSubNode.get(),
                            mSubView,
                            mOverlayManager,
                            mOverlayManager.AssignOverlayId(DrawOrderStruct(labelDrawOrder, 0,  1)),
                            track->text,
                            *fontMaterial,
                            ConvertMaterialFontId(track->materialId),
                            pl,
                            track->width);
                        label->SetLayoutPriority(track->priority);
                        mElements.push_back(label);
                        elementAssistInfo.mLayoutElementList.push_back(label);
                    }
                    else
                    {
                        mStaticLabelTile->AddRoadLabel(mOverlayManager.AssignOverlayId(DrawOrderStruct(labelDrawOrder, 0, 0)),
                            mSubNode.get(),
                            track->text,
                            ConvertMaterialFontId(track->materialId),
                            *fontMaterial,
                            pl,
                            track->width,
                            track->priority,
                            layer->GetLayerInfo().nearVisibility,
                            layer->GetLayerInfo().farVisibility);
                    }
                }
                vs.clear();
                if (isLastPtExisting)
                {
                    vs.push_back(lastPt);
                }
            }
            vs.push_back(*pt);
        }

        if (vs.size() >= 2)
        {
            NBRE_Polyline2f pl(vs);
            if (mIsNav)
            {
                NBGM_RoadLabel* label = NBRE_NEW NBGM_RoadLabel(*mNBGMContext,
                    mSubNode.get(),
                    mSubView,
                    mOverlayManager,
                    mOverlayManager.AssignOverlayId(DrawOrderStruct(labelDrawOrder, 0, 1)),
                    track->text,
                    *fontMaterial,
                    ConvertMaterialFontId(track->materialId),
                    pl,
                    track->width);
                label->SetLayoutPriority(track->priority);
                mElements.push_back(label);
                elementAssistInfo.mLayoutElementList.push_back(label);
            }
            else
            {
                mStaticLabelTile->AddRoadLabel(mOverlayManager.AssignOverlayId(DrawOrderStruct(labelDrawOrder, 0, 0)),
                    mSubNode.get(),
                    track->text,
                    ConvertMaterialFontId(track->materialId),
                    *fontMaterial,
                    pl,
                    track->width,
                    track->priority,
                    layer->GetLayerInfo().nearVisibility,
                    layer->GetLayerInfo().farVisibility);
            }
        }
        NBRE_DELETE track;
    }

    mElementAssistInfoList.push_back(elementAssistInfo);
}

void
NBGM_LabelLayerTile::CreateTileBuildingLabels(NBGM_NBMLayerData* layer, const NBRE_Point3d& /*refCenter*/, uint8 /*drawOrder*/, int32 /*subDrawOrder*/, uint8 labelDrawOrder, const NBRE_String& materialCategoryName)
{
    ElementAssistInfo elementAssistInfo;
    elementAssistInfo.mNearVisibility = layer->GetLayerInfo().nearVisibility;
    elementAssistInfo.mFarVisibility = layer->GetLayerInfo().farVisibility;
    elementAssistInfo.mType = NBRE_LDT_MESH;

    NBGM_NBMMESHLayerData* meshLayer = static_cast<NBGM_NBMMESHLayerData*> (layer);
    for (uint32 i = 0; i < meshLayer->GetPointLabels().size(); ++i)
    {
        const NBGM_PointLabelData* pointLabelData = meshLayer->GetPointLabels()[i];
        if (pointLabelData == NULL || pointLabelData->text == NULL)
        {
            continue;
        }

        const NBRE_FontMaterial* fontMaterial = mLayer->GetFontMaterial(materialCategoryName, pointLabelData->text->fontMaterialId);
        if (fontMaterial == NULL)
        {
            continue;
        }

        if (fontMaterial->maxFontHeightPixels == 0.0f)
        {
            continue;
        }

        uint32 langCode = 0;
        const char* str = GetLabel(*pointLabelData->text, langCode);
        if (str == NULL)
        {
            continue;
        }

        NBRE_Vector3d position = pointLabelData->position;

        NBGM_PointLabel* label = NBRE_NEW NBGM_PointLabel(*mNBGMContext,
            mSubNode.get(),
            mSubView,
            mOverlayManager.AssignOverlayId(DrawOrderStruct(labelDrawOrder, 0, 0)),
            NBRE_Font::ToUnicode(str, langCode),
            *fontMaterial,
            ConvertMaterialFontId(pointLabelData->text->fontMaterialId),
            METER_TO_MERCATOR(10.0f),
                position,
                FALSE);
        label->SetLayoutPriority(pointLabelData->text->GetPriority());
        label->SetTypeId(NBGM_LET_BUILDING_LABEL);
        mElements.push_back(label);
        elementAssistInfo.mLayoutElementList.push_back(label);
    }

    mElementAssistInfoList.push_back(elementAssistInfo);
}

void
NBGM_LabelLayerTile::CreateTilePointLabels(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const NBRE_String& materialCategoryName)
{
    ElementAssistInfo elementAssistInfo;
    elementAssistInfo.mNearVisibility = layer->GetLayerInfo().nearVisibility;
    elementAssistInfo.mFarVisibility = layer->GetLayerInfo().farVisibility;
    elementAssistInfo.mType = NBRE_LDT_PNTS;

    NBGM_NBMPNTSLayerData* pntsLayer = static_cast<NBGM_NBMPNTSLayerData*> (layer);
    for (NBRE_Vector<NBGM_PointLabelData*>::const_iterator iter = pntsLayer->GetPointLabels().begin(); iter != pntsLayer->GetPointLabels().end(); ++iter)
    {
        NBGM_PointLabelData* pointLabelData = *iter;
        if(pointLabelData == NULL || pointLabelData->text == NULL)
        {
            continue;
        }
        uint32 langCode = 0;
        const char* str = GetLabel(*pointLabelData->text, langCode);
        if (str == NULL)
        {
            continue;
        }

        NBGM_LayoutElement* elem = NULL;

        if (pointLabelData->material != NULL)
        {
            NBRE_Vector3d position = pointLabelData->position + refCenter;
            if (pointLabelData->material->Type() == NBRE_MMT_ShieldMaterial && pointLabelData->text->fontMaterial != NULL)
            {
                // Point shield
                const NBRE_FontMaterial* fontMaterial = mLayer->GetFontMaterial(materialCategoryName, pointLabelData->text->fontMaterialId);
                if (fontMaterial == NULL)
                {
                    continue;
                }

                elem = CreateTilePointShield(*pointLabelData->material,
                    *fontMaterial,
                    pointLabelData->text->fontMaterialId,
                    str,
                    langCode,
                    position,
                    mOverlayManager.AssignOverlayId(DrawOrderStruct(labelDrawOrder, 0, 0)));
            }
            else if (pointLabelData->material->Type() == NBRE_MMT_StaticPOIMaterial)
            {
                const NBRE_FontMaterial* fontMaterial = mLayer->GetFontMaterial(materialCategoryName, pointLabelData->text->fontMaterialId);
                if (fontMaterial == NULL)
                {
                    continue;
                }

                // Static POI
                elem = CreateTileStaticPoi(
                    pointLabelData->id,
                    *pointLabelData->material,
                    fontMaterial,
                    pointLabelData->text->fontMaterialId,
                    str,
                    langCode,
                    pointLabelData->position,
                    mOverlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, subDrawOrder, 0)),
                    pointLabelData->text->GetPriority());
                mHaveStaticPoi = TRUE;
#ifdef SHOW_LABEL_DEBUG_INFO

                NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreateDebugPointEntity(mNBGMContext->renderingEngine->Context()
                    , (float)(pointLabelData->position.x)
                    , (float)(pointLabelData->position.y)
                    )));
                NBRE_EntityPtr ent(NBRE_NEW NBRE_Entity(mNBGMContext->renderingEngine->Context(),NBRE_ModelPtr(model), mOverlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, subDrawOrder, 0))));
                ent->SetPriority(1);
                mSubNode->AttachObject(ent);
#endif
            }
        }
        else if(pointLabelData->text->fontMaterial != NULL)
        {
            const NBRE_FontMaterial* fontMaterial = mLayer->GetFontMaterial(materialCategoryName, pointLabelData->text->fontMaterialId);
            if (fontMaterial == NULL)
            {
                continue;
            }

            NBRE_Vector3d position = pointLabelData->position;
            const NBRE_MapFontMaterial* mapFontMaterial = static_cast<const NBRE_MapFontMaterial*>(pointLabelData->text->fontMaterial);
            elem = NBRE_NEW NBGM_PointLabel(*mNBGMContext,
                mSubNode.get(),
                mSubView,
                mOverlayManager.AssignOverlayId(DrawOrderStruct(labelDrawOrder, 0, 0)),
                NBRE_Font::ToUnicode(str, langCode),
                *fontMaterial,
                ConvertMaterialFontId(pointLabelData->text->fontMaterialId),
                METER_TO_MERCATOR(10.0f),
                position,
                mapFontMaterial->IsWatermark());

#ifdef SHOW_LABEL_DEBUG_INFO
            NBRE_Vector3d refCenter = mSubNode->WorldPosition();
            float pixelToMercator = mNBGMContext->WorldToModel(METER_TO_MERCATOR(CalcPixelResolution(CalcZoomLevel((layer->GetLayerInfo().nearVisibility * 0.9f + layer->GetLayerInfo().farVisibility * 0.1f)))));
            float rcSize = fontMaterial->minFontHeightPixels * pixelToMercator;

            NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreateDebugRectEntity(mNBGMContext->renderingEngine->Context()
                , (float)(position.x)
                , (float)(position.y)
                , rcSize
                , rcSize
                , NBRE_Color(0, 0, 1, 1)
                )));
            NBRE_EntityPtr ent(NBRE_NEW NBRE_Entity(mNBGMContext->renderingEngine->Context(),NBRE_ModelPtr(model), mLabelDrawOrder));
            ent->SetPriority(1);
            mSubNode->AttachObject(ent);
#endif
        }

        if (elem != NULL)
        {
            NBGM_LayoutElementType et = elem->GetTypeId();
            if (et == NBGM_LET_POINT_SHIELD)
            {
                NBGM_Shield* shield = static_cast<NBGM_Shield*>(elem);
                shield->SetLayoutPriority(pointLabelData->GetPriority());
                elementAssistInfo.mLayoutElementList.push_back(shield);
            }
            else if (et == NBGM_LET_STATIC_POI)
            {
                NBGM_StaticPoi* poi = static_cast<NBGM_StaticPoi*>(elem);
                // Note: current priority is based on storage order
                // This cannot ensure the right priority between tiles
                poi->SetLayoutPriority(pointLabelData->GetPriority());
                elementAssistInfo.mLayoutElementList.push_back(poi);
            }
            else
            {
                NBGM_PointLabel* label = static_cast<NBGM_PointLabel*>(elem);
                switch(layer->GetLayerInfo().layerType)
                {
                case NBRE_DLT_LandmarkBuilding:
                case NBRE_DLT_UnTextureLandmarkBuilding:
                case NBRE_DLT_PointLabel:
                case NBRE_DLT_UnTextureBuilding:
                    label->SetLayoutPriority(pointLabelData->text->GetPriority());
                    break;
                default:
                    label->SetLayoutPriority(pointLabelData->text->GetPriority());
                    break;
                }
                elementAssistInfo.mLayoutElementList.push_back(label);
            }

            mElements.push_back(elem);
        }
    }
    mElementAssistInfoList.push_back(elementAssistInfo);
}

void
NBGM_LabelLayerTile::CreateTileShields(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter, uint8 /*drawOrder*/, int32 /*subDrawOrder*/, uint8 labelDrawOrder, const NBRE_String& materialCategoryName)
{
    ElementAssistInfo elementAssistInfo;
    elementAssistInfo.mNearVisibility = layer->GetLayerInfo().nearVisibility;
    elementAssistInfo.mFarVisibility = layer->GetLayerInfo().farVisibility;
    elementAssistInfo.mType = NBRE_LDT_TPSH;

    NBGM_NBMTPSHLayerData* tpshLayer = static_cast<NBGM_NBMTPSHLayerData*> (layer);

    if (!mIsNav && tpshLayer->GetShields().size() > 0 && mStaticLabelTile == NULL)
    {
        mStaticLabelTile = NBRE_NEW NBGM_StaticLabelTile(*mNBGMContext, mSubNode.get());
    }

    for (NBRE_Vector<NBGM_ShieldData*>::const_iterator iter = tpshLayer->GetShields().begin(); iter != tpshLayer->GetShields().end(); ++iter)
    {
        NBGM_ShieldData* shieldData = *iter;
        if(shieldData == NULL || shieldData->text == NULL)
        {
            continue;
        }

        uint32 vertexCount = shieldData->track.size();
        if (vertexCount < 2)
        {
            continue;
        }

        uint32 langCode = 0;
        const char* str = GetLabel(*shieldData->text, langCode);
        if (str == NULL)
        {
            continue;
        }

        NBRE_Polyline3d::Point3List vs;
        for (uint32 j = 0; j < vertexCount; ++j)
        {
            NBRE_Point2f& pt = shieldData->track[j];
            vs.push_back(NBRE_Vector3d(
                pt.x + refCenter.x,
                pt.y + refCenter.y,
                refCenter.z
                ));
        }
        NBRE_Polyline3d pl(vs);

#ifdef SHOW_LABEL_DEBUG_INFO
        if (vertexCount >= 2)
        {
            NBRE_Polyline2f pl(vertexCount);
            for (uint32 j = 0; j < vertexCount; ++j)
            {
                NBRE_Point2f& pt = shieldData->track[j];
                pl.SetVertex(j, pt);
            }
            pl.UpdateLength();
            NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(
                CreateDebugPolylineEntity(mNBGMContext->renderingEngine->Context(), pl, NBRE_Color(0, 1, 0, 1))
                ));
            NBRE_EntityPtr ent(NBRE_NEW NBRE_Entity(mNBGMContext->renderingEngine->Context(),NBRE_ModelPtr(model), mLabelDrawOrder));
            mSubNode->AttachObject(ent);
        }
#endif
        if(shieldData->material == NULL || shieldData->material->Type() != NBRE_MMT_ShieldMaterial)
        {
            continue;
        }

        const NBRE_MapShieldMaterial* shieldMaterial = static_cast<const NBRE_MapShieldMaterial*>(shieldData->material);
        NBRE_ShaderPtr imageShader = shieldMaterial->GetImageShader();

        if (!imageShader)
        {
            continue;
        }

        const NBRE_FontMaterial* fontMaterial = mLayer->GetFontMaterial(materialCategoryName, shieldData->text->fontMaterialId);
        if (fontMaterial == NULL)
        {
            continue;
        }
        NBRE_Vector2f offset = shieldMaterial->GetOffset();
        float fontHeight = METER_TO_MERCATOR(10.0f);

        if (mIsNav)
        {
            NBGM_Shield* sh = NBRE_NEW NBGM_Shield(
                *mNBGMContext,
                mSubNode.get(),
                mSubView,
                mOverlayManager.AssignOverlayId(DrawOrderStruct(labelDrawOrder, 0, 0)),
                NBRE_Font::ToUnicode(str, langCode),
                *fontMaterial,
                ConvertMaterialFontId(shieldData->text->fontMaterialId),
                fontHeight,
                imageShader,
                shieldMaterial->GetPadding(),
                shieldMaterial->GetOffset(),
                pl);
            mElements.push_back(sh);
            elementAssistInfo.mLayoutElementList.push_back(sh);
        }
        else
        {
            mStaticLabelTile->AddShieldTrack(
                mSubNode.get(),
                mSubView,
                mOverlayManager.AssignOverlayId(DrawOrderStruct(labelDrawOrder, 0, 0)),
                NBRE_Font::ToUnicode(str, langCode),
                *fontMaterial,
                ConvertMaterialFontId(shieldData->text->fontMaterialId),
                imageShader,
                shieldMaterial->GetPadding(),
                offset,
                pl,
                shieldData->GetPriority(),
                layer->GetLayerInfo().nearVisibility,
                layer->GetLayerInfo().farVisibility
                );
        }
    }
    mElementAssistInfoList.push_back(elementAssistInfo);
}

NBGM_PathArrowLayerData*
NBGM_LabelLayerTile::CreateMergedLayerData(NBGM_NBMLayerData* layer)
{
    NBGM_NBMTPARLayerData* tparLayer = static_cast<NBGM_NBMTPARLayerData*> (layer);

    NBGM_PathArrowLayerData* dataCopy = NBRE_NEW NBGM_PathArrowLayerData;
    dataCopy->mNearVisibility = layer->GetLayerInfo().nearVisibility;
    dataCopy->mFarVisibility = layer->GetLayerInfo().farVisibility;

    for(NBRE_Vector<NBGM_PathArrowData*>::const_iterator iter = tparLayer->GetPathArrows().begin(); iter != tparLayer->GetPathArrows().end(); ++iter)
    {
        NBGM_PathArrowData* pathArrowData = *iter;
        if(pathArrowData == NULL)
        {
            continue;
        }
        NBGM_PathArrowLayerData::MergedPolyline* mergedPolyline = NBRE_NEW NBGM_PathArrowLayerData::MergedPolyline;
        for(NBRE_Vector<NBRE_Point2f>::iterator i = pathArrowData->track.begin(); i != pathArrowData->track.end(); ++i)
        {
            mergedPolyline->push_back(*i);
        }
        dataCopy->mLayerData.push_back(mergedPolyline);
    }

    return dataCopy;
}

void
NBGM_LabelLayerTile::CreatePathArrows(NBGM_NBMLayerData* layer, const NBRE_Point3d& /*refCenter*/, uint8 drawOrder, int32 subDrawOrder, uint8 /*labelDrawOrder*/, const NBRE_String& materialCategoryName)
{
    if (mIsNav || layer == NULL)
    {
        return;
    }

    NBGM_NBMTPARLayerData* tparLayer = static_cast<NBGM_NBMTPARLayerData*> (layer);
    if(tparLayer->GetPathArrows().size() == 0 || tparLayer->GetPathArrows()[0]->material == NULL)
    {
        return;
    }

    NBGM_PathArrowLayerData* dataCopy = CreateMergedLayerData(layer);

    const NBRE_MapMaterial& material = *tparLayer->GetPathArrows()[0]->material;
    if (material.Type() != NBRE_MMT_PathArrowMaterial)
    {
        NBRE_DELETE dataCopy;
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_LabelLayerTile::CreatePathArrows. Mismatched material type!  type = %d", material.Type());
        return;
    }

    NBRE_String shaderName = material.GetShaderName(0);
    if (mStaticLabelTile == NULL)
    {
        mStaticLabelTile = NBRE_NEW NBGM_StaticLabelTile(*mNBGMContext, mSubNode.get());
    }

    const NBRE_MapPathArrowMaterial& paMat = static_cast<const NBRE_MapPathArrowMaterial&>(material);
    mStaticLabelTile->AddPathArrow(mSubNode.get(), dataCopy, GenerateID(materialCategoryName, shaderName), mOverlayManager.AssignOverlayId(DrawOrderStruct(drawOrder + 1, subDrawOrder, 2)),
                                  mNBGMContext->WorldToModel(paMat.GetLength()), mNBGMContext->WorldToModel(paMat.GetRepeat()),
                                  layer->GetLayerInfo().nearVisibility, layer->GetLayerInfo().farVisibility);
}

void
NBGM_LabelLayerTile::CreateTileLabels(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const NBRE_String& materialCategoryName, const NBRE_String& groupName)
{
    switch(layer->GetLayerInfo().layerDataType)
    {
    case NBRE_LDT_AREA:
        CreateTileAreaLabels(layer, refCenter, drawOrder, subDrawOrder, labelDrawOrder, materialCategoryName);
        break;
    case NBRE_LDT_TPTH:
        CreateTileRoadLabels(layer, refCenter, drawOrder, subDrawOrder, labelDrawOrder, materialCategoryName);
        break;
    case NBRE_LDT_MESH:
        CreateTileBuildingLabels(layer, refCenter, drawOrder, subDrawOrder, labelDrawOrder, materialCategoryName);
        break;
    case NBRE_LDT_PNTS:
        CreateTilePointLabels(layer, refCenter, drawOrder, subDrawOrder, labelDrawOrder, materialCategoryName);
        break;
    case NBRE_LDT_TPSH:
        CreateTileShields(layer, refCenter, drawOrder, subDrawOrder, labelDrawOrder, materialCategoryName);
        break;
    case NBRE_LDT_TPAR:
        CreatePathArrows(layer, refCenter, drawOrder, subDrawOrder, labelDrawOrder, materialCategoryName);
        break;
    case NBRE_LDT_GPIN:
        CreateTileCustomPin(layer, refCenter, drawOrder, subDrawOrder, labelDrawOrder, materialCategoryName, groupName);
        break;
    default:
        break;
    }
}

NBGM_LayoutElement*
NBGM_LabelLayerTile::CreateTilePointShield(const NBRE_MapMaterial& shieldMat, const NBRE_FontMaterial& fontMaterial, uint32 fontMaterialId, const char* str, uint32 langCode, const NBRE_Vector3d& position, int32 overlayId)
{
    if (shieldMat.Type() != NBRE_MMT_ShieldMaterial)
    {
        return NULL;
    }
    const NBRE_MapShieldMaterial& shieldMaterial = static_cast<const NBRE_MapShieldMaterial&>(shieldMat);
    NBRE_ShaderPtr imageShader = shieldMaterial.GetImageShader();

    if (!imageShader)
    {
        return NULL;
    }

    float fontHeight = METER_TO_MERCATOR(10.0f);

    return NBRE_NEW NBGM_Shield(*mNBGMContext, mNode.get(), mSubView, overlayId
        , NBRE_Font::ToUnicode(str, langCode), fontMaterial, ConvertMaterialFontId(fontMaterialId), fontHeight, imageShader
        , shieldMaterial.GetPadding(), shieldMaterial.GetOffset(), position, TRUE);
}

NBGM_LayoutElement*
NBGM_LabelLayerTile::CreateTileStaticPoi(const NBRE_String& id, const NBRE_MapMaterial& poiMat, const NBRE_FontMaterial* fontMaterial, uint32 fontMaterialId, const char* str, uint32 langCode, const NBRE_Vector3d& position, int32 overlayId, int32 labelPriority)
{
    if (poiMat.Type() != NBRE_MMT_StaticPOIMaterial || poiMat.GetShaderCount() == 0)
    {
        return NULL;
    }
    const NBRE_StaticPOIMaterial& poiMaterial = static_cast<const NBRE_StaticPOIMaterial&>(poiMat);

    NBRE_ShaderPtr normalShader(poiMaterial.NormalShader());
    NBRE_ShaderPtr selectedShader(poiMaterial.SelectedShader());
    NBRE_ShaderPtr unselectedShader(poiMaterial.UnselectedShader());
    NBRE_Point2f imageOffset(poiMaterial.XImageOffset(), poiMaterial.YImageOffset());
    NBRE_Point2f bubbleOffset(poiMaterial.XBubbleOffset(), poiMaterial.YBubbleOffset());
    bubbleOffset -= imageOffset;
    imageOffset.x = imageOffset.x-0.5f;
    imageOffset.y = 0.5f-imageOffset.y;

    imageOffset.x *= -1.f;
    imageOffset.y *= -1.f;
    bubbleOffset.x *= -1.f;
    bubbleOffset.y *= -1.f;

    // Offset from the center point, unit is percentage
    // Y axis is from bottom to up( OpenGL coordinates )
    NBRE_Vector2f size(poiMaterial.IconWidth(), poiMaterial.IconHeight());
    float distanceToPoi = poiMaterial.DistanceToPOI();
    float distanceToLabel = poiMaterial.DistanceToLabel();
    float distanceToAnother = poiMaterial.DistanceToAnother();

    NBRE_FontMaterial fm;
    fm.minFontHeightPixels = 12.0f;
    fm.maxFontHeightPixels = 12.0f;
    nb_boolean showLabel = FALSE;

    if (fontMaterial)
    {
        fm = *fontMaterial;
        showLabel = TRUE;
    }

    NBGM_StaticPoi* poi = NBRE_NEW NBGM_StaticPoi(*mNBGMContext,
        mSubNode.get(),
        mSubView,
        overlayId,
        size,
        normalShader,
        selectedShader,
        unselectedShader,
        position,
        id,
        showLabel,
        imageOffset,
        bubbleOffset,
        NBRE_Font::ToUnicode(str, langCode),
        fm,
        ConvertMaterialFontId(fontMaterialId),
        distanceToPoi,
        distanceToLabel,
        distanceToAnother,
        labelPriority);

    return poi;
}

void
NBGM_LabelLayerTile::CreateTileCustomPin(NBGM_NBMLayerData* layer, const NBRE_Point3d& /*refCenter*/, uint8 drawOrder, int32 subDrawOrder, uint8 /*labelDrawOrder*/, const NBRE_String& /*materialCategoryName*/, const NBRE_String& layerId)
{
    NBGM_NBMGPINLayerData* gpinLayer = static_cast<NBGM_NBMGPINLayerData*> (layer);
    if(gpinLayer == NULL)
    {
        return;
    }

    ElementAssistInfo elementAssistInfo;
    elementAssistInfo.mNearVisibility = layer->GetLayerInfo().nearVisibility;
    elementAssistInfo.mFarVisibility = layer->GetLayerInfo().farVisibility;
    elementAssistInfo.mType = NBRE_LDT_PNTS;

    for (NBRE_Vector<NBGM_GpinData*>::const_iterator iter = gpinLayer->GetGpins().begin(); iter != gpinLayer->GetGpins().end(); ++iter)
    {
        NBGM_GpinData* gpinData = *iter;
        if (gpinData == NULL || gpinData->material == NULL)
        {
            continue;
        }

        if(gpinData->material->Type() != NBRE_MMT_StaticPOIMaterial)
        {
            continue;
        }

        const NBRE_StaticPOIMaterial& poiMaterial = static_cast<const NBRE_StaticPOIMaterial&>(*gpinData->material);

        NBRE_ShaderPtr normalShader(poiMaterial.NormalShader());
        NBRE_ShaderPtr selectedShader(poiMaterial.SelectedShader());
        NBRE_ShaderPtr unselectedShader(poiMaterial.UnselectedShader());
        NBRE_Point2f imageOffset(poiMaterial.XImageOffset(), poiMaterial.YImageOffset());
        NBRE_Point2f bubbleOffset(poiMaterial.XBubbleOffset(), poiMaterial.YBubbleOffset());
        bubbleOffset -= imageOffset;
        imageOffset.x = imageOffset.x-0.5f;
        imageOffset.y = 0.5f-imageOffset.y;

        imageOffset.x *= -1.f;
        imageOffset.y *= -1.f;
        bubbleOffset.x *= -1.f;
        bubbleOffset.y *= -1.f;

        // Offset from the center point, unit is percentage
        // Y axis is from bottom to up( OpenGL coordinates )
        NBRE_Vector2f size(poiMaterial.IconWidth(), poiMaterial.IconHeight());
        float distanceToPoi = poiMaterial.DistanceToPOI();

        int32 overlayId = mOverlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, subDrawOrder, 0));
        NBRE_Vector3d position(gpinData->position.x, gpinData->position.y, 0);

        NBGM_CustomPin* icon = NBRE_NEW NBGM_CustomPin(*mNBGMContext,
            mSubNode.get(),
            mSubView,
            overlayId,
            size,
            normalShader,
            selectedShader,
            unselectedShader,
            position,
            gpinData->id,
            imageOffset,
            bubbleOffset,
            distanceToPoi,
            layerId,
            TRUE);
        mHaveStaticPoi = TRUE;

        icon->SetPriority(0);//no item priority now
        mElements.push_back(icon);
        elementAssistInfo.mLayoutElementList.push_back(icon);
    }

    mElementAssistInfoList.push_back(elementAssistInfo);
}

void
NBGM_LabelLayerTile::OnActive()
{
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_LayoutElement* elem = *i;
        elem->SetEnable(TRUE);
    }

    if(mStaticLabelTile)
    {
        mStaticLabelTile->SetVisible(TRUE);
    }

    ElementAssistInfoList::iterator pInfo = mElementAssistInfoList.begin();
    ElementAssistInfoList::iterator pInfoEnd = mElementAssistInfoList.end();
    for (; pInfo != pInfoEnd; ++pInfo)
    {
        pInfo->mVisible = TRUE;
    }

    mActive = TRUE;
}

void
NBGM_LabelLayerTile::OnDeactive()
{
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_LayoutElement* elem = *i;
        elem->SetEnable(FALSE);
    }

    if(mStaticLabelTile)
    {
        mStaticLabelTile->SetVisible(FALSE);
    }

    ElementAssistInfoList::iterator pInfo = mElementAssistInfoList.begin();
    ElementAssistInfoList::iterator pInfoEnd = mElementAssistInfoList.end();
    for (; pInfo != pInfoEnd; ++pInfo)
    {
        pInfo->mVisible = FALSE;
    }

    mActive = FALSE;
}


static bool
CompareLabelElements(NBGM_LayoutElement* v1, NBGM_LayoutElement* v2)
{
    return v1->GetMinPixelsPerUnit() < v2->GetMinPixelsPerUnit();
}

void
NBGM_LabelLayerTile::AddToScene()
{
    mAABB.SetNull();
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_LayoutElement* elem = *i;
        elem->AddToSence();
        mAABB.Merge(elem->GetAABB());
    }
    if (mStaticLabelTile)
    {
        mStaticLabelTile->AddToScene();
        mAABB.Merge(mStaticLabelTile->GetAABB());
    }
    std::sort(mElements.begin(), mElements.end(), CompareLabelElements);

#ifdef SHOW_LABEL_DEBUG_INFO
    NBRE_Vector3d refCenter = mSubNode->WorldPosition();
    NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreateDebugRectEntity(mNBGMContext->renderingEngine->Context()
        , (float)(mAABB.GetCenter().x - refCenter.x)
        , (float)(mAABB.GetCenter().y - refCenter.y)
        , (float) mAABB.GetSize().x
        , (float) mAABB.GetSize().y
        , NBRE_Color(1, 0, 0, 1)
        )));
    NBRE_EntityPtr ent(NBRE_NEW NBRE_Entity(mNBGMContext->renderingEngine->Context(),NBRE_ModelPtr(model), mLabelDrawOrder));
    ent->SetPriority(1);
    mSubNode->AttachObject(ent);
#endif
}

NBRE_NodePtr
NBGM_LabelLayerTile::Node()
{
    return mNode;
}

void
NBGM_LabelLayerTile::UpdateLayoutElementVisibility(float viewPointDistance)
{
    if (!mActive)
    {
        return;
    }

    ElementAssistInfoList::iterator pElementAssistInfo = mElementAssistInfoList.begin();
    ElementAssistInfoList::iterator pElementAssistInfoEnd = mElementAssistInfoList.end();

    for (; pElementAssistInfo != pElementAssistInfoEnd; ++pElementAssistInfo)
    {

        nb_boolean isVisible = CheckNearFarVisibility(viewPointDistance, pElementAssistInfo->mNearVisibility, pElementAssistInfo->mFarVisibility);
        //if (pElementAssistInfo->mVisible != isVisible)
        {
            NBGM_LayoutElementList::iterator pElement = pElementAssistInfo->mLayoutElementList.begin();
            NBGM_LayoutElementList::iterator pElementEnd = pElementAssistInfo->mLayoutElementList.end();
            for (; pElement != pElementEnd; ++pElement)
            {
                NBGM_LayoutElement* element = (*pElement);
                ///ignore visibility when static poi is selected;
                if (element->GetTypeId() == NBGM_LET_STATIC_POI)
                {
                    NBGM_StaticPoi* staticPoi = (NBGM_StaticPoi*) element;
                    if (staticPoi && staticPoi->GetSelected())
                    {
                        continue;
                    }
                }
                else if (element->GetTypeId() == NBGM_LET_STATIC_POI_LABEL)
                {
                    NBGM_StaticPoiLabel* staticPoiLabel = (NBGM_StaticPoiLabel*) element;
                    if (staticPoiLabel && staticPoiLabel->GetSelected())
                    {
                        continue;
                    }
                }

                element->SetEnable(isVisible);
            }
            pElementAssistInfo->mVisible = isVisible;
        }
    }
}

void
NBGM_LabelLayerTile::RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials)
{
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_LayoutElement* elem = *i;
        elem->RefreshFontMaterial(materials);
    }

    if (mStaticLabelTile)
    {
        mStaticLabelTile->RefreshFontMaterial(materials);
    }
}

void
NBGM_LabelLayerTile::GetFontMaterialIds(NBRE_Set<uint32>& materialIds)
{
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_LayoutElement* elem = *i;
        elem->GetFontMaterialIds(materialIds);
    }

    if (mStaticLabelTile)
    {
        mStaticLabelTile->GetFontMaterialIds(materialIds);
    }
}

void
NBGM_LabelLayerTile::CreateRepeatNode()
{
    NBGM_LayoutElementList repeatElements;
    ElementAssistInfoList repeatAssistInfos;

    for (uint32 i = 0; i < 2; ++i)
    {
        mRepeatSubNode[i] = NBRE_NodePtr(NBRE_NEW NBRE_Node());
        mNode->AddChild(mRepeatSubNode[i]);
    }
    mRepeatSubNode[2] = mRepeatSubNode[1];
    mRepeatSubNode[1] = mSubNode;
    ResetRepeatNodes();

    for (uint32 i = 0; i < 2; ++i)
    {
        NBRE_NodePtr node = mRepeatSubNode[i == 0 ? 0 : 2];
        ElementAssistInfoList::iterator pInfo = mElementAssistInfoList.begin();
        ElementAssistInfoList::iterator pInfoEnd = mElementAssistInfoList.end();
        for (; pInfo != pInfoEnd; ++pInfo)
        {
            if (pInfo->mType == NBRE_LDT_PNTS)
            {
                ElementAssistInfo elementAssistInfo;
                elementAssistInfo.mNearVisibility = pInfo->mNearVisibility;
                elementAssistInfo.mFarVisibility = pInfo->mFarVisibility;
                elementAssistInfo.mType = NBRE_LDT_PNTS;
                elementAssistInfo.mVisible = pInfo->mVisible;

                NBGM_LayoutElementList::iterator pElement = pInfo->mLayoutElementList.begin();
                NBGM_LayoutElementList::iterator pElementEnd = pInfo->mLayoutElementList.end();
                for (; pElement != pElementEnd; ++pElement)
                {
                    NBGM_LayoutElement* elem = *pElement;
                    NBGM_LayoutElement* cloneElem = elem->Clone(node.get());
                    if (cloneElem)
                    {
                        cloneElem->AddToSence();
                        repeatElements.push_back(cloneElem);
                        elementAssistInfo.mLayoutElementList.push_back(cloneElem);
                        if (elementAssistInfo.mVisible == TRUE)
                        {
                            cloneElem->SetEnable(TRUE);
                        }
                        else
                        {
                            cloneElem->SetEnable(FALSE);
                        }
                    }
                }

                if (!elementAssistInfo.mLayoutElementList.empty())
                {
                    repeatAssistInfos.push_back(elementAssistInfo);
                }
            }
        }
    }

    for (NBGM_LayoutElementList::iterator i = repeatElements.begin(); i != repeatElements.end(); ++i)
    {
        mElements.push_back(*i);
    }

    for (ElementAssistInfoList::iterator i = repeatAssistInfos.begin(); i != repeatAssistInfos.end(); ++i)
    {
        mElementAssistInfoList.push_back(*i);
    }
}

void
NBGM_LabelLayerTile::NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType)
{
    if (!mActive)
    {
        return;
    }

    if (extendType != mExtendType)
    {
        if (mRepeatSubNode[0].get() == NULL)
        {
            CreateRepeatNode();
        }

        if (mExtendType == NBGM_TMBET_RIGHT && extendType == NBGM_TMBET_LEFT)
        {
            // shift left
            NBRE_NodePtr temp = mRepeatSubNode[0];
            mRepeatSubNode[0] = mRepeatSubNode[1];
            mRepeatSubNode[1] = mRepeatSubNode[2];
            mRepeatSubNode[2] = temp;
        }
        else if (mExtendType == NBGM_TMBET_LEFT && extendType == NBGM_TMBET_RIGHT)
        {
            // shift right
            NBRE_NodePtr temp = mRepeatSubNode[2];
            mRepeatSubNode[2] = mRepeatSubNode[1];
            mRepeatSubNode[1] = mRepeatSubNode[0];
            mRepeatSubNode[0] = temp;
        }

        ResetRepeatNodes();
        mExtendType = extendType;
    }
}

void
NBGM_LabelLayerTile::ResetRepeatNodes()
{
    double twoPI = 2.0 * mNBGMContext->WorldToModel(NBRE_Math::Pi64);

    mRepeatSubNode[0]->SetPosition(mTileCenter + NBRE_Vector3d(-twoPI, 0.0, 0.0));
    mRepeatSubNode[1]->SetPosition(mTileCenter);
    mRepeatSubNode[2]->SetPosition(mTileCenter + NBRE_Vector3d(twoPI, 0.0, 0.0));
}

void
NBGM_LabelLayerTile::UpdateLayoutList(NBGM_LayoutElementList& layoutElements, shared_ptr<WorkerTaskQueue> loadingThread, shared_ptr<NBGM_TaskQueue> renderThread, NBGM_LabelLayer* labelLayer)
{
    NBGM_LayoutManager& layoutManager = *mNBGMContext->layoutManager;
    if (!mIsNav && !layoutManager.IsInFOV(mAABB))
    {
        return;
    }
    if (!layoutManager.IsInFrustum(mAABB))
    {
        return;
    }

    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_LayoutElement* elem = *i;
        if (!elem->IsEnable())
        {
            continue;
        }

        if (!mIsNav && elem->GetTypeId() == NBGM_LET_MAP_ROAD_LABEL)
        {
            if (!layoutManager.IsInFOV(elem->GetAABB()))
            {
                continue;
            }
        }

        if (elem->GetTypeId() == NBGM_LET_POINT_LABEL ||
            elem->GetTypeId() == NBGM_LET_CUSTOM_PIN ||
            layoutManager.IsInFrustum(elem->GetAABB()))
        {
            layoutElements.push_back(elem);
            if (elem->GetTypeId() == NBGM_LET_STATIC_POI)
            {
                NBGM_StaticPoi* poi = (NBGM_StaticPoi*)elem;
                NBGM_StaticPoiLabel* label = poi->GetLabel();
                if (label)
                {
                    layoutElements.push_back(label);
                }
            }
            mAABB.Merge(elem->GetAABB());
        }
    }

    if (mStaticLabelTile)
    {
        if (layoutManager.IsInFOV(mStaticLabelTile->GetAABB())
            && layoutManager.IsInFrustum(mStaticLabelTile->GetAABB()))
        {
            mStaticLabelTile->UpdateLayoutList(layoutElements, loadingThread, renderThread, labelLayer);
        }
    }
}

nb_boolean
NBGM_LabelLayerTile::OnLayoutElementsDone(uint64 id, int32 level, NBRE_Vector<NBGM_StaticLayoutResult*>* layoutResult)
{
    if (mStaticLabelTile && id == mStaticLabelTile->GetId())
    {
        return mStaticLabelTile->OnLayoutElementsDone(level, layoutResult);
    }
    return FALSE;
}

void
NBGM_LabelLayerTile::ResolveStaticConflicts(int32 level, NBGM_LabelLayerTile* tile, double epsilon)
{
    if (mStaticLabelTile && tile->mStaticLabelTile
        && mAABB.DistanceTo(tile->mAABB) < epsilon)
    {
        mStaticLabelTile->ResolveStaticConflicts(level, tile->mStaticLabelTile);
    }
}

nb_boolean
NBGM_LabelLayerTile::GetStaticPoiInfo(const NBRE_String& id, NBGM_StaticPoiInfo& info)
{
    if (!mHaveStaticPoi || !mActive)
    {
        return FALSE;
    }

    nb_boolean result = FALSE;
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        if ((*i)->GetTypeId() != NBGM_LET_STATIC_POI)
        {
            continue;
        }
        NBGM_StaticPoi* elem = (NBGM_StaticPoi*)*i;
        if (elem->GetId() == id)
        {
            info.id = id;
            info.name = elem->GetText();
            NBRE_Vector2f bo = elem->GetScreenBubbleOffset();
            info.bubbleOffset.x = bo.x;
            info.bubbleOffset.y = bo.y;
            NBRE_Vector3d pos = elem->GetPosition() + mTileCenter;
            info.position.x = mNBGMContext->ModelToWorld(pos.x);
            info.position.y = mNBGMContext->ModelToWorld(pos.y);
            info.position.z = mNBGMContext->ModelToWorld(pos.z);
            result = TRUE;
            break;
        }
    }
    return result;
}

void
NBGM_LabelLayerTile::SelectStaticPoi(const NBRE_String& id, nb_boolean selected)
{
    if (!mHaveStaticPoi || !mActive)
    {
        return;
    }

    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        if ((*i)->GetTypeId() != NBGM_LET_STATIC_POI)
        {
            continue;
        }
        NBGM_StaticPoi* elem = (NBGM_StaticPoi*)*i;
        if (elem->GetId() == id)
        {
            elem->SetSelected(selected);
            break;
        }
    }
}

void
NBGM_LabelLayerTile::GetElementsAt(NBGM_LayoutElementType type, const NBRE_Point2f& screenPosition, float radius, NBRE_Vector<NBRE_String>& ids)
{
    if (!mHaveStaticPoi || !mActive)
    {
        return;
    }

    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        if ((*i)->GetTypeId() != type)
        {
            continue;
        }

        if (type ==  NBGM_LET_STATIC_POI)
        {
            NBGM_StaticPoi* elem = (NBGM_StaticPoi*)*i;
            if (elem->HitTest(screenPosition, radius))
            {
                ids.push_back(elem->GetId());
            }
        }
        else if (type == NBGM_LET_CUSTOM_PIN)
        {
            NBGM_CustomPin* elem = (NBGM_CustomPin*)*i;
            if (elem->HitTest(screenPosition))
            {
                ids.push_back(elem->GetId());
            }
        }
    }
}

void
NBGM_LabelLayerTile::UpdateLayersLayoutState(NBRE_Map<NBRE_String, nb_boolean>& layersLayoutState)
{
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        if ((*i)->GetTypeId() == NBGM_LET_CUSTOM_PIN)
        {
            NBGM_CustomPin* elem = (NBGM_CustomPin*)*i;
            NBRE_Map<NBRE_String, nb_boolean>::iterator it = layersLayoutState.find(elem->GetLayerId());
            if (it != layersLayoutState.end())
            {
                elem->EnableCollsionDetection(it->second);
            }
        }
    }
}

void
NBGM_LabelLayerTile::EnableLayerLayout(const NBRE_String& layerId, nb_boolean enable)
{
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        if ((*i)->GetTypeId() == NBGM_LET_CUSTOM_PIN)
        {
            NBGM_CustomPin* elem = (NBGM_CustomPin*)*i;
            if (elem->GetLayerId() == layerId)
            {
                elem->EnableCollsionDetection(enable);
            }
        }
    }
}

NBGM_LabelLayerTileCreator::NBGM_LabelLayerTileCreator(NBGM_Context& nbgmContext, const NBGM_LabelLayer* layer, int32 /*billboardOverlayId*/, NBRE_BillboardSet* billboardSet, NBRE_Overlay* textOverlay,
                                                       const NBGM_LabelLayerPriority* priority, nb_boolean useNavRoadLabel, NBRE_SurfaceSubView* subView)
    :mNBGMContext(nbgmContext)
    ,mLayer(layer)
    ,mBillboardSet(billboardSet)
    ,mTextOverlay(textOverlay)
    ,mPriority(priority)
    ,mUseNavRoadLabel(useNavRoadLabel)
    ,mSubView(subView)
{
}

NBGM_LabelLayerTileCreator::~NBGM_LabelLayerTileCreator()
{
}

NBGM_IMapLayerTile*
NBGM_LabelLayerTileCreator::CreateMapLayerTile(const NBGM_VectorTileID& tileId, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, nb_boolean /*enablePicking*/, uint32 /*selectMask*/, const NBRE_String& materialCategoryName)
{
    nb_boolean hasData = FALSE;

    for(NBRE_Vector<NBGM_NBMLayerData*>::const_iterator iter = nbmData->GetLayers().begin(); iter != nbmData->GetLayers().end(); ++iter)
    {
        NBGM_NBMLayerData* layer = *iter;
        if(layer->GetLayerInfo().layerType == NBRE_DLT_Associated && layer->GetLayerInfo().layerDataType != NBRE_LDT_TPSH)
        {
            continue;
        }

        if(layer->GetLayerInfo().layerType == NBRE_DLT_RoadNetwork && layer->GetLayerInfo().layerDataType == NBRE_LDT_TPAR)
        {
            hasData = TRUE;
            break;
        }

        if (layer->GetLayerInfo().layerDataType == NBRE_LDT_AREA
            || layer->GetLayerInfo().layerDataType == NBRE_LDT_TPTH
            || layer->GetLayerInfo().layerDataType == NBRE_LDT_MESH
            || layer->GetLayerInfo().layerDataType == NBRE_LDT_PNTS
            || layer->GetLayerInfo().layerDataType == NBRE_LDT_GPIN
            || layer->GetLayerInfo().layerDataType == NBRE_LDT_TPSH)
        {
            hasData = TRUE;
            break;
        }
    }

    if (!hasData)
    {
        return NULL;
    }

    NBGM_LabelLayerTile* tileData = NBRE_NEW NBGM_LabelLayerTile(&mNBGMContext, mLayer, tileId, mSubView, overlayManager, mTextOverlay, mPriority, nbmData->GetCenter(), mUseNavRoadLabel, labelDrawOrder, drawOrder, subDrawOrder, materialCategoryName);
    for(NBRE_Vector<NBGM_NBMLayerData*>::const_iterator iter = nbmData->GetLayers().begin(); iter != nbmData->GetLayers().end(); ++iter)
    {
        NBGM_NBMLayerData* layer = *iter;
        tileData->CreateTileLabels(layer, nbmData->GetCenter(), drawOrder, subDrawOrder, labelDrawOrder, materialCategoryName, nbmData->GetGroupName());
    }
    return tileData;
}


#ifdef DEBUG_PRINT_LANGUAGE_LIST
static int strlen_utf8_c(const char *s)
{
    int i = 0;
    int j = 0;
    while (s[i])
    {
        if ((s[i] & 0xc0) != 0x80)
        {
            j++;
        }
        i++;
    }
    return j;
}


static int utf8_to_unicode(const char* utf8Str, wchar_t** out, uint32* outsize)
{
    uint8* p = (uint8*)utf8Str;
    wchar_t* result = NULL;

    uint32 len = strlen_utf8_c(utf8Str);

    result = new wchar_t[len + 1];
    nsl_memset(result, 0, sizeof(wchar_t) * (len + 1));
    wchar_t* pDst = result;

    while(*p)
    {
        uint8 ch = *p;
        if ((ch & 0xFE) == 0xFC)//1111110x
        {
            *pDst++ = ' ';
            p += 6;
        }
        else if ((ch & 0xFC) == 0xF8)//111110xx
        {
            *pDst++ = ' ';
            p += 5;
        }
        else if ((ch & 0xF8) == 0xF0)//11110xxx
        {
            *pDst++ = ' ';
            p += 4;
        }
        else if ((ch & 0xF0) == 0xE0)//1110xxxx
        {
            *pDst++ =
                ( (((p[0] << 4) & 0xF0) | ((p[1] >> 2) & 0x0F)) << 8)
                | (((p[1] << 6) & 0xC0) | ((p[2]) & 0x3F));
            p += 3;
        }
        else if ((ch & 0xE0) == 0xC0)//110xxxxx
        {
            *pDst++ =
                (((p[0] >> 2) & 0x07) << 8)
                |( ((p[0] << 6) & 0xC0) | (p[1] & 0x3F));
            p += 2;
        }
        else//0xxxxxxx
        {
            *pDst++ = *p++;
        }
    }

    *out = result;
    *outsize = len;
    return 0;
}

static void printw(const char* str, uint32 code)
{
    wchar_t* wstr = NULL;
    uint32 size;
    utf8_to_unicode(str, &wstr, &size);

    wchar_t buf[100];
    wsprintf(buf, L"%s %d\n", wstr, code);
    OutputDebugStringW(buf);
    delete[] wstr;
}
#endif

static NBRE_String gTempString;

const char*
NBGM_LabelLayerTile::GetLabel(const NBGM_TextData& text, uint32& langCode)
{
    uint8 prefCode = NBGM_GetConfig()->preferredLanguageCode;
    const char* result = NULL;
    if(text.textList.size() != 0)
    {
        if (NBGM_GetConfig()->showLabelDebugInfo)
        {
            gTempString.clear();
            uint32 size = text.textList.size();
            for (uint32 i = 0; i < size; ++i)
            {
                NBRE_Pair<uint32, NBRE_String> pair = text.textList[i];
                if (!pair.second.empty())
                {
                    if(!gTempString.empty())
                    {
                        gTempString.append("\n");
                    }
                    gTempString.append(pair.second);
                    // language code
                    char buf[50];
                    sprintf(buf, "@%u", pair.first);
                    gTempString.append(buf);
                }
            }
            if (!gTempString.empty())
            {
                result = gTempString.c_str();
            }
        }
        else
        {
            uint32 size = text.textList.size();
            uint32 retIndex = NBRE_INVALID_INDEX;
            uint32 firstValidIndex = NBRE_INVALID_INDEX;
            uint32 englishLikeIndex = NBRE_INVALID_INDEX;
            bool useEnglish = IsEnglishCode(prefCode);
            for(uint32 i = 0; i < size; ++i)
            {
                  NBRE_Pair<uint32, NBRE_String> pair = text.textList[i];
                  if (!pair.second.empty())
                  {
                      if(pair.first == prefCode)
                      {
                          retIndex = i;
                          break;
                      }
                      if(useEnglish && englishLikeIndex == NBRE_INVALID_INDEX && IsEnglishLikeCode(pair.first))
                      {
                          englishLikeIndex = i;
                      }
                      if(firstValidIndex == NBRE_INVALID_INDEX)
                      {
                          firstValidIndex = i;
                      }
                  }
#ifdef DEBUG_PRINT_LANGUAGE_LIST
                  printw(pair.second.c_str(), pair.first);
#endif
            }

            if(retIndex == NBRE_INVALID_INDEX)
            {
                retIndex = englishLikeIndex == NBRE_INVALID_INDEX?firstValidIndex:englishLikeIndex;
            }

            if(retIndex != NBRE_INVALID_INDEX)
            {
                result = text.textList[retIndex].second.c_str();
                langCode = text.textList[retIndex].first;
            }
        }
    }
    return result;
}

NBGM_PathArrowLayerData::~NBGM_PathArrowLayerData()
{
    for (uint32 i = 0; i < mLayerData.size(); ++i)
    {
        NBRE_DELETE mLayerData[i];
    }
}

NBGM_PathArrowLayerData* NBGM_PathArrowLayerData::Clone()
{
    NBGM_PathArrowLayerData* result = NBRE_NEW NBGM_PathArrowLayerData;
    result->mFarVisibility = mFarVisibility;
    result->mNearVisibility = mNearVisibility;
    for (uint32 i = 0; i < mLayerData.size(); ++i)
    {
        MergedPolyline* pl = mLayerData[i];
        if (pl == NULL)
        {
            result->mLayerData.push_back(NULL);
        }
        else
        {
            MergedPolyline* plCopy = NBRE_NEW MergedPolyline(*pl);
            result->mLayerData.push_back(plCopy);
        }
    }
    return result;
}
