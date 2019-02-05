#pragma once

#include "PTPrerequisites.h"
#include "utilities/vectors/Vec2D.h"
#include "DllRequisites.h"

class Terrain;
class TerrainMesh;
class Heightmap;
class TerrainModifier;

class PAGEDTERRAIN_EXPORT QNode
{
public:
    // Root
    QNode(Terrain* pTerrain);

    // Child node
    QNode(QNode* pParent, size_t iNodePosition);

    ~QNode();

    QNode* getParent(){return mParent;}

    QNode* getChild(size_t iChild){return mChildren[iChild];}

    Terrain* getTerrain(){return mTerrain;}

    size_t getNodePosition(){return mNodePosition;}

    bool hasChildren(){return mHasChildren;}

    TerrainMesh* getTerrainMesh(){return mTerrainMesh;}

    Heightmap* getHeightmap(){return mHeightmap;}

    //Heightmap* getParentHeightmap(){return mParentMap;}
    //void setParentHeightmap(Heightmap* pParentMap){mParentMap = pParentMap;}

    QNode* getHighestAffected(const TerrainModifier* pModifier);
    void checkModifier(const TerrainModifier* pModifier, bool force = false);

    void _testUpdate();

    void buildHeightmap();
    void buildTerrain();

    const Vec2D<size_t>& getMapOffset(){return mMapOffset;}

    const Vec2D<size_t>& getIntegerOffset(){return mIntegerOffset;}

    const Vec2D<double>& getFloatingPointOffset(){return mFloatingPointOffset;}

    size_t getIntegerWidth(){return mIntegerWidth;}

    double getFloatingPointWidth(){return mFloatingPointWidth;}

    size_t getDepth(){return mDepth;}

    void setForceSplit(bool forceSplit = false){mForceSplit = forceSplit;}

    bool isPointInNode(float fPosX, float fPosZ);

    QNode* getNodeAtPoint(float fPosX, float fPosZ);

    // Returns 0.0f if not found
    float getHeightAt(float x, float z);

    // Returns false if not found - stores result in "y"
    bool getHeightAt(float x, float z, float& y);

    bool isReadyToSplit();

    bool isReadyToUnSplit();

    bool splitNode();

    bool unsplitNode();

    void runUpdate(float rTime = 0.0f);

    void runLODChecks();

    void runQuadTreeChecks();

    void updateLODDistances();

    void updateMaterial();

    enum QPosition
    {
        NW = 0,
        NE,
        SW,
        SE,
        ROOT
    };

private:
    QNode* mParent;
    QNode* mChildren[4];

    Terrain* mTerrain;

    size_t mNodePosition;

    size_t mDepth;

    bool mHasChildren;

    TerrainMesh* mTerrainMesh;

    Heightmap* mHeightmap;
    //Heightmap* mParentMap;
    Vec2D<size_t> mMapOffset;

    Vec2D<size_t> mIntegerOffset;
    size_t mIntegerWidth;

    Vec2D<double> mFloatingPointOffset;
    double mFloatingPointWidth;

    bool mForceSplit;
};

