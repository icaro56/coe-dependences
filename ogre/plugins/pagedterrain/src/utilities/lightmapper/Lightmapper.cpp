#include "Lightmapper.h"
#include "heightmap/Heightmap.h"

#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreImage.h>
#include <OgreColourValue.h>



using namespace Ogre;




	void Lightmapper::createLightmap(
		Heightmap* pHeightmap,
		Ogre::Image& lightmapImage,
		const Ogre::Vector2& vStartPos,
		const Ogre::Real rWidth,
		const Ogre::Vector3& vLightDir,
		const Ogre::ColourValue& cAmbient,
		const Ogre::ColourValue& cLightColor )
	{
		size_t iWidth = pHeightmap->getWidth();

		uchar* lightMap = new uchar[iWidth*iWidth * 3];
		memset(lightMap, 255, iWidth*iWidth*3);

		Vector2 pos(0,0);
		Real step = rWidth / iWidth;

		for (size_t z = 0; z < iWidth; ++z)
		{
			for (size_t x = 0; x < iWidth; ++x)
			{
				size_t index = (z * iWidth + x)*3;
				// calculate diffuse light from light source
				Vector3 norm = getNormalAt(pos.x, pos.y);
				float l = std::max(0.0, -vLightDir.dotProduct(norm));

				ColourValue v = cAmbient;
				v.r = std::min(1.0f, v.r+l*cLightColor.r);
				v.g = std::min(1.0f, v.g+l*cLightColor.g);
				v.b = std::min(1.0f, v.b+l*cLightColor.b);
				lightMap[index+0] = (uchar) (255*v.r);
				lightMap[index+1] = (uchar) (255*v.g);
				lightMap[index+2] = (uchar) (255*v.b);

				pos.x += step;
			}
			pos.x = 0.0f;
			pos.y += step;
		}

		// algorithm for ray traced shadow map as described here:
		// http://gpwiki.org/index.php/Faster_Ray_Traced_Terrain_Shadow_Maps
		size_t i, j;
		size_t *x, *z;
		int iDir, jDir;
		size_t iSize, jSize;
		//float height = 0.0f;
		float lDirXAbs = fabs(vLightDir.x);
		float lDirZAbs = fabs(vLightDir.z);

		// based on the direction of light, decide in which order to traverse
		// to speed up calculations
		if (lDirXAbs > lDirZAbs)
		{
			z = &i;
			x = &j;
			iSize = iWidth;
			jSize = iWidth;
			if (vLightDir.x < 0)
			{
				j = jSize - 1;
				jDir = -1;
			}
			else
			{
				j = 0;
				jDir = 1;
			}
			if (vLightDir.z < 0)
			{
				i = iSize - 1;
				iDir = -1;
			}
			else
			{
				i = 0;
				iDir = 1;
			}
		}
		else
		{
			x = &i;
			z = &j;
			jSize = iWidth;
			iSize = iWidth;
			if (vLightDir.x < 0)
			{
				i = iSize - 1;
				iDir = -1;
			}
			else
			{
				i = 0;
				iDir = 1;
			}
			if (vLightDir.z < 0)
			{
				j = jSize - 1;
				jDir = -1;
			}
			else
			{
				j = 0;
				jDir = 1;
			}
		}

		// calculate the step size to use
		/*AxisAlignedBox extents = info.getExtents();
		Vector3 pos = extents.getMinimum();
		Vector3 step = extents.getMaximum() - extents.getMinimum();
		step.x /= width;
		step.z /= height;*/

		step = rWidth / float(iWidth-1);

		float* flagMap = new float[iWidth*iWidth];
		memset(flagMap, 0, iWidth*iWidth*sizeof(float));

		while (1)
		{
			while (1)
			{
				// travel along terrain until we:
				// (1) intersect another point
				// (2) find another point with previous collision data
				// (3) reach the edge of the map
				float px = *x;
				float pz = *z;
				size_t index = (*z) * iWidth + (*x);

				// travel along ray
				while (1)
				{
					px -= vLightDir.x;
					pz -= vLightDir.z;

					// check if we've reached the boundary
					if (px < 0 || px >= iWidth || pz < 0 || pz >= iWidth)
					{
						flagMap[index] = -1.0f;
						break;
					}

					// calculate interpolated values
					int x0 = (int)floor(px);
					int x1 = (int)ceil(px);
					int z0 = (int)floor(pz);
					int z1 = (int)ceil(pz);

					float du = px - x0;
					float dv = pz - z0;
					float invdu = 1.0 - du;
					float invdv = 1.0 - dv;
					float w0 = invdu * invdv;
					float w1 = invdu * dv;
					float w2 = du * invdv;
					float w3 = du * dv;

					// get interpolated height at position
					//Vector2 curPos = Vector3(px*step.x, pz*step.y);
					float ipHeight = getHeight(px * step, pz * step);

					// compute interpolated flagmap value
					float pixels[4];
					pixels[0] = flagMap[z0*iWidth+x0];
					pixels[1] = flagMap[z1*iWidth+x0];
					pixels[2] = flagMap[z0*iWidth+x1];
					pixels[3] = flagMap[z1*iWidth+x1];
					float ipFlag = w0*pixels[0] + w1*pixels[1] + w2*pixels[2] + w3*pixels[3];

					// get distance from original point to current point
					float realXDist = (px - *x) * step;
					float realZDist = (pz - *z) * step;
					float distance = sqrt(realXDist*realXDist + realZDist*realZDist);

					// calculate ray height at current point
					float height = getHeight((*x)*step, (*z)*step) - vLightDir.y*distance;

					// check intersection with either terrain or flagMap
					// if ipHeight < ipFlag check against flagMap value
					float val = (ipHeight < ipFlag ? ipFlag : ipHeight);
					if (height < val)
					{
						// point in shadow
						flagMap[index] = val - height;
						lightMap[index*3+0] = (uchar) (255*cLightColor.r);
						lightMap[index*3+1] = (uchar) (255*cLightColor.g);
						lightMap[index*3+2] = (uchar) (255*cLightColor.b);
						break;
					}

					// check if pixel we moved to is unshadowed
					// since the flagMap value is interpolated, we use an epsilon value to check
					// if it's close enough to -1 to indicate non-shadow
					const float epsilon = 0.5f;
					if (ipFlag < -1.0f+epsilon && ipFlag > -1.0f-epsilon)
					{
						flagMap[index] = -1.0f;
						break;
					}
				}

				// update inner loop
				j += jDir;
				if (j >= jSize) // due to size_t, if j < 0, will wrap around and be > jSize ;)
					break;
			}

			// reset inner loop starting point
			if (jDir < 0)
				j = jSize-1;
			else
				j = 0;

			// update outer loop variable
			i += iDir;
			if (i >= iSize)
				break;

		}

		delete[] flagMap;

		for (size_t i = 0; i < iWidth; ++i)
		{
			for (size_t j = 0; j < iWidth; ++j)
			{
				int col[3] = {0, 0, 0};
				// sum up all colours from 5x5 grid around the current pixel
				int cnt = 0;
				for (int x = -1; x <= 1; ++x)
				{
					if ((int)i+x < 0 || i+x >= iWidth)
						continue;
					for (int y = -1; y <= 1; ++y)
					{
						if ((int)j+y < 0 || j+y >= iWidth)
							continue;
						size_t index = (i+x + (j+y)*iWidth)*3;
						col[0] += lightMap[index+0];
						col[1] += lightMap[index+1];
						col[2] += lightMap[index+2];
						++cnt;
					}
				}
				// building average
				col[0] /= cnt;
				col[1] /= cnt;
				col[2] /= cnt;
				// write back
				size_t index = (i + j*iWidth)*3;
				lightMap[index+0] = (uchar)col[0];
				lightMap[index+1] = (uchar)col[1];
				lightMap[index+2] = (uchar)col[2];
			}
		}

		lightmapImage.loadDynamicImage(lightMap, iWidth, iWidth, 1, PF_BYTE_RGB, true);
	}

	Ogre::Real Lightmapper::getHeight(Ogre::Real x, Ogre::Real z )
	{

		size_t xi = (size_t) x, zi = (size_t) z;
		size_t iWidth = mHeightmap->getWidth();

		if (x < 0.0 || x > iWidth-1 || z < 0.0 || z > iWidth-1)
		{
			// out of bounds
			return 0.0f;
		}

		// retrieve height from heightmap via bilinear interpolation

		float xpct = x - xi, zpct = z - zi;
		if (xi == iWidth-1)
		{
			// one too far
			--xi;
			xpct = 1.0f;
		}
		if (zi == iWidth-1)
		{
			--zi;
			zpct = 1.0f;
		}

		HEIGHTMAPTYPE* pMap = mHeightmap->getData();

		// retrieve heights
		float heights[4];
		heights[0] = UNPACK_HEIGHT(pMap[xi + zi * iWidth]);
		heights[1] = UNPACK_HEIGHT(pMap[xi + (zi+1) * iWidth]);
		heights[2] = UNPACK_HEIGHT(pMap[(xi+1) + zi * iWidth]);
		heights[3] = UNPACK_HEIGHT(pMap[(xi+1) + (zi+1) * iWidth]);

		// interpolate
		float w[4];
		w[0] = (1.0 - xpct) * (1.0 - zpct);
		w[1] = (1.0 - xpct) * zpct;
		w[2] = xpct * (1.0 - zpct);
		w[3] = xpct * zpct;
		float ipHeight = w[0]*heights[0] + w[1]*heights[1] + w[2]*heights[2] + w[3]*heights[3];

		// scale to actual height
		//ipHeight *= mScale.y;

		return ipHeight * (1.0f / MAX_INT) * 100.0f;
	}

	Ogre::Vector3 Lightmapper::getNormalAt(Ogre::Real x, Ogre::Real z)
	{
		int flip = 1;
		Vector3 here (x, getHeight(x, z), z);
		Vector3 left (x-1, getHeight(x-1, z), z);
		Vector3 down (x, getHeight(x, z+1), z+1);
		if (left.x < 0.0)
		{
			flip *= -1;
			left = Vector3(x+1, getHeight(x+1, z), z);
		}
		if (down.z >= 2000.0f*(mHeightmap->getWidth()-1))
		{
			flip *= -1;
			down = Vector3(x, getHeight(x, z-1), z-1);
		}
		left -= here;
		down -= here;

		Vector3 norm = flip * left.crossProduct(down);
		norm.normalise();

		return norm;
	}
