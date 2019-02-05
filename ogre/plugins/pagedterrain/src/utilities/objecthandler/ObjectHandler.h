#pragma once

#include <OgrePrerequisites.h>
#include <OgreMaterial.h>
#include <OgreVector3.h>
#include <OgreSceneNode.h>
#include "PTPrerequisites.h"
#include "Terrain.h"
#if (USE_SINGLETON == 1)
#include <OgreSingleton.h>
#endif


class Terrain;
class ObjectHandler;

class PAGEDTERRAIN_EXPORT SceneObject
{
public:

    SceneObject(
        ObjectHandler* objectHandler,
        Ogre::MovableObject* movableObject,
        Ogre::SceneNode* sceneNode,
        Terrain::AutoClampState autoClampState,
        Ogre::Real heightBonus);

    ~SceneObject();

    void update(Ogre::Real fTime);

    Ogre::MovableObject* getObject() const { return mObject; }

    Ogre::SceneNode* getSceneNode() const { return mSceneNode; }

    ObjectHandler* getObjectHandler() const { return mObjectHandler; }

    Terrain::AutoClampState getAutoClampState() const { return mAutoClampState; }
    void setAutoClampState(Terrain::AutoClampState val) { mAutoClampState = val; }

    Ogre::Real getAutoClampTime() const { return mAutoClampTime; }
    void setAutoClampTime(Ogre::Real val) { mAutoClampTime = val; }

    Ogre::Real getHeightBonus() const { return mHeightBonus; }
    void setHeightBonus(Ogre::Real val) { mHeightBonus = val; }

    void useBoundingBoxBase();

    void biasHeightBonus(Ogre::Real val) {mHeightBonus += val;}
    void biasHeightBonusPercent(Ogre::Real percent);

    void moveObject(const Ogre::Vector3& pos = Ogre::Vector3::ZERO);
    void translateObject(const Ogre::Vector3& translation = Ogre::Vector3::ZERO);

    void setRotation(const Ogre::Quaternion& rotation = Ogre::Quaternion::IDENTITY);

    void rotateXZ(const Ogre::Degree rotation = Ogre::Degree() );

    Ogre::Vector3 getOriginalDirection() const { return mOriginalDirection; }
    void setOriginalDirection(const Ogre::Vector3& val = Ogre::Vector3::UNIT_Z) { mOriginalDirection = val; }

    void turnTowardsPoint(const Ogre::Vector3& target, bool useXZFirst = true);

    void turnTowardsPointXZ(const Ogre::Vector3& target);

    Ogre::Node* getTargetNode() const { return mTargetNode; }
    void setTargetNode(	Ogre::Node* val = 0, bool useXZRotationOnly = true)
    {
        mTargetNode = val;
        mTargetXZRotationOnly = useXZRotationOnly;
    }

    Ogre::Real getAutoRotateTime() const { return mAutoRotateTime; }
    void setAutoRotateTime(Ogre::Real val) { mAutoRotateTime = val; }

private:
    Ogre::MovableObject* mObject;

    Ogre::SceneNode* mSceneNode;
    Ogre::Vector3 mOriginalDirection;
    Ogre::Node* mTargetNode;
    bool mTargetXZRotationOnly;

    Ogre::Real mHeightBonus;
    ObjectHandler* mObjectHandler;

    Ogre::Real mAutoClampTime;

    Ogre::Real mCurClampTime;
    Ogre::Real mAutoRotateTime;
    Ogre::Real mCurRotateTime;

    Terrain::AutoClampState mAutoClampState;
};

#if (USE_SINGLETON == 1)
class PAGEDTERRAIN_EXPORT ObjectHandler : public Ogre::Singleton<ObjectHandler>
#else
class PAGEDTERRAIN_EXPORT ObjectHandler
#endif
{
public:
    ObjectHandler();
    ObjectHandler(Ogre::SceneManager* pSceneMgr, Terrain* pTerrain);

    ~ObjectHandler();

#if (USE_SINGLETON == 1)
    static ObjectHandler& getSingleton();

    static ObjectHandler* getSingletonPtr();
#endif

    SceneObject* createSceneObject(
        const Ogre::String& entityName,
        const Ogre::String& meshName,
        const Ogre::Vector3& position = Ogre::Vector3::ZERO,
        const Ogre::Quaternion& rotation = Ogre::Quaternion::IDENTITY,
        Terrain::AutoClampState autoClampState = Terrain::ACS_OFF,
        bool useObjectBBFloor = false,
        Ogre::Real heightBonus = 0.0f);

    SceneObject* createSceneObject(
        const Ogre::String& entityName,
        const Ogre::String& meshName,
        const Ogre::String& sceneNodeName,
        const Ogre::Vector3& position = Ogre::Vector3::ZERO,
        const Ogre::Quaternion& rotation = Ogre::Quaternion::IDENTITY,
        Terrain::AutoClampState autoClampState = Terrain::ACS_OFF,
        bool useObjectBBFloor = false,
        Ogre::Real heightBonus = 0.0f);

    SceneObject* createSceneObject(
        Ogre::MovableObject* movableObject,
        const Ogre::String& sceneNodeName,
        const Ogre::Vector3& position = Ogre::Vector3::ZERO,
        const Ogre::Quaternion& rotation = Ogre::Quaternion::IDENTITY,
        Terrain::AutoClampState autoClampState = Terrain::ACS_OFF,
        bool useObjectBBFloor = false,
        Ogre::Real heightBonus = 0.0f);

    SceneObject* createSceneObject(
        Ogre::MovableObject* movableObject,
        const Ogre::Vector3& position = Ogre::Vector3::ZERO,
        const Ogre::Quaternion& rotation = Ogre::Quaternion::IDENTITY,
        Terrain::AutoClampState autoClampState = Terrain::ACS_OFF,
        bool useObjectBBFloor = false,
        Ogre::Real heightBonus = 0.0f);

    void _addSceneObjectToStorage(const Ogre::String& name, SceneObject* pObject);

    void destroySceneObject(SceneObject* pObject);

    void destroyAllSceneObjects();

    void updateObjects(Ogre::Real fTime);


    Ogre::SceneManager* getSceneMgr() const { return mSceneMgr; }
    void setSceneMgr(Ogre::SceneManager* val) { mSceneMgr = val; }

    Terrain* getTerrain() const { return mTerrain; }
    void setTerrain(Terrain* val) { mTerrain = val; }

    bool getUseRootTerrainHeight() const { return mUseRootTerrainHeight; }
    void setUseRootTerrainHeight(bool val) { mUseRootTerrainHeight = val; }

    bool getUseGeoMorphTerrainHeight() const { return mUseGeoMorphTerrainHeight; }
    void setUseGeoMorphTerrainHeight(bool val) { mUseGeoMorphTerrainHeight = val; }

private:
    typedef std::map<Ogre::String, SceneObject*> SceneObjectStorage;
    SceneObjectStorage mStorage;

    Ogre::SceneManager* mSceneMgr;
    Terrain* mTerrain;

    bool mUseRootTerrainHeight;

    bool mUseGeoMorphTerrainHeight;

};
