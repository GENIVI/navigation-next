#ifndef NMEA_H
#define NMEA_H

#include <QStringList>
#include <QDateTime>
#include <QByteArray>
#include <QList>
#include <QVariant>

#include "valuetypes.h"

class NMEA
{
protected:
    NMEA();

public:
    virtual ~NMEA();

    // NOTE: ownership is changed, caller must delete
    static NMEA* parse(QString data);
    bool isValid() const;
    NMEAType::Type type() const;
    virtual QVariant value(NMEAType::Value type) const;
    virtual void setValue(NMEAType::Value type, QVariant value);

protected:
    struct NMEAPrivate* m_private;

};


// GeoBase

class GeoBase : public NMEA
{
protected:
    GeoBase();

public:
    virtual ~GeoBase();
    virtual QVariant value(NMEAType::Value type) const;
    virtual void setValue(NMEAType::Value type, QVariant value);
};


// GGA

class GGA : public GeoBase
{
public:
    virtual ~GGA();
    virtual QVariant value(NMEAType::Value type) const;
    virtual void setValue(NMEAType::Value type, QVariant value);

private:
    GGA(QStringList data);
    friend class NMEA;
};


// RMC

class RMC : public GeoBase
{
public:
    virtual ~RMC();
    virtual QVariant value(NMEAType::Value type) const;
    virtual void setValue(NMEAType::Value type, QVariant value);

private:
    RMC(QStringList data);
    friend class NMEA;
};

#endif // NMEA_H
