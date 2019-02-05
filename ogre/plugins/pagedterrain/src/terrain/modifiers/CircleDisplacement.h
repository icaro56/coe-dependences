#pragma once

#include "PTPrerequisites.h"
#include "TerrainModifier.h"
#include <OgrePrerequisites.h>
#include <OgreVector3.h>
#include "utilities/vectors/Vec2D.h"
#include "DllRequisites.h"


class PAGEDTERRAIN_EXPORT CircleDisplacement : public TerrainModifier
{
public:
    CircleDisplacement(const Ogre::Vector3& center, Ogre::Real radius, Ogre::Real displacement, bool addative = true);

    void displace(Vec2D<double> vTopLeft, double fWidth, Heightmap* pHeightmap, Ogre::Real fScale  ) const;

    void displaceTexture(Vec2D<double> vTopLeft, double fWidth, Ogre::TexturePtr pTex, int iChannel = 0) const {};

    //void displace(Ogre::Vector3* pVerts);

    bool isInBounds(const Vec2D<double>& vMin, const Vec2D<double>& vMax) const;

    const Ogre::String getType() const {return "CircleDisplacement";}

private:
    Ogre::Real mDisplacement;
    Ogre::Real mRadius;
    Ogre::Vector3 mCenter;
    bool mAddative;
};

