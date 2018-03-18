#include "stdafx.h"
#include "Camera.h"




void InitialiseCamera(Camera* camera, XMVECTOR up, XMVECTOR position, XMVECTOR lookingAt, float aspectRatio)
{
	camera->up = up;
	camera->position = position;
	camera->lookingAt = lookingAt;
	camera->direction = XMVector3Normalize(camera->lookingAt - camera->position);
	camera->viewPortDistance = 1.0f;
	camera->viewPortWidth = 2.0f;
	camera->viewPortHeight = camera->viewPortWidth * aspectRatio;

	camera->vpCentre = (camera->viewPortDistance * camera->direction) + camera->position;
	camera->vpXDir = XMVector3Normalize(XMVector3Cross(camera->up, camera->direction));
	camera->vpYDir = XMVector3Normalize(XMVector3Cross(camera->direction, camera->vpXDir));
}


void InitialiseTestCamera1(Camera* camera, float aspectRatio)
{
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR position = XMVectorSet(0.0f, 1.8f, -7.0f, 0.0f);
	XMVECTOR lookingAt = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	InitialiseCamera(camera,
		up,
		position,
		lookingAt,
		aspectRatio);
}

void InitialiseTestCamera2(Camera* camera, float aspectRatio)
{
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR position = XMVectorSet(0.0f, 1.1f, -3.0f, 0.0f);
	XMVECTOR lookingAt = XMVectorSet(0.0f, 1.1f, 0.0f, 0.0f);

	InitialiseCamera(camera,
		up,
		position,
		lookingAt,
		aspectRatio);
}

