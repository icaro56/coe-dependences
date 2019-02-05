#pragma  once

/*
A copy of Ogre::Vector3, but templated
to avoid needing Double Precision mode
from Ogre.
*/

#include "PTPrerequisites.h"
#include <OgrePrerequisites.h>
#include <OgreVector3.h>
#include "DllRequisites.h"


template<class TType>
class PAGEDTERRAIN_EXPORT Vec3D
{
public:
    TType x,y,z;

    inline Vec3D() : x(0),y(0),z(0){};

    inline Vec3D(const TType ix, const TType iy, const TType iz) : x(ix), y(iy), z(iz){};

    inline Vec3D(const Vec3D& vVec): x(vVec.x), y(vVec.y), z(vVec.z){};

    inline Vec3D(const Ogre::Vector3& vVec): x(vVec.x), y(vVec.y), z(vVec.z){};

    inline TType operator [] ( const int i ) const
    {
        assert( i < 3 );

        return *(&x+i);
    }

    inline TType& operator [] ( const int i )
    {
        assert( i < 3 );

        return *(&x+i);
    }

    inline Vec3D& operator = ( const Vec3D& vVec){
        x = vVec.x;
        y = vVec.y;
        z = vVec.z;

        return *this;
    };

    inline Vec3D& operator = ( const Ogre::Vector3& vVec){
        x = vVec.x;
        y = vVec.y;
        z = vVec.z;

        return *this;
    };

    inline bool operator == ( const Vec3D& vVec ) const
    {
        return ( x == vVec.x && y == vVec.y && z == vVec.z);
    }

    inline bool operator != ( const Vec3D& vVec ) const
    {
        return ( x != vVec.x || y != vVec.y || z != vVec.z);
    }

    inline Vec3D operator + ( const Vec3D& rkVector ) const
    {
        return Vec3D(
            x + rkVector.x,
            y + rkVector.y,
            z + rkVector.z);
    }

    inline Vec3D operator - ( const Vec3D& rkVector ) const
    {
        return Vec3D(
            x - rkVector.x,
            y - rkVector.y,
            z - rkVector.z);
    }

    inline Vec3D operator * ( const int iScalar ) const
    {
        return Vec3D(
            x * iScalar,
            y * iScalar,
            z * iScalar);
    }

    inline Vec3D operator * ( const double dScalar ) const
    {
        return Vec3D(
            x * dScalar,
            y * dScalar,
            z * dScalar);
    }

    inline Vec3D operator / ( const int iScalar ) const
    {

        return Vec3D(
            x / iScalar,
            y / iScalar,
            z / iScalar);
    }

    inline Vec3D operator / ( const double dScalar ) const
    {
        assert( dScalar != 0.0 );

        double dInv = 1.0 / dScalar;

        return Vec3D(
            x * dInv,
            y * dInv,
            z * dInv);
    }

    inline Vec3D operator / ( const Vec3D& rhs) const
    {
        return Vec3D(
            x / rhs.x,
            y / rhs.y,
            z / rhs.z);
    }

    inline const Vec3D& operator + () const
    {
        return *this;
    }

    inline Vec3D operator - () const
    {
        return Vec3D(-x, -y, -z);
    }

    // arithmetic updates
    inline Vec3D& operator += ( const Vec3D& rkVector )
    {
        x += rkVector.x;
        y += rkVector.y;
        z += rkVector.z;

        return *this;
    }

    inline Vec3D& operator += ( const TType iScalar )
    {
        x += iScalar;
        y += iScalar;
        z += iScalar;
        return *this;
    }

    inline Vec3D& operator -= ( const Vec3D& rkVector )
    {
        x -= rkVector.x;
        y -= rkVector.y;
        z -= rkVector.z;

        return *this;
    }

    inline Vec3D& operator -= ( const TType iScalar )
    {
        x -= iScalar;
        y -= iScalar;
        z -= iScalar;
        return *this;
    }

    inline Vec3D& operator *= ( const TType iScalar )
    {
        x *= iScalar;
        y *= iScalar;
        z *= iScalar;
        return *this;
    }

    inline Vec3D& operator *= ( const Vec3D& rkVector )
    {
        x *= rkVector.x;
        y *= rkVector.y;
        z *= rkVector.z;

        return *this;
    }

    inline Vec3D& operator /= ( const int iScalar )
    {
        x /= iScalar;
        y /= iScalar;
        z /= iScalar;

        return *this;
    }
    inline Vec3D& operator /= ( const double dScalar )
    {
        assert( dScalar != 0.0 );

        double dInv = 1.0 / dScalar;

        x *= dInv;
        y *= dInv;
        z *= dInv;

        return *this;
    }

    inline Vec3D& operator /= ( const Vec3D& rkVector )
    {
        x /= rkVector.x;
        y /= rkVector.y;
        z /= rkVector.z;

        return *this;
    }

    inline double length()
    {
        return sqrt(x*x + y*y + z*z);
    }

    inline void normalize()
    {
        double dInv = 1.0 / length();

        x *= dInv;
        y *= dInv;
        z *= dInv;
    }

    inline const Ogre::Vector3 getVector3(){
        return Ogre::Vector3(
            x,
            y,
            z);
    }

};

