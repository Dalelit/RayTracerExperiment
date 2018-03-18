#pragma once

#include "stdafx.h"

struct Camera
{
	XMVECTOR position;
	XMVECTOR lookingAt;
	XMVECTOR direction;
	float viewPortDistance;
	float viewPortWidth;
	float viewPortHeight;

	XMVECTOR up;
	XMVECTOR vpCentre;
	XMVECTOR vpYDir;
	XMVECTOR vpXDir;
};

void InitialiseCamera(Camera* camera, XMVECTOR up, XMVECTOR position, XMVECTOR lookingAt, float aspectRatio);

void InitialiseTestCamera1(Camera* camera, float aspectRatio);
void InitialiseTestCamera2(Camera* camera, float aspectRatio);

