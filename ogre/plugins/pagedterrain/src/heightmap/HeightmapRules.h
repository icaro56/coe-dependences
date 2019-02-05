#pragma once

#include "PTPrerequisites.h"
#include <string>
#include "DllRequisites.h"

class PAGEDTERRAIN_EXPORT HeightmapRules
{
public:
    enum HeightmapFileType
    {
        HFT_RAW_1_BYTE = 0,
        HFT_RAW_2_BYTE,
        HFT_PNG/*,
        HFT_JPEG,
        HFT_DEM*/
    };

    class PAGEDTERRAIN_EXPORT Rule
    {
    public:
        std::string mHeightmapName;
        std::string mResourceGroupName;
        HeightmapFileType mFileType;
        size_t mTotalWidth;
        size_t mLevelsPerChunk;
    };
private:
};
