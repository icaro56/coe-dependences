#ifndef SELECTIONBOX_H
#define SELECTIONBOX_H

#include "OgreManualObject.h"

#include "DllCommunityRequisites.h"

class COMMUNITY_EXPORT SelectionBox : public Ogre::ManualObject
{
public:
	SelectionBox(const Ogre::String& name);
	~SelectionBox();

	void setCorners(float left, float top, float right, float bottom);
	void setCorners(const Ogre::Vector2& topLeft, const Ogre::Vector2& bottomRight);

private:
	
};


#endif