#include "Terrain.h"
#include "quadtree/QNode.h"
#include "terrain/mesh/TerrainMesh.h"
#include "heightmap/Heightmap.h"
#include "heightmap/HeightmapReader.h"
#include "utilities/lightmapper/GPULightmapper.h"
#include "utilities/brush/Brush.h"
#include "terrain/modifiers/BrushDisplacement.h"
#include "terrain/modifiers/RectFlatten.h"
#include "utilities/objecthandler/ObjectHandler.h"
#include <OgreMaterialManager.h>
#include <OgreEntity.h>
#include <OgreSubEntity.h>
#include <OgreMaterial.h>
#include <OgreViewport.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>

using namespace Ogre;
#if (USE_SINGLETON == 1)
template<> Terrain* Ogre::Singleton<Terrain>::msSingleton = 0;
#endif

#if (USE_SINGLETON == 1)
	Terrain* Terrain::getSingletonPtr(void)
	{
		return msSingleton;
	}
	Terrain& Terrain::getSingleton(void)
	{
		assert( msSingleton );  return ( *msSingleton );
	}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Terrain::Terrain()
: mCamera(0), mTerrainNode(0), mSceneMgr(0), mTerrainHeight(0.0), mTotalNodes(0), mWidth(0.0), mHeightmap(0), mRootNode(0),
mSingleton(0), mLightmapper(0), mMaxDepth(0), mClampState(ACS_OFF), mClampBonusHeight(0.0f), mClampUseRoot(false),
mClampUseGeoMorph(false), mSkirtLength(0.0f), mMorphSpeed(1.0f), mLODCheckTime(1.0f), mPreLODCheckTime(1.0f), mPreMorphSpeed(1.0f),
mInitialized(false), mAutoUpdateLightmap(true), mDiscardGeometryData(true),	mUseChunkUVs(false), mEventListener(&mDefaultListener),
mDecalPass(0), mBrushTexState(0), mShowBrush(false), mLODDistBias(1.0f), mDeformation(true),
mEditChannel(0), mAlpha(1)
{
    mSingleton = new HeightmapReader();

    mObjectHandler = new ObjectHandler(0,this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Terrain::Terrain(const std::string& camName, Ogre::SceneManager* sceneManager)
: mCamera(0), mTerrainNode(0), mSceneMgr(0), mTerrainHeight(0.0), mTotalNodes(0), mWidth(0.0), mHeightmap(0), mRootNode(0), mSingleton(0),
mLightmapper(0), mMaxDepth(0), mClampState(ACS_OFF), mClampBonusHeight(0.0f), mClampUseRoot(false), mClampUseGeoMorph(false),
mSkirtLength(0.0f), mMorphSpeed(1.0f), mLODCheckTime(1.0f), mPreLODCheckTime(1.0f), mPreMorphSpeed(1.0f), mInitialized(false),
mAutoUpdateLightmap(true), mDiscardGeometryData(true), mUseChunkUVs(false), mEventListener(&mDefaultListener), mDecalPass(0),
mBrushTexState(0), mShowBrush(false), mLODDistBias(1.0f), mDeformation(true), mEditChannel(0), mAlpha(1)
{
    mSceneMgr = sceneManager;

    mCamera = mSceneMgr->getCamera(camName);

    mSingleton = new HeightmapReader();

    mObjectHandler = new ObjectHandler(mSceneMgr,this);

    vBrushPos = Ogre::Vector3::ZERO;
    bGetNewPos = true;

    rFlattenHeight = 0.0f;
    bGetNewHeight = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Terrain::~Terrain()
{
    destroy();

    delete mObjectHandler;
    mObjectHandler = 0;


    delete mSingleton;
    mSingleton = 0;

    if (mTerrainNode)
        mSceneMgr->getRootSceneNode()->removeAndDestroyChild(mTerrainNode->getName());


    IndexStorage::iterator it = mIndexStore.begin();
    IndexStorage::iterator itEnd = mIndexStore.end();

    while (it != itEnd)
    {
        delete it->second;
        ++it;
    }
    mIndexStore.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::addIndexData( Ogre::IndexData* pData, size_t iLOD )
{
    IndexStorage::iterator it = mIndexStore.find(iLOD);
    if (it == mIndexStore.end())
    {
        mIndexStore.insert(IndexStorage::value_type(iLOD,pData));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ogre::IndexData* Terrain::getIndexData( size_t iLOD )
{
    IndexStorage::iterator it = mIndexStore.find(iLOD);
    if (it != mIndexStore.end())
    {
        return it->second;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::setMaterial( const std::string& materialName )
{
    mTerrainMaterial = MaterialManager::getSingleton().getByName(materialName.c_str());
    if (mRootNode)
    {
        mRootNode->updateMaterial();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::initialize( const Ogre::String& heightmapName, size_t iWidth /*= 0*/ )
{
    if (mTerrainNode == 0)
        mTerrainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

    mHeightmap = new Heightmap(heightmapName,iWidth,false);
    mHeightmap->calculateMaxLevels();
    setMaxDepth(mHeightmap->getMaxLevels());

    mRootNode = new QNode(this);

    if (mSkirtLength < 1.0f)
        setSkirtLengthPercent(5);

    mRootNode->buildHeightmap();
    mRootNode->buildTerrain();

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::destroy()
{
    mObjectHandler->destroyAllSceneObjects();

    destroyBrushDecal();
    destroyAllBrushes();

    delete mLightmapper;
    mLightmapper = 0;

    delete mRootNode;
    mRootNode = 0;

    delete mHeightmap;
    mHeightmap = 0;

    if (mTerrainMaterial.isNull() == false)
    {
        mTerrainMaterial->unload();
        mTerrainMaterial.setNull();
    }
    mInitialized = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::SaveTerrainData(const std::string& filePath)
{
     mHeightmap->saveHeightmap(filePath);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double Terrain::getHeightAt( double vX, double vZ, double rBonus /*= 0.0f*/, bool bUseRoot /*= false*/, bool bUseGeoMorphing /*= false*/ )
{
      Ogre::Vector3 v = Ogre::Vector3(vX,0,vZ);
      getHeightAt(v, rBonus, bUseRoot, bUseGeoMorphing);
      return v.y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Terrain::getHeightAt( Ogre::Vector3& vPos, Ogre::Real rBonus /*= 0.0f*/, bool bUseRoot /*= false*/, bool bUseGeoMorphing /*= false*/ )
{
    if (mRootNode)
    {
        if (mRootNode->isPointInNode(vPos.x, vPos.z))
        {
            Ogre::Real h = 0.0f;
            if (bUseRoot == false)
            {
                QNode* pPointNode = mRootNode->getNodeAtPoint(vPos.x,vPos.z);
                if (mHeightmap->getMaxLevels() >= pPointNode->getDepth())
                {
                    float fPosX = (vPos.x + (mWidth * 0.5)) / mWidth;
                    float fPosZ = (vPos.z + (mWidth * 0.5)) / mWidth;

                    h = mHeightmap->getHeightAt(
                        fPosX,fPosZ,
                        mHeightmap->getMaxLevels() - pPointNode->getDepth(),
                        pPointNode->getMapOffset().x, pPointNode->getMapOffset().y);


                    QNode* pParent = pPointNode->getParent();
                    if (bUseGeoMorphing && pParent)
                    {
                        Ogre::Real h2 = mHeightmap->getHeightAt(
                            fPosX,fPosZ,
                            mHeightmap->getMaxLevels() - pParent->getDepth(),
                            pParent->getMapOffset().x, pParent->getMapOffset().y);
                        h = Lerp(h,h2,pPointNode->getTerrainMesh()->getMorph());
                    }
                }
            }
            else
            {
                float fPosX = (vPos.x + (mWidth * 0.5)) / mWidth;
                float fPosZ = (vPos.z + (mWidth * 0.5)) / mWidth;

                h = mHeightmap->getHeightAt(fPosX,fPosZ,0,0,0);
            }

            h *= (1.0f / MAX_INT) * getTerrainHeight();

            vPos.y = h + rBonus;
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::onFrameStart( double fT )
{
    Ogre::Real fTime = fT;
    static Ogre::Real sTime = 0.0f;
    if (mRootNode)
    {
        Ogre::Vector3 vPos;
        switch(mClampState)
        {
            case ACS_BELOW:
                vPos = mCamera->getDerivedPosition();
                if (getHeightAt(vPos,mClampBonusHeight,mClampUseRoot,mClampUseGeoMorph) && mCamera->getDerivedPosition().y < vPos.y)
                    mCamera->setPosition(vPos);
                break;
            case ACS_ALWAYS:
                vPos = mCamera->getDerivedPosition();
                if (getHeightAt(vPos,mClampBonusHeight,mClampUseRoot,mClampUseGeoMorph))
                    mCamera->setPosition(vPos);
                break;
            default:
                break;
        }

        if (mInitialized == false)
        {
            mQuickLoadTime -= fTime;
            if (mQuickLoadTime <= 0.0f)
            {
                setLODCheckTime(mPreLODCheckTime);
                setMorphSpeed(mPreMorphSpeed);
                mInitialized = true;
            }
        }
        sTime += fTime;
        if (sTime >= getLODCheckTime())
        {
            mRootNode->runLODChecks();
            sTime = 0.0f;
        }

        mRootNode->runUpdate(fTime);

        if (mObjectHandler)
            mObjectHandler->updateObjects(fTime);
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::onFrameEnd( Ogre::Real fTime )
{
    if (mRootNode)
        mRootNode->runQuadTreeChecks();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::showBrush(const std::string& bName, double width, double height)
{
            Ray ray = mCamera->getCameraToViewportRay(0.5,0.5);
            if (getRayHeight(ray,vBrushPos,true,false))
                bGetNewPos = false;

            cBrush = BrushPtr(new Brush(bName));
            //cBrush->setInterpolated(true);
            setBrushVisible(true);
            setBrushSize(Ogre::Vector2(width,height));
            setBrushPosition(vBrushPos);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ogre::Vector3 Terrain::mouseWorldPosition(double xMouse, double yMouse, double wWindow, double hWindow, Ogre::Camera* activeCam)
{
    Ray ray;
    if(activeCam != NULL)
        ray = activeCam->getCameraToViewportRay(xMouse/wWindow,yMouse/hWindow);
    else
        ray = mCamera->getCameraToViewportRay(xMouse/wWindow,yMouse/hWindow);

    if (getRayHeight(ray,vBrushPos,true,false))
    {
       double x =(double) vBrushPos.x;
       double y =(double) vBrushPos.y;
       double z =(double) vBrushPos.z;
       return Ogre::Vector3(x,y,z);
    }
    return Ogre::Vector3(99999*6,99999*6,99999*6);
}

void Terrain::createTerrainModifier(const std::string& bName, double width, double height, double intensity)
{
    if (bGetNewPos)
    {
        Ray ray = mCamera->getCameraToViewportRay(0.5,0.5);
        if (getRayHeight(ray,vBrushPos,true,false))
            bGetNewPos = false;
    }

    if (bGetNewPos == false)
    {
        cBrush = BrushPtr(new Brush(bName));
        //cBrush->setInterpolated(true);
        addTerrainModifier(BrushDisplacement(cBrush, vBrushPos, width, height, intensity));

        Vector4 vSplatScales;
        for (int i=0;i<4;i++)
        vSplatScales[i] = Math::RangeRandom(1.0f,1000.0f);
        setMaterialSchemeParams("vSplatScales",vSplatScales, getDefaultMaterialScheme());

        setBrushVisible(true);
        setBrushSize(Ogre::Vector2(width,height));
        setBrushPosition(vBrushPos);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 void Terrain::createTerrainModifierDown(double width, double height)
{
    Ray ray = mCamera->getCameraToViewportRay(0.5,0.5);
    Ogre::Vector3 vRayPos = Ogre::Vector3::ZERO;

    if (getRayHeight(ray,vRayPos,true,false))
    {
        if (bGetNewHeight)
        {
            rFlattenHeight = vRayPos.y;
            bGetNewHeight = false;
        }
        else
        {
            vRayPos.y = rFlattenHeight;
        }
        addTerrainModifier(RectFlatten(vRayPos,width, height));

        setBrushVisible(true);
        setBrushSize(Ogre::Vector2(width,height));
        setBrushPosition(vRayPos);
    }
}

void Terrain::setVisible(bool visible) 
{ 
	mRootNode->getTerrainMesh()->setVisible(visible); 
}

bool Terrain::isVisible() 
{ 
	return mRootNode->getTerrainMesh()->isVisible(); 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QNode* Terrain::addTerrainModifier( const TerrainModifier& modifier )
{
    if (modifier.isInBounds(mRootNode->getFloatingPointOffset(),mRootNode->getFloatingPointOffset() + mRootNode->getFloatingPointWidth()))
    {
        QNode* pHighestNode = mRootNode->getHighestAffected(&modifier);

        if (getDeformation())
        {

            modifier.displace(mRootNode->getFloatingPointOffset(),mRootNode->getFloatingPointWidth(),mHeightmap,getTerrainHeight());
            mRootNode->checkModifier(&modifier);
        }
        else
        {
            if (mSelectedTexture.isNull() == false)
                modifier.displaceTexture(mRootNode->getFloatingPointOffset(),mRootNode->getFloatingPointWidth(),mSelectedTexture,getEditChannel());

        }

        if (mAutoUpdateLightmap)
            updateLightmap();

        return pHighestNode;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GPULightmapper* Terrain::createLightmapper(
    const Ogre::String& strLightmapMatName,
    const Ogre::String& strCompositorName,
    size_t iMaxLightmapSize /*= 2048*/,
    size_t iMaxHeightmapTexSize /*= 2049*/,
    const Ogre::String& strLightmapScaleParamName /*= "vScale"*/,
    const Ogre::String& strLightmapHeightmapSizeParamName /*= "fSize"*/,
    const Ogre::String& strHeightmapTexName /*= "SPT_Heightmap_Tex"*/,
    const Ogre::String& strRenderTextureName /*= "SPT_Lightmap_RTT"*/,
    const Ogre::String& strLightmapTexUnitName /*= "Heightmap"*/,
    const Ogre::String& strTerrainTexUnitName /*= "Lightmap"*/,
    const Ogre::String& strCompositorScaleParamName /*= "vScale"*/ )
{
    if (mLightmapper)
    {
        delete mLightmapper;
        mLightmapper = 0;
    }

    mLightmapper = new GPULightmapper(
        mSceneMgr,mCamera,
        mHeightmap,
        mLightDirection,
        strLightmapMatName,
        mTerrainMaterial->getName(),
        strCompositorName,
        Ogre::Vector3(mWidth,mTerrainHeight,mWidth),
        iMaxLightmapSize,
        iMaxHeightmapTexSize,
        strLightmapScaleParamName,
        strLightmapHeightmapSizeParamName,
        strHeightmapTexName,
        strRenderTextureName,
        strLightmapTexUnitName,
        strTerrainTexUnitName,
        strCompositorScaleParamName);

    return mLightmapper;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::quickSetup(
    const Ogre::String& strHeightmapName,
    const Ogre::String& strTerrainMaterialName,
    Ogre::Real fTerrainWidth,
    Ogre::Real fTerrainHeight,
    Ogre::Real fTerrainQuickLoadTime /*= 1.0f*/ )
{
    setMaterial(strTerrainMaterialName);
    mWidth = fTerrainWidth;
    mTerrainHeight = fTerrainHeight;

    mQuickLoadTime = fTerrainQuickLoadTime;
    mPreLODCheckTime = mLODCheckTime;
    mPreMorphSpeed = mMorphSpeed;
    mMorphSpeed = 0.0f;
    mLODCheckTime = 0.0f;

    initialize(strHeightmapName,0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::setLightDirection( const Ogre::Vector3& vLightDir )
{
    mLightDirection = vLightDir;

    if (mLightmapper)
        mLightmapper->updateLightmap(vLightDir);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ogre::Vector3 Terrain::getNormalAt( Ogre::Vector3& vPos, bool bUseRoot /*= false*/, bool bUseGeoMorphing /*= false*/ )
{

    Ogre::Vector3 vTri1 = vPos;
    getHeightAt(vTri1,bUseRoot,bUseGeoMorphing);

    Ogre::Vector3 vTri2 = vPos + Ogre::Vector3(1,0,0);
    getHeightAt(vTri2,bUseRoot,bUseGeoMorphing);

    Ogre::Vector3 vTri3 = vPos + Ogre::Vector3(0,0,1);
    getHeightAt(vTri3,bUseRoot,bUseGeoMorphing);

    Ogre::Vector3 vSide1 = vTri2 - vTri1;
    Ogre::Vector3 vSide2 = vTri3 - vTri1;

    vSide1.normalise();
    vSide2.normalise();

    Ogre::Vector3 vRet = vSide2.crossProduct(vSide1);

    return vRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Terrain::getRayHeight( const Ogre::Ray& vRay, Ogre::Vector3& vReturnPos, bool bUseRoot /*= false*/, bool bUseGeoMorphing /*= false*/ )
{
    const Ogre::Vector3 vRayDir = vRay.getDirection().normalisedCopy();

    Ogre::Vector3 vRayPos = vRay.getOrigin();

    if (FLOAT_EQ(vRayDir.x,0.0f) && FLOAT_EQ(vRayDir.z,0.0f))
    {
        vReturnPos = vRayPos;
        return getHeightAt(vReturnPos,0.0f,bUseRoot,bUseGeoMorphing);
    }

    for (size_t i=0;i < size_t(mWidth * 10.0f);i++)
    {
        vRayPos += vRayDir;
        vReturnPos = vRayPos;

        if (getHeightAt(vReturnPos,0.0f,bUseRoot,bUseGeoMorphing))
        {
            if (vRayPos.y <= vReturnPos.y)
                return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::updateLightmap()
{
    if (mLightmapper)
    {
        mLightmapper->updateHeightmapBuffer();
        mLightmapper->updateLightmap(mLightDirection);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RaySceneQuery* Terrain::createRayQuery( const Ray& ray, unsigned long mask /*= 0xFFFFFFFF*/ )
{
    SPTRaySceneQuery *rsq = new SPTRaySceneQuery(this);
    rsq->setRay(ray);
    rsq->setQueryMask(mask);
    return rsq;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::_fireMeshCreated( TerrainMesh* mesh )
{
    mEventListener->meshCreated(mesh);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::_fireMeshDestroyed( TerrainMesh* mesh )
{
    mEventListener->meshDestroyed(mesh);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::_fireMeshUpdated( TerrainMesh* mesh )
{
    mEventListener->meshUpdated(mesh);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::setTerrainEventListener( TerrainEventListener* listener )
{
    if (listener == 0)
        mEventListener = &mDefaultListener;
    else
        mEventListener = listener;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::setMaterialSchemeParams( const Ogre::String& paramName, const Ogre::Vector3& paramVal, const Ogre::String& schemeName /*= "Default"*/ )
{
    Material::TechniqueIterator itTech = mTerrainMaterial->getTechniqueIterator();
    while (itTech.hasMoreElements())
    {
        Technique* tech = itTech.getNext();
        if (tech->getSchemeName() == schemeName)
        {
            Technique::PassIterator itPass = tech->getPassIterator();
            while (itPass.hasMoreElements())
            {
                Pass* pass = itPass.getNext();
                GpuProgramParametersSharedPtr params;
                if (pass->hasVertexProgram())
                {
                    params = pass->getVertexProgramParameters();
                    if (params->_findNamedConstantDefinition(paramName))
                    {
                        params->setNamedConstant(paramName,paramVal);
                    }
                }
                if (pass->hasFragmentProgram())
                {
                    params = pass->getFragmentProgramParameters();
                    if (params->_findNamedConstantDefinition(paramName))
                    {
                        params->setNamedConstant(paramName,paramVal);
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::setMaterialSchemeParams( const Ogre::String& paramName, const Ogre::Vector4& paramVal, const Ogre::String& schemeName /*= "Default"*/ )
{
    Material::TechniqueIterator itTech = mTerrainMaterial->getTechniqueIterator();
    while (itTech.hasMoreElements())
    {
        Technique* tech = itTech.getNext();
        if (tech->getSchemeName() == schemeName)
        {
            Technique::PassIterator itPass = tech->getPassIterator();
            while (itPass.hasMoreElements())
            {
                Pass* pass = itPass.getNext();
                GpuProgramParametersSharedPtr params;
                if (pass->hasVertexProgram())
                {
                    params = pass->getVertexProgramParameters();
                    if (params->_findNamedConstantDefinition(paramName))
                    {
                        params->setNamedConstant(paramName,paramVal);
                    }
                }
                if (pass->hasFragmentProgram())
                {
                    params = pass->getFragmentProgramParameters();
                    if (params->_findNamedConstantDefinition(paramName))
                    {
                        params->setNamedConstant(paramName,paramVal);
                    }
                }

            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::setMaterialSchemeParams( const Ogre::String& paramName, const Ogre::Matrix4& paramVal, const Ogre::String& schemeName /*= "Default"*/ )
{
    Material::TechniqueIterator itTech = mTerrainMaterial->getTechniqueIterator();
    while (itTech.hasMoreElements())
    {
        Technique* tech = itTech.getNext();
        if (tech->getSchemeName() == schemeName)
        {
            Technique::PassIterator itPass = tech->getPassIterator();
            while (itPass.hasMoreElements())
            {
                Pass* pass = itPass.getNext();
                GpuProgramParametersSharedPtr params;
                if (pass->hasVertexProgram())
                {
                    params = pass->getVertexProgramParameters();
                    if (params->_findNamedConstantDefinition(paramName))
                    {
                        params->setNamedConstant(paramName,paramVal);
                    }
                }
                if (pass->hasFragmentProgram())
                {
                    params = pass->getFragmentProgramParameters();
                    if (params->_findNamedConstantDefinition(paramName))
                    {
                        params->setNamedConstant(paramName,paramVal);
                    }
                }

            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::setMaterialSchemeParams( const Ogre::String& paramName, const Ogre::Real& paramVal, const Ogre::String& schemeName /*= "Default"*/ )
{
    Material::TechniqueIterator itTech = mTerrainMaterial->getTechniqueIterator();
    while (itTech.hasMoreElements())
    {
        Technique* tech = itTech.getNext();
        if (tech->getSchemeName() == schemeName)
        {
            Technique::PassIterator itPass = tech->getPassIterator();
            while (itPass.hasMoreElements())
            {
                Pass* pass = itPass.getNext();
                GpuProgramParametersSharedPtr params;
                if (pass->hasVertexProgram())
                {
                    params = pass->getVertexProgramParameters();
                    if (params->_findNamedConstantDefinition(paramName))
                    {
                        params->setNamedConstant(paramName,paramVal);
                    }
                }
                if (pass->hasFragmentProgram())
                {
                    params = pass->getFragmentProgramParameters();
                    if (params->_findNamedConstantDefinition(paramName))
                    {
                        params->setNamedConstant(paramName,paramVal);
                    }
                }

            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ogre::String Terrain::getDefaultMaterialScheme()
{
    if (mTerrainMaterial.isNull() == false)
    {
        if (mTerrainMaterial->getCompilationRequired())
            mTerrainMaterial->compile();

        return mTerrainMaterial->getBestTechnique()->getSchemeName();
    }
    return "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::initializeBrushDecal( const std::string& brushTexName, double sizex, double sizey )
{
    const Ogre::Vector2 &size = Ogre::Vector2(sizex, sizey);

    destroyBrushDecal();

    mBrushTextureName = brushTexName;

    String schemeName = mCamera->getViewport()->getMaterialScheme();
    if (schemeName == "Default" && getDefaultMaterialScheme() != schemeName)
        schemeName = getDefaultMaterialScheme();

    mDecalPass = 0;

    Material::TechniqueIterator itTech = mTerrainMaterial->getSupportedTechniqueIterator();
    while (itTech.hasMoreElements() && mDecalPass == 0)
    {
        Technique* pTech = itTech.getNext();
        if (pTech->getSchemeName() == schemeName)
        {
            mDecalPass = pTech->createPass();
            mDecalPass->setName("SPT_Auto_DecalPass");
            //mDecalPass->setLightingEnabled(false);
            //mDecalPass->setSceneBlending(SBF_DEST_ALPHA,SBF_DEST_ALPHA);
            mDecalPass->setSceneBlending(SBF_SOURCE_ALPHA,SBF_ONE_MINUS_SOURCE_ALPHA);
            //mDecalPass->setSceneBlending(SBT_REPLACE);
            mDecalPass->setVertexProgram("DecalHeight_VS");
            mDecalPass->setFragmentProgram("DecalHeight_PS");
            //mDecalPass->setDepthBias(2.5, 2.5);
            //mDecalPass->setDepthCheckEnabled(false);

            //mDecalPass->setFog(true);
            //mDecalPass->createTextureUnitState("decalBase.png");
        }
    }

    if (mDecalPass == 0)
        return;

    setBrushSize(size);

    setBrushVisible(false);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::destroyBrushDecal()
{
    if(mDecalPass)
    {
        mDecalPass->getParent()->removePass(mDecalPass->getIndex());
        mDecalPass = 0;
        mBrushTexState = 0;
    }
}

void Terrain::setBrushSize( const Ogre::Vector2& size )
{
    mBrushSize = size;

    if (mDecalPass == 0)
        return;

    if (mDecalPass->hasVertexProgram())
    {
        GpuProgramParametersSharedPtr pParams = mDecalPass->getVertexProgramParameters();
        if (pParams->_findNamedConstantDefinition("vDecalFactors"))
            pParams->setNamedConstant("vDecalFactors",_getBrushParams());
        if (pParams->_findNamedConstantDefinition("fMaxHeight"))
            pParams->setNamedConstant("fMaxHeight",getTerrainHeight());
    }


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::setBrushPosition( const Ogre::Vector3& pos )
{
    mBrushPosition = pos;

    if (mDecalPass == 0)
        return;

    if (mDecalPass->hasVertexProgram())
    {
        GpuProgramParametersSharedPtr pParams = mDecalPass->getVertexProgramParameters();
        if (pParams->_findNamedConstantDefinition("vDecalFactors"))
            pParams->setNamedConstant("vDecalFactors",_getBrushParams());
        if (pParams->_findNamedConstantDefinition("fMaxHeight"))
            pParams->setNamedConstant("fMaxHeight",getTerrainHeight());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::setBrushVisible( bool visibile /*= true*/ )
{
    mShowBrush = visibile;

    if (mDecalPass == 0)
        return;

    if (visibile && mBrushTexState == 0)
    {
        mBrushTexState = mDecalPass->createTextureUnitState(mBrushTextureName);
        //mBrushTexState->setProjectiveTexturing(true, mDecalFrustum);
        mBrushTexState->setTextureAddressingMode(TextureUnitState::TAM_BORDER);
        mBrushTexState->setTextureBorderColour(ColourValue::Black);
        mBrushTexState->setTextureFiltering(FO_POINT, FO_LINEAR, FO_NONE);
        //mBrushTexState->setAlphaOperation(LBX_ADD,);
        //mBrushTexState->setColourOperationEx(LBX_ADD);

        TextureUnitState* pElevGrad = mDecalPass->createTextureUnitState();
        pElevGrad->setTextureName("ElevationGrad.png",TEX_TYPE_1D);
        pElevGrad->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
        mBrushTexState->setTextureFiltering(FO_POINT, FO_LINEAR, FO_NONE);
    }
    else if (visibile == false && mBrushTexState)
    {
        mDecalPass->removeAllTextureUnitStates();
        mBrushTexState = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ogre::Vector4 Terrain::_getBrushParams()
{
    Ogre::Vector2 vBrushPos(mBrushPosition.x,mBrushPosition.z);
    vBrushPos -= mBrushSize * 0.5f;

    vBrushPos += (0.5 * getWidth());
    vBrushPos /= getWidth();

    Ogre::Vector2 vBrushScale(getWidth(),getWidth());
    vBrushScale /= mBrushSize;

    Vector4 vParams(vBrushPos.x,vBrushPos.y,vBrushScale.x,vBrushScale.y);

    return vParams;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BrushPtr Terrain::createBrush( const Ogre::String& brushImageName)
{
    BrushPtr pBrush;
    pBrush.setNull();

    //if (ResourceGroupManager::getSingleton().resourceExists("General",brushImageName) == false)
    //	return pBrush;

    pBrush = getBrush(brushImageName);
    if (pBrush.isNull() == false)
        return pBrush;

    Image img;
    img.load(brushImageName,"General");


    pBrush = BrushPtr(new Brush(img,brushImageName));

    mBrushStorage.insert(BrushStorage::value_type(brushImageName,pBrush));

    return pBrush;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::destroyBrush( const Ogre::String& brushImageName )
{
    BrushStorage::iterator it = mBrushStorage.find(brushImageName);
    if (it != mBrushStorage.end())
    {
        BrushPtr pBrush = it->second;
        pBrush.setNull();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::destroyBrush( BrushPtr pBrush )
{
    if (pBrush.isNull() == false)
    {
        destroyBrush(pBrush->getTexName());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::destroyAllBrushes()
{
    BrushStorage::iterator it = mBrushStorage.begin();
    BrushStorage::iterator itEnd = mBrushStorage.end();
    while (it != itEnd)
    {
        it->second.setNull();
        ++it;
    }
    mBrushStorage.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BrushPtr Terrain::getBrush( size_t index )
{
    BrushPtr pBrush;
    if (mBrushStorage.size() > index)
    {
        BrushStorage::iterator it = mBrushStorage.begin();
        for (size_t i=0;i<index;i++)
            ++it;

        return it->second;
    }
    return pBrush;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BrushPtr Terrain::getBrush( const Ogre::String& brushImageName )
{
    BrushPtr pBrush;
    BrushStorage::iterator it = mBrushStorage.find(brushImageName);
    if (it != mBrushStorage.end())
        pBrush = it->second;

    return pBrush;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::saveTerrain( const Ogre::String& filePath )
{
    assert(filePath != "" && "File path is blank!");
    mHeightmap->saveHeightmap(filePath);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Terrain::brushExists( const Ogre::String& brushImageName )
{
    BrushPtr pBrush = getBrush(brushImageName);
    return (pBrush.isNull() == false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::setLODDistBias( Ogre::Real val )
{
    mLODDistBias = val;
    if (mRootNode)
    {
        mRootNode->updateLODDistances();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::updateHeightmap()
{
    mRootNode->checkModifier(0,true);
    if (mAutoUpdateLightmap)
        updateLightmap();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::setSelectedTexture( const Ogre::String& texName )
{
    mSelectedTexture = TextureManager::getSingleton().getByName(texName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::clearSelectedTexture()
{
    mSelectedTexture.setNull();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::saveSelectedTexture( const Ogre::String& filePath )
{
    if (mSelectedTexture.isNull() == false)
    {
        HardwarePixelBufferSharedPtr readbuffer;
        readbuffer = mSelectedTexture->getBuffer(0, 0);
        readbuffer->lock(HardwareBuffer::HBL_NORMAL );
        const PixelBox &readrefpb = readbuffer->getCurrentLock();
        ::uchar *readrefdata = static_cast< ::uchar*>(readrefpb.data);

        Image img;
        img = img.loadDynamicImage (readrefdata, mSelectedTexture->getWidth(),
            mSelectedTexture->getHeight(), mSelectedTexture->getFormat());

        img.save(filePath);

        readbuffer->unlock();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Terrain::setOpacity(float alpha)
{
    mAlpha = alpha;
    for(unsigned i = 0; i < mTerrainMaterial->getTechnique(0)->getNumPasses(); i++)
    {
        Ogre::Pass* pass = mTerrainMaterial->getTechnique(0)->getPass(i);

		if(alpha < 1)
		{
			pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
			pass->setDepthWriteEnabled(false);
		}
		else
		{
			//rever necessidade dessa condição (ícaro)
			//if(pass->getName() == "Map#2")
			//{
				pass->setSceneBlending(Ogre::SBF_ONE , Ogre::SBF_ZERO);
				pass->setDepthWriteEnabled(true);
				//pass->setLightingEnabled(true);
				//pass->setDepthFunction(Ogre::CMPF_LESS_EQUAL);
			//}
		}

		if(pass->getName() != "Map#2")
		{
			Ogre::ColourValue diff = pass->getDiffuse();
			diff.a = alpha;
			pass->setDiffuse(diff);
		}
		else
		{
			for(int j = 0; j < pass->getNumTextureUnitStates(); j++)
			{
				pass->getTextureUnitState(j)->setAlphaOperation(Ogre::LBX_SOURCE1, Ogre::LBS_MANUAL, Ogre::LBS_TEXTURE, mAlpha);
			}
		}
    }

    if(mRootNode)   mRootNode->updateMaterial();

}

HEIGHTMAPTYPE* Terrain::getHeightmapData()
{
	return mHeightmap->getData();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

SPTRaySceneQuery::SPTRaySceneQuery( Terrain* terrain )
    : DefaultRaySceneQuery(terrain->getSceneManager()),
    mTerrain(terrain)
{
    mSupportedWorldFragments.insert(SceneQuery::WFT_SINGLE_INTERSECTION);
}

SPTRaySceneQuery::SPTRaySceneQuery( Ogre::SceneManager* creator )
    : DefaultRaySceneQuery(creator),
    mTerrain(0)
{

}
SPTRaySceneQuery::~SPTRaySceneQuery()
{

}

void SPTRaySceneQuery::execute( RaySceneQueryListener* listener )
{
    mWorldFragment.fragmentType = SceneQuery::WFT_SINGLE_INTERSECTION;

    Ogre::Vector3 pos = mRay.getOrigin();
    if (mTerrain->getRayHeight(mRay,pos,mTerrain->getClampUseRoot(),mTerrain->getClampUseGeoMorph()))
    {
        mWorldFragment.singleIntersection = pos;
        if (!listener->queryResult(&mWorldFragment,
            (mWorldFragment.singleIntersection - mRay.getOrigin()).length()))
            return;
    }
    DefaultRaySceneQuery::execute(listener);
}

