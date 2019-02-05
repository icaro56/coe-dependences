#pragma once

#include "PTPrerequisites.h"
#include "TerrainModifier.h"
#include <OgrePrerequisites.h>
#include <OgreVector3.h>
#include <OgreVector2.h>
#include "utilities/vectors/Vec2D.h"
#include "DllRequisites.h"


class PAGEDTERRAIN_EXPORT RectFlatten : public TerrainModifier
{
public:
    RectFlatten(const Ogre::Vector3& center, Ogre::Real width, Ogre::Real height);

    void displace(Vec2D<double> vTopLeft, double fWidth, Heightmap* pHeightmap, float fScale  ) const;

    void displaceTexture(Vec2D<double> vTopLeft, double fWidth, Ogre::TexturePtr pTex, int iChannel = 0) const {};

    bool isInBounds(const Vec2D<double>& vMin, const Vec2D<double>& vMax) const;

    const Ogre::String getType() const {return "RectFlatten";}

private:
    Ogre::Vector3 mCenter;
    Ogre::Real mWidth;
    Ogre::Real mHeight;
    Ogre::Vector2 mMin;
    Ogre::Vector2 mMax;
};

