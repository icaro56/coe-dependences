#pragma once

#include <OgrePrerequisites.h>
#include <OgreVertexIndexData.h>
#include <OgreMaterial.h>
#include <OgreVector3.h>
#include <OgreSceneManager.h>
#include "PTPrerequisites.h"
#include "terrain/modifiers/TerrainModifier.h"
#if (USE_SINGLETON == 1)
#include <OgreSingleton.h>
#endif

#include "DllRequisites.h"

struct TerrainModification
{
	int x;
	int y;
	HEIGHTMAPTYPE data;

	TerrainModification(int x1, int y1, HEIGHTMAPTYPE data1)
	{
		x = x1;
		y = y1;
		data = data1;
	}
};

#include <vector>
typedef std::vector<Ogre::Vector2> PointList;
typedef std::vector<Ogre::Vector3> MemList;
typedef std::vector<float> HeightList;
typedef std::vector<TerrainModification> ModifyList;

class ObjectHandler;
class SceneObject;
class Terrain;
class Camera;

class PAGEDTERRAIN_EXPORT SPTRaySceneQuery : public Ogre::DefaultRaySceneQuery
{
public:
    SPTRaySceneQuery(Ogre::SceneManager* creator);
    SPTRaySceneQuery(Terrain* terrain);
    ~SPTRaySceneQuery();

    void execute(Ogre::RaySceneQueryListener* listener);
protected:
    Ogre::SceneQuery::WorldFragment mWorldFragment;
    Terrain* mTerrain;
};

class PAGEDTERRAIN_EXPORT TerrainEventListener
{
public:
    virtual void meshCreated(TerrainMesh* mesh){}
    virtual void meshDestroyed(TerrainMesh* mesh){}
    virtual void meshUpdated(TerrainMesh* mesh){}
};


#if (USE_SINGLETON == 1)
class PAGEDTERRAIN_EXPORT Terrain : public Ogre::Singleton<Terrain>
#else
class PAGEDTERRAIN_EXPORT Terrain
#endif
{
public:
    Terrain();
    Terrain(const std::string& camName, Ogre::SceneManager* sceneManager);
    ~Terrain();

#if (USE_SINGLETON == 1)
    static Terrain& getSingleton();

    static Terrain* getSingletonPtr();
#endif

    void addIndexData(Ogre::IndexData* pData, size_t iLOD);
    Ogre::IndexData* getIndexData(size_t iLOD);

    void setCamera(Ogre::Camera* cam){mCamera = cam;}
    Ogre::Camera* getCamera(){return mCamera;}

    enum AutoClampState
    {
        ACS_OFF = 0,
        ACS_BELOW,
        ACS_ALWAYS
    };

    AutoClampState getClampState() const { return mClampState; }

    Ogre::Real getClampBonusHeight() const {return mClampBonusHeight; }

    bool getClampUseRoot() {return mClampUseRoot;}
    bool getClampUseGeoMorph(){return mClampUseGeoMorph;}

    void setClampState(
        AutoClampState val,
        Ogre::Real heightBonus = 0.0f,
        bool useRoot = false,
        bool useMorph = false)
    {
        mClampState = val;
        mClampBonusHeight = heightBonus;
        mClampUseRoot = useRoot;
        mClampUseGeoMorph = useMorph;
    }

    enum AutoClampUpdateState
    {
        ACUS_MOVEMENT = 0,
        ACUS_TIME,
        ACUS_BOTH
    };

    Ogre::Real getSkirtLength() const { return mSkirtLength; }
    void setSkirtLength(Ogre::Real val) { mSkirtLength = val; }
    void setSkirtLengthPercent(Ogre::Real val = 10.0f){ mSkirtLength = mTerrainHeight * (val / 100.0f);}

    float getMorphSpeed() const { return mMorphSpeed; }
    void setMorphSpeed(float val) { mMorphSpeed = val; }

    float getLODCheckTime() const { return mLODCheckTime; }
    void setLODCheckTime(float val) { mLODCheckTime = val; }

    Ogre::SceneNode* getTerrainNode(){return mTerrainNode;}

    void setSceneManager(Ogre::SceneManager* pSceneMgr){mSceneMgr = pSceneMgr;}
    Ogre::SceneManager* getSceneManager(){return mSceneMgr;}

    void setMaterial( const std::string& materialName );
    Ogre::MaterialPtr getMaterial(){return mTerrainMaterial;}

    void setTerrainHeight(Ogre::Real height){mTerrainHeight = height;}
    Ogre::Real getTerrainHeight() const {return mTerrainHeight;}

    void setMaxDepth(size_t iMaxDepth){mMaxDepth = iMaxDepth;}
    size_t getMaxDepth(){return mMaxDepth;}

    void setWidth(double dWidth){mWidth = dWidth;}
    double getWidth(){return mWidth;}

    void initialize(const Ogre::String& heightmapName, size_t iWidth = 0);
    void destroy();

    void updateHeightmap();

    Heightmap* getHeightmap(){return mHeightmap;}

	HEIGHTMAPTYPE* getHeightmapData();

    QNode* getRootNode(){return mRootNode;}

    void SaveTerrainData(const std::string& filePath);

    double getHeightAt( double vX, double vZ, double rBonus = 0.0, bool bUseRoot = false, bool bUseGeoMorphing = false );

    bool getHeightAt(Ogre::Vector3& vPos, Ogre::Real rBonus = 0.0f, bool bUseRoot = false, bool bUseGeoMorphing = false);

    Ogre::Vector3 getNormalAt(Ogre::Vector3& vPos, bool bUseRoot = false, bool bUseGeoMorphing = false);

    bool getRayHeight(const Ogre::Ray& vRay, Ogre::Vector3& vReturnPos, bool bUseRoot = false, bool bUseGeoMorphing = false);

    void _checkIn(){mTotalNodes++;}

    void onFrameStart(double fT);

    void onFrameEnd(Ogre::Real fTime);

    bool getAutoUpdateLightmap() const { return mAutoUpdateLightmap; }
    void setAutoUpdateLightmap(bool val) { mAutoUpdateLightmap = val; }

    void updateLightmap();

    void showBrush(const std::string& bName, double width, double height);

	Ogre::Vector3 mouseWorldPosition(double xMouse, double yMouse, double wWindow, double hWindow, Ogre::Camera* activeCam = NULL);

    void endTerrainModifier(){bGetNewPos = true; bGetNewHeight = true; setBrushVisible(false);}

    void createTerrainModifier(const std::string& bName, double width, double height, double intensity);

    void createTerrainModifierDown(double width, double height);

    QNode* addTerrainModifier(const TerrainModifier& modifier);

    GPULightmapper* createLightmapper(
        const Ogre::String& strLightmapMatName,
        const Ogre::String& strCompositorName,
        size_t iMaxLightmapSize = 2048,
        size_t iMaxHeightmapTexSize = 2049,
        const Ogre::String& strLightmapScaleParamName = "vScale",
        const Ogre::String& strLightmapHeightmapSizeParamName = "fSize",
        const Ogre::String& strHeightmapTexName = "SPT_Heightmap_Tex",
        const Ogre::String& strRenderTextureName = "SPT_Lightmap_RTT",
        const Ogre::String& strLightmapTexUnitName = "Heightmap",
        const Ogre::String& strTerrainTexUnitName = "Lightmap",
        const Ogre::String& strCompositorScaleParamName = "vScale");

    GPULightmapper* getLightmapper(){return mLightmapper;}

    void setLightDirection(const Ogre::Vector3& vLightDir);

    void quickSetup(const Ogre::String& strHeightmapName, const Ogre::String& strTerrainMaterialName, Ogre::Real fTerrainWidth, Ogre::Real fTerrainHeight, Ogre::Real fTerrainQuickLoadTime = 3.0f);

    void saveTerrain(const Ogre::String& filePath);

    ObjectHandler* getObjectHandler() const { return mObjectHandler; }
    void setObjectHandler(ObjectHandler* val) { mObjectHandler = val; }

    Ogre::RaySceneQuery*
        createRayQuery(const Ogre::Ray& ray, unsigned long mask = 0xFFFFFFFF);

    bool getDiscardGeometryData() const { return mDiscardGeometryData; }
    void setDiscardGeometryData(bool val) { mDiscardGeometryData = val; }

    bool getUseChunkUVs() const { return mUseChunkUVs; }
    void setUseChunkUVs(bool val) { mUseChunkUVs = val; }

    void setTerrainEventListener(TerrainEventListener* listener);

    void _fireMeshCreated(TerrainMesh* mesh);
    void _fireMeshDestroyed(TerrainMesh* mesh);
    void _fireMeshUpdated(TerrainMesh* mesh);

    Ogre::String getDefaultMaterialScheme();

    void setMaterialSchemeParams(const Ogre::String& paramName, const Ogre::Vector3& paramVal, const Ogre::String& schemeName = "Default");
    void setMaterialSchemeParams(const Ogre::String& paramName, const Ogre::Vector4& paramVal, const Ogre::String& schemeName = "Default");
    void setMaterialSchemeParams(const Ogre::String& paramName, const Ogre::Matrix4& paramVal, const Ogre::String& schemeName = "Default");
    void setMaterialSchemeParams(const Ogre::String& paramName, const Ogre::Real& paramVal, const Ogre::String& schemeName = "Default");

    void testApply(const Ogre::String& matName);

    BrushPtr createBrush(const Ogre::String& brushImageName);
    void destroyBrush(const Ogre::String& brushImageName);
    void destroyBrush(BrushPtr pBrush);

    void destroyAllBrushes();

    bool brushExists(const Ogre::String& brushImageName);

    BrushPtr getBrush(size_t index);
    BrushPtr getBrush(const Ogre::String& brushImageName);

    void initializeBrushDecal(const std::string& brushTexName, double sizex, double sizey);
    void destroyBrushDecal();

    void setBrushSize(const Ogre::Vector2& size);

    void setBrushPosition(const Ogre::Vector3& pos);

    void setBrushVisible(bool visibile = true);

    Ogre::Vector4 _getBrushParams();

    Ogre::Real getLODDistBias() const { return mLODDistBias; }
    void setLODDistBias(Ogre::Real val);

    void setSelectedTexture(const Ogre::String& texName);
    void clearSelectedTexture();
    Ogre::TexturePtr getSelectedTexture() const {return mSelectedTexture;}

    void saveSelectedTexture(const Ogre::String& filePath);

    bool getDeformation() const { return mDeformation; }
    void setDeformation(bool val) { mDeformation = val; }

    int getEditChannel() const { return mEditChannel; }
    void setEditChannel(int val) { mEditChannel = val; }

    Ogre::Viewport* getViewport() { return mViewport; }
    void setViewport(Ogre::Viewport* vp) { mViewport = vp; }

	void setVisible(bool visible);
	bool isVisible();

	//>
    void setOpacity(float alpha);
	float getOpacity() const { return mAlpha; }
    //<
private:
    typedef std::map<size_t, Ogre::IndexData*> IndexStorage;
    IndexStorage mIndexStore;

    Ogre::Camera* mCamera;
    Ogre::SceneNode* mTerrainNode;
    Ogre::SceneManager* mSceneMgr;
    Ogre::MaterialPtr mTerrainZFirstMaterial;
    Ogre::MaterialPtr mTerrainMaterial;

    Ogre::Real mTerrainHeight;
    Ogre::Real mSkirtLength;
    float mMorphSpeed;
    float mLODCheckTime;
    Ogre::Real mPreMorphSpeed;
    Ogre::Real mPreLODCheckTime;
    bool mInitialized;

    Ogre::Real mQuickLoadTime;

    AutoClampState mClampState;
    Ogre::Real mClampBonusHeight;
    bool mClampUseRoot;
    bool mClampUseGeoMorph;
    //TerrainMods mModifiers;

    GPULightmapper* mLightmapper;

    size_t mMaxDepth;

    size_t mTotalNodes;
    double mWidth;

    Heightmap* mHeightmap;

    QNode* mRootNode;

    HeightmapReader* mSingleton;

    ObjectHandler* mObjectHandler;

    Ogre::Vector3 mLightDirection;

    bool mAutoUpdateLightmap;

    bool mDiscardGeometryData;

    bool mUseChunkUVs;

    TerrainEventListener* mEventListener;
    TerrainEventListener mDefaultListener;

    typedef std::map<Ogre::String, BrushPtr> BrushStorage;
    BrushStorage mBrushStorage;

    Ogre::Vector2 mBrushSize;
    bool mShowBrush;
    Ogre::Vector3 mBrushPosition;
    Ogre::String mBrushTextureName;
    Ogre::TextureUnitState* mBrushTexState;
    Ogre::Pass* mDecalPass;

    Ogre::TexturePtr mSelectedTexture;

    bool mDeformation;

    int mEditChannel;
    Ogre::Real mLODDistBias;

    BrushPtr cBrush;

    bool bGetNewPos;
    Ogre::Vector3 vBrushPos;

    Ogre::Real rFlattenHeight;
    bool bGetNewHeight;

    Ogre::Viewport* mViewport;
	float mAlpha;
};
