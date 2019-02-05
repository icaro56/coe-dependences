#ifndef SQUAREDISPLACEMENT_H
#define SQUAREDISPLACEMENT_H

#include "Terrain.h"
#include "TerrainModifier.h"
#include <OgrePrerequisites.h>
#include <OgreVector3.h>
#include <OgreVector2.h>
#include <utilities/vectors/Vec2D.h>
#include <utilities/edge/Edge.h>
#include "DllRequisites.h"

class PAGEDTERRAIN_EXPORT SquareDisplacement : public TerrainModifier
{
public:
    SquareDisplacement(Terrain *t);


    //métodos de alteração do terreno
    void modify(const Ogre::Vector3 &pivot, float inc = 1) ;

    //////////////////////////////////////////

    const Ogre::String getType() const {return "SquareDisplacement";}

    void displace(Vec2D<double> vTopLeft, double fWidth, Heightmap* pHeightmap, float fScale  ) const {  }

    void displaceTexture(Vec2D<double> vTopLeft, double fWidth, Ogre::TexturePtr pTex, int iChannel = 0) const {  }

    bool isInBounds(const Vec2D<double>& vMin, const Vec2D<double>& vMax) const { return true; }

    //////////////////////////////////////////////

private:
    PointList mPoints;
    Ogre::Vector2 mMin;
    Ogre::Vector2 mMax;

    Heightmap* mHeightmap;
    Terrain* mTerrain;

    //auxiliares
    void calculateMinMax();

    double mfWidth;
    float mfScale;
};
#endif
