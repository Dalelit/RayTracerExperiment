#pragma once

#include "stdafx.h"
#include "Materials.h"

struct Sphere
{
	int id;
	XMVECTOR centre;
	float radius;
	float radiusSqrd;
	Material material;
};

struct Plane
{
	int id;
	XMVECTOR normal;
	XMVECTOR point;
	Material material;
};

struct Disc
{
	int id;
	XMVECTOR normal;
	XMVECTOR centre;
	float radius;
	float radiusSqrd;
	Material material;
};

struct TraingleIndx
{
	UINT32 v0;
	UINT32 v1;
	UINT32 v2;
};

struct TriangleArray
{
	int id;
	int verticesCount;
	XMVECTOR* vertices;
	int trianglesCount;
	TraingleIndx* triangles;
	Material material;
};

