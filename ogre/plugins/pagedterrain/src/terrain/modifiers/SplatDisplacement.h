#ifndef SPLATDISPLACEMENT_H_INCLUDED
#define SPLATDISPLACEMENT_H_INCLUDED

#include "Terrain.h"
#include "terrain/modifiers/TerrainModifier.h"
#include <OgrePrerequisites.h>
#include <OgreVector3.h>
#include <OgreVector2.h>
#include "utilities/vectors/Vec2D.h"
#include <vector>
#include "DllRequisites.h"

class PAGEDTERRAIN_EXPORT SplatDisplacement
{
    public:
        SplatDisplacement(Terrain* terrain, int textureWidth, const std::string passName);
        ~SplatDisplacement();

        void paintFree(const Ogre::Vector3 color , const Ogre::Vector2 origin, double iMapWidth, float radius, float opacity  = 1 ) ;
        void paintLine(const Ogre::Vector3 color , const Ogre::Vector2 origin, double iMapWidth, float radius, float opacity  = 1 ) ;
        void paintRectangle(const Ogre::Vector3 color , const Ogre::Vector2 origin, double iMapWidth, float radius, float opacity  = 1 ) ;
        void paintCircle(const Ogre::Vector3 color , const Ogre::Vector2 origin, double iMapWidth, float radius, float opacity  = 1 ) ;
        void erase(const Ogre::Vector2 origin, double iMapWidth, float radius, float opacity  = 1 ) ;
        void undo();
        void freeze();
        void redo();
        void clear();
        void setUndo();
        void setRedo();
        void save(const char* fileName);
        void setImage(const char* fileName, const char* dirName);
        void realocateUndo();
        void sendToLeft(Ogre::uchar* undo[], int pos, int init);
        Ogre::Vector2 getArrayPosition(Ogre::Vector2 origin, double fWidth, int imageTerrainWidth);
        void setInitPoint(Ogre::Vector2 origin);

		void setVisible(bool show);
    private:
        Terrain* mTerrain;
        Ogre::Image mUndo[10];
        int undoIdx;
        int currentIdx;

        Ogre::Vector2 initPoint;

        Ogre::Image imgCoverAlpha;
		std::string mPassName;
        Ogre::TexturePtr texCoverAlpha;
        int mImageWidth;
		bool visible;

};

#endif // SPLATDISPLACEMENT_H_INCLUDED
