#include "GPULightmapper.h"

#include "heightmap/Heightmap.h"

#include <OgreMaterialManager.h>
#include <OgreCompositor.h>
#include <OgreCompositorInstance.h>
#include <OgreCompositionTechnique.h>
#include <OgreCompositionTargetPass.h>
#include <OgreCompositionPass.h>
#include <OgreCompositorManager.h>
#include <OgreTextureManager.h>
#include <OgreSceneManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreBitwise.h>

using namespace Ogre;



	GPULightmapper::GPULightmapper(
		Ogre::SceneManager* pSceneMgr,
		Ogre::Camera* pCamera,
		Heightmap* pRootHeightmap,
		const Ogre::Vector3& vLightDir,
		const Ogre::String& strLightmapMatName,
		const Ogre::String& strTerrainMatName,
		const Ogre::String& strCompositorName,
		const Ogre::Vector3& vTerrainScale,
		size_t iMaxLightmapSize /*= 2048*/,
		size_t iMaxHeightmapTexSize /*= 2049*/,
		const Ogre::String& strLightmapScaleParamName /*= "vScale"*/,
		const Ogre::String& strLightmapHeightmapSizeParamName /*= "fSize"*/,
		const Ogre::String& strHeightmapTexName /*= "SPT_Heightmap_Tex"*/,
		const Ogre::String& strRenderTextureName /*= "SPT_Lightmap_RTT"*/,
		const Ogre::String& strLightmapTexUnitName /*= "Lightmap"*/,
		const Ogre::String& strTerrainTexUnitName /*= "Lightmap"*/,
		const Ogre::String& strCompositorScaleParamName /*= "vScale"*/ )
		: mSceneMgr(pSceneMgr),
		mRootHeightmap(pRootHeightmap),
		mLightmapTextureUnitName(strLightmapTexUnitName),
		mTerrainTextureUnitName(strTerrainTexUnitName),
		mCompositorScaleParamName(strCompositorScaleParamName),
		mCompositorInstance(0),
		mRenderTarget(0),
		mViewport(0),
		mLightDirection(vLightDir),
		mLightmapScaleParamName(strLightmapScaleParamName),
		mLightmapHeightmapSizeParamName(strLightmapHeightmapSizeParamName)
	{
		mLightmapMaterial = MaterialManager::getSingleton().getByName(strLightmapMatName);
		mTerrainMaterial = MaterialManager::getSingleton().getByName(strTerrainMatName);


		size_t iMapWidth = mRootHeightmap->getWidth();


		bool bNonPow2Enabled = mSceneMgr->getDestinationRenderSystem()->getCapabilities()->hasCapability(RSC_NON_POWER_OF_2_TEXTURES);

		size_t iLightmapWidth = (iMapWidth > iMaxLightmapSize) ? iMaxLightmapSize : iMapWidth - 1;

		if (Bitwise::isPO2(iLightmapWidth) == false && bNonPow2Enabled == false)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,"Can't use non-power-of-two texture for light-map on your machine.","GPULightmapper()");


		mCamera = mSceneMgr->createCamera("SPT_Lightmap_Camera");
		mCamera->setAspectRatio(1.0f);



		mRenderTexture = TextureManager::getSingleton().createManual(
							strRenderTextureName,"General",
							TEX_TYPE_2D,iLightmapWidth,iLightmapWidth,
							1,0,Ogre::PF_BYTE_RGBA,TU_RENDERTARGET);


		mRenderTarget = mRenderTexture->getBuffer()->getRenderTarget();
		mViewport = mRenderTarget->addViewport(mCamera);
		mViewport->setOverlaysEnabled(false);
		mViewport->setClearEveryFrame(false);

		mRenderTarget->setAutoUpdated(false);



		CompositorPtr compositor = CompositorManager::getSingleton().create(
			strCompositorName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		CompositionTechnique* mTechnique = compositor->createTechnique();
		{
			{
				CompositionTargetPass* output = mTechnique->getOutputTargetPass();
				output->setInputMode(CompositionTargetPass::IM_NONE);
				{
					CompositionPass* pass = output->createPass();
					pass->setType(CompositionPass::PT_RENDERQUAD);
					pass->setMaterialName(strLightmapMatName);
				}
			}
		}


		mCompositorInstance = CompositorManager::getSingleton().addCompositor(
			mViewport, strCompositorName);
		CompositorManager::getSingleton().setCompositorEnabled(mViewport,
			strCompositorName, false);


		size_t iHeightmapTexSize = (iMapWidth > iMaxHeightmapTexSize) ? iMaxHeightmapTexSize : iMapWidth;
		iHeightmapTexSize = bNonPow2Enabled ? iHeightmapTexSize : iHeightmapTexSize - 1;


		buildHeightmapTexture(strHeightmapTexName,iHeightmapTexSize);
		updateLightmap(mLightDirection);


		Material::TechniqueIterator itTech = mLightmapMaterial->getTechniqueIterator();
		while (itTech.hasMoreElements())
		{
			Technique::PassIterator itPass = itTech.getNext()->getPassIterator();
			while (itPass.hasMoreElements())
			{
				Pass* pPass = itPass.getNext();

				TextureUnitState* pTexState = pPass->getTextureUnitState(strLightmapTexUnitName);
				if (pTexState)
					pTexState->setTextureName(strHeightmapTexName);

				if (pPass->hasFragmentProgram())
				{
					GpuProgramParametersSharedPtr pParams = pPass->getFragmentProgramParameters();
					if (pParams->_findNamedConstantDefinition(mLightmapScaleParamName))
						pParams->setNamedConstant(mLightmapScaleParamName,vTerrainScale);

					if (pParams->_findNamedConstantDefinition(mLightmapHeightmapSizeParamName))
						pParams->setNamedConstant(mLightmapHeightmapSizeParamName,Real(iHeightmapTexSize));
				}
			}
		}

		itTech = mTerrainMaterial->getTechniqueIterator();
		while (itTech.hasMoreElements())
		{
			Technique::PassIterator itPass = itTech.getNext()->getPassIterator();
			while (itPass.hasMoreElements())
			{
				TextureUnitState* pTexState = itPass.getNext()->getTextureUnitState(strTerrainTexUnitName);
				if (pTexState)
					pTexState->setTextureName(strRenderTextureName);
			}
		}


	}


	GPULightmapper::~GPULightmapper()
	{
		/*Material::TechniqueIterator itTech = mLightmapMaterial->getTechniqueIterator();
		while (itTech.hasMoreElements())
		{
			Technique::PassIterator itPass = itTech.getNext()->getPassIterator();
			while (itPass.hasMoreElements())
			{
				Pass* pPass = itPass.getNext();

				TextureUnitState* pTexState = pPass->getTextureUnitState(mLightmapTextureUnitName);
				if (pTexState)
					pTexState->setTextureName("");

			}
		}

		itTech = mTerrainMaterial->getTechniqueIterator();
		while (itTech.hasMoreElements())
		{
			Technique::PassIterator itPass = itTech.getNext()->getPassIterator();
			while (itPass.hasMoreElements())
			{
				Pass* pPass = itPass.getNext();
				TextureUnitState* pTexState = pPass->getTextureUnitState(mTerrainTextureUnitName);
				if (pTexState)
					pTexState->setTextureName("");
			}
		}*/

		mCompositorInstance->setEnabled(false);
		String name = mCompositorInstance->getCompositor()->getName();
		CompositorManager::getSingleton().removeCompositor(mViewport,name);
		CompositorManager::getSingleton().remove(name);

		if (mHeightmapTexture.isNull() == false)
		{
			TextureManager::getSingleton().remove(mHeightmapTexture->getName());
			mHeightmapTexture->unload();
			mHeightmapTexture.setNull();
		}

		mRenderTarget->removeViewport(mViewport->getZOrder());

		if (mRenderTexture.isNull() == false)
		{
			TextureManager::getSingleton().remove(mRenderTexture->getName());
			mRenderTexture->unload();
			mRenderTexture.setNull();
		}

		mSceneMgr->destroyCamera(mCamera);


	}

	void GPULightmapper::updateHeightmapBuffer()
	{
		HardwarePixelBufferSharedPtr pBuf = mHeightmapTexture->getBuffer();
		pBuf->lock(HardwareBuffer::HBL_DISCARD);

		const PixelBox& pixelBox = pBuf->getCurrentLock();

		size_t destDepth = Ogre::PixelUtil::getNumElemBytes(pixelBox.format);
		size_t destPitch = (pixelBox.rowPitch*destDepth);

		size_t width = pixelBox.getWidth();
		size_t height = pixelBox.getHeight();

		uchar* pData = static_cast<uchar*>(pixelBox.data);

		uchar* source = reinterpret_cast<uchar*>(mRootHeightmap->getData());

		size_t iSize = sizeof(HEIGHTMAPTYPE);

		if (width == mRootHeightmap->getWidth())
		{


#if (USE_FLOAT_DATA == 1)
			Ogre::ushort* pTemp = new Ogre::ushort[width];
			float* pSource = reinterpret_cast<float*>(mRootHeightmap->getData());
			for(size_t row = 0; row < height; row++)
			{
				for (size_t col = 0; col < width; col++)
					pTemp[col] = static_cast<Ogre::ushort>(pSource[col + row * width]);

				memcpy(pData + row * destPitch, pTemp, width * 2);
			}
			delete[] pTemp;
			pTemp = 0;
#else
            size_t srcPitch = mRootHeightmap->getWidth() * iSize;
			for(size_t row = 0; row < height; row++)
				memcpy(pData + row * destPitch, source + row * srcPitch, srcPitch);
#endif
		}
		else
		{
			size_t iTexWidth = mHeightmapTexture->getWidth();
			//if (Bitwise::isPO2(iTexWidth))
			//{
				// With slightly-off size textures (i.e. 2048 texture width vs 2049 heightmap
				// it's not a big deal - you just leave one off the end.
				// However, this could seriously mess things up for 2048 texture and 4097 heightmap,
				// so we resize the image and stuff it into the texture.

			size_t iTotalByteSize = mRootHeightmap->getWidth() * mRootHeightmap->getWidth() * iSize;
			uchar* pCopy = new uchar[iTotalByteSize];
			memcpy(pCopy,source,iTotalByteSize);

			Image img;
			img.loadDynamicImage(pCopy,mRootHeightmap->getWidth(),mRootHeightmap->getWidth(),1,(iSize == 1) ? Ogre::PF_L8 : Ogre::PF_L16,true);

			img.resize(iTexWidth,iTexWidth);

			uchar* pImgSource = img.getData();

			size_t srcPitch = iTexWidth * iSize;

			for(size_t row = 0; row < height; row++)
				memcpy(pData + row * destPitch, pImgSource + row * srcPitch, srcPitch);

			/*}
			else
			{
				// We can just skip pixels here - not as accurate as bi-lerp, but much cheaper.
				size_t iSkip = ((mRootHeightmap->getWidth() - 1) / (iTexWidth-1) * iSize) - 1;

				size_t iPadding = ((pixelBox.rowPitch - iTexWidth) * iSize) - 1;

				for (size_t y=0;y<iTexWidth;y++)
				{
					for (size_t x=0;x<iTexWidth;x++)
					{
						for (size_t i=0;i<iSize;i++)
						{
							*pData++ = *source++;
						}
						*source += iSkip;
					}
					*pData += iPadding;
				}

			}*/
		}



		pBuf->unlock();


	}

	void GPULightmapper::updateLightmapWindow( const Ogre::Vector3& vScale )
	{
		mCompositorInstance->getCompositor()->getTechnique(0)
			->getOutputTargetPass()->getPass(0)->
			getMaterial()->getTechnique(0)->getPass(0)->
			getVertexProgramParameters()->setNamedConstant(mCompositorScaleParamName,vScale);
	}

	void GPULightmapper::buildHeightmapTexture( const Ogre::String& strHeightmapTexName, size_t iMapWidth )
	{
		if (mHeightmapTexture.isNull() == false)
		{
			mHeightmapTexture->unload();
			mHeightmapTexture.setNull();
		}

		PixelFormat pixelFormat;// = ((sizeof(HEIGHTMAPTYPE) == 1) ? Ogre::PF_L8 : Ogre::PF_L16);
		switch(sizeof(HEIGHTMAPTYPE))
		{
		case 1:
			pixelFormat = Ogre::PF_L8;
			break;
		default:
			pixelFormat = Ogre::PF_L16;
		    break;
		}

		mHeightmapTexture = TextureManager::getSingleton().createManual(strHeightmapTexName,"General",TEX_TYPE_2D,iMapWidth,iMapWidth,1,0,pixelFormat);

		updateHeightmapBuffer();
	}

	void GPULightmapper::updateLightmap(const Ogre::Vector3& vLightDir)
	{
		mLightDirection = vLightDir;

		Material::TechniqueIterator itTech = mLightmapMaterial->getTechniqueIterator();
		while (itTech.hasMoreElements())
		{
			Technique::PassIterator itPass = itTech.getNext()->getPassIterator();
			while (itPass.hasMoreElements())
			{
				Pass* pPass = itPass.getNext();
				if (pPass->hasFragmentProgram() && pPass->getFragmentProgramParameters()->_findNamedConstantDefinition("vLightDir"))
					pPass->getFragmentProgramParameters()->setNamedConstant("vLightDir",mLightDirection);
			}
		}

		mCompositorInstance->setEnabled(true);
		mRenderTarget->update();
		mCompositorInstance->setEnabled(false);
	}

