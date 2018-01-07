#include "palmath.h"
#include "nbrecommon.h"

template<class T>
inline nb_boolean 
NBRE_Math::IsNaN(T v)
{
    // std::isnan() is C99, not supported by all compilers
    return v != v;
}

template<class T>
inline T 
NBRE_Math::NaN()
{
    static T zero = static_cast<T>(0);
    return zero / zero;
}

inline nb_boolean NBRE_Math::IsZero(float v, float epsilon)
{
    if((v > -epsilon) && (v < epsilon))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

inline nb_boolean NBRE_Math::IsPowerOfTwo(uint32 number)
{
    // 0 is not a power of 2
    return (number == 0) ? FALSE : (((number & (number-1)) == 0) ? TRUE : FALSE);
}

inline int32 NBRE_Math::PowerOfTwo(const int32 number)
{
    int32 pow = 0;
    while(number>>pow)
    {
        ++pow;
    }
    if((1<<pow) < number)
    {
        ++pow;
    }
    return pow;
}

inline nb_boolean NBRE_Math::AreEqual(float f1, float f2, float epsilon)
{
    return (f2 >= f1-epsilon && f2 <= f1+epsilon);
}

inline nb_boolean NBRE_Math::AreEqual(double d1, double d2, double epsilon)
{
    return (d2 >= d1-epsilon && d2 <= d1+epsilon);
}

inline float NBRE_Math::RadToDeg(float radians)
{
    return radians*Rad2Deg;
}

inline double NBRE_Math::RadToDeg(double radians)
{
    return radians*Rad2Deg64;
}

inline float NBRE_Math::DegToRad(float degree)
{
    return degree*Deg2Rad;
}

inline double NBRE_Math::DegToRad(double degree)
{
    return degree*Deg2Rad64;
}

template<class T>
inline T NBRE_Math::Min(const T& a, const T& b)
{
    return a < b ? a : b;
}

template<class T>
inline T NBRE_Math::Max(const T& a, const T& b)
{
    return a > b ? a : b;
}

template<class T>
inline T NBRE_Math::Abs(const T& a)
{
    return a < static_cast<T>(0)? -a : a;
}

template<class T>
inline T NBRE_Math::Sign(const T& a)
{
    if (a < 0)
    {
        return -1;
    }
    else if (a > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

template <class T>
inline T NBRE_Math::Clamp(const T& value, const T& low, const T& high)
{
    nbre_assert(high >= low);
    return Min(Max(value, low), high);
}

template <class T1, class T2>
inline void NBRE_Math::Swap(T1& a, T2& b)
{
    T1 c(a);
    a = b;
    b = c;
}
