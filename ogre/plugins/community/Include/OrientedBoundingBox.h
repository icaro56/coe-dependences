#ifndef ORIENTED_BOUNDING_BOX
#define ORIENTED_BOUNDING_BOX

#include "DllCommunityRequisites.h"
#include <OgreSimpleRenderable.h>
#include <OgreAxisAlignedBox.h>

class COMMUNITY_EXPORT OrientedBoundingBox : public Ogre::SimpleRenderable
{
	Ogre::VertexData vertexes;
public:
	OrientedBoundingBox();
	void setupVertices(const Ogre::AxisAlignedBox& aab, const Ogre::Vector3& offset = Ogre::Vector3::ZERO);
	virtual ~OrientedBoundingBox();
	Ogre::Real getSquaredViewDepth(const Ogre::Camera*)const;
	Ogre::Real getBoundingRadius()const;
	virtual void getWorldTransforms (Ogre::Matrix4 *xform)const;

	void setColor(const Ogre::ColourValue& c);
	Ogre::ColourValue getColor();

private:
	Ogre::ColourValue color;
};

#endif