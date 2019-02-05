#include "PNGHeightmapReader.h"

#include "Heightmap.h"

#include <OgreImage.h>
#include <OgreResourceGroupManager.h>
#include <OgreStringConverter.h>
#include <OgreLogManager.h>


using namespace Ogre;



	void PNGHeightmapReader::readHeightmap(
		const std::string& strFileName,
		Heightmap* pHeightmap,
		size_t iWidth /*= 0*/,
		size_t iOffsetX /*= 0*/,
		size_t iOffsetY /*= 0*/,
		size_t iSpacing /*= 1*/,
		bool bHasBorder /*= false*/ )
	{
		//assert("Should have a width" && iWidth);

		assert("Heightmap == 0" && pHeightmap);



		Image img;
		img.load(strFileName,"General");

		size_t iMapWidth = img.getWidth();

		if (iWidth == 0)
			iWidth = iMapWidth;

		pHeightmap->setWidth(iWidth);
		pHeightmap->allocateData();

		HEIGHTMAPTYPE* pData = pHeightmap->getData();

		size_t pos = 0;

		if (img.getBPP() == 8)
		{
			Ogre::uchar* pImgData = img.getData();


			for (size_t y=iOffsetY;y<(iOffsetY + iSpacing * iWidth);y += iSpacing)
			{
				for(size_t x=iOffsetX;x<(iOffsetX + iSpacing * iWidth);x += iSpacing)
				{
					pos = x + y * iMapWidth;
					*pData++ = PACK_HEIGHT(pImgData[pos]) * 257;
				}
			}
		}
		else if (img.getBPP() == 16)
		{
			Ogre::ushort* pImgData = reinterpret_cast<Ogre::ushort*>(img.getData());

			for (size_t y=iOffsetY;y<(iOffsetY + iSpacing * iWidth);y += iSpacing)
			{
				for(size_t x=iOffsetX;x<(iOffsetX + iSpacing * iWidth);x += iSpacing)
				{
					pos = x + y * iMapWidth;
					*pData++ = PACK_HEIGHT(pImgData[pos]);
				}
			}
		}


	}
