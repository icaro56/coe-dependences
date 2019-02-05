#include "Edge.h"
Edge::Edge()
{
    start = Ogre::Vector2(0,0);
    end = Ogre::Vector2(0,0);
}

Edge::Edge(Ogre::Vector2 _start, Ogre::Vector2 _end)
{
    start = _start;
    end = _end;
}

bool Edge::interceptByRightSide(Ogre::Vector2 point)
{
    float yMin = std::min(start.y, end.y);
    float yMax = std::max(start.y, end.y);

    float y = point.y;
    float x = start.x + (y - start.y)*(end.x - start.x)/(end.y-start.y);

    bool isLeft = point.x < x;
    bool interceptSegment = y >= yMin && y <= yMax;

    return isLeft && interceptSegment;
}

float Edge::distanceToPoint(Ogre::Vector2 point, bool &inside)
{
    //vetor base
    Ogre::Vector2 base = end - start;
    //normaliza vetor base
    float baseLength = base.length();
    base.normalise();

    //vetor
    Ogre::Vector2 vec = point - start;
    //projeção =  vetor base * vetor
    float proj = base.x*vec.x + base.y*vec.y;

    //distância = módulo vetor - projeção
    float distance = vec.length() - proj;
    //std::cout << "distance is" << distance << "\n";
    if(Ogre::Math::Abs(proj) <= baseLength && distance <= 10)
        inside = true;
    else
        inside = false;

    return distance;

}