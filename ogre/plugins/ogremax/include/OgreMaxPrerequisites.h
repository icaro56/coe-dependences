#ifndef OGREMAXPREREQUISITES_H
#define OGREMAXPREREQUISITES_H
	#ifdef OGREMAX_EXPORTS
			#define OGREMAX_EXPORT __declspec (dllexport)
	#else
			#define OGREMAX_EXPORT __declspec (dllimport)
	#endif
#endif