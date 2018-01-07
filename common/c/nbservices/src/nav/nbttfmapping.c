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
 @file     nbttfmapping.c
 @defgroup natural and lane gudiance

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

#include "nbnaturallaneguidancetype.h"
#include "nbrouteparameters.h"
#include "nbrouteparametersprivate.h"
#include "nbrouteinformationprivate.h"
#include "tpslib.h"
#include "tpsio.h"
#include "navprefs.h"

#define GUIDANCE_CONFIG                         "guidanceconfig"
#define GUIDANCE_CONFIG_DISTANCE                "distance"
#define GUIDANCE_CONFIG_NAV_INSTRUCT_DISTANCE   "nav-instruct-distance"
#define GUIDANCE_CONFIG_COMMAND_INDEX           "commandindex"
#define GUIDANCE_CONFIG_COMMAND                 "command"

#define FONT_MAP_ROOT                           "font-mapping"
#define FONT_MAP_ROOT_ATTR_VERSION              "version"
#define FONT_MAPPING_ROW                        "font-mapping-row"

#define FONT_MAPPING_ROW_ATTR_COMMAND           "command"
#define FONT_MAPPING_ROW_ATTR_LANETYPE          "lane-type"
#define FONT_MAPPING_ROW_ATTR_UNSELECTEDARROWS  "unselected-arrows"
#define FONT_MAPPING_ROW_ATTR_SELECTEDARROWS    "selected-arrows"
#define FONT_MAPPING_ROW_ATTR_UNSELECTEDCHARS   "unselected-chars"
#define FONT_MAPPING_ROW_ATTR_SELECTEDCHARS     "selected-chars"
#define FONT_MAPPING_ROW_ATTR_KEY               "key"


#define FONT_MAPPING_DEFAULT_LANE_TYPE          0



#define HOV_MAP_ROOT                           "hov-mapping"
#define HOV_MAP_ROOT_ATTR_VERSION              "version"
#define HOV_MAPPING_ROW                        "hov-mapping-row"


#define HOV_MAPPING_ROW_ATTR_ARROW_CHARS   "arrow-char"
#define HOV_MAPPING_ROW_ATTR_HOV_CHARS     "hov-char"


#define SLIGHT_LEFT_MASK        0x80
#define LEFT_MASK               0x40
#define HARD_LEFT_MASK          0x20
#define UTURN_LEFT_MASK         0x10
#define HARD_RIGHT_MASK         0x08
#define RIGHT_MASK              0x04
#define SLIGHT_RIGHT_MASK       0x02
#define STRIGHT_MASK            0x01


NB_Error PhraseEntryToLaneGuidanceFontMap(tpselt tpsElements,
                                          NB_RouteParameters* pRouteParameters
                                          )
{
    NB_Error err = NE_OK;
    const char * name = NULL;
    const char * stringTmp = NULL;
    LaneGuidanceFontMap * fontMapEntry;

    if (!pRouteParameters)
    {
        return NE_INVAL;
    }
    name = te_getname(tpsElements);

    if (nsl_strcmp(FONT_MAPPING_ROW, name) != 0)
    {
        return NE_INVAL;
    }

    fontMapEntry = NULL;

    fontMapEntry = (LaneGuidanceFontMap *)nsl_malloc(sizeof(LaneGuidanceFontMap));

    if (! fontMapEntry)
    {
        return NE_NOMEM;
    }

    fontMapEntry->key = te_getattru(tpsElements, FONT_MAPPING_ROW_ATTR_KEY);

    stringTmp = te_getattrc(tpsElements, FONT_MAPPING_ROW_ATTR_UNSELECTEDCHARS);
    nsl_strlcpy(fontMapEntry->unselectedChars, stringTmp, NB_TURN_CODE_LEN-1 );
    stringTmp = te_getattrc(tpsElements, FONT_MAPPING_ROW_ATTR_SELECTEDCHARS);
    nsl_strlcpy(fontMapEntry->selectedChars, stringTmp, NB_TURN_CODE_LEN-1);

    if(!CSL_HashTableInsert(pRouteParameters->privateConfig.prefs.laneGuidanceFontMap, (byte*)&fontMapEntry->key, sizeof(fontMapEntry->key), fontMapEntry))
    {
        free(fontMapEntry);
        return NE_INVAL;
    }

    return err;
}


void DeleteEachEntryFromNewHashTable(void* value,
                                     void* userData
                                     )
{
    LaneGuidanceFontMap * map = value;
    if (!map)
    {
        free(map);
    }
}

void CopyEachEntryToNewHashTable(
                                 void * value,
                                 void * userData
                                 )
{
    CSL_HashTable * newHashTable = (CSL_HashTable *)userData;
    LaneGuidanceFontMap * fontMap = (LaneGuidanceFontMap *)value;
    LaneGuidanceFontMap * newFontMap = NULL;
    if (value && newHashTable)
    {
        newFontMap = (LaneGuidanceFontMap*)nsl_malloc(sizeof(LaneGuidanceFontMap));
        nsl_memcpy(newFontMap,fontMap,sizeof(LaneGuidanceFontMap));
        CSL_HashTableInsert(newHashTable, (byte*)&newFontMap->key, sizeof(newFontMap->key), newFontMap);
    }

}

NB_Error PhraseTpsToFontMap(NB_Context * context,
                            NB_RouteParameters* pRouteParameters,
                            tpselt tpsElements
                            )
{
    tpselt ce  = NULL;
    int iter = 0;
    NB_Error err = NE_OK;
    const char* name = NULL;

    if (!context || !pRouteParameters)
    {
        return NE_INVAL;
    }

    name = te_getname(tpsElements);

    if (nsl_strcmp(FONT_MAP_ROOT, name) != 0)
    {
        return NE_INVAL;
    }

    if ( pRouteParameters->privateConfig.prefs.laneGuidanceFontMap)
    {
        CSL_HashTableForEach(pRouteParameters->privateConfig.prefs.laneGuidanceFontMap,DeleteEachEntryFromNewHashTable, NULL);
        CSL_HashTableDeleteAll(pRouteParameters->privateConfig.prefs.laneGuidanceFontMap);
    }
    else
    {
        pRouteParameters->privateConfig.prefs.laneGuidanceFontMap = CSL_HashTableCreate(MAX_NUM_FONT_MAP, NULL);
    }

    if (!pRouteParameters->privateConfig.prefs.laneGuidanceFontMap)
    {
        return NE_INVAL;
    }

    pRouteParameters->privateConfig.prefs.laneGuidanceFontMapVersion = te_getattru(tpsElements, FONT_MAP_ROOT_ATTR_VERSION);

    while ((ce = te_nextchild(tpsElements, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), FONT_MAPPING_ROW) == 0)
        {
            err = err ? err : PhraseEntryToLaneGuidanceFontMap(ce, pRouteParameters);
        }
    }
    return err;
}

NB_Error PhraseEntryToHOVFontMap(tpselt tpsElements,
                                   NB_RouteParameters* pRouteParameters
                                   )
{
    NB_Error err = NE_OK;
    const char * name = NULL;
    const char * stringTmp = NULL;
    int i = 0;

    if (!pRouteParameters)
    {
        return NE_INVAL;
    }
    name = te_getname(tpsElements);

    if (nsl_strcmp(HOV_MAPPING_ROW, name) != 0)
    {
        return NE_INVAL;
    }

    i = pRouteParameters->privateConfig.prefs.numHovFontMap;

    if (i >= MAX_NUM_HOV_MAP )
    {
        return NE_NOMEM;
    }

    stringTmp = te_getattrc(tpsElements, HOV_MAPPING_ROW_ATTR_ARROW_CHARS);
    nsl_strlcpy(pRouteParameters->privateConfig.prefs.hovFontMap[i].arrowChars, stringTmp, NB_TURN_CODE_LEN-1 );
    stringTmp = te_getattrc(tpsElements, HOV_MAPPING_ROW_ATTR_HOV_CHARS);
    nsl_strlcpy(pRouteParameters->privateConfig.prefs.hovFontMap[i].hovChars, stringTmp, NB_TURN_CODE_LEN-1);

    if (err == NE_OK)
    {
        pRouteParameters->privateConfig.prefs.numHovFontMap ++;
    }
    return err;
}

NB_Error PhraseTpsToHOVFontMap(NB_Context * context,
                               NB_RouteParameters* pRouteParameters,
                               tpselt tpsElements
                               )
{
    tpselt ce  = NULL;
    int iter = 0;
    NB_Error err = NE_OK;
    const char* name = NULL;

    if (!context || !pRouteParameters)
    {
        return NE_INVAL;
    }

    name = te_getname(tpsElements);

    if (nsl_strcmp(HOV_MAP_ROOT, name) != 0)
    {
        return NE_INVAL;
    }

    pRouteParameters->privateConfig.prefs.hovFontMapVersion = te_getattru(tpsElements, HOV_MAP_ROOT_ATTR_VERSION);

    while ((ce = te_nextchild(tpsElements, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), HOV_MAPPING_ROW) == 0)
        {
            err = err ? err : PhraseEntryToHOVFontMap(ce, pRouteParameters);
        }
    }
    return err;
}


NB_Error
GetFontMapFromBuffer(NB_Context * context,
                     char *datalib,
                     nb_size datalibSize,
                     char * fontMapBuffer,
                     nb_size fontMapBufferSize,
                     NB_RouteParameters *pRouteParameters
                     )
{
    struct tps_unpackstate *tpsups = NULL;
    struct tpslib*            tl = NULL;
    tpselt                    configTps;
    NB_Error err = NE_OK;

    if (! context || !pRouteParameters || !datalib || !fontMapBuffer)
    {
        return NE_INVAL;
    }

    tl = tpslib_preload(datalib, datalibSize);

    if (tl == NULL)
    {
        return NE_INVAL;
    }

    tpsups = tps_unpack_start(tl, fontMapBufferSize);

    if (tpsups == NULL)
    {
        return NE_INVAL;
    }

    (void) tps_unpack_feed(tpsups, fontMapBuffer, fontMapBufferSize); // tps_unpack_result will report failure

    if (tps_unpack_result(tpsups, &configTps) != TIO_READY)
    {
        return NE_BADDATA;
    }

    err =  PhraseTpsToFontMap(context, pRouteParameters, configTps);

    if (tl != NULL)
    {
        tpslib_dealloc(tl);
    }

    if (configTps != NULL)
    {
        te_dealloc(configTps);
    }

    return err;
}


NB_Error
GetHOVFontMapFromBuffer(NB_Context * context,
                        char *datalib,
                        nb_size datalibSize,
                        char * fontMapBuffer,
                        nb_size fontMapBufferSize,
                        NB_RouteParameters *pRouteParameters
                        )
{
    struct tps_unpackstate *tpsups = NULL;
    struct tpslib*            tl = NULL;
    tpselt                    configTps;
    NB_Error err = NE_OK;

    if (! context || !pRouteParameters || !datalib || !fontMapBuffer)
    {
        return NE_INVAL;
    }

    tl = tpslib_preload(datalib, datalibSize);

    if (tl == NULL)
    {
        return NE_INVAL;
    }

    tpsups = tps_unpack_start(tl, fontMapBufferSize);

    if (tpsups == NULL)
    {
        return NE_INVAL;
    }

    (void) tps_unpack_feed(tpsups, fontMapBuffer, fontMapBufferSize); // tps_unpack_result will report failure

    if (tps_unpack_result(tpsups, &configTps) != TIO_READY)
    {
        return NE_BADDATA;
    }

    err =  PhraseTpsToHOVFontMap(context, pRouteParameters, configTps);

    if (tl != NULL)
    {
        tpslib_dealloc(tl);
    }

    if (configTps != NULL)
    {
        te_dealloc(configTps);
    }

    return err;
}

NB_Error PhraseNavInstructToInstruction(tpselt tpsElements,
                                        NB_NavigationInstruction * instruct,
                                        NB_ManueverType * maneuverType
                                        )
{
    const char* name = NULL;

    if (!instruct || !maneuverType)
    {
        return NE_INVAL;
    }

    name = te_getname(tpsElements);
    if (nsl_strcmp(GUIDANCE_CONFIG_NAV_INSTRUCT_DISTANCE, name) != 0)
    {
        return NE_INVAL;
    }

    te_getattrf(tpsElements, "bdm", &instruct->baseDistMul);
    te_getattrf(tpsElements, "bda", &instruct->baseDistAdd);
    instruct->mult[base_dist] = 1.0;
    te_getattrf(tpsElements, "adm", &instruct->mult[announce_dist]);
    te_getattrf(tpsElements, "cdm", &instruct->mult[continue_dist]);
    te_getattrf(tpsElements, "pdm", &instruct->mult[prepare_dist]);
    te_getattrf(tpsElements, "pmm", &instruct->mult[prepare_min_dist]);
    te_getattrf(tpsElements, "idm", &instruct->mult[instruct_dist]);
    te_getattrf(tpsElements, "xdm", &instruct->mult[show_stack_dist]);
    te_getattrf(tpsElements, "vdm", &instruct->mult[show_vector_dist]);
    te_getattrf(tpsElements, "ldm", &instruct->mult[lane_guidance_dist]);
    te_getattrf(tpsElements, "stdm", &instruct->ShowTapeDistanceInMeters);
    te_getattrf(tpsElements, "stdi", &instruct->ShowTapeDistanceInInches);
    te_getattrf(tpsElements, "tdi", &instruct->TapeDistanceInInches);
    te_getattrf(tpsElements, "tdm", &instruct->TapeDistanceInMeters);

    name = te_getattrc(tpsElements, "name");

    if (nsl_strcmp(name, "normal") == 0)
    {
        *maneuverType = NBMT_Normal;
    }
    else if (nsl_strcmp(name, "highway") == 0)
    {
        *maneuverType = NBMT_Highway;
    }
    else if (nsl_strcmp(name, "merge") == 0)
    {
        *maneuverType = NBMT_Merge;
    }
    else if (nsl_strcmp(name, "origin") == 0)
    {
        *maneuverType = NBMT_Origin;
    }
    else if (nsl_strcmp(name, "destination") == 0)
    {
        *maneuverType = NBMT_Destination;
    }
    else if (nsl_strcmp(name, "camera") == 0)
    {
        *maneuverType = NBMT_Camera;
    }
    else if (nsl_strcmp(name, "ferry") == 0)
    {
        *maneuverType = NBMT_Ferry;
    }
    else
    {
        *maneuverType = NBMT_Max;
    };

    return NE_OK;
}

void NavigationInstructionInit(NB_NavigationInstruction *instruct)
{
    int i  = 0;
    if (!instruct)
    {
        return;
    }
    instruct->baseDistAdd = DISTANCE_CONFIG_INVALID_VALUE;
    instruct->baseDistMul = DISTANCE_CONFIG_INVALID_VALUE;
    instruct->ShowTapeDistanceInInches  = DISTANCE_CONFIG_INVALID_VALUE;
    instruct->ShowTapeDistanceInMeters  = DISTANCE_CONFIG_INVALID_VALUE;
    instruct->TapeDistanceInInches      = DISTANCE_CONFIG_INVALID_VALUE;
    instruct->TapeDistanceInMeters      = DISTANCE_CONFIG_INVALID_VALUE;
    for (i = 0 ; i < NBIDT_Max; i++)
    {
        instruct->mult[i] = DISTANCE_CONFIG_INVALID_VALUE;
    }

}

NB_Error PhraseDistanceToConfiguration(tpselt tpsElements,
                                       NB_RouteParameters* pRouteParameters
                                       )
{

    NB_NavigationInstruction instruct = {0.0};
    NB_ManueverType maneuverType = NBMT_Max;
    tpselt ce  = NULL;
    int iter = 0;
    NB_Error err = NE_OK;

    if (!pRouteParameters)
    {
        return NE_INVAL;
    }

    while ((ce = te_nextchild(tpsElements, &iter)) != NULL)
    {
        NavigationInstructionInit(&instruct);
        maneuverType = NBMT_Max;
        err = PhraseNavInstructToInstruction(ce, &instruct, &maneuverType);
        if (err == NE_OK)
        {
            NB_RouteParametersSetNavigationInstruction(pRouteParameters, &instruct, maneuverType);
        }
    }
    return NE_OK;

}


NB_Error PhraseNavCommandIndex(tpselt tpsElements,
                               const char ** command,
                               NB_ManueverType * maneuverType
                               )
{
    const char * index = NULL;
    const char * name = NULL;
    if (!maneuverType || !command)
    {
        return NE_INVAL;
    }

    name = te_getname(tpsElements);
    if (nsl_strcmp(GUIDANCE_CONFIG_COMMAND, name) != 0)
    {
        return NE_INVAL;
    }

    *command = te_getattrc(tpsElements, "name");
    index = te_getattrc(tpsElements, "index");
    if (nsl_strcmp(index, "normal") == 0)
    {
        *maneuverType = NBMT_Normal;
    }
    else if (nsl_strcmp(index, "highway") == 0)
    {
        *maneuverType = NBMT_Highway;
    }
    else if (nsl_strcmp(index, "merge") == 0)
    {
        *maneuverType = NBMT_Merge;
    }
    else if (nsl_strcmp(index, "origin") == 0)
    {
        *maneuverType = NBMT_Origin;
    }
    else if (nsl_strcmp(index, "destination") == 0)
    {
        *maneuverType = NBMT_Destination;
    }
    else if (nsl_strcmp (index, "camera") == 0)
    {
        *maneuverType = NBMT_Camera;
    }
    else if (nsl_strcmp (index, "ferry") == 0)
    {
        *maneuverType = NBMT_Ferry;
    }
    else
    {
        *maneuverType = NBMT_Max;
    };

    return NE_OK;

}

NB_Error PhraseCommandIndexToConfiguration(tpselt tpsElements,
                                           NB_RouteParameters* pRouteParameters
                                           )
{
    tpselt ce  = NULL;
    int iter = 0;
    NB_Error err = NE_OK;
    const char * command = NULL;
    NB_ManueverType type = NBMT_Normal;

    if (!pRouteParameters)
    {
        return NE_INVAL;
    }

    while ((ce = te_nextchild(tpsElements, &iter)) != NULL)
    {
        err = PhraseNavCommandIndex(ce, &command, &type);
        if (err == NE_OK)
        {
            NB_RouteParametersSetNavigationCommandType(pRouteParameters, command, type);
        }
    }
    return err;

}

NB_Error PhraseTpsToConfiguration(NB_Context * context,
                                  NB_RouteParameters* pRouteParameters,
                                  tpselt tpsElements
                                  )
{
    tpselt ce  = NULL;
    int iter = 0;
    NB_Error err = NE_OK;
    const char* name = NULL;

    if (!context || !pRouteParameters)
    {
        return NE_INVAL;
    }

    name = te_getname(tpsElements);

    if (nsl_strcmp(GUIDANCE_CONFIG, name) != 0)
    {
        return NE_INVAL;
    }

    while ((ce = te_nextchild(tpsElements, &iter)) != NULL)
    {
        name = te_getname(ce);
        if (nsl_strcmp(te_getname(ce), GUIDANCE_CONFIG_DISTANCE) == 0)
        {
            err = err ? err : PhraseDistanceToConfiguration(ce, pRouteParameters);
        }
        else if (nsl_strcmp(te_getname(ce), GUIDANCE_CONFIG_COMMAND_INDEX) == 0)
        {
            err = err ? err : PhraseCommandIndexToConfiguration(ce, pRouteParameters);
        }
        else if (nsl_strcmp(te_getname(ce), FONT_MAP_ROOT) == 0)
        {
            err = err ? err : PhraseTpsToFontMap(context, pRouteParameters, ce);
        }
    }
    return err;
}

NB_Error
GetConfigurationDataFromBuffer(NB_Context * context,
                               char *datalib,
                               nb_size datalibSize,
                               char * configBuffer,
                               nb_size configBufferSize,
                               NB_RouteParameters *pRouteParameters
                               )
{
    struct tps_unpackstate *tpsups = NULL;
    struct tpslib*            tl = NULL;
    tpselt                    configTps = NULL;
    NB_Error err = NE_OK;

    if (! context || !pRouteParameters || !datalib || !configBuffer)
    {
        return NE_INVAL;
    }

    tl = tpslib_preload(datalib, datalibSize);

    if (tl == NULL)
    {
        return NE_INVAL;
    }

    tpsups = tps_unpack_start(tl, configBufferSize);

    if (tpsups == NULL)
    {
        return NE_INVAL;
    }

    (void) tps_unpack_feed(tpsups, configBuffer, configBufferSize); // tps_unpack_result will report failure

    if (tps_unpack_result(tpsups, &configTps) != TIO_READY)
    {
        return NE_BADDATA;
    }

    err =  PhraseTpsToConfiguration(context, pRouteParameters, configTps);

    if (tl != NULL)
    {
        tpslib_dealloc(tl);
    }

    if (configTps != NULL)
    {
        te_dealloc(configTps);
    }

    return err;
}



/*! @} */
