#include "stdafx.h"
#include "World.h"


void InitialiseEmptyWorld(World* world)
{
	world->background.colour = Colors::SkyBlue;

	world->planeCount = 0;
	world->sphereCount = 0;
	world->discCount = 0;
	world->triangleArrayCount = 0;
	world->lightCount = 0;
}

void InitialiseTestWorld1(World* world)
{
	world->background.colour = Colors::SkyBlue;

	world->fog.colour = Colors::LightGray * 0.5f;
	world->fogStartDistance = 50.0f;
	world->fogRange = 20.0f;
	world->fogFullDistance = world->fogStartDistance + world->fogRange;

	world->planeCount = 1;
	world->planes = (Plane*)malloc(sizeof(Plane) * world->planeCount);

	Plane* p = world->planes;
	p->id = 100;
	p->normal = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	p->point = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	InitialiseMaterial(&p->material, Colors::SandyBrown, 0.8f, 0.1f, 1.0f);
	p->material.reflect = Colors::White * 0.8f;
	p->material.scatter = 0.80f;


	world->sphereCount = 6;
	world->spheres = (Sphere*)malloc(sizeof(Sphere) * world->sphereCount);

	Sphere* s = world->spheres;
	s->id = 200;
	s->centre = { 0.0f, 0.0f, 0.0f, 0.0f };
	s->radius = 1.0f;
	s->radiusSqrd = s->radius * s->radius;
	InitialiseMaterial(&s->material, Colors::DarkBlue, 0.9f, 0.1f, 1.0f);
	//s->material.scatter = 0.9f;
	s++;
	s->id = 201;
	s->centre = XMVectorSet(2.0f, 2.0f, 5.0f, 0.0f);
	s->radius = 1.0f;
	s->radiusSqrd = s->radius * s->radius;
	InitialiseMaterial(&s->material, Colors::ForestGreen, 0.5f, 0.5f, 5.0f);
	//s->material.scatter = 0.2f;
	s++;
	s->id = 202;
	s->centre = XMVectorSet(-7.5f, 0.0f, 3.0f, 0.0f);
	s->radius = 1.5f;
	s->radiusSqrd = s->radius * s->radius;
	InitialiseMaterial(&s->material, Colors::Red, 0.8f, 0.1f, 1.0f);
	s->material.emit = Colors::Red;
	//s->material.scatter = 0.2f;
	s++;
	s->id = 203;
	s->centre = XMVectorSet(3.0f, 1.0f, -3.0f, 0.0f);
	s->radius = 0.5f;
	s->radiusSqrd = s->radius * s->radius;
	InitialiseMaterial(&s->material, Colors::White, 0.1f, 0.9f, 0.10f);
	//s->material.scatter = 0.2f;
	s++;
	s->id = 204;
	s->centre = XMVectorSet(-5.0f, 0.0f, 5.0f, 0.0f);
	s->radius = 1.0f;
	s->radiusSqrd = s->radius * s->radius;
	InitialiseMaterial(&s->material, Colors::Gray, 0.5f, 0.1f, 0.10f);
	s->material.scatter = 0.5f;
	s++;
	s->id = 205;
	s->centre = XMVectorSet(4.0f, 1.0f, 1.5f, 0.0f);
	s->radius = 0.5f;
	s->radiusSqrd = s->radius * s->radius;
	InitialiseMaterial(&s->material, Colors::Purple, 0.8f, 0.0f, 0.0f);
	s->material.emit = Colors::Purple;
	//s->material.scatter = 0.2f;

	world->discCount = 1;
	world->discs = (Disc*)malloc(sizeof(Disc) * world->discCount);

	Disc* d = world->discs;
	d->id = 300;
	d->centre = XMVectorSet(-3.0f, 3.0f, 1.5f, 0.0f);
	d->normal = XMVector3Normalize(-d->centre);
	d->radius = 1.5f;
	d->radiusSqrd = d->radius * d->radius;
	InitialiseMaterial(&d->material, Colors::MediumPurple, 0.8f, 0.9f, 1.0f);

	world->triangleArrayCount = 2;
	world->triangleArrays = (TriangleArray*)malloc(sizeof(TriangleArray) * world->triangleArrayCount);

	TriangleArray* ta = world->triangleArrays;
	ta->id = 400;
	ta->verticesCount = 3;
	ta->vertices = new XMVECTOR[3];
	ta->vertices[0] = { 1.0f, 0.2f, 1.1f, 0.0f };
	ta->vertices[1] = { 2.0f, 0.2f, 1.1f, 0.0f };
	ta->vertices[2] = { 1.5f, 1.2f, 1.1f, 0.0f };
	ta->trianglesCount = 1;
	ta->triangles = new TraingleIndx[1];
	ta->triangles[0].v0 = 0;
	ta->triangles[0].v1 = 1;
	ta->triangles[0].v2 = 2;
	InitialiseMaterial(&ta->material, Colors::Yellow, 0.9f, 0.01f, 1.0f);
	ta++;
	ta->id = 401;
	ta->verticesCount = 4;
	ta->vertices = new XMVECTOR[4];
	ta->vertices[0] = { -2.0f, 0.2f, 1.1f, 0.0f };
	ta->vertices[1] = { -1.0f, 0.2f, 1.1f, 0.0f };
	ta->vertices[2] = { -1.0f, 1.2f, 1.1f, 0.0f };
	ta->vertices[3] = { -2.0f, 1.2f, 1.1f, 0.0f };
	ta->trianglesCount = 2;
	ta->triangles = new TraingleIndx[2];
	ta->triangles[0] = { 0,1,2 };
	ta->triangles[1] = { 2,3,0 };
	InitialiseMaterial(&ta->material, Colors::Yellow, 0.9f, 0.01f, 1.0f);

	world->lightCount = 2;
	world->lights = (Light*)malloc(sizeof(Light) * world->lightCount);

	Light* l = world->lights;
	l->position = {10.0f, 15.0f, 15.0f, 0.0f};
	l->colour = Colors::White * 0.8f;
	l++;
	l->position = { -10.0f, 15.0f, -15.0f, 0.0f };
	l->colour = Colors::White * 0.8f;

}

void InitialiseTestWorld2(World* world)
{
	world->background.colour = Colors::SkyBlue;

	world->planeCount = 1;
	world->planes = (Plane*)malloc(sizeof(Plane) * world->planeCount);

	Plane* p = world->planes;
	p->id = 100;
	p->normal = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	p->point = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	InitialiseMaterial(&p->material, Colors::DarkBlue, 0.8f, 0.8f, 1.0f);
	//p->material.scatter = 0.2f;


	world->sphereCount = 1;
	world->spheres = (Sphere*)malloc(sizeof(Sphere) * world->sphereCount);

	Sphere* s = world->spheres;
	s->id = 203;
	s->centre = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	s->radius = 0.25f;
	s->radiusSqrd = s->radius * s->radius;
	InitialiseMaterial(&s->material, Colors::White, 0.1f, 0.9f, 0.10f);
	//s->material.scatter = 0.2f;

	world->discCount = 0;
	world->triangleArrayCount = 0;

	world->lightCount = 1;
	world->lights = (Light*)malloc(sizeof(Light) * world->lightCount);

	Light* l = world->lights;
	l->position = { 10.0f, 15.0f, 15.0f, 0.0f };
	l->colour = Colors::White * 0.8f;
}

void InitialiseRandomSphereWorld(World* world)
{
	world->background.colour = Colors::SkyBlue;

	world->planeCount = 0;
	world->discCount = 0;
	world->triangleArrayCount = 0;
	world->lightCount = 0;

	world->sphereCount = 1;
	world->spheres = (Sphere*)malloc(sizeof(Sphere) * world->sphereCount);

	Sphere* s = world->spheres;
	s->id = 203;
	s->centre = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	s->radius = 0.25f;
	s->radiusSqrd = s->radius * s->radius;
	InitialiseMaterial(&s->material, Colors::White, 0.1f, 0.9f, 0.10f);
	//s->material.scatter = 0.2f;

}
