#pragma once

#include "stdafx.h"
#include <Windows.h> // TODO: only doing this for the UINT defs... remove later.

struct Material
{
	XMVECTOR colour; // albedo
	XMVECTOR emit; // emit colour
	XMVECTOR reflect; // refect colour
	//XMVECTOR specular;
	float scatter;

	//float ka = 0.8f; // ambient coefficient - basic colour with no light source
	//float kr = 0.3f; // reflection coefficient
	//float kd; // diffuse coefficient - with light source
	float ks; // specular coefficient
};

//XMVECTOR CreateColorVector(UINT8 r, UINT8 g, UINT8 b);

void InitialiseMaterial(Material* mat, UINT8 r, UINT8 g, UINT8 b, float ambient, float reflect, float specular);
void InitialiseMaterial(Material* mat, XMVECTOR colour, float ambient, float reflection, float specular);
