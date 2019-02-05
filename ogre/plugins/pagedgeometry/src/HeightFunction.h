#ifndef HEIGHTFUNCTION_H
#define HEIGHTFUNCTION_H


//This provides functions that can be used to easily get the height of Ogre's terrain at any x/z point.
//Simply call HeightFunction::initialize(), then use HeightFunction::getTerrainHeight() as needed.

//This file is used by the PagedGeometry examples to place trees on the terrain.

#include "Ogre.h"
#include <Terrain.h>
#include "PGPrerequisites.h"


namespace HeightFunction
{
	class PAGEDGEOMETRY_EXPORT MyRaySceneQueryListener: public Ogre::RaySceneQueryListener
	{
	public:
		inline bool queryResult(Ogre::SceneQuery::WorldFragment *fragment, Ogre::Real distance)
		{
			resultDistance = distance;
			return false;
		}
		inline bool queryResult(Ogre::MovableObject *obj, Ogre::Real distance)
		{
			resultDistance = distance;
			return false;
		}

		float resultDistance;
	};

	extern bool initialized;
	extern Ogre::RaySceneQuery* raySceneQuery;
	extern Ogre::Ray updateRay;
	extern MyRaySceneQueryListener *raySceneQueryListener;
    extern Terrain* mt;
	//Initializes the height function. Call this before calling getTerrainHeight()
	PAGEDGEOMETRY_EXPORT inline void initialize(Ogre::SceneManager *sceneMgr,  Terrain* t){
		mt = t;
		if (!initialized){
			initialized = true;
			updateRay.setOrigin(Ogre::Vector3::ZERO);
			updateRay.setDirection(Ogre::Vector3::NEGATIVE_UNIT_Y);
			raySceneQuery = sceneMgr->createRayQuery(updateRay);
			//raySceneQuery->setQueryTypeMask(Ogre::SceneManager::WORLD_GEOMETRY_TYPE_MASK);
			//raySceneQuery->setWorldFragmentType(Ogre::SceneQuery::WFT_SINGLE_INTERSECTION);
			raySceneQueryListener = new MyRaySceneQueryListener;
		}
	}

	//Gets the height of the terrain at the specified x/z coordinate
	//The userData parameter isn't used in this implementation of a height function, since
	//there's no need for extra data other than the x/z coordinates.
	PAGEDGEOMETRY_EXPORT inline Ogre::Real getTerrainHeight(const Ogre::Real x, const Ogre::Real z, void *userData = NULL)
	{
		//updateRay.setOrigin(Ogre::Vector3(x, 0.0f, z));
		//updateRay.setDirection(Ogre::Vector3::UNIT_Y);
		//raySceneQuery->setRay(updateRay);
		//raySceneQuery->execute(raySceneQueryListener);

		//return raySceneQueryListener->resultDistance;
		return mt->getHeightAt(x,z,0,true,false);
	}
}

#endif
