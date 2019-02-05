#pragma once

#include "PTPrerequisites.h"
#include <OgrePrerequisites.h>
#include <OgreMaterial.h>
#include <OgreTexture.h>
#include <OgreVector3.h>
#include "DllRequisites.h"

class PAGEDTERRAIN_EXPORT GPULightmapper
{
public:
    GPULightmapper(
        Ogre::SceneManager* pSceneMgr,
        Ogre::Camera* pCamera,
        Heightmap* pRootHeightmap,
        const Ogre::Vector3& vLightDir,
        const Ogre::String& strLightmapMatName,
        const Ogre::String& strTerrainMatName,
        const Ogre::String& strCompositorName,
        const Ogre::Vector3& vTerrainScale,
        size_t iMaxLightmapSize = 2048,
        size_t iMaxHeightmapTexSize = 2049,
        const Ogre::String& strLightmapScaleParamName = "vScale",
        const Ogre::String& strLightmapHeightmapSizeParamName = "fSize",
        const Ogre::String& strHeightmapTexName = "SPT_Heightmap_Tex",
        const Ogre::String& strRenderTextureName = "SPT_Lightmap_RTT",
        const Ogre::String& strLightmapTexUnitName = "Heightmap",
        const Ogre::String& strTerrainTexUnitName = "Lightmap",
        const Ogre::String& strCompositorScaleParamName = "vScale");

    ~GPULightmapper();

    void buildHeightmapTexture(const Ogre::String& strHeightmapTexName, size_t iMapWidth);

    void updateHeightmapBuffer();

    void updateLightmapWindow(const Ogre::Vector3& vScale);

    void updateLightmap(const Ogre::Vector3& vLightDir);

    const Ogre::String& getLightmapTextureName(){return mRenderTexture->getName();}

    Ogre::Viewport* getViewport(){return mViewport;}

private:
    Ogre::SceneManager* mSceneMgr;
    Ogre::CompositorInstance* mCompositorInstance;
    Ogre::RenderTarget* mRenderTarget;
    Ogre::Viewport* mViewport;
    Ogre::String mLightmapTextureUnitName;
    Ogre::String mTerrainTextureUnitName;
    Ogre::String mCompositorScaleParamName;
    Ogre::MaterialPtr mLightmapMaterial;
    Ogre::MaterialPtr mTerrainMaterial;
    Ogre::TexturePtr mHeightmapTexture;
    Ogre::TexturePtr mRenderTexture;
    Heightmap* mRootHeightmap;
    Ogre::Vector3 mLightDirection;
    Ogre::String mLightmapScaleParamName;
    Ogre::String mLightmapHeightmapSizeParamName;
    Ogre::Camera* mCamera;
};
