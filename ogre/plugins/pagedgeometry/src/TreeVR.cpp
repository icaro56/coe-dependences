#include "PagedGeometry.h"
#include "BatchPage.h"
#include "WindBatchPage.h"
#include "ImpostorPage.h"
#include "TreeLoader2D.h"
#include "TreeVR.h"

//Include "HeightFunction.h", a header that provides some useful functions for quickly and easily
//getting the height of the terrain at a given point.

#include "HeightFunction.h"
//[NOTE] Remember that this "HeightFunction.h" file is not related to the PagedGeometry library itself
//in any way. It's simply a utility that's included with all these examples to make getting the terrain
//height easy. You can use it in your games/applications if you want, although if you're using a
//collision/physics library with a faster alternate, you may use that instead.


#include <Terrain.h>

TreeVR::TreeVR(Ogre::SceneManager* scene, std::string& camName, Terrain*t, double PageSize, double lt, double tp, double rt, double bt, double maxRange, double distance, double ImaxRange, double Idistance, bool isInfinity, bool wind)
:mSceneMgr(scene), mTerrain(t)
{

    mTrees = new Forests::PagedGeometry();

    mTrees->setCamera(mSceneMgr->getCamera(camName));	//Set the camera so PagedGeometry knows how to calculate LODsmT

    mTrees->setPageSize(PageSize);	//Set the size of each page of geometry

    if(isInfinity)
        mTrees->setInfinite();		//Use infinite paging mode

    if(wind)
        mTrees->addDetailLevel<Forests::WindBatchPage>(maxRange, distance);
    else
        mTrees->addDetailLevel<Forests::BatchPage>(maxRange, distance);
    mTrees->addDetailLevel<Forests::ImpostorPage>(ImaxRange, Idistance);


    mTreeLoader = new Forests::TreeLoader2D(mTrees, Forests::TBounds(lt, tp, rt, bt));
    mTrees->setPageLoader(mTreeLoader);	//Assign the "treeLoader" to be used to load geometry for the PagedGeometry instance
    //Supply a height function to TreeLoader2D so it can calculate tree Y values
    HeightFunction::initialize(mSceneMgr, t);

    mTreeLoader->setHeightFunction(&HeightFunction::getTerrainHeight);


}

TreeVR::~TreeVR()
{
    if(mTrees)
    {
        delete mTrees;
        mTrees = 0;
    }
    if(mTreeLoader)
    {
        delete mTreeLoader;
        mTreeLoader = 0;
    }
    if(mTerrain)
    {
        delete mTerrain;
        mTerrain = 0;
    }
    if(TreeEntity)
    {
        delete TreeEntity;
        TreeEntity = 0;
    }
}

void TreeVR::createTree(std::string& name, std::string& mesh)
{
       TreeEntity = mSceneMgr->createEntity(name, mesh);
       TreeEntity->setCastShadows(true);
}

void TreeVR::addTree(Ogre::Vector3 &pos, double yaw, double scale)
{
       mTreeLoader->addTree(TreeEntity, pos, (Ogre::Degree)yaw, scale);
}

void TreeVR::setMaterialName(std::string& name)
{
       TreeEntity->setMaterialName(name);
}

void TreeVR::setWind(double FactorX, double FactorY)
{
    mTrees->setCustomParam(TreeEntity->getName(), "windFactorX", FactorX);
    mTrees->setCustomParam(TreeEntity->getName(), "windFactorY", FactorY);
}

void TreeVR::setColorMap(std::string& map)
{
    mTreeLoader->setColorMap(map);
}

void TreeVR::update()
{
       mTrees->update();
}
