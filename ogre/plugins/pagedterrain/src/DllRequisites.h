#ifndef DLLREQUISITES_H
#define DLLREQUISITES_H

#ifdef PAGEDTERRAIN_EXPORTS
		#define PAGEDTERRAIN_EXPORT __declspec (dllexport)
#else
		#define PAGEDTERRAIN_EXPORT __declspec (dllimport)
#endif

#endif

