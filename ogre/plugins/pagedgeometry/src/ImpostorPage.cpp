/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich
Modified 2008 by Erik Hjortsberg (erik.hjortsberg@iteam.se)

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
    1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

//ImpostorPage.cpp
//ImposterPage is an extension to PagedGeometry which displays entities as imposters.
//-------------------------------------------------------------------------------------

#include <OgreRoot.h>
#include <OgreTimer.h>
#include <OgreCamera.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreEntity.h>
#include <OgreSubEntity.h>
#include <OgreHardwarePixelBuffer.h>

#include "ImpostorPage.h"
#include "StaticBillboardSet.h"


using namespace Forests;

// static members initialization
Ogre::uint  Forests::ImpostorPage::s_nImpostorResolution    = 128;
Ogre::uint  Forests::ImpostorPage::s_nSelfInstances         = 0;
Ogre::uint  Forests::ImpostorPage::s_nUpdateInstanceID      = 0;
Ogre::ColourValue Forests::ImpostorPage::s_clrImpostorBackground  = Ogre::ColourValue(0.0f, 0.3f, 0.0f, 0.0f);
Ogre::BillboardOrigin Forests::ImpostorPage::s_impostorPivot      = Ogre::BBO_CENTER;



//-----------------------------------------------------------------------------
/// Default constructor
ImpostorPage::ImpostorPage() :
m_pSceneMgr    (NULL),
m_pPagedGeom   (NULL),
m_blendMode    (ALPHA_REJECT_IMPOSTOR),
m_nInstanceID  (0),
m_nAveCount    (0),
m_vecCenter    (0, 0, 0)
{
   ++s_nSelfInstances;
}


//-----------------------------------------------------------------------------
/// Destructor
ImpostorPage::~ImpostorPage()
{
   TImpostorBatchs::iterator iter = m_mapImpostorBatches.begin(), iend = m_mapImpostorBatches.end();
   while (iter != iend)
   {
      delete iter->second;
      ++iter;
   }

   if (--s_nSelfInstances == 0 && m_pPagedGeom)
   {
      if (m_pPagedGeom->getSceneNode())
      {
         m_pPagedGeom->getSceneNode()->removeAndDestroyChild("ImpostorPage::renderNode");
         m_pPagedGeom->getSceneNode()->removeAndDestroyChild("ImpostorPage::cameraNode");
      }
      else if (m_pSceneMgr)
      {
         m_pSceneMgr->destroySceneNode("ImpostorPage::renderNode");
         m_pSceneMgr->destroySceneNode("ImpostorPage::cameraNode");
      }
      else
      {
         assert(false && "Who must delete scene node???");
      }

      Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup("Impostors");
   }
}


//-----------------------------------------------------------------------------
///
void ImpostorPage::init(PagedGeometry *geom, const Ogre::Any &data)
{
   assert(geom && "Null pointer to PagedGeometry");
	m_pSceneMgr    = geom->getSceneManager();
	m_pPagedGeom   = geom;
		
	if (s_nSelfInstances == 1)  // first instance
   {
		// Set up a single instance of a scene node which will be used when rendering impostor textures
		geom->getSceneNode()->createChildSceneNode("ImpostorPage::renderNode");
		geom->getSceneNode()->createChildSceneNode("ImpostorPage::cameraNode");
      Ogre::ResourceGroupManager::getSingleton().createResourceGroup("Impostors");
	}
}


//-----------------------------------------------------------------------------
///
void ImpostorPage::setRegion(Ogre::Real left, Ogre::Real top, Ogre::Real right, Ogre::Real bottom)
{
	// Calculate center of region
	m_vecCenter.x  = (left + right) * 0.5f;
	m_vecCenter.z  = (top + bottom) * 0.5f;
	m_vecCenter.y  = 0.0f; // The center.y value is calculated when the entities are added
	m_nAveCount    = 0;
}


//-----------------------------------------------------------------------------
///
void ImpostorPage::addEntity(Ogre::Entity *ent, const Ogre::Vector3 &position, const Ogre::Quaternion &rotation, const Ogre::Vector3 &scale, const Ogre::ColourValue &color)
{
	//Get the impostor batch that this impostor will be added to
	ImpostorBatch *ibatch = ImpostorBatch::getBatch(this, ent);

	//Then add the impostor to the batch
	ibatch->addBillboard(position, rotation, scale, color);

	//Add the Y position to the center.y value (to be averaged later)
	m_vecCenter.y += position.y + ent->getBoundingBox().getCenter().y * scale.y;
	++m_nAveCount;
}


//-----------------------------------------------------------------------------
///
void ImpostorPage::build()
{
   if (m_mapImpostorBatches.empty())
      return;

	// Calculate the average Y value of all the added entities
   m_vecCenter.y = m_nAveCount > 0 ? m_vecCenter.y /= m_nAveCount : 0;

	//Build all batches
	TImpostorBatchs::iterator it = m_mapImpostorBatches.begin(), iend = m_mapImpostorBatches.end();
   while (it != iend)
   {
      it->second->build();
      ++it;
   }
}

//-----------------------------------------------------------------------------
///
void ImpostorPage::setVisible(bool visible)
{
	// Update visibility status of all batches
	TImpostorBatchs::iterator it = m_mapImpostorBatches.begin(), iend = m_mapImpostorBatches.end();
   while (it != iend)
   {
      it->second->setVisible(visible);
      ++it;
   }
}


//-----------------------------------------------------------------------------
///
void ImpostorPage::setFade(bool enabled, Ogre::Real visibleDist, Ogre::Real invisibleDist)
{
	// Update fade status of all batches
	TImpostorBatchs::iterator it = m_mapImpostorBatches.begin(), iend = m_mapImpostorBatches.end();
   while (it != iend)
   {
      it->second->setFade(enabled, visibleDist, invisibleDist);
      ++it;
   }
}


//-----------------------------------------------------------------------------
///
void ImpostorPage::removeEntities()
{
	TImpostorBatchs::iterator iter = m_mapImpostorBatches.begin(), iend = m_mapImpostorBatches.end();
   while (iter != iend)
   {
      iter->second->clear();
      ++iter;
	}

	//Reset y center
	m_vecCenter.y  = 0;
	m_nAveCount    = 0;
}


//-----------------------------------------------------------------------------
///
void ImpostorPage::update()
{
   if (m_mapImpostorBatches.empty())  // SVA speed up
      return;

	//Calculate the direction the impostor batches should be facing
	Ogre::Vector3 camPos = m_pPagedGeom->_convertToLocal(m_pPagedGeom->getCamera()->getDerivedPosition());
	
	// Update all batches
   Ogre::Real distX = camPos.x - m_vecCenter.x;
	Ogre::Real distZ = camPos.z - m_vecCenter.z;
	Ogre::Real distY = camPos.y - m_vecCenter.y;
	Ogre::Real distRelZ = Ogre::Math::Sqrt(distX * distX + distZ * distZ);
	Ogre::Radian pitch = Ogre::Math::ATan2(distY, distRelZ);

	Ogre::Radian yaw;
	if (distRelZ > m_pPagedGeom->getPageSize() * 3)
   {
		yaw = Ogre::Math::ATan2(distX, distZ);
	}
   else
   {
		Ogre::Vector3 dir = m_pPagedGeom->_convertToLocal(m_pPagedGeom->getCamera()->getDerivedDirection());
		yaw = Ogre::Math::ATan2(-dir.x, -dir.z);
	}

   TImpostorBatchs::iterator iter = m_mapImpostorBatches.begin(), iend = m_mapImpostorBatches.end();
   while (iter != iend)
   {
      iter->second->setAngle(pitch.valueDegrees(), yaw.valueDegrees());
      ++iter;
   }
}

void ImpostorPage::regenerate(Ogre::Entity *ent)
{
	ImpostorTexture *tex = ImpostorTexture::getTexture(NULL, ent);
	if (tex != NULL)
		tex->regenerate();
}

void ImpostorPage::regenerateAll()
{
	ImpostorTexture::regenerateAll();
}


//-------------------------------------------------------------------------------------

unsigned long ImpostorBatch::GUID = 0;

ImpostorBatch::ImpostorBatch(ImpostorPage *group, Ogre::Entity *entity) :
m_pTexture  (NULL)
{
	// Render impostor texture for this entity
	m_pTexture = ImpostorTexture::getTexture(group, entity);
	
	//Create billboard set
   PagedGeometry *pg = group->getParentPagedGeometry();
   bbset = new StaticBillboardSet(pg->getSceneManager(), pg->getSceneNode());
	bbset->setTextureStacksAndSlices(IMPOSTOR_PITCH_ANGLES, IMPOSTOR_YAW_ANGLES);

	setBillboardOrigin(ImpostorPage::getImpostorPivot());

	//Default the angle to 0 degrees
	pitchIndex = -1;
	yawIndex = -1;
	setAngle(0.0f, 0.0f);

	//Init. variables
	igroup = group;
}

ImpostorBatch::~ImpostorBatch()
{
	//Delete billboard set
	delete bbset;

	// Delete texture
	ImpostorTexture::removeTexture(m_pTexture);
}

//Returns a pointer to an ImpostorBatch for the specified entity in the specified
//ImpostorPage. If one does not already exist, one will automatically be created.
ImpostorBatch *ImpostorBatch::getBatch(ImpostorPage *group, Ogre::Entity *entity)
{
	//Search for an existing impostor batch for this entity
	Ogre::String entityKey = ImpostorBatch::generateEntityKey(entity);
   ImpostorBatch *batch = group->getImpostorBatch(entityKey);
   if (batch)  // If found return it
      return batch;

   // Otherwise, create a new batch
   batch = new ImpostorBatch(group, entity);
   group->injectImpostorBatch(entityKey, batch);   // warning! function can return false
   return batch;
}

//Rotates all the impostors to the specified angle (virtually - it actually changes
//their materials to produce this same effect)
void ImpostorBatch::setAngle(Ogre::Real pitchDeg, Ogre::Real yawDeg)
{
	// Calculate pitch material index
	int newPitchIndex = 0;
#ifdef IMPOSTOR_RENDER_ABOVE_ONLY
	if (pitchDeg > 0)
   {
		float maxPitchIndexDeg = (90.0f * (IMPOSTOR_PITCH_ANGLES-1)) / IMPOSTOR_PITCH_ANGLES;
		newPitchIndex = (int)(IMPOSTOR_PITCH_ANGLES * (pitchDeg / maxPitchIndexDeg));
		if (newPitchIndex > IMPOSTOR_PITCH_ANGLES-1) newPitchIndex = IMPOSTOR_PITCH_ANGLES-1;
	}
#else
	float minPitchIndexDeg = -90.0f;
	float maxPitchIndexDeg = ((180.0f * (IMPOSTOR_PITCH_ANGLES-1)) / IMPOSTOR_PITCH_ANGLES) - 90.0f;
	newPitchIndex = (int)(IMPOSTOR_PITCH_ANGLES * ((pitchDeg - minPitchIndexDeg) / (maxPitchIndexDeg - minPitchIndexDeg)));
	if (newPitchIndex > IMPOSTOR_PITCH_ANGLES-1) newPitchIndex = IMPOSTOR_PITCH_ANGLES-1;
	if (newPitchIndex < 0) newPitchIndex = 0;
#endif
	
	// Calculate yaw material index
   int newYawIndex = yawDeg > 0 ? int(IMPOSTOR_YAW_ANGLES * (yawDeg / 360.0f) + 0.5f) % IMPOSTOR_YAW_ANGLES :
      int(IMPOSTOR_YAW_ANGLES + IMPOSTOR_YAW_ANGLES * (yawDeg / 360.0f) + 0.5f) % IMPOSTOR_YAW_ANGLES;
	
	// Change materials if necessary
	if (newPitchIndex != pitchIndex || newYawIndex != yawIndex)
   {
		pitchIndex = newPitchIndex;
		yawIndex = newYawIndex;
		bbset->setMaterial(m_pTexture->material[pitchIndex][yawIndex]->getName());
	}
}

void ImpostorBatch::setBillboardOrigin(Ogre::BillboardOrigin origin)
{
	bbset->setBillboardOrigin(origin);

	if (bbset->getBillboardOrigin() == Ogre::BBO_CENTER)
		entityBBCenter = m_pTexture->entityCenter;
	else if (bbset->getBillboardOrigin() == Ogre::BBO_BOTTOM_CENTER)
		entityBBCenter = Ogre::Vector3(m_pTexture->entityCenter.x, m_pTexture->entityCenter.y - m_pTexture->entityRadius, m_pTexture->entityCenter.z);
}

Ogre::String ImpostorBatch::generateEntityKey(Ogre::Entity *entity)
{
	Ogre::StringUtil::StrStreamType entityKey;
	entityKey << entity->getMesh()->getName();
	for (unsigned int i = 0; i < entity->getNumSubEntities(); ++i)
   {
		entityKey << "-" << entity->getSubEntity(i)->getMaterialName();
	}
	entityKey << "-" << IMPOSTOR_YAW_ANGLES << "_" << IMPOSTOR_PITCH_ANGLES;
#ifdef IMPOSTOR_RENDER_ABOVE_ONLY
	entityKey << "_RAO";
#endif
	return entityKey.str();
}

//-------------------------------------------------------------------------------------


ImpostorTextureResourceLoader::ImpostorTextureResourceLoader(ImpostorTexture& impostorTexture)
: texture(impostorTexture)
{
}

void ImpostorTextureResourceLoader::loadResource (Ogre::Resource *resource)
{
	if (resource->getLoadingState() == Ogre::Resource::LOADSTATE_UNLOADED) {
		texture.regenerate();
	}
}

//-------------------------------------------------------------------------------------


std::map<Ogre::String, ImpostorTexture *> ImpostorTexture::selfList;
unsigned long ImpostorTexture::GUID = 0;

//Do not use this constructor yourself - instead, call getTexture()
//to get/create an ImpostorTexture for an Entity.
ImpostorTexture::ImpostorTexture(ImpostorPage *group, Ogre::Entity *entity) :
loader(0)
{
	//Store scene manager and entity
   ImpostorTexture::sceneMgr = group->getParentPagedGeometry()->getSceneManager();
	ImpostorTexture::entity = entity;
	ImpostorTexture::group = group;

	//Add self to list of ImpostorTexture's
	entityKey = ImpostorBatch::generateEntityKey(entity);
	typedef std::pair<Ogre::String, ImpostorTexture *> ListItem;
	selfList.insert(ListItem(entityKey, this));
	
	//Calculate the entity's bounding box and it's diameter
	boundingBox = entity->getBoundingBox();

	entityRadius = Ogre::Math::boundingRadiusFromAABB(boundingBox);
	entityDiameter = 2.0f * entityRadius;
	entityCenter = boundingBox.getCenter();
	
	//Render impostor textures
	renderTextures(false);
	
	//Set up materials
	for (int o = 0; o < IMPOSTOR_YAW_ANGLES; ++o){
	for (int i = 0; i < IMPOSTOR_PITCH_ANGLES; ++i){
		material[i][o] = Ogre::MaterialManager::getSingleton().create(getUniqueID("ImpostorMaterial"), "Impostors");

		Ogre::Material *m = material[i][o].getPointer();
		Ogre::Pass *p = m->getTechnique(0)->getPass(0);
		
		Ogre::TextureUnitState *t = p->createTextureUnitState(texture->getName());
		
		t->setTextureUScroll((float)o / IMPOSTOR_YAW_ANGLES);
		t->setTextureVScroll((float)i / IMPOSTOR_PITCH_ANGLES);

		p->setLightingEnabled(false);
		m->setReceiveShadows(false);
		
		if (group->getBlendMode() == ALPHA_REJECT_IMPOSTOR){
			p->setAlphaRejectSettings(Ogre::CMPF_GREATER_EQUAL, 128);
			//p->setAlphaRejectSettings(CMPF_GREATER_EQUAL, 64);
		} else if (group->getBlendMode() == ALPHA_BLEND_IMPOSTOR){
			p->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
			p->setDepthWriteEnabled(false);  
		}
	}
	}
}

void ImpostorTexture::updateMaterials()
{
	for (int o = 0; o < IMPOSTOR_YAW_ANGLES; ++o){
		for (int i = 0; i < IMPOSTOR_PITCH_ANGLES; ++i){
			Ogre::Material *m = material[i][o].getPointer();
			Ogre::Pass *p = m->getTechnique(0)->getPass(0);

			Ogre::TextureUnitState *t = p->getTextureUnitState(0);

			t->setTextureName(texture->getName());
		}
	}
}

ImpostorTexture::~ImpostorTexture()
{
	//Delete textures
	assert(!texture.isNull());
	Ogre::String texName(texture->getName());
		
	texture.setNull();
	if (Ogre::TextureManager::getSingletonPtr())
		Ogre::TextureManager::getSingleton().remove(texName);
	
	//Delete materials
	for (int o = 0; o < IMPOSTOR_YAW_ANGLES; ++o){
	for (int i = 0; i < IMPOSTOR_PITCH_ANGLES; ++i){
		assert (!material[i][o].isNull());
		Ogre::String matName(material[i][o]->getName());

		material[i][o].setNull();
		if (Ogre::MaterialManager::getSingletonPtr())
			Ogre::MaterialManager::getSingleton().remove(matName);
	}
	}
	
	//Remove self from list of ImpostorTexture's
	selfList.erase(entityKey);
}

void ImpostorTexture::regenerate()
{
	assert(!texture.isNull());
	Ogre::String texName(texture->getName());
	texture.setNull();
	if (Ogre::TextureManager::getSingletonPtr())
		Ogre::TextureManager::getSingleton().remove(texName);

	renderTextures(true);
	updateMaterials();
}

void ImpostorTexture::regenerateAll()
{
	std::map<Ogre::String, ImpostorTexture *>::iterator iter;
	for (iter = selfList.begin(); iter != selfList.end(); ++iter){
		iter->second->regenerate();
	}
}

void ImpostorTexture::renderTextures(bool force)
{
#ifdef IMPOSTOR_FILE_SAVE
	Ogre::TexturePtr renderTexture;
#else
	TexturePtr renderTexture(texture);
	//if we're not using a file image we need to set up a resource loader, so that the texture is regenerated if it's ever unloaded (such as switching between fullscreen and the desktop in win32)
	loader = std::auto_ptr<ImpostorTextureResourceLoader>(new ImpostorTextureResourceLoader(*this));
#endif
	Ogre::RenderTexture *renderTarget;
	Ogre::Camera *renderCamera;
	Ogre::Viewport *renderViewport;
	Ogre::SceneNode *camNode;

	//Set up RTT texture
   Ogre::uint textureSize = ImpostorPage::getImpostorResolution();
	if (renderTexture.isNull())
   {
	renderTexture = Ogre::TextureManager::getSingleton().createManual(getUniqueID("ImpostorTexture"), "Impostors",
				Ogre::TEX_TYPE_2D, textureSize * IMPOSTOR_YAW_ANGLES, textureSize * IMPOSTOR_PITCH_ANGLES, 0, Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET, loader.get());
	}
	renderTexture->setNumMipmaps(Ogre::MIP_UNLIMITED);
	
	//Set up render target
	renderTarget = renderTexture->getBuffer()->getRenderTarget(); 
	renderTarget->setAutoUpdated(false);
	
	//Set up camera
	camNode = sceneMgr->getSceneNode("ImpostorPage::cameraNode");
	renderCamera = sceneMgr->createCamera(getUniqueID("ImpostorCam"));
	camNode->attachObject(renderCamera);
	renderCamera->setLodBias(1000.0f);
	renderViewport = renderTarget->addViewport(renderCamera);
	renderViewport->setOverlaysEnabled(false);
	renderViewport->setClearEveryFrame(true);
	renderViewport->setShadowsEnabled(false);
	renderViewport->setBackgroundColour(ImpostorPage::getImpostorBackgroundColor());
	
	//Set up scene node
	Ogre::SceneNode* node = sceneMgr->getSceneNode("ImpostorPage::renderNode");
	
	Ogre::SceneNode* oldSceneNode = entity->getParentSceneNode();
	if (oldSceneNode) {
		oldSceneNode->detachObject(entity);
	}

	Ogre::SceneNode *n1= node->createChildSceneNode();
	n1->attachObject(entity);
	n1->setPosition(-entityCenter + Ogre::Vector3(10,0,10));

	Ogre::Entity *e2 = entity->clone(entity->getName() + "_clone");
	Ogre::SceneNode *n2= node->createChildSceneNode();
	n2->attachObject(e2);
	n2->setPosition(-entityCenter + Ogre::Vector3(10,0,10));
	
	//Set up camera FOV
	const Ogre::Real objDist = entityRadius * 100;
	const Ogre::Real nearDist = objDist - (entityRadius + 1); 
	const Ogre::Real farDist = objDist + (entityRadius + 1);
	
	renderCamera->setAspectRatio(1.0f);
	renderCamera->setFOVy(Ogre::Math::ATan(entityDiameter / objDist));
	renderCamera->setNearClipDistance(nearDist);
	renderCamera->setFarClipDistance(farDist);
	
	//Disable mipmapping (without this, masked textures look bad)
	Ogre::MaterialManager *mm = Ogre::MaterialManager::getSingletonPtr();
	Ogre::FilterOptions oldMinFilter = mm->getDefaultTextureFiltering(Ogre::FT_MIN);
	Ogre::FilterOptions oldMagFilter = mm->getDefaultTextureFiltering(Ogre::FT_MAG);
	Ogre::FilterOptions oldMipFilter = mm->getDefaultTextureFiltering(Ogre::FT_MIP);
	mm->setDefaultTextureFiltering(Ogre::FO_POINT, Ogre::FO_LINEAR, Ogre::FO_NONE);

	//Disable fog
	Ogre::FogMode oldFogMode = sceneMgr->getFogMode();
	Ogre::ColourValue oldFogColor = sceneMgr->getFogColour();
	Ogre::Real oldFogDensity = sceneMgr->getFogDensity();
	Ogre::Real oldFogStart = sceneMgr->getFogStart();
	Ogre::Real oldFogEnd = sceneMgr->getFogEnd();
	sceneMgr->setFog(Ogre::FOG_NONE);
	
	// Get current status of the queue mode
	Ogre::SceneManager::SpecialCaseRenderQueueMode OldSpecialCaseRenderQueueMode = sceneMgr->getSpecialCaseRenderQueueMode();
	//Only render the entity
	sceneMgr->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_INCLUDE); 
   sceneMgr->addSpecialCaseRenderQueue(group->getParentPagedGeometry()->getRenderQueue() + 1);

	Ogre::uint8 oldRenderQueueGroup = entity->getRenderQueueGroup();
	entity->setRenderQueueGroup(group->getParentPagedGeometry()->getRenderQueue() + 1);
	e2->setRenderQueueGroup(entity->getRenderQueueGroup());
	bool oldVisible = entity->getVisible();
	entity->setVisible(true);
	e2->setVisible(true);
   Ogre::Real oldMaxDistance = entity->getRenderingDistance();
	entity->setRenderingDistance(0);
	e2->setRenderingDistance(0);

	bool needsRegen = true;
#ifdef IMPOSTOR_FILE_SAVE
	//Calculate the filename hash used to uniquely identity this render
	Ogre::String strKey = entityKey;
	char key[32] = {0};
	Ogre::uint32 i = 0;
	for (Ogre::String::const_iterator it = entityKey.begin(); it != entityKey.end(); ++it)
	{
		key[i] ^= *it;
		i = (i+1) % sizeof(key);
	}
	for (i = 0; i < sizeof(key); ++i)
		key[i] = (key[i] % 26) + 'A';

	Ogre::String tempdir = this->group->getParentPagedGeometry()->getTempdir();
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(tempdir, "FileSystem", "BinFolder");

	Ogre::String fileNamePNG = "Impostor." + Ogre::String(key, sizeof(key)) + '.' + Ogre::StringConverter::toString(textureSize) + ".png";
	Ogre::String fileNameDDS = "Impostor." + Ogre::String(key, sizeof(key)) + '.' + Ogre::StringConverter::toString(textureSize) + ".dds";

	//Attempt to load the pre-render file if allowed
	needsRegen = force;
	if (!needsRegen){
		try{
			texture = Ogre::TextureManager::getSingleton().load(fileNameDDS, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, Ogre::MIP_UNLIMITED);
		}
		catch (...){
			try{
				texture = Ogre::TextureManager::getSingleton().load(fileNamePNG, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, Ogre::MIP_UNLIMITED);
			}
			catch (...){
				needsRegen = true;
			}
		}
	}
#endif

	if (needsRegen){
		//If this has not been pre-rendered, do so now
		const float xDivFactor = 1.0f / IMPOSTOR_YAW_ANGLES;
		const float yDivFactor = 1.0f / IMPOSTOR_PITCH_ANGLES;
		for (int o = 0; o < IMPOSTOR_PITCH_ANGLES; ++o){ //4 pitch angle renders
#ifdef IMPOSTOR_RENDER_ABOVE_ONLY
			Ogre::Radian pitch = Ogre::Degree((90.0f * o) * yDivFactor); //0, 22.5, 45, 67.5
#else
			Ogre::Radian pitch = Degree((180.0f * o) * yDivFactor - 90.0f);
#endif

			for (int i = 0; i < IMPOSTOR_YAW_ANGLES; ++i){ //8 yaw angle renders
				Ogre::Radian yaw = Ogre::Degree((360.0f * i) * xDivFactor); //0, 45, 90, 135, 180, 225, 270, 315
					
				//Position camera
				camNode->setPosition(0, 0, 0);
                camNode->setOrientation(Ogre::Quaternion(yaw, Ogre::Vector3::UNIT_Y) * Ogre::Quaternion(-pitch, Ogre::Vector3::UNIT_X));
                camNode->translate(Ogre::Vector3(0, 0, objDist), Ogre::Node::TS_LOCAL);
						
				//Render the impostor
				renderViewport->setDimensions((float)(i) * xDivFactor, (float)(o) * yDivFactor, xDivFactor, yDivFactor);
				renderTarget->update();
			}
		}
	
#ifdef IMPOSTOR_FILE_SAVE
		//Save RTT to file with respecting the temp dir
		renderTarget->writeContentsToFile(tempdir + fileNamePNG);

		//Load the render into the appropriate texture view
		texture = Ogre::TextureManager::getSingleton().load(fileNamePNG, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, Ogre::MIP_UNLIMITED);
#else
		texture = renderTexture;
#endif
	}
	

	entity->setVisible(oldVisible);
	entity->setRenderQueueGroup(oldRenderQueueGroup);
	entity->setRenderingDistance(oldMaxDistance);

	sceneMgr->destroyEntity(e2);

	sceneMgr->removeSpecialCaseRenderQueue(group->getParentPagedGeometry()->getRenderQueue() + 1);
	// Restore original state
	sceneMgr->setSpecialCaseRenderQueueMode(OldSpecialCaseRenderQueueMode); 

	//Re-enable mipmapping
	mm->setDefaultTextureFiltering(oldMinFilter, oldMagFilter, oldMipFilter);

	//Re-enable fog
	sceneMgr->setFog(oldFogMode, oldFogColor, oldFogDensity, oldFogStart, oldFogEnd);

	//Delete camera
	renderTarget->removeViewport(0);
	renderCamera->getSceneManager()->destroyCamera(renderCamera);
	
	//Delete scene node
	node->detachAllObjects();
	n2->detachAllObjects();
	n1->detachAllObjects();
	node->removeAndDestroyAllChildren();
	if (oldSceneNode) {
		oldSceneNode->attachObject(entity);
	}

#ifdef IMPOSTOR_FILE_SAVE
	//Delete RTT texture
	assert(!renderTexture.isNull());
	Ogre::String texName2(renderTexture->getName());

	renderTexture.setNull();
	if (Ogre::TextureManager::getSingletonPtr())
		Ogre::TextureManager::getSingleton().remove(texName2);
#endif
}

Ogre::String ImpostorTexture::removeInvalidCharacters(Ogre::String s)
{
	Ogre::StringUtil::StrStreamType s2;

	for (Ogre::uint32 i = 0; i < s.length(); ++i){
		char c = s[i];
		if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '\"' || c == '<' || c == '>' || c == '|'){
			s2 << '-';
		} else {
			s2 << c;
		}
	}

	return s2.str();
}

void ImpostorTexture::removeTexture(ImpostorTexture* Texture)
{
	//Search for an existing impostor texture, in case it was already deleted
	for(std::map<Ogre::String, ImpostorTexture *>::iterator iter=selfList.begin();
		iter!=selfList.end(); ++iter)
	{
		if(iter->second==Texture)
		{
			delete Texture;
			return;
		}
	}
	// no need to anything if it was not found, chances are that it was already deleted
}

ImpostorTexture *ImpostorTexture::getTexture(ImpostorPage *group, Ogre::Entity *entity)
{
	//Search for an existing impostor texture for the given entity
	Ogre::String entityKey = ImpostorBatch::generateEntityKey(entity);
	std::map<Ogre::String, ImpostorTexture *>::iterator iter;
	iter = selfList.find(entityKey);
	
	//If found..
	if (iter != selfList.end()){
		//Return it
		return iter->second;		
	} else {
		if (group){
			//Otherwise, return a new texture
			return (new ImpostorTexture(group, entity));
		} else {
			//But if group is null, return null
			return NULL;
		}
	}
}
