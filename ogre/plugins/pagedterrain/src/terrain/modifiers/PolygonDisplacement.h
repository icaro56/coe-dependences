/*#ifndef POLIGONDISPLACEMENT_H
#define POLIGONDISPLACEMENT_H

#include "Terrain.h"
#include "TerrainModifier.h"
#include <OgrePrerequisites.h>
#include <OgreVector3.h>
#include <OgreVector2.h>
#include <utilities/vectors/Vec2D.h>
#include <utilities/edge/Edge.h>
#include "DllRequisites.h"

class PAGEDTERRAIN_EXPORT PolygonDisplacement : public TerrainModifier
{
public:
    PolygonDisplacement(Terrain *t);

    //métodos de alteração do vetor de pontos
    void insert(const Ogre::Vector3& v  );

    void clear();
    /////////////////////////////////////////

    //métodos de alteração do terreno
    void modify(bool show = false ) ;

    void planefy(double verticalHeight, double diffXReal, double k3dToReal, bool show = false, bool softy = false);

    void undo(bool show = false);

    void redo(bool show = false);

    void drawManualObject();

     void setHeight(const Ogre::Vector3& v);

	float getDistance(const Ogre::Vector3 &v, bool &s);

    Ogre::SceneNode* getPlaneNode();
	float getVolume();
   //////////////////////////////////////////

    const Ogre::String getType() const {return "PolygonDisplacement";}

    void displace(Vec2D<double> vTopLeft, double fWidth, Heightmap* pHeightmap, float fScale  ) const {  }

    void displaceTexture(Vec2D<double> vTopLeft, double fWidth, Ogre::TexturePtr pTex, int iChannel = 0) const {  }

    bool isInBounds(const Vec2D<double>& vMin, const Vec2D<double>& vMax) const { return true; }

    //////////////////////////////////////////////

private:
    PointList mPoints;
    //MemList mUndo;
	ModifyList mUndo;
    //MemList mRedo;
	ModifyList mRedo;
    HeightList mHeight;
    Ogre::Vector2 mMin;
    Ogre::Vector2 mMax;

    Ogre::Image imgMaskAlpha;
    Ogre::TexturePtr texMaskAlpha;
    int mImageWidth;

    Heightmap* mHeightmap;
    Terrain* mTerrain;

    //auxiliares
    float getHeight(Ogre::Vector2 p);
    bool inside(Ogre::Vector2 p);
    void calculateMinMax();
    bool interceptEdgeByRightSide(Ogre::Vector2 start, Ogre::Vector2 end, Ogre::Vector2 point);
    void clearVertices();

    Ogre::ManualObject* mManualObject;
    Ogre::SceneNode* mNode;
    Ogre::SceneNode* verticesNodes[50];
    Ogre::SceneNode* planeNode;
    Ogre::Entity* verticesEntities[50];
    Ogre::Entity* planeEntity;

    double mfWidth;
    float mfScale;

	float totalVolume;
    float mPlane;
};



#endif
*/