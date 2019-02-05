#include "HeightmapReader.h"
#include "Heightmap.h"

#include <OgreString.h>

#include "RAWHeightmapReader.h"
#include "PNGHeightmapReader.h"


using namespace Ogre;

template<> HeightmapReader* Ogre::Singleton<HeightmapReader>::msSingleton = 0;

	HeightmapReader* HeightmapReader::getSingletonPtr(void)
	{
		return msSingleton;
	}
	HeightmapReader& HeightmapReader::getSingleton(void)
	{
		assert( msSingleton );  return ( *msSingleton );
	}

	void HeightmapReader::readHeightmap(
		const std::string& strFileName,
		Heightmap* pHeightmap,
		size_t iWidth /*= 0*/,
		size_t iOffsetX /*= 0*/,
		size_t iOffsetY /*= 0*/,
		size_t iSpacing /*= 1*/,
		bool bHasBorder /*= false*/ )
	{
		if (StringUtil::endsWith(strFileName,".raw"))
		{
			RAWHeightmapReader rawReader;
			rawReader.readHeightmap(strFileName,pHeightmap,iWidth,iOffsetX,iOffsetY,iSpacing);
		}
		else if (StringUtil::endsWith(strFileName,".png"))
		{
			PNGHeightmapReader pngReader;
			pngReader.readHeightmap(strFileName,pHeightmap,iWidth,iOffsetX,iOffsetY,iSpacing,bHasBorder);
		}
	}


