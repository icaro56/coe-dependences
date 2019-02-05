#include "ScreenSpaceRender.h"

#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
#include <OgreCompositorManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreTechnique.h>
#include <OgreCompositorInstance.h>
#include <OgreCompositionTechnique.h>
#include <OgreCompositionTargetPass.h>
#include <OgreCompositionPass.h>

using namespace Ogre;



	ScreenSpaceRender::ScreenSpaceRender(
		Ogre::Camera* pCamera,
		const size_t width,
		const size_t height,
		const Ogre::String& materialName,
		const Ogre::String& compositorName )
		: mCamera(pCamera),
		mFramesToCompletion(1),
		mCurrentFrame(0),
		mOffsetAndScale(0.0f,0.0f,1.0f)
	{

		mRTT = TextureManager::getSingleton().createManual(compositorName + "_RTT","General",TEX_TYPE_2D,width,height,1,0,Ogre::PF_BYTE_RGBA,TU_RENDERTARGET);

		mTarget = mRTT->getBuffer()->getRenderTarget();
		Viewport* pViewport = mTarget->addViewport(pCamera);
		pViewport->setOverlaysEnabled(false);
		pViewport->setClearEveryFrame(false);

		mTarget->setAutoUpdated(false);

		mScreenMaterial = MaterialManager::getSingleton().getByName(materialName);
		mVertexParameters = mScreenMaterial->getBestTechnique()->getPass(0)->getVertexProgramParameters();
		mFragmentParameters = mScreenMaterial->getBestTechnique()->getPass(0)->getFragmentProgramParameters();


		mCompositor = CompositorManager::getSingleton().create(
			compositorName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		CompositionTechnique* mTechnique = mCompositor->createTechnique();
		{
			{
				CompositionTargetPass* output = mTechnique->getOutputTargetPass();
				output->setInputMode(CompositionTargetPass::IM_NONE);
				{
					CompositionPass* pass = output->createPass();
					pass->setType(CompositionPass::PT_RENDERQUAD);
					pass->setMaterialName(materialName);
				}
			}
		}
	}

	ScreenSpaceRender::~ScreenSpaceRender()
	{
		//CompositorManager::getSingleton().removeCompositor(mRTT->getViewport(0),mCompositor->getName());
		CompositorManager::getSingleton().remove(mCompositor->getName());
		mCompositor.setNull();

		MaterialManager::getSingleton().remove(mScreenMaterial->getName());
		mScreenMaterial.setNull();

		TextureManager::getSingleton().remove(mRTT->getName());

		mVertexParameters.setNull();
		mFragmentParameters.setNull();
	}
