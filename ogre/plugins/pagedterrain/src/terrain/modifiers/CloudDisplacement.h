#ifndef CLOUDDISPLACEMENT_H
#define CLOUDDISPLACEMENT_H

#include "Terrain.h"
#include "TerrainModifier.h"
#include <OgrePrerequisites.h>
#include <OgreVector3.h>
#include <OgreVector2.h>
#include <utilities/vectors/Vec2D.h>
#include <utilities/edge/Edge.h>
#include "DllRequisites.h"

class PAGEDTERRAIN_EXPORT CloudDisplacement : public TerrainModifier
{
public:
    CloudDisplacement(Terrain *t);

    //métodos de alteração do terreno
    void modify(const Ogre::Vector3 &point  ) ;

    //////////////////////////////////////////

    const Ogre::String getType() const {return "CloudDisplacement";}

    void displace(Vec2D<double> vTopLeft, double fWidth, Heightmap* pHeightmap, float fScale  ) const {  }

    void displaceTexture(Vec2D<double> vTopLeft, double fWidth, Ogre::TexturePtr pTex, int iChannel = 0) const {  }

    bool isInBounds(const Vec2D<double>& vMin, const Vec2D<double>& vMax) const { return true; }

    //////////////////////////////////////////////

private:
    Heightmap* mHeightmap;
    Terrain* mTerrain;

    double mfWidth;
    float mfScale;
};



#endif
