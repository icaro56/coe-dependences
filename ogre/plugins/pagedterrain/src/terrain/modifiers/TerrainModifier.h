#pragma once

#include "PTPrerequisites.h"
#include <OgrePrerequisites.h>
#include <OgreTexture.h>
#include "DllRequisites.h"
#include "utilities\vectors\Vec2D.h"

class Heightmap;


class PAGEDTERRAIN_EXPORT TerrainModifier
{
public:
    virtual ~TerrainModifier(){}

    //virtual void displace(Ogre::Vector3* pVerts) = 0;
    virtual void displace(Vec2D<double> vTopLeft, double fWidth, Heightmap* pHeightmap, float fScale ) const = 0;

    virtual void displaceTexture(Vec2D<double> vTopLeft, double fWidth, Ogre::TexturePtr pTex, int iChannel = 0) const = 0;

    virtual bool isInBounds(const Vec2D<double> & vMin, const Vec2D<double> & vMax) const = 0;

    virtual const Ogre::String getType() const = 0;
};

