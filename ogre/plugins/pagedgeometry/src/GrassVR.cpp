#include "PagedGeometry.h"
#include "GrassLoader.h"
#include "GrassVR.h"
//Include "HeightFunction.h", a header that provides some useful functions for quickly and easily
//getting the height of the terrain at a given point.
#include "HeightFunction.h"
//[NOTE] Remember that this "HeightFunction.h" file is not related to the PagedGeometry library itself
//in any way. It's simply a utility that's included with all these examples to make getting the terrain
//height easy. You can use it in your games/applications if you want, although if you're using a
//collision/physics library with a faster alternate, you may use that instead.

#include <Terrain.h>


//PagedGeometry's classes and functions are under the "Forests" namespace
using namespace Forests;

GrassVR::GrassVR(Ogre::SceneManager* scene, std::string& camName, double pageSize, double detailSize, std::string& material, Terrain* t)
: mSceneMgr(scene), mTerrain(t)
{
    mGrass = new Forests::PagedGeometry(mSceneMgr->getCamera(camName), pageSize);
	mGrass->addDetailLevel<GrassPage>(detailSize);

	//Create a GrassLoader object
	mGrassLoader = new Forests::GrassLoader(mGrass);
	mGrass->setPageLoader(mGrassLoader);	//Assign the "treeLoader" to be used to load geometry for the PagedGeometry instance

	//Supply a height function to GrassLoader so it can calculate grass Y values
	HeightFunction::initialize(mSceneMgr, mTerrain);
	mGrassLoader->setHeightFunction( & HeightFunction::getTerrainHeight);

	//Add some grass to the scene with GrassLoader::addLayer()
	mLayer = mGrassLoader->addLayer(material);

}

GrassVR::~GrassVR()
{
    if(mTerrain)
    {
        delete mTerrain;
        mTerrain = 0;

    }
    if(mGrass)
    {
        delete mGrass;
        mGrass = 0;

    }
	if(mGrassLoader)
	{
	    delete mGrassLoader;
	    mGrassLoader = 0;
    }
}

void GrassVR::setMinimumSize(double width, double height)
{
        mLayer->setMinimumSize(width, height);
}

void GrassVR::setMaximumSize(double width, double height)
{
        mLayer->setMaximumSize(width, height);
}

void GrassVR::setAnimationEnabled(bool animated)
{
        mLayer->setAnimationEnabled(animated);
}

void GrassVR::setSwayDistribution(double freq)
{
        mLayer->setSwayDistribution(freq);
}

void GrassVR::setSwayLength(double leng)
{
        mLayer->setSwayLength(leng);
}
void GrassVR::setSwaySpeed(double sec)
{
        mLayer->setSwaySpeed(sec);
}

void GrassVR::setDensity(double den)
{
        mLayer->setDensity(den);
}

void GrassVR::setFadeTechnique(FadeTechnique ft)
{
        mLayer->setFadeTechnique((Forests::FadeTechnique)ft);
}

void GrassVR::setRenderTechnique(GrassTechnique qt, bool blendBase)
{
        mLayer->setRenderTechnique((Forests::GrassTechnique)qt, blendBase);
}

void GrassVR::setColorMap(std::string& texture)
{
        mLayer->setColorMap(texture);
}

void GrassVR::setHeightRange(double minimun, double maximum)
{
        mLayer->setHeightRange(minimun, maximum);
}
void GrassVR::setDensityMap(std::string& densityMap)
{
        mLayer->setDensityMap(densityMap);
}

void GrassVR::setMapBounds(double left, double top, double right, double bottom)
{
        mLayer->setMapBounds(Forests::TBounds(left, top, right, bottom));
}

void GrassVR::setMaterialName(std::string& material)
{
        mLayer->setMaterialName(material);
}

void GrassVR::setMaxSlope(double inc)
{
        mLayer->setMaxSlope(inc);
}

void GrassVR::update()
{
        mGrass->update();
}

