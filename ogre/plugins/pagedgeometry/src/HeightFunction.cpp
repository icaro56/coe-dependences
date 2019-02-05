#include "HeightFunction.h"

namespace HeightFunction
{
    bool initialized = false;
    Ogre::RaySceneQuery* raySceneQuery;
    Ogre::Ray updateRay;
    MyRaySceneQueryListener *raySceneQueryListener;
    Terrain* mt;
}
