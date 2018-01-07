#include "nmea.h"

#include <QObject>
#include <QtMath>

#include <QDebug>


// NMEAPrivate
struct NMEAPrivate
{
    NMEAPrivate() : m_checksum(0), m_type(NMEAType::UndefinedType) {
    }
    int m_checksum;
    NMEAType::Type m_type;
    QTime m_taken;
};


// NMEA
NMEA::NMEA() : m_private(new NMEAPrivate)
{
}

NMEA::~NMEA()
{
    delete m_private;
    m_private = NULL;
}

NMEA* NMEA::parse(QString data)
{
    NMEA* nmea(Q_NULLPTR);

    int lastIndex(data.lastIndexOf("*"));
    int checksum(0);
    for (int index(data.indexOf("$") + 1); index < lastIndex; index++) {
        checksum ^= (int)data.at(index).toLatin1();
    }

    QString checksumString(QString::number(checksum, 16));
    if (checksum <= 0xf) {
        checksumString.prepend("0");
    }
    QString compareChecksum(data.mid(lastIndex + 1));
    if (compareChecksum.compare(checksumString)) {
        qWarning("NMEA::parse - Error invalid checksum %s != %s", qPrintable(compareChecksum), qPrintable(QString::number(checksum, 16)));
        return nmea;
    }

    QStringList dataList(data.left(lastIndex).split(","));
    if (dataList.count() <= 1) {
        return nmea;
    }

    QString type(dataList.at(0));

    if (type.compare("$GPGGA", Qt::CaseInsensitive) == 0) {
        nmea = new GGA(dataList);
    }

    else if (type.compare("$GPRMC", Qt::CaseInsensitive) == 0) {
        nmea = new RMC(dataList);
    }

    if (!nmea) {
        qWarning("NMEA::parse - Error NMEA creation error");
    }
    else if (nmea->isValid()) {
        nmea->m_private->m_checksum = checksum;
    }
    else {
        qWarning("NMEA::parse - Error NMEA is not valid, deleting it");
        delete nmea;
        nmea = Q_NULLPTR;
    }

    return nmea;
}

bool NMEA::isValid() const
{
    Q_ASSERT(m_private != Q_NULLPTR);
    return (m_private && m_private->m_type != NMEAType::UndefinedType);
}

NMEAType::Type NMEA::type() const
{
    Q_ASSERT(m_private != Q_NULLPTR);
    return (m_private ? m_private->m_type : NMEAType::UndefinedType);
}

QVariant NMEA::value(NMEAType::Value type) const
{
    Q_ASSERT(m_private != Q_NULLPTR);
    if (type == NMEAType::UTCTime) {
        return QVariant::fromValue<QTime>(m_private->m_taken);
    }
    else if (type == NMEAType::CheckSum) {
        return QVariant::fromValue<int>(m_private->m_checksum);
    }
    else return QVariant();
}

void NMEA::setValue(NMEAType::Value type, QVariant value)
{
    Q_ASSERT(m_private != Q_NULLPTR);
    if (type == NMEAType::UTCTime && value.type() == QVariant::Time) {
        m_private->m_taken = value.toTime();
    }
    else if (type == NMEAType::CheckSum && value.type() == QVariant::Int) {
        m_private->m_checksum = value.toInt();
    }
}

// GeoBasePrivate
struct GeoBasePrivate : public NMEAPrivate
{
    GeoBasePrivate() : m_latitude(0.0), m_longitude(0.0) {
    }
    qreal m_latitude;
    qreal m_longitude;
};


// GeoBasePrivate
GeoBase::GeoBase()
{
}

GeoBase::~GeoBase()
{
}

QVariant GeoBase::value(NMEAType::Value type) const
{
    Q_ASSERT(m_private != Q_NULLPTR);
    if (type >= (NMEAType::Value)NMEAType::NMEAStart && type <= (NMEAType::Value)NMEAType::NMEAEnd) {
        return NMEA::value(type);
    }
    else {
        GeoBasePrivate* geoBasePrivate((GeoBasePrivate*)m_private);
        switch (type) {
        case NMEAType::Latitude : return QVariant::fromValue<qreal>(geoBasePrivate->m_latitude);
        case NMEAType::Longitude : return QVariant::fromValue<qreal>(geoBasePrivate->m_longitude);
        default: /* do nothing */ break;
        }
    }
    return QVariant();
}

void GeoBase::setValue(NMEAType::Value type, QVariant value)
{
    Q_ASSERT(m_private != Q_NULLPTR);
    if (type >= (NMEAType::Value)NMEAType::NMEAStart && type <= (NMEAType::Value)NMEAType::NMEAEnd) {
        NMEA::setValue(type, value);
    }
    else {
        GeoBasePrivate* geoBasePrivate((GeoBasePrivate*)m_private);
        switch (type) {
        case NMEAType::Latitude :  geoBasePrivate->m_latitude = value.toDouble(); break;
        case NMEAType::Longitude : geoBasePrivate->m_longitude = value.toDouble(); break;
        default: /* do nothing */ break;
        }
    }
}


// GGA
struct GGAPrivate : public GeoBasePrivate
{
    GGAPrivate() : m_quality(0), m_satellites(0), m_dilution(0.0), m_altitude(0.0), m_geoid(0.0) {
    }
    int m_quality;
    int m_satellites;
    qreal m_dilution;
    qreal m_altitude;
    qreal m_geoid;
};

GGA::~GGA() {
}

QVariant GGA::value(NMEAType::Value type) const
{
    Q_ASSERT(m_private != Q_NULLPTR);
    if (type >= (NMEAType::Value)NMEAType::NMEAStart && type <= (NMEAType::Value)NMEAType::GeoBaseEnd) {
        return GeoBase::value(type);
    }
    else {
        GGAPrivate* ggaPrivate((GGAPrivate*)m_private);
        switch (type) {
        case NMEAType::GGAQuality : return QVariant::fromValue<int>(ggaPrivate->m_quality);
        case NMEAType::GGASatellites : return QVariant::fromValue<int>(ggaPrivate->m_satellites);
        case NMEAType::GGADilution : return QVariant::fromValue<qreal>(ggaPrivate->m_dilution);
        case NMEAType::GGAAltitude : return QVariant::fromValue<qreal>(ggaPrivate->m_altitude);
        case NMEAType::GGAGeoid : return QVariant::fromValue<qreal>(ggaPrivate->m_geoid);
        default: /* do nothing */ break;
        }
    }
    return QVariant();

}

void GGA::setValue(NMEAType::Value type, QVariant value)
{
    Q_ASSERT(m_private != Q_NULLPTR);
    if (type >= (NMEAType::Value)NMEAType::NMEAStart && type <= (NMEAType::Value)NMEAType::GeoBaseEnd) {
        return GeoBase::setValue(type, value);
    }
    else {
        GGAPrivate* ggaPrivate((GGAPrivate*)m_private);
        switch (type) {
        case NMEAType::GGAQuality : ggaPrivate->m_quality = value.toInt(); break;
        case NMEAType::GGASatellites : ggaPrivate->m_satellites = value.toInt(); break;
        case NMEAType::GGADilution : ggaPrivate->m_dilution = value.toDouble(); break;
        case NMEAType::GGAAltitude : ggaPrivate->m_altitude = value.toDouble(); break;
        case NMEAType::GGAGeoid : ggaPrivate->m_geoid = value.toDouble(); break;
        default: /* do nothing */ break;
        }
    }
}

GGA::GGA(QStringList data)
{
    GGAPrivate* newPrivate = new GGAPrivate;
    if (data.count() == 15) {
        QString utcTime(data.at(1));
        if (utcTime.length() > 6) {
            newPrivate->m_taken = QTime::fromString(utcTime, "hhmmss.z");
        }
        else {
            newPrivate->m_taken = QTime::fromString(utcTime, "hhmmss");
        }

        bool ok(false);
        qreal position(data.at(2).toDouble(&ok));
        if (ok) {
            qreal degrees(qFloor(0.01 * position));
            newPrivate->m_latitude = (degrees + ((position - (100.0 * degrees)) / 60));
            if (data.at(3).compare("S", Qt::CaseInsensitive) == 0) {
                newPrivate->m_latitude *= -1.0;
            }
        }
        else {
            qWarning("GGA::GGA - Error in latitude parse");
        }

        position = data.at(4).toDouble(&ok);
        if (ok) {
            qreal degrees(qFloor(0.01 * position));
            newPrivate->m_longitude = (degrees + ((position - (100.0 * degrees)) / 60));
            if (data.at(5).compare("W", Qt::CaseInsensitive) == 0) {
                newPrivate->m_longitude *= -1.0;
            }
        }
        else {
            qWarning("GGA::GGA - Error in longitude parse");
        }
        newPrivate->m_quality = data.at(6).toInt(&ok);
        if (!ok) {
            qWarning("GGA::GGA - Error in quality parse");
        }
        newPrivate->m_satellites = data.at(7).toInt(&ok);
        if (!ok) {
            qWarning("GGA::GGA - Error in satellites parse");
        }
        newPrivate->m_dilution = data.at(8).toDouble(&ok);
        if (!ok) {
            qWarning("GGA::GGA - Error in dilution parse");
        }
        newPrivate->m_altitude = data.at(9).toDouble(&ok);
        if (!ok) {
            qWarning("GGA::GGA - Error in altitude parse");
        }
        if (data.at(10).compare("M")) {
            qWarning("GGA::GGA - Error in altitude type parse");
        }
        newPrivate->m_geoid = data.at(11).toDouble(&ok);
        if (!ok) {
            qWarning("GGA::GGA - Error in height of geoid parse");
        }
        if (data.at(12).compare("M")) {
            qWarning("GGA::GGA - Error in height of geoid type parse");
        }
        if (!data.at(13).isEmpty()) {
            qWarning("GGA::GGA - Error in not empty field 14 - data: %s", qPrintable(data.at(13)));
        }
        if (!data.at(14).isEmpty()) {
            qWarning("GGA::GGA - Error in not empty field 15 - data: %s", qPrintable(data.at(14)));
        }
    }
    else {
        qWarning("GGA::GGA - Error incorrect amount of data %d available!", data.count());
    }
    delete m_private;
    newPrivate->m_type = NMEAType::GGAType;
    m_private = newPrivate;
}


// RMC
struct RMCPrivate : public GeoBasePrivate
{
    RMCPrivate() : m_active(false), m_speed(0.0), m_angle(0.0), m_variation(0.0) {
    }
    bool m_active;
    qreal m_speed;
    qreal m_angle;
    QDate m_date;
    qreal m_variation;
};

RMC::~RMC()
{
}

QVariant RMC::value(NMEAType::Value type) const
{
    Q_ASSERT(m_private != Q_NULLPTR);
    if (type >= (NMEAType::Value)NMEAType::NMEAStart && type <= (NMEAType::Value)NMEAType::GeoBaseEnd) {
        return GeoBase::value(type);
    }
    else {
        RMCPrivate* rmcPrivate((RMCPrivate*)m_private);
        switch (type) {
        case NMEAType::RMCActive : return QVariant::fromValue<bool>(rmcPrivate->m_active);
        case NMEAType::RMCSpeed : return QVariant::fromValue<qreal>(rmcPrivate->m_speed);
        case NMEAType::RMCAngle : return QVariant::fromValue<qreal>(rmcPrivate->m_angle);
        case NMEAType::RMCDate : return QVariant::fromValue<QDate>(rmcPrivate->m_date);
        case NMEAType::RMCVariation : return QVariant::fromValue<qreal>(rmcPrivate->m_variation);
        default: /* do nothing */ break;
        }
    }
    return QVariant();
}

void RMC::setValue(NMEAType::Value type, QVariant value)
{
    Q_ASSERT(m_private != Q_NULLPTR);
    if (type >= (NMEAType::Value)NMEAType::NMEAStart && type <= (NMEAType::Value)NMEAType::GeoBaseEnd) {
        return GeoBase::setValue(type, value);
    }
    else {
        RMCPrivate* rmcPrivate((RMCPrivate*)m_private);
        switch (type) {
        case NMEAType::RMCActive : rmcPrivate->m_active = value.toBool(); break;
        case NMEAType::RMCSpeed : rmcPrivate->m_speed = value.toDouble(); break;
        case NMEAType::RMCAngle : rmcPrivate->m_angle = value.toDouble(); break;
        case NMEAType::RMCDate : rmcPrivate->m_date = value.toDate(); break;
        case NMEAType::RMCVariation : rmcPrivate->m_variation = value.toDouble(); break;
        default: /* do nothing */ break;
        }
    }
}

RMC::RMC(QStringList data)
{
    RMCPrivate* newPrivate = new RMCPrivate;
    if (data.count() == 13) {
        QString utcTime(data.at(1));
        if (utcTime.length() > 6) {
            newPrivate->m_taken = QTime::fromString(utcTime, "hhmmss.z");
        }
        else {
            newPrivate->m_taken = QTime::fromString(utcTime, "hhmmss");
        }

        newPrivate->m_active = (data.at(2).compare("A") == 0);

        bool ok(false);
        qreal position(data.at(3).toDouble(&ok));
        if (ok) {
            qreal degrees(qFloor(0.01 * position));
            newPrivate->m_latitude = (degrees + ((position - (100.0 * degrees)) / 60));
            if (data.at(4).compare("S", Qt::CaseInsensitive) == 0) {
                newPrivate->m_latitude *= -1.0;
            }
        }
        else {
            qWarning("RMC::RMC - Error in latitude parse");
        }
        position = data.at(5).toDouble(&ok);
        if (ok) {
            qreal degrees(qFloor(0.01 * position));
            newPrivate->m_longitude = (degrees + ((position - (100.0 * degrees)) / 60));
            if (data.at(6).compare("W", Qt::CaseInsensitive) == 0) {
                newPrivate->m_longitude *= -1.0;
            }
        }
        else  {
            qWarning("RMC::RMC - Error in longitude parse");
        }
        newPrivate->m_speed = data.at(7).toDouble(&ok);
        if (!ok) {
            qWarning("RMC::RMC - Error in speed parse");
        }
        newPrivate->m_angle = data.at(8).toDouble(&ok);
        if (!ok) {
            qWarning("RMC::RMC - Error in angle parse, data: %s", qPrintable(data.at(8)));
        }
        newPrivate->m_date = QDate::fromString(data.at(9), "ddMMyy");

        newPrivate->m_variation = data.at(10).toDouble(&ok);
        if (!ok) {
            qWarning("RMC::RMC - Error in variation parse");
        }
        if (data.at(11).compare("W", Qt::CaseInsensitive) == 0) {
            newPrivate->m_variation *= -1.0;
        }
    }
    else {
        qWarning("RMC::RMC - Error incorrect amount of data %d available!", data.count());
    }
    delete m_private;
    newPrivate->m_type = NMEAType::RMCType;
    m_private = newPrivate;
}
