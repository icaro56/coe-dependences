#include "ObjectHandler.h"



#include <OgreSceneManager.h>
#include <OgreEntity.h>
#include <OgreMovableObject.h>



using namespace Ogre;
using namespace std;

#if (USE_SINGLETON == 1)
template<> ObjectHandler* Ogre::Singleton<ObjectHandler>::msSingleton = 0;
#endif


	SceneObject::SceneObject(
		ObjectHandler* objectHandler,
		Ogre::MovableObject* movableObject,
		Ogre::SceneNode* sceneNode,
		Terrain::AutoClampState autoClampState,
		Ogre::Real heightBonus )
		: mObjectHandler(objectHandler),
		mObject(movableObject),
		mSceneNode(sceneNode),
		mAutoClampState(autoClampState),
		mHeightBonus(heightBonus),
		mOriginalDirection(Ogre::Vector3::UNIT_Z),
		mTargetNode(0),
		mTargetXZRotationOnly(true),
		mAutoClampTime(0.0f),
		mAutoRotateTime(0.0f),
		mCurClampTime(0.0f),
		mCurRotateTime(0.0f)
	{

	}

	SceneObject::~SceneObject()
	{
		if (mSceneNode)
		{
			mSceneNode->detachAllObjects();
			mSceneNode->getParentSceneNode()->removeAndDestroyChild(mSceneNode->getName());
		}
		if (mObject)
		{
			mObjectHandler->getSceneMgr()->destroyMovableObject(mObject);
		}
	}

	void SceneObject::update( Ogre::Real fTime )
	{
		Ogre::Vector3 pos = mSceneNode->getPosition();
		switch(mAutoClampState)
		{
		case Terrain::ACS_ALWAYS:
			mCurClampTime += fTime;
			if (mCurClampTime >= mAutoClampTime)
			{
				mCurClampTime = 0.0f;
				if (mObjectHandler->getTerrain()->getHeightAt(pos,mHeightBonus,
					mObjectHandler->getUseRootTerrainHeight(),
					mObjectHandler->getUseGeoMorphTerrainHeight()))
				{
					mSceneNode->setPosition(pos);
				}
			}
			break;
		case Terrain::ACS_BELOW:
			mCurClampTime += fTime;
			if (mCurClampTime >= mAutoClampTime)
			{
				mCurClampTime = 0.0f;
				if (mObjectHandler->getTerrain()->getHeightAt(
					pos,mHeightBonus,
					mObjectHandler->getUseRootTerrainHeight(),
					mObjectHandler->getUseGeoMorphTerrainHeight()))
				{
					if (pos.y > mSceneNode->getPosition().y)
						mSceneNode->setPosition(pos);
				}
			}
		    break;
		default:
		    break;
		}

		if (mTargetNode)
		{
			mCurRotateTime += fTime;
			if (mCurRotateTime >= mAutoRotateTime)
			{
				mCurRotateTime = 0.0f;
				if (mTargetXZRotationOnly)
					turnTowardsPointXZ(mTargetNode->_getDerivedPosition());
				else
					turnTowardsPoint(mTargetNode->_getDerivedPosition());
			}
		}
	}

	void SceneObject::useBoundingBoxBase()
	{
		const AxisAlignedBox& aabb = mObject->getBoundingBox();
		Real bottom = aabb.getMinimum().y;
		Real center = aabb.getCenter().y;
		mHeightBonus = center - bottom;
	}

	void SceneObject::biasHeightBonusPercent( Ogre::Real percent )
	{
		const AxisAlignedBox& aabb = mObject->getBoundingBox();
		Real bottom = aabb.getMinimum().y;
		Real top = aabb.getMaximum().y;
		Real total = top - bottom;
		mHeightBonus += total * (percent / 100.0f);
	}

	void SceneObject::moveObject( const Ogre::Vector3& pos /*= Ogre::Vector3::ZERO*/ )
	{
		mSceneNode->setPosition(pos);
	}

	void SceneObject::translateObject( const Ogre::Vector3& translation /*= Ogre::Vector3::ZERO*/ )
	{
		mSceneNode->translate(translation);
	}

	void SceneObject::setRotation( const Ogre::Quaternion& rotation /*= Ogre::Quaternion::IDENTITY*/ )
	{
		mSceneNode->setOrientation(rotation);
	}

	void SceneObject::rotateXZ( const Ogre::Degree rotation /*= Ogre::Degree() */ )
	{
		Ogre::Quaternion yRot(rotation, Ogre::Vector3::UNIT_Y);
		mSceneNode->rotate(yRot);
	}

	void SceneObject::turnTowardsPoint( const Ogre::Vector3& target, bool useXZFirst /*= true*/ )
	{
		Ogre::Vector3 direction = target - mSceneNode->getPosition();
		Ogre::Quaternion qRotXZ = mOriginalDirection.getRotationTo(Ogre::Vector3(direction.x,mOriginalDirection.y,direction.z));
		Ogre::Quaternion qRotY = Ogre::Vector3(direction.x,0.0f,direction.z).normalisedCopy().getRotationTo(direction);
		mSceneNode->setOrientation(qRotY * qRotXZ);
	}

	void SceneObject::turnTowardsPointXZ( const Ogre::Vector3& target )
	{
		Ogre::Vector3 direction = target - mSceneNode->getPosition();
		Ogre::Quaternion qRotXZ = mOriginalDirection.getRotationTo(Ogre::Vector3(direction.x,mOriginalDirection.y,direction.z));
		//Quaternion qRotY = Vector3(direction.x,0.0f,direction.z).normalisedCopy().getRotationTo(direction);
		mSceneNode->setOrientation(qRotXZ);// * qRotY);
	}
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

#if (USE_SINGLETON == 1)
	ObjectHandler* ObjectHandler::getSingletonPtr(void)
	{
		return msSingleton;
	}
	ObjectHandler& ObjectHandler::getSingleton(void)
	{
		assert( msSingleton );  return ( *msSingleton );
	}
#endif

	ObjectHandler::ObjectHandler()
		: mSceneMgr(0),
		mTerrain(0),
		mUseGeoMorphTerrainHeight(false),
		mUseRootTerrainHeight(false)
	{

	}

	ObjectHandler::ObjectHandler( Ogre::SceneManager* pSceneMgr, Terrain* pTerrain )
		: mSceneMgr(pSceneMgr),
		mTerrain(pTerrain),
		mUseGeoMorphTerrainHeight(false),
		mUseRootTerrainHeight(false)
	{

	}


	ObjectHandler::~ObjectHandler()
	{
		destroyAllSceneObjects();
	}

	SceneObject* ObjectHandler::createSceneObject(
		const Ogre::String& entityName,
		const Ogre::String& meshName,
		const Ogre::Vector3& position /*= Ogre::Vector3::ZERO*/,
		const Ogre::Quaternion& rotation /*= Ogre::Quaternion::IDENTITY*/,
		Terrain::AutoClampState autoClampState /*= Terrain::AutoClampState::ACS_OFF*/,
		bool useObjectBBFloor /*= false*/,
		Ogre::Real heightBonus /*= 0.0f*/ )
	{
		assert(mSceneMgr);

		Ogre::Entity* pEnt = mSceneMgr->createEntity(entityName,meshName);
		Ogre::SceneNode* pNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(position,rotation);
		pNode->attachObject(pEnt);

		SceneObject* pObject = new SceneObject(this,pEnt,pNode,autoClampState,heightBonus);
		if (useObjectBBFloor)
		{
			pObject->useBoundingBoxBase();
		}

		_addSceneObjectToStorage(entityName,pObject);
		return pObject;
	}

	SceneObject* ObjectHandler::createSceneObject(
		const Ogre::String& entityName,
		const Ogre::String& meshName,
		const Ogre::String& sceneNodeName,
		const Ogre::Vector3& position /*= Ogre::Vector3::ZERO*/,
		const Ogre::Quaternion& rotation /*= Ogre::Quaternion::IDENTITY*/,
		Terrain::AutoClampState autoClampState /*= Terrain::AutoClampState::ACS_OFF*/,
		bool useObjectBBFloor /*= false*/,
		Ogre::Real heightBonus /*= 0.0f*/ )
	{
		assert(mSceneMgr);

		Ogre::Entity* pEnt = mSceneMgr->createEntity(entityName,meshName);
		Ogre::SceneNode* pNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(sceneNodeName,position,rotation);
		pNode->attachObject(pEnt);

		SceneObject* pObject = new SceneObject(this,pEnt,pNode,autoClampState,heightBonus);
		if (useObjectBBFloor)
		{
			pObject->useBoundingBoxBase();
		}

		_addSceneObjectToStorage(entityName,pObject);
		return pObject;
	}

	SceneObject* ObjectHandler::createSceneObject(
		Ogre::MovableObject* movableObject,
		const Ogre::String& sceneNodeName,
		const Ogre::Vector3& position /*= Ogre::Vector3::ZERO*/,
		const Ogre::Quaternion& rotation /*= Ogre::Quaternion::IDENTITY*/,
		Terrain::AutoClampState autoClampState /*= Terrain::AutoClampState::ACS_OFF*/,
		bool useObjectBBFloor /*= false*/,
		Ogre::Real heightBonus /*= 0.0f*/ )
	{
		assert(mSceneMgr);

		Ogre::SceneNode* pNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(sceneNodeName,position,rotation);
		pNode->attachObject(movableObject);

		SceneObject* pObject = new SceneObject(this,movableObject,pNode,autoClampState,heightBonus);
		if (useObjectBBFloor)
		{
			pObject->useBoundingBoxBase();
		}

		_addSceneObjectToStorage(movableObject->getName(),pObject);
		return pObject;
	}

	SceneObject* ObjectHandler::createSceneObject(
		Ogre::MovableObject* movableObject,
		const Ogre::Vector3& position /*= Ogre::Vector3::ZERO*/,
		const Ogre::Quaternion& rotation /*= Ogre::Quaternion::IDENTITY*/,
		Terrain::AutoClampState autoClampState /*= Terrain::AutoClampState::ACS_OFF*/,
		bool useObjectBBFloor /*= false*/,
		Ogre::Real heightBonus /*= 0.0f*/ )
	{
		assert(mSceneMgr);

		Ogre::SceneNode* pNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(position,rotation);
		pNode->attachObject(movableObject);

		SceneObject* pObject = new SceneObject(this,movableObject,pNode,autoClampState,heightBonus);
		if (useObjectBBFloor)
		{
			pObject->useBoundingBoxBase();
		}

		_addSceneObjectToStorage(movableObject->getName(),pObject);
		return pObject;
	}

	void ObjectHandler::_addSceneObjectToStorage( const Ogre::String& name, SceneObject* pObject )
	{
		SceneObjectStorage::iterator it = mStorage.find(name);
/*#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32)
		if (it != mStorage.end())
		{
			it = mStorage.erase(it);
		}

		mStorage.insert(it,SceneObjectStorage::value_type(name,pObject));
#else*/
		if (it != mStorage.end())
		{
			mStorage.erase(it);
		}

		mStorage.insert(SceneObjectStorage::value_type(name,pObject));
//#endif
	}

	void ObjectHandler::destroySceneObject( SceneObject* pObject )
	{
		String name = pObject->getObject()->getName();
		SceneObjectStorage::iterator it = mStorage.find(name);
		if (it != mStorage.end())
		{
			mStorage.erase(it);
		}
	}

	void ObjectHandler::updateObjects( Ogre::Real fTime )
	{
		SceneObjectStorage::iterator it = mStorage.begin();
		while (it != mStorage.end())
		{
			it->second->update(fTime);
			++it;
		}
	}

	void ObjectHandler::destroyAllSceneObjects()
	{
		SceneObjectStorage::iterator it = mStorage.begin();
		while (it != mStorage.end())
		{
			delete it->second;
			++it;
		}
		mStorage.clear();
	}
