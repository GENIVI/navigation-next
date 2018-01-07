#include "gaugenode.h"

#include <QtQuick/qsgnode.h>
#include <QtQuick/qsgflatcolormaterial.h>
#include <QtMath>

#define EXTRAVERTICES 3

GaugeNode::GaugeNode(const int &numVertices, const QColor &color = QColor(255, 0, 0),
                     const bool &doNotFill = false)
    : QSGGeometryNode()
    //Could be optimized more. If only geometry update used we do not need to map textured points.
    //, m_geometry(QSGGeometry::defaultAttributes_Point2D(), numVertices+EXTRAVERTICES)
    , m_geometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), numVertices+EXTRAVERTICES)
    , m_material(nullptr)
    , m_numVertices(numVertices)
    , m_doNotFill(doNotFill)
    , m_color(color)
    , m_cutRad(0.0)
    , m_updateGeometry(true)
    , m_lefttoright(true)
    , m_dirtyBits(0)
{
    initGeometry();
}

GaugeNode::~GaugeNode()
{
    if (m_material)
        delete m_material;
}

void GaugeNode::setColor(const QColor &color)
{
    if (m_color == color)
        return;
    m_color = color;
    m_dirtyBits |= QSGNode::DirtyMaterial;
}

void GaugeNode::setCutRad(const float &cutRad)
{
    if (m_cutRad == cutRad)
        return;

    m_cutRad = cutRad;
    if (!m_updateGeometry)
        m_dirtyBits |= QSGNode::DirtyMaterial;
}

void GaugeNode::setDoNotFill(const bool &doNotFill)
{
    if (m_doNotFill == doNotFill)
        return;

    m_doNotFill = doNotFill;

    if (m_doNotFill)
        m_geometry.setDrawingMode(GL_LINE_STRIP);
    else
        m_geometry.setDrawingMode(GL_TRIANGLE_STRIP);

    m_dirtyBits |= QSGNode::DirtyGeometry;
}

void GaugeNode::setBackCutRad(const double &backCutRad)
{
    if (backCutRad == m_backCutRad)
        return;

    m_backCutRad = backCutRad;
    m_dirtyBits |= QSGNode::DirtyGeometry;
}

void GaugeNode::setRadius(const double &radius)
{
    if (m_radius == radius)
        return;

    m_radius = radius;
    m_dirtyBits |= QSGNode::DirtyGeometry;
}

void GaugeNode::setArcDistPerVert(const double &dist)
{
    if (dist == m_arc_dist_per_vertices)
        return;
    m_arc_dist_per_vertices = dist;
    m_dirtyBits |= QSGNode::DirtyGeometry;
}

void GaugeNode::setNumVertices(const int &numVertices)
{
    if (numVertices == m_numVertices)
        return;

    m_numVertices = numVertices;
    m_geometry.allocate(m_numVertices + 3);
    m_dirtyBits |= QSGNode::DirtyGeometry;
}

void GaugeNode::setFillWidth(const double &fillWidth)
{
    if (m_fillWidth == fillWidth)
        return;

    m_fillWidth = fillWidth;
    m_dirtyBits |= QSGNode::DirtyGeometry;
}

void GaugeNode::setBoundingRect(const QRectF &rect)
{
    if (rect.width() == m_width && rect.height() == m_height)
        return;

    m_height = rect.height();
    m_width = rect.width();
    setCenterPoint(rect.center());
    m_dirtyBits |= QSGNode::DirtyGeometry;
}

void GaugeNode::setCenterPoint(const QPointF &center)
{
    m_center_x = center.x();
    m_center_y = center.y();
}

void GaugeNode::setUpdateGeometry(const bool &updateGeometry)
{
    if (m_updateGeometry == updateGeometry)
        return;

    m_updateGeometry = updateGeometry;

    if (m_material)
        delete m_material;

    if (m_updateGeometry) {
        QSGFlatColorMaterial *material = new QSGFlatColorMaterial;
        m_material = static_cast<QSGMaterial *>(material);
        material->setColor(m_color);
        setMaterial(m_material);
    } else {
        QSGSimpleMaterial<GaugeState> *material = GaugeShader::createMaterial();
        m_material = static_cast<QSGMaterial *>(material);
        material->state()->color = m_color;
        material->state()->cutRad = m_cutRad;
        material->state()->leftToRight = m_lefttoright;
        material->setFlag(QSGMaterial::Blending);
        setMaterial(m_material);
    }
    m_dirtyBits |= QSGNode::DirtyMaterial;
}

void GaugeNode::setLeftToRight(const bool &ltr)
{
    if (m_lefttoright == ltr)
        return;

    m_lefttoright = ltr;
    m_dirtyBits |= QSGNode::DirtyMaterial;
}

void GaugeNode::drawGeometryTexturePoint2D()
{
    QSGGeometry::TexturedPoint2D *vertices = m_geometry.vertexDataAsTexturedPoint2D();

    double current_angle_rad = 0.0;
    double currentRadius = m_radius;
    double d_arc_point_x = m_center_x + (currentRadius - m_fillWidth) * cos(m_backCutRad);
    double d_arc_point_y = m_center_y + (currentRadius - m_fillWidth) * sin(m_backCutRad);

    vertices[0].set(d_arc_point_x, d_arc_point_y,
                    d_arc_point_x / m_width, d_arc_point_y / m_height);
    d_arc_point_x = m_center_x + currentRadius * cos(m_backCutRad);
    d_arc_point_y = m_center_y + currentRadius * sin(m_backCutRad);
    vertices[1].set(d_arc_point_x, d_arc_point_y,
                    d_arc_point_x / m_width, d_arc_point_y / m_height);
    d_arc_point_x = 0;
    d_arc_point_y = 0;

    for (int i = 0; i < m_numVertices; ++i) {
        current_angle_rad = m_backCutRad + i * m_arc_dist_per_vertices + m_arc_dist_per_vertices;

        if (i % 2 == 0)
            currentRadius -= m_fillWidth;
        else
            currentRadius += m_fillWidth;

        d_arc_point_x = m_center_x + currentRadius * cos(current_angle_rad);
        d_arc_point_y = m_center_y + currentRadius * sin(current_angle_rad);
        vertices[i + 2].set(d_arc_point_x, d_arc_point_y,
                            d_arc_point_x / m_width, d_arc_point_y / m_height);
    }
    d_arc_point_x = m_center_x + (currentRadius - m_fillWidth) * cos(current_angle_rad);
    d_arc_point_y = m_center_y + (currentRadius - m_fillWidth) * sin(current_angle_rad);

    vertices[m_numVertices + 2].set(d_arc_point_x, d_arc_point_y,
                                    d_arc_point_x / m_width, d_arc_point_y / m_height);
}

void GaugeNode::drawMaterial()
{
    if (m_updateGeometry) {
        static_cast<QSGFlatColorMaterial *>(m_material)->setColor(m_color);
    } else {
        GaugeState *s = static_cast<QSGSimpleMaterial<GaugeState> *>(m_material)->state();
        s->color = m_color;
        s->cutRad = m_cutRad;
        s->leftToRight = m_lefttoright;
    }
}

void GaugeNode::draw()
{
    if (m_dirtyBits == 0)
        return;

    if (m_dirtyBits.testFlag(QSGNode::DirtyGeometry))
        drawGeometryTexturePoint2D();
    if (m_dirtyBits.testFlag(QSGNode::DirtyMaterial))
        drawMaterial();

    markDirty(m_dirtyBits);
    m_dirtyBits = 0;
}

//Could be used to optimize vertices if only geometry is changed
void GaugeNode::drawGeometry()
{
    QSGGeometry::Point2D *vertices = m_geometry.vertexDataAsPoint2D();

    double d_arc_point_x = 0.0;
    double d_arc_point_y = 0.0;
    double current_angle_rad = 0.0;
    double currentRadius = m_radius;

    vertices[0].set(m_center_x + (currentRadius - m_fillWidth) * cos(m_backCutRad), m_center_y
                    + (currentRadius - m_fillWidth) * sin(m_backCutRad));
    vertices[1].set(m_center_x + currentRadius * cos(m_backCutRad), m_center_y
                    + currentRadius * sin(m_backCutRad));

    for (int i = 0; i < m_numVertices; ++i) {
        current_angle_rad = m_backCutRad + i * m_arc_dist_per_vertices + m_arc_dist_per_vertices;

        if (i % 2 == 0)
            currentRadius -= m_fillWidth;
        else
            currentRadius += m_fillWidth;

        d_arc_point_x = m_center_x + currentRadius * cos(current_angle_rad);
        d_arc_point_y = m_center_y + currentRadius * sin(current_angle_rad);
        vertices[i + 2].set(d_arc_point_x, d_arc_point_y);
    }

    vertices[m_numVertices + 2].set(m_center_x + (currentRadius - m_fillWidth)
                                    * cos(current_angle_rad), m_center_y
                                    + (currentRadius - m_fillWidth) * sin(current_angle_rad));

    markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
}

void GaugeNode::initGeometry()
{
    m_geometry.setLineWidth(1);
    if (m_doNotFill)
        m_geometry.setDrawingMode(GL_LINE_STRIP);
    else
        m_geometry.setDrawingMode(GL_TRIANGLE_STRIP);

    setGeometry(&m_geometry);

    QSGFlatColorMaterial *material = new QSGFlatColorMaterial;
    material->setColor(m_color);

    m_material = static_cast<QSGMaterial *>(material);
    setMaterial(m_material);
}
