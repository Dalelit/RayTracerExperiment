#pragma once

#include "stdafx.h"
#include <Windows.h>
#include "World.h"
#include "Lights.h"
#include "Camera.h"

struct Pixel
{
	UINT8 b;
	UINT8 g;
	UINT8 r;
	UINT8 a;
};

Pixel XMVECTORToPixel_RTE(XMVECTOR v);


struct Bitmap
{
	int height;
	int width;
	Pixel* pixels;
};

inline int BitMapArraySize(Bitmap bmp)
{
	return bmp.height * bmp.width * sizeof(Pixel);
}

struct Ray
{
	XMVECTOR origin;
	XMVECTOR direction;
};

struct HitInfo
{
	XMVECTOR position;
	XMVECTOR normal;
	float distance;
	Material* material;
	int hitId;
};

struct Fragment
{
	int xStart;
	int yStart;
	int xUpTo;
	int yUpTo;
	World* world;
	Camera* camera;
	Bitmap bitmap;

	UINT32 seed;

	Stats stats;
};

bool TraceWorld(World* world, Ray* ray, HitInfo* hitInfo);

void TraceFragment(Fragment* fragment);

void TraceBitmap(Bitmap bitmap, World* world, Camera* camera);

void PrintXMVector3F32(XMVECTOR v);

void RunQuickTest();
