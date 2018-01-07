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

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildTranslateMatrix(Real x, Real y, Real z)
{
    return NBRE_Matrix4x4<Real>(
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, 1);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildScaleMatrix(Real x, Real y, Real z)
{
    return NBRE_Matrix4x4<Real>(
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildRotateMatrix(Real radians, Real x, Real y, Real z)
{
    Real vecLengthInverse = static_cast<Real>(1.0 / nsl_sqrt(x * x + y * y + z * z ));

    if (vecLengthInverse != vecLengthInverse)
    {
        return NBRE_Matrix4x4<Real>();
    }

    x *= vecLengthInverse;
    y *= vecLengthInverse;
    z *= vecLengthInverse;

    Real sinSave = static_cast<Real>(nsl_sin(radians));
    Real cosSave = static_cast<Real>(nsl_cos(radians));
    Real oneMinusCos = static_cast<Real>(1.0 - cosSave);
    Real xx = x * x;
    Real yy = y * y;
    Real zz = z * z;
    Real xy = x * y;
    Real yz = y * z;
    Real zx = z * x;
    Real xs = x * sinSave;
    Real ys = y * sinSave;
    Real zs = z * sinSave;

    return NBRE_Matrix4x4<Real>(
        (oneMinusCos * xx) + cosSave, (oneMinusCos * xy) - zs     , (oneMinusCos * zx) + ys     , 0,
        (oneMinusCos * xy) + zs     , (oneMinusCos * yy) + cosSave, (oneMinusCos * yz) - xs     , 0,
        (oneMinusCos * zx) - ys     , (oneMinusCos * yz) + xs     , (oneMinusCos * zz) + cosSave, 0,
        0                           , 0                           , 0                           , 1);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildRotateDegreesMatrix(Real degrees, Real x, Real y, Real z)
{
    return BuildRotateMatrix(static_cast<Real>(NBRE_Math::Pi64 * degrees / 180.0), x, y, z);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildTranslateMatrix(const NBRE_Vector3<Real>& translate)
{
    return BuildTranslateMatrix(translate.x, translate.y, translate.z);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildScaleMatrix(const NBRE_Vector3<Real>& scale)
{
    return BuildScaleMatrix(scale.x, scale.y, scale.z);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildRotateMatrix(Real radians, const NBRE_Vector3<Real>& axis)
{
    return BuildRotateMatrix(radians, axis.x, axis.y, axis.z);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildRotateDegreesMatrix(Real degrees, const NBRE_Vector3<Real>& axis)
{
    return BuildRotateDegreesMatrix(degrees, axis.x, axis.y, axis.z);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildRotateAtMatrix(Real radians, const NBRE_Vector3<Real>& axis, const NBRE_Vector3<Real>& position)
{
    return BuildTranslateMatrix(position) * BuildRotateMatrix(radians, axis) * BuildTranslateMatrix(-position);
}

template <typename Real>
inline NBRE_Vector3<Real> 
NBRE_Transformation<Real>::CoordinatesGetTranslate(const NBRE_Matrix4x4<Real>& matrix)
{
    return NBRE_Vector3<Real>(matrix[0][3], matrix[1][3], matrix[2][3]);
}

template <typename Real>
inline NBRE_Vector3<Real> 
NBRE_Transformation<Real>::CoordinatesGetXAxis(const NBRE_Matrix4x4<Real>& matrix)
{
    return NBRE_Vector3<Real>(matrix[0][0], matrix[1][0], matrix[2][0]);
}

template <typename Real>
inline NBRE_Vector3<Real> 
NBRE_Transformation<Real>::CoordinatesGetYAxis(const NBRE_Matrix4x4<Real>& matrix)
{
    return NBRE_Vector3<Real>(matrix[0][1], matrix[1][1], matrix[2][1]);
}

template <typename Real>
inline NBRE_Vector3<Real> 
NBRE_Transformation<Real>::CoordinatesGetZAxis(const NBRE_Matrix4x4<Real>& matrix)
{
    return NBRE_Vector3<Real>(matrix[0][2], matrix[1][2], matrix[2][2]);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildLookAtMatrix(const NBRE_Vector3<Real>& eye, const NBRE_Vector3<Real>& center, const NBRE_Vector3<Real>& up)
{
    NBRE_Vector3<Real> z = eye - center;
    z.Normalise();
    NBRE_Vector3<Real> x = up.CrossProduct(z);
    x.Normalise();
    NBRE_Vector3<Real> y = z.CrossProduct(x);
    y.Normalise();

    return NBRE_Matrix4x4<Real>(
        x.x, x.y, x.z, -x.DotProduct(eye),
        y.x, y.y, y.z, -y.DotProduct(eye),
        z.x, z.y, z.z, -z.DotProduct(eye),
        0,   0,   0,      1);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildOrthoMatrix(Real left, Real right, Real bottom, Real top, Real zNear, Real zFar)
{
    Real dxInv = 1 / (right - left);
    Real dyInv = 1 / (top - bottom);
    Real dzInv = 1 / (zFar - zNear);

    return NBRE_Matrix4x4<Real>(
        2 * dxInv,          0,          0,  (right + left) * -dxInv,
                0,  2 * dyInv,          0,  (top + bottom) * -dyInv,
                0,          0, -2 * dzInv,  (zFar + zNear) * -dzInv,
                0,          0,          0,                        1);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildFrustumMatrix(Real left, Real right, Real bottom, Real top, Real nearZ, Real farZ)
{
    Real dxInv = 1 / (right - left);
    Real dyInv = 1 / (top - bottom);
    Real dzInv = 1 / (farZ - nearZ);
    Real near2 = 2 * nearZ;

    return NBRE_Matrix4x4<Real>(
        near2 * dxInv,              0,  (right + left) * dxInv,                     0,
                    0,  near2 * dyInv,  (top + bottom) * dyInv,                     0,
                    0,              0, (farZ + nearZ) * -dzInv, near2 * farZ * -dzInv,
                    0,              0,                      -1,                     0);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildPerspectiveMatrix(Real fovYDegrees, Real aspect, Real nearZ, Real farZ)
{
    Real top = static_cast<Real>(nsl_tan(fovYDegrees * 0.5 * NBRE_Math::Pi64 / 180.0) * nearZ);
    Real right = top * aspect;
    return BuildFrustumMatrix(-right, right, -top, top, nearZ, farZ);
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Transformation<Real>::BuildViewportMatrix(Real x, Real y, Real width, Real height)
{
    Real halfW = width / 2;
    Real halfH = height / 2;

    return NBRE_Matrix4x4<Real>(
        halfW,     0, 0, halfW + x,
            0, halfH, 0, halfH + y,
            0,     0, 1,         0,
            0,     0, 0,         1);
}

template <typename Real>
inline void 
NBRE_Transformation<Real>::Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Vector3<Real>& vec3)
{
    Real fInvW = 1 / (matrix[3][0] + matrix[3][1] + matrix[3][2] + matrix[3][3]);
    Real x = (matrix[0][0] * vec3.x + matrix[0][1] * vec3.y + matrix[0][2] * vec3.z + matrix[0][3]) * fInvW;
    Real y = (matrix[1][0] * vec3.x + matrix[1][1] * vec3.y + matrix[1][2] * vec3.z + matrix[1][3]) * fInvW;
    Real z = (matrix[2][0] * vec3.x + matrix[2][1] * vec3.y + matrix[2][2] * vec3.z + matrix[2][3]) * fInvW;
    vec3.x = x;
    vec3.y = y;
    vec3.z = z;
}

template <typename Real>
inline void 
NBRE_Transformation<Real>::Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Vector4<Real>& vec4)
{
    Real x = matrix[0][0] * vec4.x + matrix[0][1] * vec4.y + matrix[0][2] * vec4.z + matrix[0][3] * vec4.w;
    Real y = matrix[1][0] * vec4.x + matrix[1][1] * vec4.y + matrix[1][2] * vec4.z + matrix[1][3] * vec4.w;
    Real z = matrix[2][0] * vec4.x + matrix[2][1] * vec4.y + matrix[2][2] * vec4.z + matrix[2][3] * vec4.w;
    Real w = matrix[3][0] * vec4.x + matrix[3][1] * vec4.y + matrix[3][2] * vec4.z + matrix[3][3] * vec4.w;
    vec4.x = x;
    vec4.y = y;
    vec4.z = z;
    vec4.w = w;
}

template <typename Real>
inline void 
NBRE_Transformation<Real>::Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Ray3<Real>& ray)
{
    NBRE_Vector3<Real> endPoint(ray.origin + ray.direction);
    Transform(matrix, ray.origin);
    Transform(matrix, endPoint);
    ray.direction = endPoint - ray.origin;    
    ray.direction.Normalise();
}

template <typename Real>
inline void 
NBRE_Transformation<Real>::Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Segment3<Real>& segment)
{
    Transform(matrix, segment.startPoint);
    Transform(matrix, segment.endPoint);
}

template <typename Real>
inline void 
NBRE_Transformation<Real>::Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Plane<Real>& plane)
{
    NBRE_Vector3<Real>& origin(plane.normal * -plane.d);
    Transform(matrix, plane.normal);
    Transform(matrix, origin);
    plane.normal.Normalise();
    plane.d = -plane.normal.DotProduct(origin);
}

template <typename Real>
inline void 
NBRE_Transformation<Real>::Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_AxisAlignedBox3<Real>& aabb)
{
    NBRE_Vector3<Real> corners[8];
    aabb.GetCorners(corners);
    for (uint32 i = 0; i < 8; ++i)
    {
        Transform(matrix, corners[i]);
    }
    aabb.Set(corners, 8);
}

template <typename Real>
inline void 
NBRE_Transformation<Real>::Transform(NBRE_Matrix4x4<Real> matrix, NBRE_NoneAxisAlignedBox3<Real>& box)
{
    Transform(matrix, box.center);
    matrix[0][3] = 0;
    matrix[1][3] = 0;
    matrix[2][3] = 0;
    Transform(matrix, box.up);
    Transform(matrix, box.forward);
    Transform(matrix, box.right);
}

template <typename Real>
inline void 
NBRE_Transformation<Real>::Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Frustum3<Real>& frustum)
{
    NBRE_Plane<Real> frustumPlanes[] = 
        { frustum.GetLeft(), frustum.GetRight()
        , frustum.GetBottom(), frustum.GetTop()
        , frustum.GetNear(), frustum.GetFar()};

    for (uint32 i = 0; i < 6; ++i)
    {
        Transform(matrix, frustumPlanes[i]);
    }
    frustum = NBRE_Frustum3<Real>(frustumPlanes[0], frustumPlanes[1], frustumPlanes[2], frustumPlanes[3], frustumPlanes[4], frustumPlanes[5]);
}

template <typename Real>
inline void 
NBRE_Transformation<Real>::Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Triangle3<Real>& triangle)
{
    Transform(matrix, triangle.vertices[0]);
    Transform(matrix, triangle.vertices[1]);
    Transform(matrix, triangle.vertices[2]);
}

template <typename Real>
inline NBRE_Vector2<Real> 
NBRE_Transformation<Real>::WorldToScreen(const NBRE_Matrix4x4<Real>& modelViewProjectViewportMatrix, const NBRE_Vector3<Real>& vec, Real* pW)
{
    NBRE_Vector4<Real> clipPoint(vec);
    Transform(modelViewProjectViewportMatrix, clipPoint);
    if (pW != NULL)
    {
        *pW = clipPoint.w;
    }
    Real invW = 1 / clipPoint.w;
    return NBRE_Vector2<Real>(clipPoint.x * invW, clipPoint.y * invW);
}

template <typename Real>
inline NBRE_Vector2<Real> 
NBRE_Transformation<Real>::WorldToScreen(const NBRE_Matrix4x4<Real>& modelViewProjectViewportMatrix, const NBRE_Vector3<Real>& vec)
{
    return WorldToScreen(modelViewProjectViewportMatrix, vec, NULL);
}

template <typename Real>
inline NBRE_Vector3<Real> 
NBRE_Transformation<Real>::ScreenToWorld(const NBRE_Matrix4x4<Real>& modelViewMatrix, const NBRE_Matrix4x4<Real>& projectMatrix, const NBRE_Matrix4x4<Real>& viewportMatrix, const NBRE_Vector2<Real>& position, Real depth)
{
    NBRE_Vector3<Real> worldPosition(position.x, position.y, depth);
    Transform((viewportMatrix * projectMatrix * modelViewMatrix).Inverse(), worldPosition);
    return worldPosition;
}

template <typename Real>
inline NBRE_Ray3<Real> 
NBRE_Transformation<Real>::GetPickingRay(const NBRE_Matrix4x4<Real>& modelViewMatrix, const NBRE_Matrix4x4<Real>& projectMatrix, const NBRE_Matrix4x4<Real>& viewportMatrix, const NBRE_Vector2<Real>& position)
{
    const NBRE_Vector3<Real>& endPosition = ScreenToWorld(modelViewMatrix, projectMatrix, viewportMatrix, position, 1);
    const NBRE_Vector3<Real>& startPosition = CoordinatesGetTranslate(modelViewMatrix.Inverse());
    return NBRE_Ray3<Real>(startPosition, endPosition - startPosition);
}

template <typename Real>
inline Real 
NBRE_Transformation<Real>::GetPixelsPerUnit(const NBRE_Matrix4x4<Real>& modelView, const NBRE_Matrix4x4<Real>& project, const NBRE_Matrix4x4<Real>& viewport, const NBRE_Vector3<Real>& position, Real unit)
{
    const NBRE_Matrix4x4<Real>& mvpv = viewport * project * modelView;
    NBRE_Vector3<Real> left(position);
    NBRE_Vector3<Real> right(position);

    right.x += modelView[0][0] * unit;
    right.y += modelView[0][1] * unit;
    right.z += modelView[0][2] * unit;    
    return WorldToScreen(mvpv, left).DistanceTo(WorldToScreen(mvpv, right));
}

template <typename Real>
inline NBRE_Matrix3x3<Real>
NBRE_Transformation<Real>::BuildTranslateMatrix2D(Real x, Real y)
{
    return NBRE_Matrix3x3<Real>(
        1, 0, x,
        0, 1, y,
        0, 0, 1);
}

template <typename Real>
inline NBRE_Matrix3x3<Real>
NBRE_Transformation<Real>::BuildTranslateMatrix2D(const NBRE_Vector2<Real>& translate)
{
    return BuildTranslateMatrix2D(translate.x, translate.y);
}

template <typename Real>
inline NBRE_Matrix3x3<Real>
NBRE_Transformation<Real>::BuildScaleMatrix2D(Real x, Real y)
{
    return NBRE_Matrix3x3<Real>(
        x, 0, 0,
        0, y, 0,
        0, 0, 1);
}

template <typename Real>
inline NBRE_Matrix3x3<Real>
NBRE_Transformation<Real>::BuildScaleMatrix2D(const NBRE_Vector2<Real>& scale)
{
    return BuildScaleMatrix2D(scale.x, scale.y);
}

template <typename Real>
inline NBRE_Matrix3x3<Real>
NBRE_Transformation<Real>::BuildRotateMatrix2D(Real radians)
{
    Real cs = static_cast<Real>(nsl_cos(radians));
    Real ss = static_cast<Real>(nsl_sin(radians));
    return NBRE_Matrix3x3<Real>(
         cs, ss, 0,
        -ss, cs, 0,
          0,  0, 1);
}

template <typename Real>
inline NBRE_Matrix3x3<Real>
NBRE_Transformation<Real>::BuildRotateDegreesMatrix2D(Real degrees)
{
    return BuildRotateMatrix2D(static_cast<Real>(NBRE_Math::Pi64 * degrees / 180.0));
}
