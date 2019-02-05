#pragma once

#include "PTPrerequisites.h"
#include "DllRequisites.h"

class PAGEDTERRAIN_EXPORT RAWHeightmapReader
{
public:
    void readHeightmap(
        const std::string& strFileName,
        Heightmap* pHeightmap,
        size_t iWidth = 0,
        size_t iOffsetX = 0,
        size_t iOffsetY = 0,
        size_t iSpacing = 1);

private:
};
