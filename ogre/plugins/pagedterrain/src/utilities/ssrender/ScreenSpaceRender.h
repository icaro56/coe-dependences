#pragma once

#include "PTPrerequisites.h"
#include <OgrePrerequisites.h>
#include <OgreMaterial.h>
#include <OgreTexture.h>
#include <OgreGpuProgram.h>
#include <OgreCompositor.h>
#include <OgreVector3.h>
#include "DllRequisites.h"


//class SSRenderManager;
class PAGEDTERRAIN_EXPORT ScreenSpaceRender
{
public:
    ScreenSpaceRender(Ogre::Camera* pCamera, const size_t width, const size_t height, const Ogre::String& materialName, const Ogre::String& compositorName);
    ~ScreenSpaceRender();
protected:
    Ogre::TexturePtr mRTT;
    Ogre::RenderTarget* mTarget;
    Ogre::MaterialPtr mScreenMaterial;
    Ogre::GpuProgramParametersSharedPtr mVertexParameters;
    Ogre::GpuProgramParametersSharedPtr mFragmentParameters;
    Ogre::CompositorPtr mCompositor;
    Ogre::Camera* mCamera;
    size_t mFramesToCompletion;
    size_t mCurrentFrame;
    Ogre::Vector3 mOffsetAndScale;

    //SSRenderManager* mManager;
};
