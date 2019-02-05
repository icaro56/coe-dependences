#ifndef DLLCOMMUNITYREQUISITES_H
#define DLLCOMMUNITYREQUISITES_H

#ifdef COMMUNITY_EXPORTS
		#define COMMUNITY_EXPORT __declspec (dllexport)
#else
		#define COMMUNITY_EXPORT __declspec (dllimport)
#endif

#endif

