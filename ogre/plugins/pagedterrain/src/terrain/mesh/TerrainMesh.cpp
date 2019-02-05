#include "TerrainMesh.h"
#include "Terrain.h"
#include "quadtree/QNode.h"
#include "heightmap/Heightmap.h"
#include "utilities/vectors/Vec2D.h"
//#include "QuadTree.h"

#include <OgreLogManager.h>

#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
#include <OgreHardwareBuffer.h>
#include <OgreHardwareBufferManager.h>
#include <OgreVertexIndexData.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreTimer.h>
#include <OgreManualObject.h>

#include <OgreViewport.h>
#include <OgreStringConverter.h>
#include <OgreGpuProgram.h>
#include <OgreGpuProgramManager.h>

using namespace Ogre;

	TerrainMesh::TerrainMesh( Terrain* pTerrain, QNode* pParentNode )
		: mBuilt(false),
		mRenderLevel(0),
		mVertexData(0),
		mIndexData(0),
		mLastNextLevel(-1),
		mDeltaBuffers(0),
		mMinLevelDistSqr(0),
		mCamDistance(0.0),
		mLODMorphFactor(0.0),
		mLastLODMorphFactor(0.0),
		mParentNode(0),
		mWidth(0.0),
		mVertexPositions(0),
		mCamera(0),
		mTerrain(pTerrain),
		mUVCoordinates(0),
		mSplitState(TSS_NONE),
		mTargetRenderLevel(0),
		mDebugNode(0),
		mDebugText(0)
	{
		mCamera = mTerrain->getCamera(); //OK

		mViewport = mTerrain->getViewport();

		mParentNode = mTerrain->getTerrainNode()->createChildSceneNode(); //OK

#if (USE_DEBUG_DISPLAYS == 1)
		mDebugNode = mParentNode->createChildSceneNode(); //OK
#endif
		setMaterial(mTerrain->getMaterial()); //OK

		mParentQNode = pParentNode;//OK

		createGeometryData();//OK

		mTerrain->_fireMeshCreated(this);//OK

	}


	TerrainMesh::~TerrainMesh()
	{
		destroyGeometryData();
		destroyGeometry();
		destroyMaterial();
		destroyRenderedLighting();
		mTerrain->_fireMeshDestroyed(this);
	}

	void TerrainMesh::setMaterial( const Ogre::String& MatName )
	{
		mMaterial = MaterialManager::getSingleton().getByName(MatName);
		mMaterial = mMaterial->clone(MatName + mParentNode->getName());
	}

	Ogre::Real TerrainMesh::_calculateCFactor()
	{
		Real A, T;
		// Turn off detail compression at higher FOVs
		A = 1.0f;

       int  vertRes = mViewport->getActualHeight();
       //int  vertRes = GRender->getRenderView("render1")->getViewport()->getActualHeight();

		T = 2 *  3.0 / ( double ) vertRes;

		return A / T;
	}

	void TerrainMesh::_calculateMinLevelDist2( Ogre::Real C )
	{
		//level 0 has no delta.
		mMinLevelDistSqr[ 0 ] = 0;

		int i, j;
		int shortRow = VERTEX_WIDTH - 2;
		int longRow = VERTEX_WIDTH + 2;

		//Real rMin = 50000000.0;
		//Real rMax = -rMin;

		for ( int level = 1; level < mMaxLOD; level++ )
		{
			mMinLevelDistSqr[ level ] = 0;

			int step = 1 << level;
			// The step of the next higher LOD
			//int higherstep = step >> 1;

			float* pDeltas = 0;

			if (USE_MORPH)
			{
				// Create a set of delta values (store at index - 1 since 0 has none)
				mDeltaBuffers[level - 1]  = createDeltaBuffer();
				// Lock, but don't discard (we want the pre-initialised zeros)
				pDeltas = static_cast<float*>(
					mDeltaBuffers[level - 1]->lock(HardwareBuffer::HBL_NORMAL));
			}


			for ( j = 0; j < VERTEX_WIDTH - step; j += step )
			{
				for ( i = 0; i < VERTEX_WIDTH - step; i += step )
				{


					Ogre::Vector3 v1 = getVertexPosition(i,j);//(_vertex( i, j, 0 ), _vertex( i, j, 1 ), _vertex( i, j, 2 ));
					Ogre::Vector3 v2 = getVertexPosition(i + step,j);//(_vertex( i + step, j, 0 ), _vertex( i + step, j, 1 ), _vertex( i + step, j, 2 ));
					Ogre::Vector3 v3 = getVertexPosition(i,j + step);//(_vertex( i, j + step, 0 ), _vertex( i, j + step, 1 ), _vertex( i, j + step, 2 ));
					Ogre::Vector3 v4 = getVertexPosition(i + step,j + step);//(_vertex( i + step, j + step, 0 ), _vertex( i + step, j + step, 1 ), _vertex( i + step, j + step, 2 ));

					//v1.y -= 50.0;
					//v2.y -= 50.0;
					//v3.y -= 50.0;
					//v4.y -= 50.0;

					Plane t1, t2;

					//t1.redefine(v1, v3, v2);
					//t2.redefine(v2, v3, v4);
					t1.redefine(v1, v3, v4);
					t2.redefine(v1, v4, v2);



					// include the bottommost row of vertices if this is the last row
					int zubound = (j == (VERTEX_WIDTH - step)? step : step - 1);
					for ( int z = 0; z <= zubound; z++ )
					{
						// include the rightmost col of vertices if this is the last col
						int xubound = (i == (VERTEX_WIDTH - step)? step : step - 1);
						for ( int x = 0; x <= xubound; x++ )
						{
							int fulldetailx = i + x;
							int fulldetailz = j + z;
							if ( fulldetailx % step == 0 &&
								fulldetailz % step == 0 )
							{
								// Skip, this one is a vertex at this level
								continue;
							}

							Real zpct = (Real)z / (Real)step;
							Real xpct = (Real)x / (Real)step;

							//interpolated height
							Ogre::Vector3 actualPos = getVertexPosition(fulldetailx,fulldetailz);
							//actualPos.y -= 50.0;

							Real interp_h;
							// Determine which tri we're on
							//if ((xpct + zpct <= 1.0f))
							if ((xpct + (1-zpct) <= 1.0f))
							{
								// Solve for x/z
								interp_h =
									(-(t1.normal.x * actualPos.x)
									- t1.normal.z * actualPos.z
									- t1.d) / t1.normal.y;
							}
							else
							{
								// Second tri
								interp_h =
									(-(t2.normal.x * actualPos.x)
									- t2.normal.z * actualPos.z
									- t2.d) / t2.normal.y;
							}

							Real actual_h = actualPos.y;// - 50.0;//_vertex( fulldetailx, fulldetailz, 1 );
							Real delta = fabs( interp_h - actual_h );

							Real D2 = delta * delta * C * C;

							if ( mMinLevelDistSqr[ level ] < D2 )
								mMinLevelDistSqr[ level ] = D2;

							// Should be save height difference?
							// Don't morph along edges
							if (USE_MORPH )//&& fulldetailx != 0  && fulldetailx != (VERTEX_WIDTH - 1) && fulldetailz != 0  && fulldetailz != (VERTEX_WIDTH - 1) )
							{
								// Save height difference
								Real diff = (interp_h - actual_h);
								//rMin = std::min(rMin, diff);
								//rMax = std::max(rMax, diff);
								pDeltas[fulldetailx + (fulldetailz * longRow) + shortRow + 1] = diff;
								//pDeltas[fulldetailx + (fulldetailz * VERTEX_WIDTH) + shortRow + 1 + (fulldetailz * 2)] = diff;
							}

						}

					}
				}
			}

			// Unlock morph deltas if required
			if (USE_MORPH)
			{
				mDeltaBuffers[level - 1]->unlock();
			}
		}

		//LogManager::getSingleton().logMessage("Min/Max: " + StringConverter::toString(rMin) + "/" + StringConverter::toString(rMax));



		// Post validate the whole set
		for ( i = 1; i < mMaxLOD; i++ )
		{


			//make sure the levels are increasing...
			if ( mMinLevelDistSqr[ i ] < mMinLevelDistSqr[ i - 1 ] )
			{
				mMinLevelDistSqr[ i ] = mMinLevelDistSqr[ i - 1 ];
			}
		}

		// Now reverse traverse the list setting the 'next level down'
		Real lastDist = -1;
		int lastIndex = 0;
		for (i = mMaxLOD-1; i >= 0; --i)
		{
			if (i == mMaxLOD-1)
			{
				// Last one is always 0
				lastIndex = i;
				lastDist = mMinLevelDistSqr[i];
				mNextLevelDown[i] = 0;
			}
			else
			{
				mNextLevelDown[i] = lastIndex;
				if (mMinLevelDistSqr[i] != lastDist)
				{
					lastIndex = i;
					lastDist = mMinLevelDistSqr[i];
				}
			}

		}


	}

	Ogre::HardwareVertexBufferSharedPtr TerrainMesh::createDeltaBuffer( void )
	{
		size_t totalVerts = (VERTEX_WIDTH * VERTEX_WIDTH) + (VERTEX_WIDTH * 2) + ((VERTEX_WIDTH-1) * 2);
		// Delta buffer is a 1D float buffer of height offsets

		HardwareVertexBufferSharedPtr buf =
			HardwareBufferManager::getSingleton().createVertexBuffer(
			VertexElement::getTypeSize(VET_FLOAT1),
			totalVerts,
			HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		// Fill the buffer with zeros, we will only fill in delta
		void* pVoid = buf->lock(HardwareBuffer::HBL_DISCARD);
		memset(pVoid, 0, totalVerts * sizeof(float));
		buf->unlock();

		return buf;
	}

	void TerrainMesh::build()
	{
		if (mBuilt) //OK
			return;

        updateGeometryData(); //OK

		updateVertices();  // OK

		setCastShadows(false); //OK

		mParentNode->attachObject(this); // OK

		setRenderQueueGroup(40); //OK

		mBuilt = true; //OK

		if (mTerrain->getDiscardGeometryData()) //OK
		{
            destroyGeometryData();              //OK
		}

	}

	void TerrainMesh::getRenderOperation( Ogre::RenderOperation& op )
	{
		op.useIndexes = true;
		op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
		op.vertexData = mVertexData;
		op.indexData = mIndexData;
	}

	Ogre::IndexData* TerrainMesh::getIndexData( size_t iLOD /*= 0*/ )
	{
		if (iLOD > (size_t)mMaxLOD)
			iLOD = (size_t)mMaxLOD;

		IndexData* indexData = mTerrain->getIndexData(iLOD);
		if (indexData)
			return indexData;

		int numIndexes = 0;
		int step = 1 << iLOD;

		int numFaces = (VERTEX_WIDTH-1) >> iLOD;


		int new_length = (((numFaces * numFaces) + (numFaces*4))) * 3 * 2;

		indexData = new IndexData;
		indexData->indexBuffer =
			HardwareBufferManager::getSingleton().createIndexBuffer(
			HardwareIndexBuffer::IT_16BIT,
			new_length, HardwareBuffer::HBU_STATIC_WRITE_ONLY);//, false);

		//mSceneManager->_getIndexCache().mCache.push_back( indexData );
		//mMainIndexMap->addToCache(indexData);
		//mSettings->_indexStorage->addToCache(indexData);

		unsigned short* pIdx = static_cast<unsigned short*>(
			indexData->indexBuffer->lock(0,
			indexData->indexBuffer->getSizeInBytes(),
			HardwareBuffer::HBL_DISCARD));

		int pos0,pos1,pos2,pos3;
		int shortRow = VERTEX_WIDTH-2;
		int longRow = VERTEX_WIDTH+2;

		if (numFaces > 1)
		{
			// Top-left corner
			pos0 = (VERTEX_WIDTH-2);
			pos1 = step-1;
			pos2 = pos0 + 1;
			pos3 = pos2 + step;

			*pIdx++ = pos3;
			*pIdx++ = pos0;
			*pIdx++ = pos2;

			*pIdx++ = pos3;
			*pIdx++ = pos1;
			*pIdx++ = pos0;

			numIndexes += 6;

			if (numFaces > 2)
			{
				// Top skirt
				for (int x=0;x<(VERTEX_WIDTH-3 - ((step-1) * 2));x += step)
				{
					pos0 = x+step-1;
					pos1 = pos0 + step;
					pos2 = pos0 + VERTEX_WIDTH;
					pos3 = pos2 + step;

					*pIdx++ = pos3;
					*pIdx++ = pos0;
					*pIdx++ = pos2;

					*pIdx++ = pos3;
					*pIdx++ = pos1;
					*pIdx++ = pos0;

					numIndexes += 6;
				}

			}




			// Top-right corner
			pos0 = shortRow - step;
			pos1 = longRow + shortRow - 1;
			pos2 = pos1 - 1 - step;
			pos3 = pos1 - 1;

			*pIdx++ = pos3;
			*pIdx++ = pos0;
			*pIdx++ = pos2;

			*pIdx++ = pos3;
			*pIdx++ = pos1;
			*pIdx++ = pos0;

			numIndexes += 6;

		}
		else
		{
			// Lowest detail has one set of faces on top
			pos0 = shortRow;
			pos1 = longRow + shortRow - 1;
			pos2 = pos0 + 1;
			pos3 = pos1 - 1;

			*pIdx++ = pos3;
			*pIdx++ = pos0;
			*pIdx++ = pos2;

			*pIdx++ = pos3;
			*pIdx++ = pos1;
			*pIdx++ = pos0;

			numIndexes += 6;
		}

		// Left skirts
		for (int y=0;y<VERTEX_WIDTH-1;y += step)
		{
			pos0 = shortRow + y * (VERTEX_WIDTH+2);
			pos1 = pos0 + 1;
			pos2 = pos0 + (VERTEX_WIDTH+2) * step;
			pos3 = pos2 + 1;

			*pIdx++ = pos3;
			*pIdx++ = pos0;
			*pIdx++ = pos2;

			*pIdx++ = pos3;
			*pIdx++ = pos1;
			*pIdx++ = pos0;

			numIndexes += 6;
		}

		// Core
		for (int y=0;y<VERTEX_WIDTH-1;y += step)
		{
			for (int x=0;x<VERTEX_WIDTH-1;x += step)
			{
				pos0 = shortRow + x + y * (VERTEX_WIDTH+2) + 1;
				pos1 = pos0 + step;
				pos2 = pos0 + (VERTEX_WIDTH+2) * step;
				pos3 = pos2 + step;

				*pIdx++ = pos3;
				*pIdx++ = pos0;
				*pIdx++ = pos2;

				*pIdx++ = pos3;
				*pIdx++ = pos1;
				*pIdx++ = pos0;

				numIndexes += 6;

			}
		}

		// Right skirts
		for (int y=0;y<VERTEX_WIDTH-1;y += step)
		{
			pos0 = shortRow + (y+1) * (VERTEX_WIDTH+2) - 2;
			pos1 = pos0 + 1;
			pos2 = pos0 + (VERTEX_WIDTH+2) * step;
			pos3 = pos2 + 1;

			*pIdx++ = pos3;
			*pIdx++ = pos0;
			*pIdx++ = pos2;

			*pIdx++ = pos3;
			*pIdx++ = pos1;
			*pIdx++ = pos0;

			numIndexes += 6;
		}

		int startLastRow = longRow * (VERTEX_WIDTH-1) + shortRow;
		int bottomRow = startLastRow + longRow;

		if (numFaces > 1)
		{
			// Bottom-left corner
			pos0 = startLastRow + 1;
			pos1 = pos0 + step;
			pos2 = startLastRow;
			pos3 = bottomRow + step-1;

			*pIdx++ = pos3;
			*pIdx++ = pos0;
			*pIdx++ = pos2;

			*pIdx++ = pos3;
			*pIdx++ = pos1;
			*pIdx++ = pos0;

			numIndexes += 6;

			if (numFaces > 2)
			{

				// Bottom skirt
				for (int x=0;x<(VERTEX_WIDTH-3 - ((step-1) * 2));x += step)
				{
					//pos0 = x + 2 + startLastRow;
					pos0 = startLastRow + 1 + step + x;
					pos1 = pos0 + step;
					pos2 = bottomRow + x + step - 1;
					pos3 = pos2 + step;

					*pIdx++ = pos3;
					*pIdx++ = pos0;
					*pIdx++ = pos2;

					*pIdx++ = pos3;
					*pIdx++ = pos1;
					*pIdx++ = pos0;

					numIndexes += 6;
				}
			}


			// Bottom-right corner
			pos0 = bottomRow - 2 - step;
			pos1 = bottomRow - 2;
			pos2 = bottomRow + shortRow - step;
			pos3 = bottomRow - 1;

			*pIdx++ = pos3;
			*pIdx++ = pos0;
			*pIdx++ = pos2;

			*pIdx++ = pos3;
			*pIdx++ = pos1;
			*pIdx++ = pos0;

			numIndexes += 6;
		}
		else
		{
			// Lowest detail has one set of faces on bottom
			pos0 = startLastRow + 1;
			pos1 = bottomRow - 2;
			pos2 = startLastRow;
			pos3 = pos1 + 1;

			*pIdx++ = pos3;
			*pIdx++ = pos0;
			*pIdx++ = pos2;

			*pIdx++ = pos3;
			*pIdx++ = pos1;
			*pIdx++ = pos0;

			numIndexes += 6;
		}



		indexData->indexBuffer->unlock();
		indexData->indexCount = numIndexes;
		indexData->indexStart = 0;

		mTerrain->addIndexData(indexData,iLOD);

		return indexData;

	}

	void TerrainMesh::setRenderLevel( size_t iLOD /*= 0*/ )
	{
		if (iLOD > (size_t)mMaxLOD)
			iLOD = (size_t)mMaxLOD;

		mIndexData = getIndexData(iLOD);

#if (USE_MORPH)
		if (mRenderLevel < mMaxLOD - 1)
			mVertexData->vertexBufferBinding->setBinding(1, mDeltaBuffers[mRenderLevel]);
		else
			mVertexData->vertexBufferBinding->setBinding(1, mDeltaBuffers[0]);
#endif

	}

	const Ogre::Vector3& TerrainMesh::getVertexPosition( size_t x, size_t z )
	{
		if (mVertexPositions == 0)
			return Ogre::Vector3::ZERO;

		return mVertexPositions[x+1 + MAP_WIDTH * (z+1)];
	}

	void TerrainMesh::_notifyCurrentCamera( Ogre::Camera* cam )
	{
		MovableObject::_notifyCurrentCamera(cam);
	}

	void TerrainMesh::_updateCustomGpuParameter(
		const Ogre::GpuProgramParameters::AutoConstantEntry& constantEntry,
		Ogre::GpuProgramParameters* params ) const
	{
		if (constantEntry.data == MORPH_CONSTANT_ID)
		{
			// Update morph LOD factor
			params->_writeRawConstant(constantEntry.physicalIndex, mLODMorphFactor);
		}
		else if (constantEntry.data == PARENT_UV_OFFSET_ID)
		{
			params->_writeRawConstant(constantEntry.physicalIndex, mParentUVOffset);
		}
		else
		{
			Renderable::_updateCustomGpuParameter(constantEntry, params);
		}
	}

	void TerrainMesh::checkLOD()
	{
		mTargetRenderLevel = mMaxLOD - 1;


		for (int i=0;i<mMaxLOD;i++)
		{
			if (mMinLevelDistSqr[i] > mCamDistance)
			{
				mTargetRenderLevel = i-1;
				break;
			}
		}

		if (mTargetRenderLevel < 0)
			mTargetRenderLevel = mMaxLOD - 1;

	}

	void TerrainMesh::update(Ogre::Real rTime /*= 0.0f*/)
	{
		Ogre::Vector3 cpos = mCamera->getDerivedPosition();
		const AxisAlignedBox& aabb = getWorldBoundingBox(true);
		Ogre::Vector3 diff(0, 0, 0);
		diff.makeFloor(cpos - aabb.getMinimum());
		diff.makeCeil(cpos - aabb.getMaximum());

		mCamDistance = diff.squaredLength();
#if (USE_MORPH > 0)


		const Real rUpdateSpeed = rTime / mTerrain->getMorphSpeed();

		if (mRenderLevel > mTargetRenderLevel)
		{

			mLODMorphFactor -= rUpdateSpeed;
			if (mLODMorphFactor <= 0.0f)
			{
				mRenderLevel -= 1;
				mLODMorphFactor = 1.0f;

				setRenderLevel(mRenderLevel);




			}

		}
		else if (mTargetRenderLevel > mRenderLevel)
		{
			mLODMorphFactor += rUpdateSpeed;
			if (mLODMorphFactor >= 1.0f)
			{
				mRenderLevel += 1;
				if (mRenderLevel < mMaxLOD - 1)
					mLODMorphFactor = 0.0f;
				else
					mLODMorphFactor = 1.0f;

				setRenderLevel(mRenderLevel);


			}
		}




		if (mTargetRenderLevel == mRenderLevel && (mRenderLevel != mMaxLOD - 1))
		{


			Real range = mMinLevelDistSqr[mRenderLevel + 1] - mMinLevelDistSqr[mRenderLevel];

			mLastLODMorphFactor = mLODMorphFactor;

			Real percent = (mCamDistance - mMinLevelDistSqr[mRenderLevel]) / range;

			percent = std::min(std::max(percent,0.0),1.0);

			Real rescale = 1.0f / (1.0f - MORPH_START);
			mLODMorphFactor = std::max((percent - MORPH_START) * rescale,
				static_cast<Real>(0.0));


			Real morphDiff = (mLODMorphFactor - mLastLODMorphFactor) * mTerrain->getMorphSpeed();

			//static const Real cMaxSpeed = MORPH_SPEED * 0.01f;

			if (morphDiff > rUpdateSpeed)
				mLODMorphFactor = mLastLODMorphFactor + rUpdateSpeed;
			else if (morphDiff < -rUpdateSpeed)
				mLODMorphFactor = mLastLODMorphFactor - rUpdateSpeed;

			mLODMorphFactor = std::min(std::max(mLODMorphFactor,0.0),1.0);


		}


#else
		setRenderLevel(mTargetRenderLevel);
#endif


		mSplitState = TSS_NONE;
		if (mRenderLevel == mTargetRenderLevel)
		{
			if (mRenderLevel == 0 && (mLODMorphFactor - 0.01f) < 0.0f && mCamDistance < mSplitRadius)
			{
				mSplitState = TSS_SPLIT;
			}
			else if (mRenderLevel == (mMaxLOD - 1) && mCamDistance > mUnsplitRadius)
			{
				mSplitState = TSS_UNSPLIT;
			}
		}


#if (USE_DEBUG_DISPLAYS == 1)
		String strState;
		switch(mSplitState)
		{
		case TSS_SPLIT:
			strState = "SPLIT\nCam: " + StringConverter::toString(mCamDistance) + "\nSplitRad: " + StringConverter::toString(mSplitRadius);
			break;
		case TSS_UNSPLIT:
			strState = "UNSPLIT\nCam: " + StringConverter::toString(mCamDistance) + "\nUnSplitRad: " + StringConverter::toString(mUnsplitRadius);
			break;
		default:
			strState = "NONE";
			break;
		}

		mDebugText->setCaption("Depth: " + StringConverter::toString(mQuadTreeDepth) +
			"\nL: " + StringConverter::toString(mRenderLevel) +
			" TL: " + StringConverter::toString(mTargetRenderLevel) +
			"\nMorph: " + StringConverter::toString(mLODMorphFactor) +
			"\nState: " + strState );
#endif

	}

	void TerrainMesh::justSplit()
	{
		setRenderLevel(0);
		checkLOD();
		mLODMorphFactor = 1.0f;
	}

	void TerrainMesh::justUnSplit()
	{
		setRenderLevel(0);
		checkLOD();
		mLODMorphFactor = 0.0f;
	}

	void TerrainMesh::destroyGeometryData()
	{
		if (mUVCoordinates)
		{
			size_t iUVSets = (mTerrain->getUseChunkUVs() ? 2 : 1);
			for (size_t i=0;i<iUVSets;i++)
			{
				delete[] mUVCoordinates[i];
				mUVCoordinates[i] = 0;
			}
		}


		delete[] mUVCoordinates;
		mUVCoordinates = 0;

		delete[] mNormals;
		mNormals = 0;

		delete[] mVertexPositions;
		mVertexPositions = 0;
	}

	void TerrainMesh::destroyGeometry()
	{
		mParentNode->detachObject(this);

#if (USE_DEBUG_DISPLAYS == 1)
		mDebugNode->detachObject(mDebugText);
		mParentNode->removeAndDestroyChild(mDebugNode->getName());

		delete mDebugText;
		mDebugText = 0;
#endif
		mParentNode->getParentSceneNode()->removeAndDestroyChild(mParentNode->getName());



		if (mVertexData)
		{
			delete mVertexData;
			mVertexData = 0;
		}
		mMainBuffer.setNull();

		if (mDeltaBuffers)
			delete [] mDeltaBuffers;

		if ( mMinLevelDistSqr != 0 )
			delete [] mMinLevelDistSqr;

		mBuilt = false;
	}

	void TerrainMesh::destroyRenderedLighting()
	{
		if (mRenderedLighting.isNull() == false)
		{
			mRenderedLighting->unload();
			mRenderedLighting.setNull();
		}
	}

	void TerrainMesh::destroyMaterial()
	{
		if (mMaterial.isNull() == false)
		{
			mMaterial->unload();
			mMaterial.setNull();
		}
	}

	void TerrainMesh::createGeometryData()
	{
		mVertexPositions = new Ogre::Vector3[MAP_WIDTH * MAP_WIDTH];
		mNormals = new Ogre::Vector3[VERTEX_WIDTH * VERTEX_WIDTH];

		size_t iUVSets = (mTerrain->getUseChunkUVs() ? 2 : 1);
		mUVCoordinates = new Ogre::Vector2*[iUVSets];
		for (size_t i=0;i<iUVSets;i++)
		{
			mUVCoordinates[i] = new Ogre::Vector2[VERTEX_WIDTH * VERTEX_WIDTH];
		}




		mVertexData = new Ogre::VertexData();
		mVertexData->vertexStart = 0;
		mVertexData->vertexCount = (VERTEX_WIDTH * VERTEX_WIDTH) + (VERTEX_WIDTH * 2) + ((VERTEX_WIDTH-1) * 2);

		Ogre::VertexDeclaration* decl = mVertexData->vertexDeclaration;
		Ogre::VertexBufferBinding* bind = mVertexData->vertexBufferBinding;

		size_t currOffset = 0;
		// positions
		decl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION);
		currOffset += Ogre::VertexElement::getTypeSize(VET_FLOAT3);

		// normals


		decl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);



		//Tangent

		/*
		if (mTerrainSettings->bUseTangents && mTerrainSettings->bUseNormals)
		{
		decl->addElement(0, currOffset, VET_FLOAT3, VES_TANGENT);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		}*/



		// two dimensional texture coordinates
		for (size_t i=0;i<iUVSets;i++)
		{
			decl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES,i);
			currOffset += Ogre::VertexElement::getTypeSize(VET_FLOAT2);
		}


		mMainBuffer = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(decl->getVertexSize(0),mVertexData->vertexCount,HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		bind->setBinding(0,mMainBuffer);

		if (USE_MORPH)
		{
			// Create additional element for delta
			decl->addElement(1, 0, VET_FLOAT1, VES_BLEND_WEIGHTS);
			// NB binding is not set here, it is set when deriving the LOD
		}

		mRenderLevel = 0;

		mMaxLOD = MAX_LOD+1;//(int)(log((double)(VERTEX_WIDTH-1))/log(2.0) + 0.01);
		// Adjust max LOD to keep corners (i.e. 4x4)
		//mMaxLOD -= 2;

		mMinLevelDistSqr = new Real[ mMaxLOD ];


		if (USE_MORPH)
		{
			// Create delta buffer for all except the lowest mip
			mDeltaBuffers = new HardwareVertexBufferSharedPtr[mMaxLOD - 1];
		}
	}

	void TerrainMesh::updateVertices()
	{
		size_t iUVSets = (mTerrain->getUseChunkUVs() ? 2 : 1);//OK

		Ogre::Real min = 500000000.0f, max = -min;
		Ogre::Vector3 vMin(min,min,min);
		Ogre::Vector3 vMax(max,max,max);

		//Vec3D<double> vVec;
		Ogre::Vector3 vVec;
		Ogre::Vector2 vUV;

		Ogre::Vector3 vNormal;
		Ogre::Vector3 vTangent;

     	//double dHalfWidth = mWidth * 0.5;
		//const double dPosDelta = (static_cast<double>(mWidth) / static_cast<double>(mSettings->_vertWidth-1));

		//const Vec2D<double> vTopLeft(-dHalfWidth,-dHalfWidth);

		//vVec.x = -dHalfWidth;
		//vVec.z = -dHalfWidth;
		//vUV.x = 0.0;
		//vUV.y = 0.0;

		//const double dUVInv = 1.0 / ((double)(VERTEX_WIDTH-1));

		//double dUVDelta = dUVInv;
		//double dUVStartX = 0.0;

		//mVertexPositions = new Vector3[mSettings->_vertWidth * mSettings->_vertWidth];
		Ogre::Vector3* pVec = mVertexPositions;
		//Vector2* pUV = mUVCoordinates;
		//float* H = mMap;

		//Vector3* pNorm = mVertexNormals;

		//float fSkirtLength = mWidth * 0.5f;

		float* pVertex = static_cast<float*>(mMainBuffer->lock(HardwareBuffer::HBL_DISCARD));//OK

		int x,z;

		float fSkirtLength = mTerrain->getSkirtLength(); //OK

		// Top skirt
		//vUV.y = 0.0f;
		//vUV.x = dUVStartX + dUVDelta;


		for (int ix = 1;ix<VERTEX_WIDTH-1;ix++) //OK
		{
			vVec = pVec[ix + 1 + MAP_WIDTH];
			vVec.y -= fSkirtLength;

			// Vertex Position
			*pVertex++ = vVec.x;
			*pVertex++ = vVec.y;
			*pVertex++ = vVec.z;

			vNormal = mNormals[ix];

			// Normals
			*pVertex++ = vNormal.x;
			*pVertex++ = vNormal.y;
			*pVertex++ = vNormal.z;

			//vUV = pUV[ix];
			// UV coordinates

			for (size_t uv=0;uv<iUVSets;uv++)
			{
				vUV = mUVCoordinates[uv][ix];
				*pVertex++ = vUV.x;
				*pVertex++ = vUV.y;
			}

			// Get min/max for BoundingBox
			for (int k=0;k<3;k++)
			{
				if(vVec[k] > vMax[k])
					vMax[k] = vVec[k];
				if(vVec[k] < vMin[k])
					vMin[k] = vVec[k];
			}


			//vUV.x += dUVDelta;
		}

		// Bulk
		//vUV.x = 0.0f;
		//vUV.y = 0.0f;

		for (int iz = 0;iz<VERTEX_WIDTH;iz++) //OK
		{
			//vVec.x = vTopLeft.x;
			//vUV.x = dUVStartX;
			for (int ix = -1;ix<=VERTEX_WIDTH;ix++)
			{


				x = std::min(std::max(ix,0),VERTEX_WIDTH-1);
				z = std::min(std::max(iz,0),VERTEX_WIDTH-1);

				vVec = pVec[x+1 + (z+1) * MAP_WIDTH];

				if (ix == -1 || ix == VERTEX_WIDTH)
					vVec.y -= fSkirtLength;


				// Vertex Position
				*pVertex++ = vVec.x;
				*pVertex++ = vVec.y;
				*pVertex++ = vVec.z;

				vNormal = mNormals[x + z * VERTEX_WIDTH];

				// Normals
				*pVertex++ = vNormal.x;
				*pVertex++ = vNormal.y;
				*pVertex++ = vNormal.z;

				//vVec.normalise();

				//if (pNorm)
				//{
				//vNormal = *pNorm++;
				//*pVertex++ = vNormal.x;
				//*pVertex++ = vNormal.y;
				//*pVertex++ = vNormal.z;
				//}

				//*pVertex++ = vVec.x;
				//*pVertex++ = vVec.y;
				//*pVertex++ = vVec.z;



				//vUV = pUV[x + z * VERTEX_WIDTH];
				// UV coordinates


				for (size_t uv=0;uv<iUVSets;uv++)
				{
					vUV = mUVCoordinates[uv][x + z * VERTEX_WIDTH];
					*pVertex++ = vUV.x;
					*pVertex++ = vUV.y;
				}

				//*pVertex++ = mDebugColor.r;
				//*pVertex++ = mDebugColor.g;
				//*pVertex++ = mDebugColor.b;

				// Get min/max for BoundingBox
				for (int k=0;k<3;k++)
				{
					if(vVec[k] > vMax[k])
						vMax[k] = vVec[k];
					if(vVec[k] < vMin[k])
						vMin[k] = vVec[k];
				}

				//vVec.x += dPosDelta;
				//if (ix > -1 && ix < VERTEX_WIDTH)
				//	vUV.x += dUVDelta;
			}
			//vVec.z += dPosDelta;
			//vUV.y += dUVDelta;

		}

		// Bottom skirt
		//vUV.y = 0.0f;
		//vUV.x = dUVStartX + dUVDelta;
		for (int ix = 1;ix<VERTEX_WIDTH-1;ix++) //OK
		{
			vVec = pVec[ix+1 + (VERTEX_WIDTH) * MAP_WIDTH];
			vVec.y -= fSkirtLength;

			// Vertex Position
			*pVertex++ = vVec.x;
			*pVertex++ = vVec.y;
			*pVertex++ = vVec.z;

			vNormal = mNormals[ix + (VERTEX_WIDTH-1) * VERTEX_WIDTH];

			// Normals
			*pVertex++ = vNormal.x;
			*pVertex++ = vNormal.y;
			*pVertex++ = vNormal.z;

			//vUV = pUV[ix + (VERTEX_WIDTH-1) * VERTEX_WIDTH];

			// UV coordinates
			for (size_t uv=0;uv<iUVSets;uv++)
			{
				vUV = mUVCoordinates[uv][ix + (VERTEX_WIDTH-1) * VERTEX_WIDTH];
				*pVertex++ = vUV.x;
				*pVertex++ = vUV.y;
			}


			// Get min/max for BoundingBox
			for (int k=0;k<3;k++)
			{
				if(vVec[k] > vMax[k])
					vMax[k] = vVec[k];
				if(vVec[k] < vMin[k])
					vMin[k] = vVec[k];
			}

			//vUV.x += dUVDelta;
		}

        mMainBuffer->unlock(); //OK

		// Create boundingbox/sphere

		mBounds.setExtents(vMin,vMax); //OK
		mCenter = mBounds.getCenter(); //OK
		mBoundingRadius = (vMax - vMin).length() * 0.5; //OK
		//
		// 		if (USE_MORPH)
		// 		{
		// 			// Create delta buffer for all except the lowest mip
		// 			mDeltaBuffers = new HardwareVertexBufferSharedPtr[mMaxLOD - 1];
		// 		}

		//Real C = _calculateCFactor();

		//_calculateMinLevelDist2( C );

		_calculateMinLevelDist2(_calculateCFactor()); //OK


		calculateLODDistances(); //OK

		//for (size_t i=0;i<mMaxLOD;i++)
		//	LogManager::getSingleton().logMessage("Pre - Level: " + StringConverter::toString(i) + " Dist2: " + StringConverter::toString(mMinLevelDistSqr[i]));
		//for (size_t i=0;i<mMaxLOD;i++)
		//	LogManager::getSingleton().logMessage("Post - Level: " + StringConverter::toString(i) + " Dist2: " + StringConverter::toString(mMinLevelDistSqr[i]));

        mRenderLevel = 0;
		mTargetRenderLevel = mMaxLOD - 1;


#if (USE_DEBUG_DISPLAYS == 1)
		mDebugText = new MovableText(mParentNode->getName() + "_TXT","D","BlueHighway",mWidth / 20.0f,ColourValue::Red);
		mDebugText->setTextAlignment(MovableText::H_CENTER, MovableText::V_ABOVE);
		mDebugText->setAdditionalHeight(5.0f);
		mDebugNode->attachObject(mDebugText);
		mDebugNode->setPosition(mCenter.x,vMax.y,mCenter.z);
#endif
		setRenderLevel(mRenderLevel); //OK

		mVertexData->vertexBufferBinding->setBinding(1,mDeltaBuffers[0]); //OK

		mParentNode->_updateBounds();  // OK

		mTerrain->_fireMeshUpdated(this); //OK

	}

	void TerrainMesh::updateGeometryData()
	{
		mWidth = mParentQNode->getFloatingPointWidth();
		Vec2D<double> vPos = mParentQNode->getFloatingPointOffset();
		Vec2D<double> vEndPos = vPos + double(mWidth);



		mQuadTreeDepth = mParentQNode->getDepth();
		mNodePos = mParentQNode->getNodePosition();
		double dTotalWidth = mTerrain->getWidth();


		HEIGHTMAPTYPE* pHeights = mParentQNode->getHeightmap()->getData();
		float fScale = (1.0f / MAX_INT) * mTerrain->getTerrainHeight();

		float fDelta = mWidth / float(VERTEX_WIDTH - 1);
		//float fHalfWidth = mWidth * 0.5f;
		float fPosX = vPos.x - fDelta;
		float fPosZ = vPos.y - fDelta;

		Ogre::Vector3* pVec = mVertexPositions;

		for (size_t y=0;y<MAP_WIDTH;y++)
		{
			fPosX = vPos.x - fDelta;
			for (size_t x=0;x<MAP_WIDTH;x++)
			{
				*pVec++ = Ogre::Vector3(fPosX, UNPACK_HEIGHT(pHeights[x + y * MAP_WIDTH]) * fScale,fPosZ);

				fPosX += fDelta;
			}
			fPosZ += fDelta;
		}

		//Ogre::Vector3* pNorm = mNormals;
		//size_t x0,x1,y0,y1;
		Ogre::Vector3 A,B,C,D;
		Ogre::Vector3 vNorm;
		const Real rSxz = mWidth / Real(VERTEX_WIDTH-1);
		//const Real rSy = mTerrain->getTerrainHeight();
		//Real rYScale = 2.0f * ( mWidth / Real(VERTEX_WIDTH-1) / mTerrain->getTerrainHeight());
		const Real rYScale = 2.0f * rSxz;// / rSy;
		for (size_t y=0;y<VERTEX_WIDTH;y++)
		{


			for (size_t x=0;x<VERTEX_WIDTH;x++)
			{


				A = mVertexPositions[(x+2) + (y+1) * MAP_WIDTH];
				B = mVertexPositions[(x+1) + y * MAP_WIDTH];
				C = mVertexPositions[x + (y+1) * MAP_WIDTH];
				D = mVertexPositions[(x+1) + (y+2) * MAP_WIDTH];


				vNorm.x = C.y - A.y;
				vNorm.z = D.y - B.y;
				vNorm.y = rYScale;

				vNorm.normalise();

				mNormals[x + y * VERTEX_WIDTH] = vNorm;


			}

		}


		Ogre::Vector2 vUV;

		Ogre::Vector2* pUV = 0;


		Vec2D<double> vLandscapeUVStart = (vPos / dTotalWidth) + 0.5;
		float fLandscapeUVDelta = (mWidth / dTotalWidth) / float(VERTEX_WIDTH-1);

		pUV = mUVCoordinates[0];
		vUV = Ogre::Vector2(vLandscapeUVStart.x,vLandscapeUVStart.y);

		for (size_t y=0;y<VERTEX_WIDTH;y++)
		{
			vUV.x = vLandscapeUVStart.x;
			for (size_t x=0;x<VERTEX_WIDTH;x++)
			{
				*pUV++ = vUV;
				vUV.x += fLandscapeUVDelta;
			}
			vUV.y += fLandscapeUVDelta;
		}

		if (mTerrain->getUseChunkUVs())
		{
			Vec2D<double> vMeshUVStart = Vec2D<double>(0.0,0.0);
			float fMeshUVDelta = 1.0f / float(VERTEX_WIDTH-1);

			pUV = mUVCoordinates[1];
			vUV = Ogre::Vector2(vMeshUVStart.x,vMeshUVStart.y);

			for (size_t y=0;y<VERTEX_WIDTH;y++)
			{
				vUV.x = vMeshUVStart.x;
				for (size_t x=0;x<VERTEX_WIDTH;x++)
				{
					*pUV++ = vUV;
					vUV.x += fMeshUVDelta;
				}
				vUV.y += fMeshUVDelta;
			}

			mParentUVOffset = Ogre::Vector3::ZERO;
			mParentUVOffset.z = 0.5f;
			if (mParentQNode->getParent())
			{
				switch(mParentQNode->getNodePosition())
				{
				case QNode::NE:
					mParentUVOffset.x = 0.5f;
					break;
				case QNode::SW:
					mParentUVOffset.y = 0.5f;
					break;
				case QNode::SE:
					mParentUVOffset = 0.5f;
					break;
				default:
					break;
				}
			}
			else
				mParentUVOffset = Ogre::Vector3(0,0,1);
		}


	}

	void TerrainMesh::getImportantStats( int& renderLevel, int& targetRenderLevel, float& LODMorph)
	{
		renderLevel = mRenderLevel;
		targetRenderLevel = mTargetRenderLevel;
		LODMorph = mLODMorphFactor;
	}

	void TerrainMesh::setImportantStats( int renderLevel, int targetRenderLevel, float LODMorph )
	{
		mRenderLevel = renderLevel;
		mTargetRenderLevel = targetRenderLevel;
		mLODMorphFactor = LODMorph;
		setRenderLevel(mRenderLevel);
	}

	void TerrainMesh::calculateLODDistances()
	{
		for (int i=0;i<mMaxLOD;i++)
		{
			mMinLevelDistSqr[i] = (((((Real)i)/(Real)(mMaxLOD-1))) * mBoundingRadius * 2.0f) * mTerrain->getLODDistBias();
			mMinLevelDistSqr[i] *= mMinLevelDistSqr[i];
		}

		mSplitRadius = mBoundingRadius * 0.5 * mTerrain->getLODDistBias();
		mSplitRadius *= mSplitRadius;
		mUnsplitRadius = mBoundingRadius * 2.0f * mTerrain->getLODDistBias();
		//mUnsplitRadius = mMinLevelDistSqr[mMaxLOD-1] * 3.0f;
		mUnsplitRadius *= mUnsplitRadius;
	}

