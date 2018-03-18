#pragma once

#include "stdafx.h"
#include "Materials.h"
#include "Objects.h"
#include "Lights.h"


struct Config
{
	int samples = 4;
	int maxBounces = 4;
};

struct Stats
{
	UINT64 rayCount;
	UINT64 hitCount;
};

struct World
{
	Config config;
	Stats stats;

	Material background;

	Material fog;
	float fogStartDistance;
	float fogRange;
	float fogFullDistance;

	int planeCount;
	Plane* planes;

	int sphereCount;
	Sphere* spheres;

	int discCount;
	Disc* discs;

	int triangleArrayCount;
	TriangleArray* triangleArrays;

	int lightCount;
	Light* lights;
};

void InitialiseEmptyWorld(World* world);

void InitialiseTestWorld1(World* world);

void InitialiseTestWorld2(World* world);

void InitialiseRandomSphereWorld(World* world);
