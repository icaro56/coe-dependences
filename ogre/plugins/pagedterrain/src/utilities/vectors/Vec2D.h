#pragma once

/*
A nearly direct copy of Ogre::Vector2, but
templated.
*/

#include "PTPrerequisites.h"
#include "DllRequisites.h"

template<class TType>
class PAGEDTERRAIN_EXPORT Vec2D
{
public:
    TType x,y;

    inline Vec2D() : x(0),y(0){};

    inline Vec2D(const TType ix, const TType iy) : x(ix), y(iy){};

    inline Vec2D(const Vec2D& vVec): x(vVec.x), y(vVec.y){};

    inline Vec2D& operator = ( const Vec2D& vVec){
        x = vVec.x;
        y = vVec.y;

        return *this;
    };

    inline bool operator == ( const Vec2D& vVec ) const
    {
        return ( x == vVec.x && y == vVec.y );
    }

    inline bool operator != ( const Vec2D& vVec ) const
    {
        return ( x != vVec.x || y != vVec.y );
    }

    inline Vec2D operator + ( const Vec2D& rkVector ) const
    {
        return Vec2D(
            x + rkVector.x,
            y + rkVector.y);
    }

    inline Vec2D operator - ( const Vec2D& rkVector ) const
    {
        return Vec2D(
            x - rkVector.x,
            y - rkVector.y);
    }

    inline Vec2D operator * ( const int iScalar ) const
    {
        return Vec2D(
            x * iScalar,
            y * iScalar);
    }

    inline Vec2D operator + ( const double dScalar ) const
    {
        return Vec2D(
            x + dScalar,
            y + dScalar);
    }

    inline Vec2D operator - ( const double dScalar ) const
    {
        return Vec2D(
            x - dScalar,
            y - dScalar);
    }

    inline Vec2D operator * ( const double dScalar ) const
    {
        return Vec2D(
            x * dScalar,
            y * dScalar);
    }

    inline Vec2D operator / ( const int iScalar ) const
    {

        return Vec2D(
            x / iScalar,
            y / iScalar);
    }

    inline Vec2D operator / ( const double dScalar ) const
    {
        assert( dScalar != 0.0 );

        double dInv = 1.0 / dScalar;

        return Vec2D(
            x * dInv,
            y * dInv);
    }

    inline Vec2D operator / ( const Vec2D& rhs) const
    {
        return Vec2D(
            x / rhs.x,
            y / rhs.y);
    }

    inline const Vec2D& operator + () const
    {
        return *this;
    }

    inline Vec2D operator - () const
    {
        return Vec2D(-x, -y);
    }

    // arithmetic updates
    inline Vec2D& operator += ( const Vec2D& rkVector )
    {
        x += rkVector.x;
        y += rkVector.y;

        return *this;
    }

    inline Vec2D& operator += ( const TType iScalar )
    {
        x += iScalar;
        y += iScalar;
        return *this;
    }

    inline Vec2D& operator -= ( const Vec2D& rkVector )
    {
        x -= rkVector.x;
        y -= rkVector.y;

        return *this;
    }

    inline Vec2D& operator -= ( const TType iScalar )
    {
        x -= iScalar;
        y -= iScalar;
        return *this;
    }

    inline Vec2D& operator *= ( const TType iScalar )
    {
        x *= iScalar;
        y *= iScalar;
        return *this;
    }

    inline Vec2D& operator *= ( const Vec2D& rkVector )
    {
        x *= rkVector.x;
        y *= rkVector.y;

        return *this;
    }

    inline Vec2D& operator /= ( const int iScalar )
    {
        x /= iScalar;
        y /= iScalar;

        return *this;
    }
    inline Vec2D& operator /= ( const double dScalar )
    {
        assert( dScalar != 0.0 );

        double dInv = 1.0 / dScalar;

        x *= dInv;
        y *= dInv;

        return *this;
    }

    inline Vec2D& operator /= ( const Vec2D& rkVector )
    {
        x /= rkVector.x;
        y /= rkVector.y;

        return *this;
    }
};

