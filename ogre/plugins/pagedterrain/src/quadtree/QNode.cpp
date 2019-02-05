#include "QNode.h"
#include "heightmap/Heightmap.h"
#include "terrain/mesh/TerrainMesh.h"
#include "Terrain.h"






	QNode::QNode( Terrain* pTerrain )
		: mTerrain(pTerrain),
		mParent(0),
		mNodePosition(ROOT),
		mHasChildren(false),
		mTerrainMesh(0),
		mHeightmap(0),
		mForceSplit(false)//,
		//mParentMap(0)
	{
		for (size_t i=0;i<4;i++)
			mChildren[i] = 0;

		mIntegerWidth = 1 << 31;
		mFloatingPointWidth = mTerrain->getWidth();
		double dHalfWidth = -(mFloatingPointWidth * 0.5);

		mIntegerOffset = Vec2D<size_t>(0,0);

		mFloatingPointOffset = Vec2D<double>(dHalfWidth,dHalfWidth);

		mMapOffset = Vec2D<size_t>(0,0);

		mDepth = 0;

	}

	QNode::QNode( QNode* pParent, size_t iNodePosition )
		: mParent(pParent),
		mNodePosition(iNodePosition),
		mHasChildren(false),
		mTerrainMesh(0),
		mHeightmap(0),
		//mParentMap(0),
		mForceSplit(false)
	{
		for (size_t i=0;i<4;i++)
			mChildren[i] = 0;

		mTerrain = mParent->getTerrain();

		mDepth = mParent->getDepth() + 1;

		//mParentMap = mParent->getParentHeightmap();
		//if (mParentMap)
		//{
		size_t iParentMapMaxDepth = mTerrain->getHeightmap()->getMaxLevels();//mParentMap->getStartDepth() + mParentMap->getMaxLevels();
		if (iParentMapMaxDepth >= mDepth)
		{
			mMapOffset = mParent->getMapOffset();
			size_t iCurrentOffset = (mTerrain->getHeightmap()->getWidth() - 1) >> (iParentMapMaxDepth - (iParentMapMaxDepth - mDepth));
			switch(mNodePosition)
			{
			case NE:
				mMapOffset.x += iCurrentOffset;
				break;
			case SW:
				mMapOffset.y += iCurrentOffset;
				break;
			case SE:
				mMapOffset += iCurrentOffset;
				break;
			default:
				break;
			}
		}
		else
		{
			mMapOffset = Vec2D<size_t>(0,0);
		}
		//}


		mIntegerWidth = (1 << 31) >> mDepth;
		mFloatingPointWidth = mTerrain->getWidth() / double(1 << mDepth);

		mIntegerOffset = mParent->getIntegerOffset();

		mFloatingPointOffset = mParent->getFloatingPointOffset();

		switch(mNodePosition)
		{
		case NE:
			mIntegerOffset.x += mIntegerWidth;
			mFloatingPointOffset.x += mFloatingPointWidth;
			break;
		case SW:
			mIntegerOffset.y += mIntegerWidth;
			mFloatingPointOffset.y += mFloatingPointWidth;
			break;
		case SE:
			mIntegerOffset += mIntegerWidth;
			mFloatingPointOffset += mFloatingPointWidth;
		    break;
		default:
		    break;
		}
	}

	QNode::~QNode()
	{
		// Destroy everything
		delete mTerrainMesh;
		mTerrainMesh = 0;

		delete mHeightmap;
		mHeightmap = 0;

		for (size_t i=0;i<4;i++)
			delete mChildren[i];
	}

	bool QNode::isReadyToSplit()
	{
		if (mTerrainMesh && mHasChildren == false)
			return (mTerrainMesh->getSplitState() == TerrainMesh::TSS_SPLIT);

		return false;
	}

	bool QNode::isReadyToUnSplit()
	{
		if (mHasChildren)
		{
			for (size_t i=0;i<4;i++)
			{
				if (mChildren[i]->hasChildren())
					return false;

				if (mChildren[i]->getTerrainMesh())
				{
					if (mChildren[i]->getTerrainMesh()->getSplitState() != TerrainMesh::TSS_UNSPLIT)
						return false;
				}
			}
			return true;
		}

		return false;
	}

	bool QNode::splitNode()
	{
		if ((mForceSplit == false) && (mHasChildren || mDepth == mTerrain->getMaxDepth() || (isReadyToSplit() == false)))
		{
		    	return false;
		}

		for (size_t i=0;i<4;i++)
		{
			mChildren[i] = new QNode(this,i);
			mChildren[i]->buildHeightmap();
			mChildren[i]->buildTerrain();
			mChildren[i]->getTerrainMesh()->justSplit();
		}

		delete mTerrainMesh;

        mTerrainMesh = 0;
		mHasChildren = true;
		return true;
	}

	bool QNode::unsplitNode()
	{
		if ((mHasChildren == false) || (isReadyToUnSplit() == false) || mForceSplit)
			return false;

		for (size_t i=0;i<4;i++)
		{
			delete mChildren[i];
			mChildren[i] = 0;
		}

		//mTerrainMesh = new TerrainMesh(mQuadTree->getTerrain(),this);
		//mTerrainMesh->build();
		buildHeightmap();
		buildTerrain();
		mTerrainMesh->justUnSplit();

		mHasChildren = false;

		return true;
	}

	void QNode::buildHeightmap()
	{
		if (mHeightmap)
			return;

		Heightmap* pRootMap = mTerrain->getHeightmap();
		if (pRootMap->getMaxLevels() < mDepth)
		{
			mHeightmap = new Heightmap();
		}
		else
		{
			mHeightmap = new Heightmap();
			size_t iParentMapMaxDepth = pRootMap->getMaxLevels();
			pRootMap->getSubdivision(mHeightmap,MAP_WIDTH,mMapOffset.x,mMapOffset.y,1 << (iParentMapMaxDepth - mDepth));

		}

		/*if (mHeightmap)
			return;

		mHeightmap = new Heightmap();

		if (mParentMap)
		{
			size_t iParentMapMaxDepth = mParentMap->getStartDepth() + mParentMap->getMaxLevels();
			mParentMap->getSubdivision(mHeightmap,MAP_WIDTH,mMapOffset.x,mMapOffset.y,1 << (iParentMapMaxDepth - mDepth));
		}*/
	}

	void QNode::buildTerrain()
	{
		if (mTerrainMesh)  //OK
			return;

        mTerrainMesh = new TerrainMesh(mTerrain,this);

		mTerrainMesh->build();


		Heightmap* pRootMap = mTerrain->getHeightmap();//OK

		if (pRootMap->getMaxLevels() >= mDepth) //OK
		{
			delete mHeightmap;
			mHeightmap = 0;
		}

	}

	void QNode::runQuadTreeChecks()
	{
		if (unsplitNode() == false)
		{
			if (splitNode() || mHasChildren)
			{
			    for (size_t i=0;i<4;i++)
				{
                    mChildren[i]->runQuadTreeChecks();
				}
			}
        }
	}

	void QNode::runLODChecks()
	{
		if (mTerrainMesh)
			mTerrainMesh->checkLOD();

		if (mHasChildren)
			for (size_t i=0;i<4;i++)
				mChildren[i]->runLODChecks();
	}

	void QNode::runUpdate(float rTime /*= 0.0f*/)
	{
		if (mTerrainMesh)
			mTerrainMesh->update(rTime);

		if (mHasChildren)
			for (size_t i=0;i<4;i++)
				mChildren[i]->runUpdate(rTime);

		mTerrain->_checkIn();
	}

	bool QNode::isPointInNode( float fPosX, float fPosZ )
	{
		if (
			fPosX < mFloatingPointOffset.x ||
			fPosZ < mFloatingPointOffset.y ||
			fPosX > mFloatingPointOffset.x + mFloatingPointWidth ||
			fPosZ > mFloatingPointOffset.y + mFloatingPointWidth)
			return false;

		return true;
	}

	QNode* QNode::getNodeAtPoint( float fPosX, float fPosZ )
	{
		if (mHasChildren)
		{
			QNode* pChild = 0;
			for (size_t i=0;i<4;i++)
			{
				pChild = mChildren[i];
				if (pChild->isPointInNode(fPosX,fPosZ))
					return pChild->getNodeAtPoint(fPosX,fPosZ);
			}
		}

		return this;
	}

	void QNode::_testUpdate()
	{
		if (mHasChildren)
		{
			for (size_t i=0;i<4;i++)
			{
				mChildren[i]->_testUpdate();
			}
		}
		else
		{
			int renderLevel, targetRenderLevel;
			float LODMorph;
			mTerrainMesh->getImportantStats(renderLevel,targetRenderLevel,LODMorph);

			delete mTerrainMesh;
			mTerrainMesh = 0;

			buildHeightmap();
			buildTerrain();

			mTerrainMesh->setImportantStats(renderLevel,targetRenderLevel,LODMorph);
		}
	}

	void QNode::checkModifier( const TerrainModifier* pModifier, bool force /*= false*/ )
	{
		if (force || pModifier->isInBounds(mFloatingPointOffset,mFloatingPointOffset + mFloatingPointWidth))
		{
			if (mHasChildren == false)
			{
				int renderLevel, targetRenderLevel;
				float LODMorph;
				if (mTerrain->getDiscardGeometryData())
				{

					mTerrainMesh->getImportantStats(renderLevel,targetRenderLevel,LODMorph);

					delete mTerrainMesh;
					mTerrainMesh = 0;

					buildHeightmap();
					buildTerrain();
					mTerrainMesh->setImportantStats(renderLevel,targetRenderLevel,LODMorph);
				}
				else
				{
					mTerrainMesh->getImportantStats(renderLevel,targetRenderLevel,LODMorph);

					buildHeightmap();
					mTerrainMesh->updateGeometryData();
					mTerrainMesh->updateVertices();

					mTerrainMesh->setImportantStats(renderLevel,targetRenderLevel,LODMorph);


					Heightmap* pRootMap = mTerrain->getHeightmap();
					if (pRootMap->getMaxLevels() >= mDepth)
					{
						delete mHeightmap;
						mHeightmap = 0;
					}
				}



			}
			else
			{
				for (size_t i=0;i<4;i++)
				{
					mChildren[i]->checkModifier(pModifier,force);
				}
			}
		}
	}

	QNode* QNode::getHighestAffected( const TerrainModifier* pModifier )
	{
		if (mHasChildren)
		{
			QNode* pChild;
			size_t iTotalHits = 0;
			for (size_t i=0;i<4;i++)
			{
				pChild = mChildren[i];
				if (pModifier->isInBounds(pChild->getFloatingPointOffset(),pChild->getFloatingPointOffset() + pChild->getFloatingPointWidth()))
					iTotalHits++;
			}
			if (iTotalHits == 1)
			{
				for (size_t i=0;i<4;i++)
				{
					return mChildren[i]->getHighestAffected(pModifier);
				}
			}
		}

		return this;
	}

	void QNode::updateLODDistances()
	{
		if (mHasChildren)
		{
			for (size_t i=0;i<4;i++)
			{
				mChildren[i]->updateLODDistances();
			}
		}
		else
		{
			if (mTerrainMesh)
			{
				mTerrainMesh->calculateLODDistances();
			}
		}
	}

	void QNode::updateMaterial()
	{
		if (mHasChildren)
		{
			for (size_t i=0;i<4;i++)
			{
				mChildren[i]->updateMaterial();
			}
		}
		else
		{
			if (mTerrainMesh)
			{
				mTerrainMesh->setMaterial(mTerrain->getMaterial());
			}
		}
	}
