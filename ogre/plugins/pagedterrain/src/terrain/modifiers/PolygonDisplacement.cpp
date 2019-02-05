/*#include "PolygonDisplacement.h"
#include "quadtree/QNode.h"
#include "TerrainModifier.h"
#include "heightmap/Heightmap.h"
#include <OgreImage.h>
#include <OgreColourValue.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreLogManager.h>
#include <OgreStringConverter.h>
#include <OgreManualObject.h>
#include <OgreMaterialManager.h>
#include <OgreEntity.h>

using namespace Ogre;

////////////////////////////////////////////////////////////////////////////////////////
    PolygonDisplacement::PolygonDisplacement(Terrain *t)
    : mImageWidth(2048), 
	  mTerrain(t), 
	  mPlane(-1), 
	  mfWidth(t->getRootNode()->getFloatingPointWidth()), 
	  mfScale(t->getTerrainHeight())
    {
        mHeightmap = t->getHeightmap();

        Ogre::uchar* pImgData = new Ogre::uchar[mImageWidth * mImageWidth * 4];

        int iMaskPos;
        for(int i = 0; i < mImageWidth * 4; i += 4)
        {
            for(int j = 0; j < mImageWidth * 4; j += 4)
            {
                iMaskPos = mImageWidth * i + j;
                pImgData[ iMaskPos ] = 0; //B
                pImgData[ iMaskPos + 1] = 0; //G
                pImgData[ iMaskPos + 2] = 0; //R
                pImgData[ iMaskPos + 3] = 0; //A
            }
        }
        imgMaskAlpha.loadDynamicImage(pImgData, mImageWidth, mImageWidth, 1, Ogre::PF_A8R8G8B8 );

        //CARREGANDO IMAGEM PARA TEXTURA
        texMaskAlpha = TextureManager::getSingleton().loadImage("__maskAlpha","General", imgMaskAlpha,Ogre::TEX_TYPE_2D);

        if(mTerrain->getMaterial()->getTechnique(0)->getPass("__maskAlpha") == NULL)
        {
            Ogre::Pass* pass = mTerrain->getMaterial()->getTechnique(0)->createPass();
            pass->setName("__maskAlpha");
            TextureUnitState * tus = pass->createTextureUnitState("__maskAlpha");
            pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
            tus->setTextureAnisotropy(16);
            tus->setTextureFiltering(Ogre::FT_MIP, Ogre::FO_ANISOTROPIC);
        }


        mNode = mTerrain->getSceneManager()->getRootSceneNode()->createChildSceneNode();
        mManualObject = mTerrain->getSceneManager()->createManualObject("__PolygonDisplacementShape");
        mManualObject->setRenderQueueGroup(41);

        Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().create("__PlaneHeightMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        mat->setReceiveShadows(false);
        mat->getTechnique(0)->getPass(0)->setAmbient(0,0,0);
        mat->getTechnique(0)->getPass(0)->setDiffuse(1,0,0,1);
        mat->getTechnique(0)->getPass(0)->setSpecular(0,0,0,1);
        mat->getTechnique(0)->getPass(0)->setSelfIllumination(0,0,0);
        mat->getTechnique(0)->getPass(0)->setDepthFunction(Ogre::CMPF_ALWAYS_PASS);

        planeEntity = mTerrain->getSceneManager()->createEntity("__PlaneHeightEntity","alfinete_red.mesh");
        //planeEntity->setMaterialName("__PlaneHeightMaterial");

        planeNode = mTerrain->getSceneManager()->getRootSceneNode()->createChildSceneNode();
		planeNode->attachObject(planeEntity);
        planeNode->setVisible(false);
        

        mat = Ogre::MaterialManager::getSingleton().create("__PolygonDisplacementMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        mat->setReceiveShadows(false);
        mat->getTechnique(0)->getPass(0)->setAmbient(0,0,0);
        mat->getTechnique(0)->getPass(0)->setDiffuse(0,0,1,1);
        mat->getTechnique(0)->getPass(0)->setSpecular(0,0,0,1);
        mat->getTechnique(0)->getPass(0)->setSelfIllumination(0,0,0);
        mat->getTechnique(0)->getPass(0)->setDepthFunction(Ogre::CMPF_ALWAYS_PASS);
        mNode->attachObject(mManualObject);
    }

///////////////////////////////////////////////////////////
    void PolygonDisplacement::insert(const Ogre::Vector3& v)
    {
        if( v.x == 599994 && v.y == 599994 && v.z == 599994 )
            return;

        mPoints.push_back(Ogre::Vector2(v.x, v.z));
        mHeight.push_back(v.y);

        int idx = mPoints.size()-1;
        if(idx < 50)
        {
            if(!mTerrain->getSceneManager()->hasEntity(std::string("__vertices" + Ogre::StringConverter::toString(idx)).c_str()))
            {
                verticesEntities[idx] = mTerrain->getSceneManager()->createEntity(std::string("__vertices" + Ogre::StringConverter::toString(idx)), "alfinete_blue.mesh");
                //verticesEntities[idx]->setMaterialName("__PolygonDisplacementMaterial");
                verticesNodes[idx] = mTerrain->getSceneManager()->getRootSceneNode()->createChildSceneNode();
                verticesNodes[idx]->attachObject(verticesEntities[idx]);
            }
            else
                verticesNodes[idx]->setVisible(true);

            verticesNodes[idx]->setPosition(v.x, v.y + 2, v.z);
        }
        if(mPoints.size() > 1)
        {
            drawManualObject();
        }
    }

//////////////////////////////////////////////////////////
    void PolygonDisplacement::drawManualObject()
    {
        mManualObject->clear();
        mManualObject->begin("__PolygonDisplacementMaterial", Ogre::RenderOperation::OT_LINE_STRIP);

        int size = mPoints.size();

        double x12, x25, x37, x50, x62, x75, x87;
        double y12, y25, y37, y50, y62, y75, y87;

        mManualObject->colour(1,0,0);
        for(int i = 0; i < size; i++)
        {
            mManualObject->position(mPoints.at(i).x, mHeight.at(i) + 2, mPoints.at(i).y);

            x50 = (mPoints.at(i).x + mPoints.at( (i == size - 1)? 0 : i + 1).x)/2;
            y50 = (mPoints.at(i).y + mPoints.at( (i == size - 1)? 0 : i + 1).y)/2;

            x25 = (mPoints.at(i).x + x50)/2;
            y25 = (mPoints.at(i).y + y50)/2;

            x12 = (mPoints.at(i).x + x25)/2;
            y12 = (mPoints.at(i).y + y25)/2;

            x37 = (x25 + x50)/2;
            y37 = (y25 + y50)/2;

            x75 = (mPoints.at((i == size - 1)? 0 : i + 1).x + x50)/2;
            y75 = (mPoints.at((i == size - 1)? 0 : i + 1).y + y50)/2;

            x87 = (mPoints.at((i == size - 1)? 0 : i + 1).x + x75)/2;
            y87 = (mPoints.at((i == size - 1)? 0 : i + 1).y + y75)/2;

            x62 = (x50 + x75)/2;
            y62 = (y50 + y75)/2;

            //PONTO a 12%
            mManualObject->position(x12, mTerrain->getHeightAt(x12, y12, 2, true), y12);

            //PONTO a 25%
            mManualObject->position(x25, mTerrain->getHeightAt(x25, y25, 2, true), y25);

            //PONTO a 37%
            mManualObject->position(x37, mTerrain->getHeightAt(x37, y37, 2, true), y37);

            //PONTO a 50%
            mManualObject->position(x50, mTerrain->getHeightAt(x50, y50, 2, true), y50);

            //PONTO a 62%
            mManualObject->position(x62, mTerrain->getHeightAt(x62, y62, 2, true), y62);

            //PONTO a 75%
            mManualObject->position(x75, mTerrain->getHeightAt(x75, y75, 2, true), y75);

            //PONTO a 87%
            mManualObject->position(x87, mTerrain->getHeightAt(x87, y87, 2, true), y87);
        }
        mManualObject->position(mPoints.at(0).x, mHeight.at(0) + 2, mPoints.at(0).y);
        mManualObject->end();
    }

///////////////////////////////////////////////////////////
    void PolygonDisplacement::clear()
    {
        clearVertices();
        mPoints.clear();
        mHeight.clear();
        mManualObject->clear();
    }

//////////////////////////////////////////////////////////
    void PolygonDisplacement::calculateMinMax()
    {
        mMin = mPoints.at(0);
        mMax = mPoints.at(0);

        PointList::iterator itr = mPoints.begin();
        for (itr; itr != mPoints.end() ; ++itr)
        {
            mMin.x = mMin.x > (*itr).x ? (*itr).x : mMin.x;
            mMax.x = mMax.x < (*itr).x ? (*itr).x : mMax.x;

            mMin.y = mMin.y > (*itr).y ? (*itr).y : mMin.y;
            mMax.y = mMax.y < (*itr).y ? (*itr).y : mMax.y;
        }
    }

////////////////////////////////////////////////////////////
    void PolygonDisplacement::modify(bool show  )
    {
        if(mPoints.size() == 0)
            return;

        const int iMapWidth = (int)mHeightmap->getWidth(); // 8193

		const float fMapWidth = (float)(iMapWidth - 1); // 8193 - 1 = 8192

		const float fMapScale = fMapWidth / mfWidth; // 8192/32000 = 0.256

		const float fInvMapScale = 1.0f / fMapScale; // 1.0f/  0.256 = 3.90625

		const float fHalfWidth = mfWidth * 0.5f; // 16000

		const float fMax = MAX_INT * mfScale; //65535.0 * 2248.0 = 147.322.680

		const float fInvScale = (1.0f / MAX_INT) * mfScale; // = 2248.0

        calculateMinMax();

		Ogre::Vector2 vMin = mMin;
		Ogre::Vector2 vMax = mMax;

		// [-half,+half] -> [0,width]
		vMin += fHalfWidth;
		vMax += fHalfWidth;

		// [0,width] -> [0,mapPixels]
		vMin *= fMapScale;
		vMax *= fMapScale;

		// Grab the pixel range inside the brush rect
		Vec2D<int> iMin(Math::ICeil(vMin.x),Math::ICeil(vMin.y));
		Vec2D<int> iMax(Math::IFloor(vMax.x),Math::IFloor(vMax.y));

		// Make sure it doesn't go out of bounds
		iMin.x = std::min(std::max(iMin.x,0),iMapWidth-1);
		iMin.y = std::min(std::max(iMin.y,0),iMapWidth-1);
		iMax.x = std::min(std::max(iMax.x,0),iMapWidth-1);
		iMax.y = std::min(std::max(iMax.y,0),iMapWidth-1);

        // Get the world position of pixels
		vMin = Ogre::Vector2(iMin.x,iMin.y) * fInvMapScale;
		vMax = Ogre::Vector2(iMax.x,iMax.y) * fInvMapScale;

		vMin -= fHalfWidth;
		vMax -= fHalfWidth;

		// Terrain position delta
		//const float fDelta = mfWidth / float(iMapWidth-1);
		//const float fDelta = mfWidth / fMapWidth;
		const float fDelta = fInvMapScale;  //excluir variavel e usar fInvMapScale

		Ogre::Vector2 vTerrainPos = vMin;
		float fTerrainStartX = vMin.x;

        // Get pointer to the heightmap data
		HEIGHTMAPTYPE* pData = mHeightmap->getData(); 
		float height = 0.0f;

		mUndo.clear();
		mRedo.clear();

        Ogre::uchar* pImgData = imgMaskAlpha.getData();

        //Loop
		for (int y=iMin.y;y<=iMax.y;y++)
		{
			vTerrainPos.x = fTerrainStartX;
			for (int x=iMin.x;x<iMax.x;x++)
			{
				if ( inside(Ogre::Vector2(vTerrainPos.x,vTerrainPos.y)))
				{

					height = getHeight(Ogre::Vector2(vTerrainPos.x,vTerrainPos.y));

                    //mUndo.push_back(Ogre::Vector3(x, UNPACK_HEIGHT(pData[x + y * iMapWidth]), y));
					mUndo.push_back(TerrainModification(x, y, pData[x + y * iMapWidth]));

					pData[x + y * iMapWidth] = PACK_HEIGHT(std::min(std::max(height / fInvScale,0.0f),MAX_INT));

                    if( show )
                    {
                        int _x = ((float)x / iMapWidth) * mImageWidth;
                        int _y = ((float)y / iMapWidth) * mImageWidth;
                        int iMaskPos = mImageWidth * _y * 4 + (_x * 4 - 1);
                        pImgData[ iMaskPos ] = 200; //A
                        pImgData[ iMaskPos - 1] = 50; //R
                        pImgData[ iMaskPos - 2] = 50; //G
                        pImgData[ iMaskPos - 3] = 200; //B
                    }
				}
				vTerrainPos.x += fDelta;
			}
            vTerrainPos.y += fDelta;
		}

        clearVertices();
        mPoints.clear();
        mHeight.clear();
        mManualObject->clear();
        
		if( show)
        {
			imgMaskAlpha.loadDynamicImage(pImgData, mImageWidth, mImageWidth, 1, Ogre::PF_A8R8G8B8 );
            texMaskAlpha->unload();
            texMaskAlpha->loadImage(imgMaskAlpha);
        }

        mTerrain->getRootNode()->checkModifier(this);
    }

//////////////////////////////////////////////////////////////////////////////////////////////
    void PolygonDisplacement::undo(bool show)
    {
        const int iMapWidth = (int)mHeightmap->getWidth();
        HEIGHTMAPTYPE* pData = mHeightmap->getData();

        Ogre::uchar* pImgData = imgMaskAlpha.getData();

		ModifyList::iterator itr = mUndo.begin();
		for (itr; itr != mUndo.end() ; ++itr)
        {
            mRedo.push_back(TerrainModification((*itr).x, (*itr).y, pData[(*itr).x + (*itr).y * iMapWidth]));
            pData[(*itr).x + (*itr).y * iMapWidth] = (*itr).data;

            int _x = ((float)(*itr).x / iMapWidth) * mImageWidth;
            int _y = ((float)(*itr).y / iMapWidth) * mImageWidth;
            int iMaskPos = mImageWidth * _y * 4 + (_x * 4 - 1);

            pImgData[ iMaskPos ] = 0; //A
            pImgData[ iMaskPos - 1] = 0; //R
            pImgData[ iMaskPos - 2] = 0; //G
            pImgData[ iMaskPos - 3] = 0; //B

        }

        /*MemList::iterator itr = mUndo.begin();
        for (itr; itr != mUndo.end() ; ++itr)
        {
            mRedo.push_back(Ogre::Vector3((*itr).x, UNPACK_HEIGHT(pData[int((*itr).x + (*itr).z * iMapWidth)]), (*itr).z));
            pData[int((*itr).x + (*itr).z * iMapWidth)] = PACK_HEIGHT((*itr).y);

            int _x = ((*itr).x / iMapWidth) * mImageWidth;
            int _y = ((*itr).z / iMapWidth) * mImageWidth;
            int iMaskPos = mImageWidth * _y * 4 + (_x * 4 - 1);
            pImgData[ iMaskPos ] = 0; //A
            pImgData[ iMaskPos - 1] = 0; //R
            pImgData[ iMaskPos - 2] = 0; //G
            pImgData[ iMaskPos - 3] = 0; //B

        }*//*

        if( show )
        {
            imgMaskAlpha.loadDynamicImage(pImgData, mImageWidth, mImageWidth, 1, Ogre::PF_A8R8G8B8 );
            texMaskAlpha->unload();
            texMaskAlpha->loadImage(imgMaskAlpha);
        }

        mUndo.clear();

		mTerrain->getRootNode()->checkModifier(this);
    }

//////////////////////////////////////////////////////////////////////////////////////////////
    void PolygonDisplacement::redo(bool show)
    {
        const int iMapWidth = (int)mHeightmap->getWidth();
        HEIGHTMAPTYPE* pData = mHeightmap->getData();

        Ogre::uchar* pImgData = imgMaskAlpha.getData();

		ModifyList::iterator itr = mRedo.begin();
        for (itr; itr != mRedo.end() ; ++itr)
        {
            mUndo.push_back(TerrainModification((*itr).x, (*itr).y, pData[(*itr).x + (*itr).y * iMapWidth]));
            pData[int((*itr).x + (*itr).y * iMapWidth)] = (*itr).data;

            int _x = ((float)(*itr).x / iMapWidth) * mImageWidth;
            int _y = ((float)(*itr).y / iMapWidth) * mImageWidth;
            int iMaskPos = mImageWidth * _y * 4 + (_x * 4 - 1);
            pImgData[ iMaskPos ] = 200; //A
            pImgData[ iMaskPos - 1] = 50; //R
            pImgData[ iMaskPos - 2] = 50; //G
            pImgData[ iMaskPos - 3] = 200; //B
        }

        /*MemList::iterator itr = mRedo.begin();
        for (itr; itr != mRedo.end() ; ++itr)
        {
            mUndo.push_back(Ogre::Vector3((*itr).x, UNPACK_HEIGHT(pData[int((*itr).x + (*itr).z * iMapWidth)]), (*itr).z));
            pData[int((*itr).x + (*itr).z * iMapWidth)] = PACK_HEIGHT((*itr).y);

            int _x = ((*itr).x / iMapWidth) * mImageWidth;
            int _y = ((*itr).z / iMapWidth) * mImageWidth;
            int iMaskPos = mImageWidth * _y * 4 + (_x * 4 - 1);
            pImgData[ iMaskPos ] = 200; //A
            pImgData[ iMaskPos - 1] = 50; //R
            pImgData[ iMaskPos - 2] = 50; //G
            pImgData[ iMaskPos - 3] = 200; //B
        }*//*

        if( show )
        {
            imgMaskAlpha.loadDynamicImage(pImgData, mImageWidth, mImageWidth, 1, Ogre::PF_A8R8G8B8 );
            texMaskAlpha->unload();
            texMaskAlpha->loadImage(imgMaskAlpha);
        }
        mRedo.clear();
		mTerrain->getRootNode()->checkModifier(this);
    }

//////////////////////////////////////////////////////////////////////////////////////////////
    void PolygonDisplacement::planefy(double verticalHeight, double diffXReal, double k3dToReal, bool show, bool softy)
    {
        if(mPoints.size() == 0)
            return;

        if(mPlane == -1)
            return;

        int iMapWidth = (int)mHeightmap->getWidth(); // 8193

		const float fMapWidth = (float)(iMapWidth - 1); // 8193 - 1 = 8192

		const float fMapScale = fMapWidth / mfWidth; // 8192/32000 = 0.256

		const float fInvMapScale = 1.0f / fMapScale; // 1.0f/  0.256 = 3.90625

		const float fHalfWidth = mfWidth * 0.5; // 16000

		const float fMax = MAX_INT * mfScale; //65535.0 * 2248.0 = 147.322.680

		const float fInvScale = (1.0f / MAX_INT) * mfScale; // = 2248.0

        calculateMinMax();

		Ogre::Vector2 vMin = mMin;
		Ogre::Vector2 vMax = mMax;

		// [-half,+half] -> [0,width]
		vMin += fHalfWidth;
		vMax += fHalfWidth;

		// [0,width] -> [0,mapPixels]
		vMin *= fMapScale;
		vMax *= fMapScale;

		// Grab the pixel range inside the brush rect
		Vec2D<int> iMin(Math::ICeil(vMin.x),Math::ICeil(vMin.y));
		Vec2D<int> iMax(Math::IFloor(vMax.x),Math::IFloor(vMax.y));

		// Make sure it doesn't go out of bounds
		iMin.x = std::min(std::max(iMin.x,0),iMapWidth-1);
		iMin.y = std::min(std::max(iMin.y,0),iMapWidth-1);
		iMax.x = std::min(std::max(iMax.x,0),iMapWidth-1);
		iMax.y = std::min(std::max(iMax.y,0),iMapWidth-1);

        // Get the world position of pixels
		vMin = Ogre::Vector2(iMin.x,iMin.y) * fInvMapScale;
		vMax = Ogre::Vector2(iMax.x,iMax.y) * fInvMapScale;

		vMin -= (mfWidth * 0.5);
		vMax -= (mfWidth * 0.5);

		// Terrain position delta
		const float fDelta = mfWidth / float(iMapWidth-1);

		Ogre::Vector2 vTerrainPos = vMin;
		float fTerrainStartX = vMin.x;

        // Get pointer to the heightmap data
		HEIGHTMAPTYPE* pData = mHeightmap->getData();
		float height = 0.0f;

        mUndo.clear();
		mRedo.clear();

        Ogre::uchar* pImgData = imgMaskAlpha.getData();

		totalVolume = 0.0f;
        
		// Loop
		for (int y=iMin.y;y<=iMax.y;y++)
		{
			vTerrainPos.x = fTerrainStartX;
		
			for (int x=iMin.x;x<iMax.x;x++)
			{
				if ( inside(Ogre::Vector2(vTerrainPos.x,vTerrainPos.y)))
				{
					mUndo.push_back(TerrainModification(x, y, pData[x + y * iMapWidth]));

                    float h = UNPACK_HEIGHT(pData[x + y * iMapWidth]) * (1.0f / MAX_INT) * mTerrain->getTerrainHeight();

                    if(softy)
                        height = getDistance(Ogre::Vector3(vTerrainPos.x, h, vTerrainPos.y), show);
                    else
                        height = planeNode->getPosition().y;

                    double newHeight = (UNPACK_HEIGHT(pData[x + y * iMapWidth]) * (1.0f / MAX_INT) * verticalHeight) * k3dToReal;

                    pData[x + y * iMapWidth] = PACK_HEIGHT(std::min(std::max(height / fInvScale,0.0f),MAX_INT));

                    double oldHeight = (UNPACK_HEIGHT(pData[x + y * iMapWidth]) * (1.0f / MAX_INT) * verticalHeight) * k3dToReal;

                    totalVolume += newHeight - oldHeight;
                    

                    if( show )
                    {
                        int _x = ((float)x / iMapWidth) * mImageWidth;
                        int _y = ((float)y / iMapWidth) * mImageWidth;
                        int iMaskPos = mImageWidth * _y * 4 + (_x * 4 - 1);
                        pImgData[ iMaskPos ] = 200; //A
                        pImgData[ iMaskPos - 1] = 50; //R
                        pImgData[ iMaskPos - 2] = 50; //G
                        pImgData[ iMaskPos - 3] = 200; //B
                    }

				}
				vTerrainPos.x += fDelta;
			}
            vTerrainPos.y += fDelta;
		}
        //Calculando volume

        double m3 = diffXReal/(float)iMapWidth;
        totalVolume = totalVolume * m3 * m3;

        clearVertices();
        mPoints.clear();
        mHeight.clear();
        mManualObject->clear();
		
		if( show )
        {
            imgMaskAlpha.loadDynamicImage(pImgData, mImageWidth, mImageWidth, 1, Ogre::PF_A8R8G8B8 );

            texMaskAlpha->unload();
            texMaskAlpha->loadImage(imgMaskAlpha);
        }
        mTerrain->getRootNode()->checkModifier(this);
    }
	////////////////////////////////////////////////////////////////
    float PolygonDisplacement::getDistance(const Ogre::Vector3 &current, bool &s)
    {
        //s = false;
        Ogre::Vector2 point(current.x, current.z);
        float height = planeNode->getPosition().y;

        Edge e;
        float total = MAX_INT;
        float h = 0.0f;
        int times = 0;

        int size = mPoints.size();
        for(int i = 0; i < size ; i++)
        {
            float d1, d2;
            if(i != size - 1)
            {
                e = Edge(mPoints.at(i), mPoints.at(i + 1));
                d1 = point.distance(mPoints.at(i));
                d2 = point.distance(mPoints.at(i+1));
            }
            else
            {
                e = Edge(mPoints.at(i), mPoints.at(0));
                d1 = point.distance(mPoints.at(i));
                d2 = point.distance(mPoints.at(0));
            }
            bool inside;
            float dist = e.distanceToPoint(point, inside);
            float delta = dist/10.0f;
            delta = std::min(delta, 1.0f);

            if(delta < total)
            {
                total = delta;
                //h = Lerp(current.y, height, delta);
                //h = height;
                //if(inside)
                //{
                    h = Lerp(current.y, height, delta);
                    //h = height;
                    //s = true;
                //}
            }
        }
        return h;

    }
////////////////////////////////////////////////////////////////
    void PolygonDisplacement::clearVertices()
    {
        int idx = mPoints.size() <= 50 ? mPoints.size() : 50;
        for(int i = 0; i < idx; i++)
        {
            verticesEntities[i]->setVisible(false);
        }
        planeNode->setVisible(false);
        mPlane = -1;
    }


////////////////////////////////////////////////////////////////
    void PolygonDisplacement::setHeight(const Ogre::Vector3& v)
    {
        planeNode->setPosition(v.x, v.y, v.z);
        planeNode->setVisible(true);
        mPlane = v.y;
    }
////////////////////////////////////////////////////////////////
    Ogre::SceneNode* PolygonDisplacement::getPlaneNode()
    {
        return planeNode;
    }
////////////////////////////////////////////////////////////////
    bool PolygonDisplacement::inside(Ogre::Vector2 p)
    {
        int times = 0;
        int size = mPoints.size();
        for(int i = 0; i < size ; i++)
        {
            Edge e;
            if(i != size - 1)
                e = Edge(mPoints.at(i), mPoints.at(i + 1));
            else
                e = Edge(mPoints.at(i), mPoints.at(0));

            if(e.interceptByRightSide( p))
                times++;

        }
        return times % 2 == 1;
    }

//////////////////////////////////////////////////////////////
    bool PolygonDisplacement::interceptEdgeByRightSide(Ogre::Vector2 start, Ogre::Vector2 end, Ogre::Vector2 point)
    {
        float yMin = std::min(start.y, end.y);
        float yMax = std::max(start.y, end.y);

        float y = point.y;
        float x = start.x + (y - start.y)*(end.x - start.x)/(end.y - start.y);

        bool isLeft = point.x < x;
        bool interceptSegment = y >= yMin && y <= yMax;

        return isLeft && interceptSegment;
    }

////////////////////////////////////////////////////////////////
    float PolygonDisplacement::getHeight(Ogre::Vector2 p)
    {
        float size = mPoints.size();
        float height = 0.0f;
        float distanceTotal = 0.0f;
        float distance;

        float x25, x50, x75;
        float y25, y50, y75;

        for(int i = 0; i < size ; i++)
        {
            //Heurística para suavizaçao dos calombos///////////////////
            x50 = (mPoints.at(i).x + mPoints.at((i == size - 1)? 0 : i + 1).x)/2;
            y50 = (mPoints.at(i).y + mPoints.at((i == size - 1)? 0 : i + 1).y)/2;

            x75 = (x50 + mPoints.at((i == size - 1)? 0 : i + 1).x)/2;
            y75 = (y50 + mPoints.at((i == size - 1)? 0 : i + 1).y)/2;

            x25 = (mPoints.at(i).x + x50)/2;
            y25 = (mPoints.at(i).y + y50)/2;

            distance =  1 / Math::Pow(Math::Sqrt( Math::Pow((p.x - x50),2) + Math::Pow((p.y - y50),2)), 2) ;
            height += mTerrain->getHeightAt(x50, y50, 0, true) * distance;
            distanceTotal += distance;

            distance =  1 / Math::Pow(Math::Sqrt( Math::Pow((p.x - x25),2) + Math::Pow((p.y - y25),2)), 2) ;
            height += mTerrain->getHeightAt(x25, y25, 0, true) * distance;
            distanceTotal += distance;

            distance =  1 / Math::Pow(Math::Sqrt( Math::Pow((p.x - x75),2) + Math::Pow((p.y - y75),2)), 2) ;
            height += mTerrain->getHeightAt(x75, y75, 0, true) * distance;
            distanceTotal += distance;
            ///////////////////////////////////////////////////////////////

            distance =  1 / Math::Pow(Math::Sqrt( Math::Pow((p.x - mPoints.at(i).x),2) + Math::Pow((p.y - mPoints.at(i).y),2)), 2) ;
            height += mHeight.at(i) * distance;
            distanceTotal += distance;
        }

        return height / distanceTotal;
    }

float PolygonDisplacement::getVolume()
    {
        return Ogre::Math::Abs(totalVolume);
    }*/