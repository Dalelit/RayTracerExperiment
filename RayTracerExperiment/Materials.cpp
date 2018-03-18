#include "stdafx.h"
#include "Materials.h"

void InitialiseMaterial(Material* mat, UINT8 r, UINT8 g, UINT8 b, float ambient, float reflection, float specular)
{
	XMCOLOR colour;

	colour.r = r;
	colour.g = g;
	colour.b = b;

	InitialiseMaterial(mat, XMLoadColor(&colour), ambient, reflection, specular);
}

void InitialiseMaterial(Material* mat, XMVECTOR colour, float ambient, float reflection, float specular)
{
	//mat->ka = ambient;
	//mat->kr = reflection;
	mat->ks = specular;
	mat->scatter = 0.0f;

	mat->colour = ambient * colour;
	mat->reflect = reflection * colour;
	mat->emit = g_XMZero;
	//mat->specular = specular * colour;
}