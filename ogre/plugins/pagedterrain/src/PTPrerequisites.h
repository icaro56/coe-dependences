//#ifndef PREREQUISITES_H
//#define PREREQUISITES_H

#pragma once

#include <OgrePlatform.h>
#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

#if (OGRE_PLATFORM == OGRE_PLATFORM_LINUX)
#include <cassert>
#endif

// desativa warning: conversion from 'size_t' to 'DWORD', possible loss of data
//#pragma warning(disable: 4267)
#include <string>

template<class TType>class Vec2D;

template<class TType>class Vec3D;

class Terrain;
class TerrainMesh;
class TerrainModifier;
class ObjectHandler;
class SceneObject;
class QuadTree;
class QNode;
class Heightmap;
class HeightmapRefinement;
class HeightmapGenerator;
class HeightmapReader;
class HeightmapRules;
class AtmosphericCubeMap;
class GPULightmapper;

#include "utilities/brush/Brush.h"
class Brush;
typedef Ogre::SharedPtr< Brush > BrushPtr;

#include <climits>


#define USE_SINGLETON 1
#define VERTEX_WIDTH 33
#define MAP_WIDTH (VERTEX_WIDTH+2)
#define USE_MORPH 1
#define MORPH_START 0.2f
#define MORPH_CONSTANT_ID 77
#define PARENT_UV_OFFSET_ID 99
#define EPS 0.0001
#define FLOAT_EQ(x,v) (((v - EPS) < x) && (x <( v + EPS)))
//#define MORPH_SPEED 1.0f
#define MAX_LOD 1
//#define SKIRT_LENGTH 700.0f
#define USE_DEBUG_DISPLAYS 0

//terreno de física ao usar data fica mais preciso quando heightmaptype é igual a float
#define USE_FLOAT_DATA 1
#if (USE_FLOAT_DATA == 1)
	typedef float HEIGHTMAPTYPE;
#define UNPACK_HEIGHT(h) (h)
#define PACK_HEIGHT(h) (static_cast<float>(h))
#else
	typedef unsigned short HEIGHTMAPTYPE;
#define UNPACK_HEIGHT(h) (static_cast<Ogre::Real>(h))
#define PACK_HEIGHT(h) (static_cast<HEIGHTMAPTYPE>(h))
#endif

const Ogre::Real MAX_INT = Ogre::Real(USHRT_MAX);

	template<typename T>
	inline T Lerp(T a, T b, T x)	{ return a + x * (b - a); }

//#endif