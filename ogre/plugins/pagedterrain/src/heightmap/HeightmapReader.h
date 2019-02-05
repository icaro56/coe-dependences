#pragma once

#include "PTPrerequisites.h"
#include <OgrePrerequisites.h>
#include <OgreSingleton.h>
#include "DllRequisites.h"

class PAGEDTERRAIN_EXPORT HeightmapReader : public Ogre::Singleton<HeightmapReader>
{
public:
    HeightmapReader(){};

    virtual ~HeightmapReader(){};

    static HeightmapReader& getSingleton();

    static HeightmapReader* getSingletonPtr();

    void readHeightmap(
        const std::string& strFileName,
        Heightmap* pHeightmap,
        size_t iWidth = 0,
        size_t iOffsetX = 0,
        size_t iOffsetY = 0,
        size_t iSpacing = 1,
        bool bHasBorder = false);


private:
};
