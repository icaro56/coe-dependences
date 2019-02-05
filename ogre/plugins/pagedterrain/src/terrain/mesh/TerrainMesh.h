#pragma once

#include <OgrePrerequisites.h>
#include <OgreMaterial.h>
#include <OgreMovableObject.h>
#include <OgreRenderable.h>
#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include <OgreTexture.h>
#include <OgreSceneManager.h>
#include "PTPrerequisites.h"
#include "DllRequisites.h"
#include "utilities/movabletext3d/MovableText.h"


class PAGEDTERRAIN_EXPORT TerrainMesh : public Ogre::MovableObject, public Ogre::Renderable
{
public:

    TerrainMesh(Terrain* pTerrain, QNode* pParentNode);

    ~TerrainMesh();

    /// Overridden from Renderable to allow the morph LOD entry to be set
    void _updateCustomGpuParameter(
        const Ogre::GpuProgramParameters::AutoConstantEntry& constantEntry,
        Ogre::GpuProgramParameters* params) const;

    void setRenderLevel(size_t iLOD = 0);

    size_t getRenderLevel(){return mRenderLevel;}

    Ogre::Real _calculateCFactor();
    void _calculateMinLevelDist2( Ogre::Real C );

    void calculateLODDistances();

    Ogre::HardwareVertexBufferSharedPtr createDeltaBuffer(void);

    void _notifyCurrentCamera(Ogre::Camera* cam);

    void createGeometryData();
    void updateVertices();
    void updateGeometryData();
    void build();

    void getImportantStats(int& renderLevel, int& targetRenderLevel, float& LODMorph);
    void setImportantStats(int renderLevel, int targetRenderLevel, float LODMorph);

    void setPreCalculatedLight(Ogre::TexturePtr pRenderedLight){mRenderedLighting = pRenderedLight;}

    void destroyGeometry();
    void destroyGeometryData();
    void destroyRenderedLighting();
    void destroyMaterial();

    bool getIsBuilt() const { return mBuilt; }

    Ogre::Vector3* getVertexPositions() const { return mVertexPositions; }
    Ogre::VertexData* getVertexData() const { return mVertexData; }
    Ogre::IndexData* getIndexData(size_t iLOD = 0);

    const Ogre::Vector3& getVertexPosition(size_t x, size_t z);

    enum TerrainSplitState
    {
        TSS_NONE = 0,
        TSS_SPLIT,
        TSS_UNSPLIT
    };

    void update(Ogre::Real rTime = 0.0f);

    void checkLOD();

    TerrainSplitState getSplitState(){return mSplitState;}

    Ogre::Real getMorph(){return mLODMorphFactor;}

    void justSplit();

    void justUnSplit();


    // REQUIRED VIRTUALS:
    const Ogre::String& getMovableType(void) const
    {
        static Ogre::String movType = "SPT_TerrainMesh";
        return movType;
    }
    const Ogre::AxisAlignedBox& getBoundingBox( void ) const
    {
        return mBounds;
    }
    Ogre::Real getBoundingRadius(void) const { return mBoundingRadius; };
    void _updateRenderQueue( Ogre::RenderQueue* queue )
    {
        mLightListDirty = true;
        queue->addRenderable(this,mRenderQueueID);
    }
    const Ogre::MaterialPtr& getMaterial(void) const
    {
        return mMaterial;
    }

    const Ogre::String& getMaterialName()
    {
        return mMaterial->getName();
    }

    void setMaterial(Ogre::MaterialPtr pMaterial)
    {
        mMaterial = pMaterial;
    }

    void setMaterial(const Ogre::String& MatName);

    void getRenderOperation( Ogre::RenderOperation& op );
    void getWorldTransforms( Ogre::Matrix4* xform ) const
    {
        *xform = mParentNode->_getFullTransform();
    }
    const Ogre::Quaternion& getWorldOrientation(void) const
    {
        return mParentNode->_getDerivedOrientation();
    }
    const Ogre::Vector3& getWorldPosition(void) const
    {
        return mParentNode->_getDerivedPosition();
    }
    Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const
    {
        Ogre::Vector3 diff = mCenter - cam->getDerivedPosition();
        // Use squared length to avoid square root
        return diff.squaredLength();
    }
    const Ogre::LightList& getLights( void ) const
    {
        if (mLightListDirty)
        {
            getParentSceneNode()->getCreator()->_populateLightList(
                mCenter, this->getBoundingRadius(), mLightList);
            mLightListDirty = false;
        }
        return mLightList;
    }

   	void visitRenderables(Renderable::Visitor* visitor,
			bool debugRenderables = false)
	{}

private:

    Ogre::AxisAlignedBox mBounds;
    Ogre::Real mBoundingRadius;
    Ogre::Real mHalfRadius;
    Ogre::Real mUnsplitRadius;
    Ogre::Real mSplitRadius;
    Ogre::Real mCamDistance;
    Ogre::Vector3 mCenter;
    mutable bool mLightListDirty;
    Ogre::MaterialPtr mMaterial;
    Ogre::MaterialPtr mDefaultMaterial;
    Ogre::MaterialPtr mZFirstMaterial;
    bool mBuilt;
    bool mVisible;

    size_t mQuadTreeDepth;
    size_t mNodePos;

    Ogre::HardwareVertexBufferSharedPtr mMainBuffer;
    Ogre::HardwareVertexBufferSharedPtr* mDeltaBuffers;
    Ogre::VertexData* mVertexData;
    Ogre::IndexData* mIndexData;
    int mRenderLevel;
    int mLastRenderLevel;
    int mTargetRenderLevel;
    Ogre::SceneNode* mParentNode;
    std::vector<Ogre::Real> mLODDistances; // R^2 * i

    int mLastNextLevel;
    /// The morph factor between this and the next LOD level down
    Ogre::Real mLODMorphFactor;
    Ogre::Real mLastLODMorphFactor;
    /// List of squared distances at which LODs change
    Ogre::Real *mMinLevelDistSqr;
    int mNextLevelDown[10];

    Ogre::Real mWidth;
    Ogre::TexturePtr mRenderedLighting;
    Ogre::Vector3* mVertexPositions;
    Ogre::Vector3* mNormals;
    Ogre::Vector2** mUVCoordinates;
    //size_t mUVSets;
    Ogre::Vector3 mParentUVOffset;
    Ogre::Camera* mCamera;

    int mMaxLOD;

    Terrain* mTerrain;

    QNode* mParentQNode;

    TerrainSplitState mSplitState;

    Ogre::MovableText* mDebugText;
    Ogre::SceneNode* mDebugNode;

    Ogre::Viewport* mViewport;

};
