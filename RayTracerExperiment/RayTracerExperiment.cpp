//////////////////////////////////////
// To do list
//
// - better random number functions



#include "stdafx.h"
#include "RayTracerExperiment.h"

using namespace DirectX;

#define A_SMALL_NUMBER 0.0001f // TO DO - what should this number be!

float DotProdToDegrees(float dotProd)
{
	return acosf(dotProd) / XM_PI * 180.0f;
}

/*void PrintXMFloat3(XMFLOAT3 v)
{
	printf("[%f, %f, %f]", v.x, v.y, v.z);
}*/

void PrintXMVector3F32(XMVECTOR v)
{
	printf("[%f, %f, %f]", XMVectorGetX(v), XMVectorGetY(v), XMVectorGetZ(v));
}

void PrintXMVectorMask(XMVECTOR v)
{
	printf("[%x, %x, %x]", XMVectorGetIntX(v), XMVectorGetIntY(v), XMVectorGetIntZ(v));
}

Pixel XMVECTORToPixel_RTE(XMVECTOR v)
{
	XMCOLOR c;
	XMStoreColor(&c, v);

	Pixel p;
	p.r = c.r;
	p.g = c.g;
	p.b = c.b;

	return p;
}

inline UINT32 XORshift32(UINT32* state)
{
	// https://en.wikipedia.org/wiki/Xorshift
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	UINT32 x = *state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	*state = x;
	return x;
}

inline XMVECTOR ClampMin(XMVECTOR v, XMVECTOR min)
{
	XMVECTOR mask = XMVectorGreater(v, min);
	return XMVectorSelect(min, v, mask);
}

inline XMVECTOR ClampMax(XMVECTOR v, XMVECTOR max)
{
	XMVECTOR mask = XMVectorLess(v, max);
	return XMVectorSelect(max, v, mask);
}

inline XMVECTOR Clamp(XMVECTOR v, XMVECTOR min, XMVECTOR max)
{
	return ClampMax( ClampMin(v, min) , max);
}

inline float RandomFloatZeroToOne_RTE(UINT32* state)
{
	return (float)XORshift32(state) / (float)UINT32_MAX;
}

inline float RandomFloatExcludingMax_RTE(UINT32* state, float min, float max)
{
	float r = RandomFloatZeroToOne_RTE(state); // random number
	return min + r * (max - min); // return adjusted for range
}

inline float RandomFloatBilateral_RTE(UINT32* state)
{
	float r = RandomFloatZeroToOne_RTE(state); // random number
	return -1.0f + r * 2.0f; // return adjusted for range
}

inline XMVECTOR RandomXMVECTORBilateral_RTE(UINT32* state)
{
	return XMVectorSet(RandomFloatBilateral_RTE(state),
						RandomFloatBilateral_RTE(state),
						RandomFloatBilateral_RTE(state),
						0.0f);
}

inline XMVECTOR Lerp_RTE(XMVECTOR v1, XMVECTOR v2, XMVECTOR lerp)
{
	return v1 * (g_XMOne - lerp) + v2 * lerp;
}

#define MAX_FOG_ODDS 0.25f

inline XMVECTOR ApplyFullFog(Fragment* fragment, Material* hitMaterial)
{
	if (RandomFloatZeroToOne_RTE(&fragment->seed) < MAX_FOG_ODDS)
	{
		return fragment->world->fog.colour;
	}
	else
	{
		return hitMaterial->colour;
	}
}

inline XMVECTOR ApplyFog(Fragment* fragment, HitInfo* hitInfo)
{
	if (hitInfo->distance > fragment->world->fogFullDistance)
	{
		return ApplyFullFog(fragment, hitInfo->material);
	}
	else // in fog range... assuming you wouldn't call this if not!
	{
		float fogChance = (hitInfo->distance - fragment->world->fogStartDistance) / fragment->world->fogRange; // 0-1 range within fog starting zone
		fogChance *= MAX_FOG_ODDS; // scale to make range align to max fog chance

		if (RandomFloatZeroToOne_RTE(&fragment->seed) < fogChance)
		{
			return fragment->world->fog.colour;
		}
		else
		{
			return hitInfo->material->colour;
		}
	}
}

__forceinline bool TraceWorld(World* world, Ray* ray, HitInfo* hitInfo)
{
	bool hit = false;
	hitInfo->distance = FLT_MAX;

	/////////////////////////////////
	// plane intersections
	{
		Plane* plane = world->planes;
		for (int i = 0; i < world->planeCount; i++)
		{
			float denom = XMVectorGetX(XMVector3Dot(plane->normal, ray->direction));

			if (fabs(denom) > A_SMALL_NUMBER)
			{
				float tval = XMVectorGetX(XMVector3Dot((plane->point - ray->origin), plane->normal)) / denom;

				if (tval >= 0.0f && tval < hitInfo->distance)
				{
					hit = true;
					hitInfo->distance = tval;
					hitInfo->material = &plane->material;
					hitInfo->position = ray->origin + tval * ray->direction;
					hitInfo->normal   = plane->normal;
					hitInfo->hitId    = plane->id;
				}
			}

			plane++;
		}
	}

	/////////////////////////////////
	// sphere intersections
	{
		Sphere* sphere = world->spheres;
		for (int i = 0; i < world->sphereCount; i++)
		{
			XMVECTOR l = ray->origin - sphere->centre;

			float a = XMVectorGetX(XMVector3Dot(ray->direction, ray->direction));
			float b = 2 * XMVectorGetX(XMVector3Dot(ray->direction, l));
			float c = XMVectorGetX(XMVector3Dot(l, l)) - sphere->radiusSqrd;

			// solve quadratic
			float x0, x1;
			float discr = b * b - 4 * a * c;
			if (discr >= 0.0f)
			{
				if (discr == 0.0f) x0 = x1 = -0.5f * b / a;
				else // (discr > 0.0f)
				{
					float q = (b > 0.0f) ?
						-0.5f * (b + sqrtf(discr)) :
						-0.5f * (b - sqrtf(discr));
					x0 = q / a;
					x1 = c / q;
				}

				float tval = (x0 < x1) ? x0 : x1; // closest value
				if (tval < 0.0f) tval = x1; // but not if it's less than zero

				if (tval > 0.0f && tval < hitInfo->distance)
				{
					hit = true;
					hitInfo->distance = tval;
					hitInfo->material = &sphere->material;
					hitInfo->position = ray->origin + tval * ray->direction;
					hitInfo->normal   = XMVector3Normalize(hitInfo->position - sphere->centre);
					hitInfo->hitId    = sphere->id;
				}
			}

			sphere++;
		}
	}

	/////////////////////////////////
	// disc intersections
	{
		Disc* disc = world->discs;
		for (int i = 0; i < world->discCount; i++)
		{
			float denom = XMVectorGetX(XMVector3Dot(disc->normal, ray->direction));

			if (fabs(denom) > A_SMALL_NUMBER)
			{
				float tval = XMVectorGetX(XMVector3Dot((disc->centre - ray->origin), disc->normal)) / denom;

				// hit the plane
				if (tval >= 0 && tval < hitInfo->distance)
				{
					XMVECTOR pointOnPlane = ray->origin + tval * ray->direction;
					XMVECTOR pointToCentre = pointOnPlane - disc->centre;
					float pointToCentreDistSqrd = XMVectorGetX(XMVector3LengthSq(pointToCentre));

					// hit the plane within the radius of the disc
					if (pointToCentreDistSqrd <= disc->radiusSqrd)
					{
						hit = true;
						hitInfo->distance = tval;
						hitInfo->material = &disc->material;
						hitInfo->position = ray->origin + tval * ray->direction;
						hitInfo->normal   = disc->normal;
						hitInfo->hitId    = disc->id;
					}
				}
			}

			disc++;
		}
	}

	/////////////////////////////////
	// triangle intersections
	{
		TriangleArray* triArray = world->triangleArrays;
		for (int i = 0; i < world->triangleArrayCount; i++)
		{
			TraingleIndx* triIndx = triArray->triangles;
			for (int j = 0; j < triArray->trianglesCount; j++)
			{
				XMVECTOR v0 = triArray->vertices[triIndx->v0];
				XMVECTOR v1 = triArray->vertices[triIndx->v1];
				XMVECTOR v2 = triArray->vertices[triIndx->v2];
				XMVECTOR edge0 = XMVector3Normalize(v1 - v0);
				XMVECTOR edge1 = XMVector3Normalize(v2 - v1);
				XMVECTOR edge2 = XMVector3Normalize(v0 - v2);
				XMVECTOR normal = XMVector3Normalize(XMVector3Cross(edge0, -edge2));
				float denom = XMVectorGetX(XMVector3Dot(normal, ray->direction));

				if (fabs(denom) > A_SMALL_NUMBER)
				{
					float tval = XMVectorGetX(XMVector3Dot((v0 - ray->origin), normal)) / denom;

					if (tval >= 0.0f && tval < hitInfo->distance)
					{
						XMVECTOR hitPoint = ray->origin + tval * ray->direction;

						if (XMVectorGetX(XMVector3Dot(normal, XMVector3Cross(edge0, hitPoint - v0))) > 0.0f &&
							XMVectorGetX(XMVector3Dot(normal, XMVector3Cross(edge1, hitPoint - v1))) > 0.0f &&
							XMVectorGetX(XMVector3Dot(normal, XMVector3Cross(edge2, hitPoint - v2))) > 0.0f)
						{
							hit = true;
							hitInfo->distance = tval;
							hitInfo->material = &triArray->material;
							hitInfo->position = hitPoint;
							hitInfo->normal = normal;
							hitInfo->hitId = triArray->id;
						}
					}
				}

				triIndx++;
			}

			triArray++;
		}
	}

	return hit;
}

void TraceBitmap(Bitmap bitmap, World* world, Camera* camera)
{
	Fragment f;
	f.bitmap = bitmap;
	f.world = world;
	f.camera = camera;

	f.xStart = 0;
	f.yStart = 0;
	f.xUpTo = bitmap.width;
	f.yUpTo = bitmap.height;

	TraceFragment(&f);
}

void TraceFragment(Fragment* fragment) 
{
	// TODO: do I need to set these?
	fragment->stats.hitCount = 0;
	fragment->stats.rayCount = 0;

	// TODO: tidy up that the pixels may need to be shifted half...?
	//       maybe use a random unilateral from the corner?
	// TODO: no need to do for every fragment?
	float vpdW = fragment->camera->viewPortWidth / (float)fragment->bitmap.width;
	float vpdH = fragment->camera->viewPortHeight / (float)fragment->bitmap.height;
	float vpdWHalf = vpdW / 2.0f;
	float vpdHHalf = vpdH / 2.0f;

	// control variables
	float vpLeftStart = (-fragment->camera->viewPortWidth / 2.0f) + (fragment->xStart * vpdW);
	float vpy = (-fragment->camera->viewPortHeight / 2.0f) + (fragment->yStart * vpdH);
	float vpx;

	for (int bmpy = fragment->yStart; bmpy < fragment->yUpTo; bmpy++)
	{
		Pixel* pixel = fragment->bitmap.pixels + (bmpy * fragment->bitmap.width) + fragment->xStart;

		vpx = vpLeftStart;
		for (int bmpx = fragment->xStart; bmpx < fragment->xUpTo; bmpx++)
		{
			XMVECTOR samplePixel = g_XMZero;
			int sampleCount = fragment->world->config.samples;

			//if (!(bmpx % 200) || !(bmpy % 200) ) { samplePixel = Colors::Green; sampleCount = 0; }

			// create the ray
			XMVECTOR vpStartPosition = fragment->camera->vpCentre + fragment->camera->vpXDir * vpx + fragment->camera->vpYDir * vpy;

			while (sampleCount > 0)
			{
				// create sampling jittering
				float wJitter = RandomFloatExcludingMax_RTE(&fragment->seed, -vpdWHalf, vpdWHalf);
				float hJitter = RandomFloatExcludingMax_RTE(&fragment->seed, -vpdHHalf, vpdHHalf);
				XMVECTOR vpSampleJitter = XMVectorSet(wJitter, hJitter, 0.0f, 0.0f);
				XMVECTOR vpPosition = vpStartPosition + vpSampleJitter;

				Ray ray;
				ray.origin = fragment->camera->position; // TODO could this start from the viewport, rather than the eye?
				ray.direction = XMVector3Normalize(vpPosition - fragment->camera->position);

				int bounces = 0; // fragment->world->config.maxBounces;
				XMVECTOR attenuation = g_XMOne;
				XMVECTOR attenuationPrev = attenuation;

				while (bounces < fragment->world->config.maxBounces)
				{
					fragment->stats.rayCount++;

					HitInfo hitInfo;
					bool hit = TraceWorld(fragment->world, &ray, &hitInfo);

					if (hit)
					{
						fragment->stats.hitCount++;

						// Fog
						//if (hitInfo.distance > fragment->world->fogStartDistance)
						//{
						//	samplePixel += attenuation * ApplyFog(fragment, &hitInfo);
						//	bounces = fragment->world->config.maxBounces; // stop bouncing //TODO should we stop?
						//}
						//else
						{
							float distSqrInv = 1.0f;
							if (hitInfo.distance > 1.0f) distSqrInv = 1.0f / (hitInfo.distance * hitInfo.distance);

							// shading
							// I = ka * Ia + fatt * Ilight ( kd * cos + ks * cos to the nshiny )

							// mat colour is the ambient, then add the light, then specular.
							// TODO: treating diffuse and ambient the same for now... fix this
							samplePixel += attenuation * hitInfo.material->colour;
							samplePixel += attenuation * hitInfo.material->emit;
							samplePixel += distSqrInv * hitInfo.material->emit * attenuationPrev;

							// Lighting
							/*Light* light = fragment->world->lights;

							for (int l = 0; l < fragment->world->lightCount; l++)
							{
								// TODO: no shadows
								// TODO: distance?
								XMVECTOR vToLight = light->position - hitInfo.position;
								XMVECTOR distToLight = XMVector3Length(vToLight);
								XMVECTOR distToLightSqrd = distToLight * distToLight;
								XMVECTOR coslight = XMVector3Dot(XMVector3Normalize(vToLight), hitInfo.normal) / distToLightSqrd;
								coslight = ClampMin(coslight, g_XMZero);

								samplePixel += attenuation * hitInfo.material->reflect * light->colour * coslight;

								float specular = pow(XMVectorGetX(coslight), hitInfo.material->ks);
								samplePixel += attenuation * hitInfo.material->colour * specular;
								//samplePixel += attenuation * hitInfo.material->specular * coslight * coslight; // / (distToLight * distToLight);

								light++;
							}*/


							// update attenuation for next bounce
							XMVECTOR facingRatio = XMVector3Dot(hitInfo.normal, -ray.direction);
							facingRatio = ClampMin(facingRatio, g_XMZero);
							attenuationPrev = attenuation;
							attenuation *= hitInfo.material->reflect *facingRatio;

							// update the ray for the bounce
							// reflected around the normal
							ray.origin = hitInfo.position;
							ray.direction = XMVector3Reflect(ray.direction, hitInfo.normal); // 2 * XMVector3Dot(ray.direction, hitInfo.normal) * hitInfo.normal - ray.direction;
							ray.origin += ray.direction * A_SMALL_NUMBER; // TODO check why I need this, and if I play around with the hit distance when calulcating the hit point
						
							// Scatter if required, lerping between pure bounce and a random bounce
							if (hitInfo.material->scatter > 0.0f)
							{
								XMVECTOR randomReflect = XMVector3Normalize(hitInfo.normal + RandomXMVECTORBilateral_RTE(&fragment->seed));
								ray.direction = XMVector3Normalize(Lerp_RTE(ray.direction, randomReflect, g_XMOne * hitInfo.material->scatter));
							}

							bounces++;
						}

						// stop if attenuation is close enough to zero
						//if (XMVectorGetX(XMVector3LengthEst(attenuation)) < A_SMALL_NUMBER) bounces = fragment->world->config.maxBounces;

					}
					else
					{
						samplePixel += attenuation * fragment->world->background.colour;
						bounces = fragment->world->config.maxBounces;

						/* playing around with fog
						// FOG
						XMVECTOR forwardRatio = XMVector3Dot(fragment->camera->direction, ray.direction);
						forwardRatio = ClampMin(forwardRatio, g_XMZero);
						samplePixel += attenuation * forwardRatio * ApplyFullFog(fragment, &fragment->world->background);
						samplePixel += attenuation * (g_XMOne - forwardRatio) * fragment->world->background.colour;
						*/
					}
				}

				sampleCount--;
			}


			*pixel = XMVECTORToPixel_RTE(samplePixel / (float)fragment->world->config.samples);

			/////////////////////////////////
			// next pixel
			vpx += vpdW;
			pixel++;
		}

		vpy += vpdH;
	}
}


void RunQuickTest()
{
	XMVECTOR v1 = { 1.0f, 1.0f, 0.0f, 0.0f };
	XMVECTOR v2 = { 0.50f, 1.0f, 1.0f, 0.0f };

	v1 = XMVector3Normalize(v1);
	v2 = XMVector3Normalize(v2);

	printf("V1 ");
	PrintXMVector3F32(v1);
	printf(" V2 ");
	PrintXMVector3F32(v2);
	printf(" V1.2 ");
	PrintXMVector3F32(XMVector3Dot(v1, v2));
	printf(" V2.1 ");
	PrintXMVector3F32(XMVector3Dot(v2, v1));
	printf("\n");
}