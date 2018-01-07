#ifndef GAUGE_H
#define GAUGE_H

#include <QQuickItem>

class Gauge : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(qreal value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(qreal angle READ angle NOTIFY angleChanged)
    Q_PROPERTY(int numVertices READ numVertices WRITE setNumVertices NOTIFY numVerticesChanged)

    Q_PROPERTY(bool updateGeometry READ updateGeometry WRITE setUpdateGeometry NOTIFY updateGeometryChanged)
    Q_PROPERTY(double fillWidth READ fillWidth WRITE setFillWidth NOTIFY fillWidthChanged)
    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)

    Q_PROPERTY(double minAngle READ minAngle WRITE setMinAngle NOTIFY minAngleChanged)
    Q_PROPERTY(double maxAngle READ maxAngle WRITE setMaxAngle NOTIFY maxAngleChanged)

    Q_PROPERTY(double minValue READ minValue WRITE setMinValue NOTIFY minValueChanged)
    Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue NOTIFY maxValueChanged)

    Q_PROPERTY(bool doNotFill READ doNotFill WRITE setDoNotFill NOTIFY doNotFillChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    Gauge(QQuickItem *parent = 0);
    ~Gauge();

    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

    qreal value() const { return m_value; }
    qreal angle() const { return m_angle; }
    bool updateGeometry() const { return m_updateGeometry; }
    int numVertices() const { return m_numVertices; }
    double fillWidth() const { return m_fillWidth; }
    int  radius() const { return m_radius; }
    double minAngle() const { return m_minAngle; }
    double maxAngle() const { return m_maxAngle; }
    double minValue() const { return m_minValue; }
    double maxValue() const { return m_maxValue; }
    bool doNotFill() const { return m_doNotFill; }
    QColor color() const { return m_color; }

    void setValue(qreal value);
    void setNumVertices(int numVertices);
    void setFillWidth(double fillWidth);
    void setRadius(int radius);
    void setMinAngle(double minAngle);
    void setMaxAngle(double maxAngle);
    void setMinValue(double minValue);
    void setMaxValue(double maxValue);
    void setDoNotFill(bool doNotFill);
    void setColor(QColor color);
    void setUpdateGeometry(bool updateGeometry);

signals:
    void valueChanged(qreal value);
    void angleChanged(qreal angle);
    void numVerticesChanged(int numVertices);
    void fillWidthChanged(double fillWidth);
    void radiusChanged(int radius);
    void minAngleChanged(double minAngle);
    void maxAngleChanged(double maxAngle);
    void minValueChanged(double minValue);
    void maxValueChanged(double maxValue);
    void doNotFillChanged(bool doNotFill);
    void colorChanged(QColor color);
    void updateGeometryChanged(bool updateGeometry);

public slots:

private:
    void calcArc();
    float calcValueAsRad(qreal value);
    void updateValue();

private:
    qreal m_value;
    double m_angle;
    int m_numVertices;
    double m_fillWidth;
    int m_radius;
    bool m_updateGeometry;
    bool m_lefttoright;

    double m_minAngle;
    double m_maxAngle;
    double m_minValue;
    double m_maxValue;

    bool m_doNotFill;
    QColor m_color;

    //Internal
    double arc_length;
    double arc_dist_per_vertices;

    double frontCutDeg;
    double backCutDeg;

    double frontCutRad;
    double backCutRad;

    float m_cutRad;
};

#endif // GAUGE_H
