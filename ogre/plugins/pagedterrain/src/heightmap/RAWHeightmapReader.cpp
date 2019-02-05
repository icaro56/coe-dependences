#include "RAWHeightmapReader.h"
#include "Heightmap.h"


#include <OgreDataStream.h>
#include <OgreResourceGroupManager.h>
#include <OgreStringConverter.h>
#include <OgreLogManager.h>


using namespace Ogre;




	void RAWHeightmapReader::readHeightmap(
		const std::string& strFileName,
		Heightmap* pHeightmap,
		size_t iWidth /*= 0*/,
		size_t iOffsetX /*= 0*/,
		size_t iOffsetY /*= 0*/,
		size_t iSpacing /*= 1*/ )
	{
		assert("Should have a width" && iWidth);

		assert("Heightmap == 0" && pHeightmap);

		// TODO
		// Add not-found protection

		DataStreamPtr pFile = ResourceGroupManager::getSingleton().openResource(strFileName);

		//const size_t fileSize = pFile->size();

		const size_t byteSize = 2;//fileSize / (iWidth * iWidth);

		const size_t fileWidth = 1025;

		pHeightmap->setWidth(iWidth);
		pHeightmap->allocateData();

		//assert("Only 8 and 16-bit RAW supported for now" && byteSize < 3);

		HEIGHTMAPTYPE* pData = pHeightmap->getData();


		size_t pos = 0;
		Ogre::ushort height = 0;
		for (size_t y=iOffsetY;y<(iOffsetY + iSpacing * iWidth);y += iSpacing)
		{
			for(size_t x=iOffsetX;x<(iOffsetX + iSpacing * iWidth);x += iSpacing)
			{
				pos = x + y * fileWidth;
				pos *= byteSize;

				pFile->seek(pos);

				pFile->read(&height,byteSize);

				*pData++ = PACK_HEIGHT(height);

			}
		}



		pFile->close();

	}
