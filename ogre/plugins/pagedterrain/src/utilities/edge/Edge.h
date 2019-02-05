#pragma once

#include <OgreVector2.h>

#include "DllRequisites.h"

class PAGEDTERRAIN_EXPORT Edge
{
    public:
        Edge();
        Edge(Ogre::Vector2 _start, Ogre::Vector2 _end);
        bool interceptByRightSide(Ogre::Vector2 point);
		float distanceToPoint(Ogre::Vector2, bool &inside);

    private:
        Ogre::Vector2 start;
        Ogre::Vector2 end;
};
