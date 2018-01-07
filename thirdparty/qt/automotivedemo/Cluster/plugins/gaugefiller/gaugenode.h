#ifndef GAUGENODE_H
#define GAUGENODE_H

#include <QSGNode>
#include <QSGSimpleMaterial>
#include <QColor>

struct GaugeState
{
    QColor color;
    GLfloat cutRad;
    bool leftToRight;

    int compare(const GaugeState *other) const
    {
        const unsigned int c = color.rgba();
        const unsigned int co = other->color.rgba();
        return std::tie(c, cutRad) > std::tie(co, other->cutRad);
    }
};

class GaugeShader : public QSGSimpleMaterialShader<GaugeState>
{
    QSG_DECLARE_SIMPLE_COMPARABLE_SHADER(GaugeShader, GaugeState)

public:
    const char *vertexShader() const {
        return
                "attribute highp vec4 aVertex;                              \n"
                "attribute highp vec2 aTexCoord;                            \n"
                "uniform highp mat4 qt_Matrix;                              \n"
                "varying highp vec2 texCoord;                               \n"
                "void main() {                                              \n"
                "    gl_Position = qt_Matrix * aVertex;                     \n"
                "    texCoord = aTexCoord;                                  \n"
                "}";
    }

    const char *fragmentShader() const {
        return
                "uniform lowp float qt_Opacity;                             \n"
                "uniform lowp vec4 color;                                   \n"
                "uniform highp float cutRad;                                \n"
                "uniform lowp bool leftToRight;                             \n"
                "varying highp vec2 texCoord;                               \n"
                "void main() {\n"
                "    highp vec2 uv = vec2(.5 - texCoord.y, .5 - texCoord.x);\n"
                "    if (leftToRight ? (-atan(uv.y,uv.x) < cutRad) : (-atan(uv.y,uv.x) > cutRad)) {\n"
                "        gl_FragColor = color * qt_Opacity;\n"
                "    } else {\n"
//debug color                "        gl_FragColor = vec4(0.,1.,0.,1.0);\n"
                "        gl_FragColor = vec4(0.,0.,0.,0.);\n"
                "    } \n"
                "}\n";
    }

    QList<QByteArray> attributes() const
    {
        return QList<QByteArray>() << "aVertex" << "aTexCoord";
    }

    void updateState(const GaugeState *state, const GaugeState *)
    {
        program()->setUniformValue(id_color, state->color);
        program()->setUniformValue(id_cutRad, state->cutRad);
        program()->setUniformValue(id_leftToRight, state->leftToRight);
    }

    void resolveUniforms()
    {
        id_color = program()->uniformLocation("color");
        id_cutRad = program()->uniformLocation("cutRad");
        id_leftToRight = program()->uniformLocation("leftToRight");
    }

private:
    int id_color;
    int id_cutRad;
    int id_leftToRight;
};

class GaugeNode : public QSGGeometryNode
{
public:
    GaugeNode(const int &numVertices, const QColor &color, const bool &doNotFill);
    ~GaugeNode();

    void setColor(const QColor &color);
    void setCutRad(const float &cutRad);

    void setDoNotFill(const bool &doNotFill);

    void setBackCutRad(const double &backCutRad);
    void setRadius(const double &radius);
    void setArcDistPerVert(const double &dist);
    void setNumVertices(const int &numVertices);

    void setFillWidth(const double &fillWidth);
    void setBoundingRect(const QRectF &rect);
    void setUpdateGeometry(const bool &updateGeometry);

    void setLeftToRight(const bool &ltr);

    void draw();

private:
    void initGeometry();
    void setCenterPoint(const QPointF &center);

    void drawGeometry();
    void drawGeometryTexturePoint2D();
    void drawMaterial();

private:
    QSGGeometry m_geometry;
    QSGMaterial *m_material;
    int m_numVertices;
    bool m_doNotFill;
    QColor m_color;
    float m_cutRad;
    double m_radius;
    bool m_updateGeometry;
    bool m_lefttoright;

    qreal m_width;
    qreal m_height;
    double m_center_y;
    double m_center_x;
    double m_backCutRad;
    double m_fillWidth;
    double m_arc_dist_per_vertices;

    DirtyState m_dirtyBits;
};

#endif // GAUGENODE_H
