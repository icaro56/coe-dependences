#pragma once

#include "PTPrerequisites.h"
#include <OgrePrerequisites.h>
#include "DllRequisites.h"


class PAGEDTERRAIN_EXPORT Heightmap
{
public:
    Heightmap();
    Heightmap(const std::string& strFileName, size_t iWidth = 0, bool bHasBorder = true);
    //Heightmap(float* fData, size_t iWidth);
    ~Heightmap();

    void allocateData();
    HEIGHTMAPTYPE* getData(){return mData;}

    size_t getWidth(){return mWidth;}
    void setWidth(size_t iWidth);

    size_t getMaxLevels(){return mMaxLevels;}
    void setMaxLevels(size_t iMaxLevels){mMaxLevels = iMaxLevels;}

    void setStartDepth(size_t iStartDepth){mStartDepth = iStartDepth;}
    size_t getStartDepth(){return mStartDepth;}

    size_t calculateMaxLevels();

    HEIGHTMAPTYPE getHeightAt(int x, int y);

    float getHeightAt(float fPosX, float fPosZ, size_t iDepth, size_t iOffsetX, size_t iOffsetY);

    void setHeightAt(int x, int y, HEIGHTMAPTYPE val);

    void setHeightAt(int pos, HEIGHTMAPTYPE val);

    float getHeightAndNormalAt(float fPosX, float fPosZ, const Ogre::Vector3& vScale, size_t iDepth, size_t iOffsetX, size_t iOffsetY, Ogre::Vector3& vNormal);

    void getSubdivision(Heightmap* pSubMap, size_t iWidth, size_t iOffsetX, size_t iOffsetY, size_t iSpacing);

    void saveHeightmap(const Ogre::String& filePath);

private:
    size_t mAllocatedWidth;
    size_t mWidth;
    HEIGHTMAPTYPE* mData;

    size_t mStartDepth;
    size_t mMaxLevels;

    bool mHasBorder;
};

