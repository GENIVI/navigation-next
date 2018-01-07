#ifndef VALUETYPES
#define VALUETYPES

namespace NMEAType
{
    enum {
        NMEAStart = 0x01,
        NMEAEnd = 0x0f,

        GeoBaseStart = 0x10,
        GeoBaseEnd = 0x1f,

        GGAStart  = 0x20,
        GGAEnd = 0x2f,

        RMCStart = 0x30,
        RMCEnd = 0x3f,
    };

    enum Type {
        UndefinedType = 0,
        GGAType,
        RMCType
    };

    enum Value {
        UTCTime = NMEAStart,
        CheckSum,

        Latitude = GeoBaseStart,
        Longitude,

        GGAQuality = GGAStart,
        GGASatellites,
        GGADilution,
        GGAAltitude,
        GGAGeoid,

        RMCActive = RMCStart,
        RMCSpeed,
        RMCAngle,
        RMCDate,
        RMCVariation

    };
}

#endif // VALUETYPES

