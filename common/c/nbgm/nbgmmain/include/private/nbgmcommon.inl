#include "nbgmmapscaleutility.h"
#include "palmath.h"
inline int8 CalcZoomLevel(float distance)
{
    float relativeCameraHeight = NBGM_GetConfig()->relativeCameraDistance;
    int8 relativeZoomLevel = NBGM_GetConfig()->relativeZoomLevel;

    double zoomlevel = relativeZoomLevel - nsl_log2(distance / relativeCameraHeight);
    if (zoomlevel > 22)
    {
        zoomlevel = 22;
    }
    if (zoomlevel < 0)
    {
        zoomlevel = 0;
    }
    // For now, float is taken as the type of zoom level instead of double internally.
    // We should follow the unifom data type.
    // Because precision lost will occur in convertion between double and float,
    // and the result of truncation is quite diffrent.
    return static_cast<int8>(static_cast<float>(zoomlevel));
}

inline float
CalcZoomLevelFloat(float distance)
{
    float relativeCameraHeight = NBGM_GetConfig()->relativeCameraDistance;
    int8 relativeZoomLevel = NBGM_GetConfig()->relativeZoomLevel;

    double zoomlevel = relativeZoomLevel - nsl_log2(distance / relativeCameraHeight);
    if (zoomlevel > 22)
    {
        zoomlevel = 22;
    }
    if (zoomlevel < 0)
    {
        zoomlevel = 0;
    }

    return static_cast<float>(zoomlevel);
}

inline float
CalcDistanceFromZoomLevel(double zoomLevel)
{
    float relativeCameraHeight = NBGM_GetConfig()->relativeCameraDistance;
    int8 relativeZoomLevel = NBGM_GetConfig()->relativeZoomLevel;
    return static_cast<float>(relativeCameraHeight * pow(2, relativeZoomLevel - zoomLevel));
}

inline float CalcPixelResolution(int8 zoomlevel)
{
    int8 relativeZoomLevel = NBGM_GetConfig()->relativeZoomLevel;
    float mapCellSizeOnRelativeZoomlevel =NBGM_GetConfig()->metersPerPixelOnRelativeZoomLevel;
    return static_cast<float>(mapCellSizeOnRelativeZoomlevel * nsl_pow(2, (relativeZoomLevel - zoomlevel)));
}

inline nb_boolean CheckNearFarVisibility(float distance, float nearVisibility, float farVisibility)
{
    distance = distance / NBGM_RELATIVE_CAMERA_DISTANCE_MULTIPLE;
    distance = static_cast<float>(static_cast<int32>(distance));
    if (!NBGM_GetConfig()->useRoadBuildingVisibility)
    {
        return TRUE;
    }

    if (nearVisibility >= farVisibility)
    {
        return TRUE;
    }

    if (nearVisibility < 0.0f || farVisibility < 0.0f)
    {
        return TRUE;
    }

    if(distance < 1e-6f)
    {
        return TRUE;
    }

    if (distance <= nearVisibility || distance > farVisibility)
    {
        return FALSE;
    }

    return TRUE;
}

inline float METER_TO_MERCATOR(float x){return (float)( x/RADIUS_EARTH_METERS); }
inline float MERCATER_TO_METER(float x){ return (float)( x * RADIUS_EARTH_METERS); }
inline double METER_TO_MERCATOR(double x){ return x/RADIUS_EARTH_METERS; }
inline double MERCATER_TO_METER(double  x){ return x * RADIUS_EARTH_METERS;}
inline float MM_TO_PIXEL(float mm, float dpi){ return mm*dpi/ 25.4f;}

inline nb_boolean VERIFY_NBM_INDEX(uint16 index)
{
    return index != NBRE_INVALID_SHORT_INDEX;
}

inline nb_boolean VERIFY_NBM_INDEX(uint32 index)
{
    return index != NBRE_INVALID_INDEX;
}

inline nb_boolean VERIFY_NBM_INTERNAL_INDEX(uint16 index)
{
    if ((index & 0x8000) == 0)
    {
        return TRUE;
    }
    return FALSE;
}

inline uint16 MAKE_NBM_INDEX(uint16 index)
{
    return index & 0x7fff;
}

inline float DP_TO_PIXEL(float dp)
{
    return dp*NBGM_GetConfig()->dpi / 160.0f;
}

inline float PIXEL_TO_DP(float pixel)
{
    return pixel*160.0f / NBGM_GetConfig()->dpi;
}

inline NBRE_String GenerateID(const NBRE_String& categoryName, const NBRE_String& name)
{
    return NBRE_String(categoryName + "\\" + name);
}

inline float GetPolylineScaleFactor(int8 zoomLevel, nb_boolean isGEOPolyline)
{
    float zoomLevelFrom16To22[] = {1.5f, 1.0f, 0.7f, 0.5f, 0.3f, 0.2f, 0.1f};

    if(zoomLevel < 0 || zoomLevel > 22)
    {
        return 1.0f;
    }

    if(zoomLevel >= 16)
    {
        return zoomLevelFrom16To22[zoomLevel - 16];
    }

    if(isGEOPolyline)
    {
        return 1.5f * 2.0f * static_cast<float>(nsl_pow(2, (15 - zoomLevel)));
    }
    else
    {
        return 1.5f;
    }
}