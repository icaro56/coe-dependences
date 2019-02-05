#pragma once

#include "PTPrerequisites.h"
#include <OgrePrerequisites.h>
#include <OgreVector3.h>
#include "DllRequisites.h"

class PAGEDTERRAIN_EXPORT Lightmapper
{
public:
    void createLightmap(
        Heightmap* pHeightmap,
        Ogre::Image& lightmapImage,
        const Ogre::Vector2& vStartPos,
        const Ogre::Real rWidth,
        const Ogre::Vector3& vLightDir,
        const Ogre::ColourValue& cAmbient,
        const Ogre::ColourValue& cLightColor );

    Ogre::Real getHeight(Ogre::Real x, Ogre::Real z);
    Ogre::Vector3 getNormalAt(Ogre::Real x, Ogre::Real z);
private:
    Heightmap* mHeightmap;
    Ogre::Vector3 mScale;
};
