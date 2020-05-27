#pragma once
#include <stdio.h>
#include "csc/csc_math.h"

static int ray_triangle_intersect (float const o[3], float const dir[3], float const v0[3], float const v1[3], float const v2[3], float * t0)
{
	float v0v1[3];
	float v0v2[3];
	float n[3];
	float NdotRayDirection;
	float d;
	float t;
	float kEpsilon = 0.01f;

	vvf32_sub (3, v0v1, v1, v0);
	vvf32_sub (3, v0v2, v2, v0);
	v3f32_cross (n, v0v1, v0v2);
	NdotRayDirection = vvf32_dot (3, n, dir);
	printf ("NdotRayDirection %f\n", NdotRayDirection);
	if (fabs(NdotRayDirection) < kEpsilon) // almost 0
	{
		return 0; // they are parallel so they don't intersect !
	}

	// compute d parameter using equation 2
	d = vvf32_dot (3, n, v0);

	// compute t (equation 3)
	t = (vvf32_dot (3, n, o) + d) / NdotRayDirection;
	if (t < 0)
	{
		return 0; // the triangle is behind
	}

	// compute the intersection point using equation 1
	//Vec3f P = orig + t * dir;
	float p[3];
	vsf32_mul (3, p, dir, t);
	vvf32_add (3, p, p, o);

	// Step 2: inside-outside test
	float c[3]; // vector perpendicular to triangle's plane


	float edge0[3];
	float vp0[3];
	vvf32_sub (3, edge0, v1, v0);
	vvf32_sub (3, vp0, p, v0);
	v3f32_cross (c, edge0, vp0);
	if (vvf32_dot(3, n, c) < 0)
	{
		return 0;// P is on the right side
	}

	float edge1[3];
	float vp1[3];
	vvf32_sub (3, edge1, v2, v1);
	vvf32_sub (3, vp1, p, v1);
	v3f32_cross (c, edge1, vp1);
	if (vvf32_dot(3, n, c) < 0)
	{
		return 0;// P is on the right side
	}

	float edge2[3];
	float vp2[3];
	vvf32_sub (3, edge2, v0, v2);
	vvf32_sub (3, vp2, p, v2);
	v3f32_cross (c, edge2, vp2);
	if (vvf32_dot(3, n, c) < 0)
	{
		return 0;// P is on the right side
	}

	return 1;


	/*
	// compute plane's normal
	Vec3f v0v1 = v1 - v0;
	Vec3f v0v2 = v2 - v0;
	// no need to normalize
	Vec3f N = v0v1.crossProduct(v0v2); // N
	float area2 = N.length();

	// Step 1: finding P

	// check if ray and plane are parallel ?
	float NdotRayDirection = N.dotProduct(dir);
	if (fabs(NdotRayDirection) < kEpsilon) // almost 0
		return false; // they are parallel so they don't intersect !

	// compute d parameter using equation 2
	float d = N.dotProduct(v0);

	// compute t (equation 3)
	t = (N.dotProduct(orig) + d) / NdotRayDirection;
	// check if the triangle is in behind the ray
	if (t < 0) return false; // the triangle is behind

	// compute the intersection point using equation 1
	Vec3f P = orig + t * dir;

	// Step 2: inside-outside test
	Vec3f C; // vector perpendicular to triangle's plane

	// edge 0
	Vec3f edge0 = v1 - v0;
	Vec3f vp0 = P - v0;
	C = edge0.crossProduct(vp0);
	if (N.dotProduct(C) < 0) return false; // P is on the right side

	// edge 1
	Vec3f edge1 = v2 - v1;
	Vec3f vp1 = P - v1;
	C = edge1.crossProduct(vp1);
	if (N.dotProduct(C) < 0)  return false; // P is on the right side

	// edge 2
	Vec3f edge2 = v0 - v2;
	Vec3f vp2 = P - v2;
	C = edge2.crossProduct(vp2);
	if (N.dotProduct(C) < 0) return false; // P is on the right side;

	return true; // this ray hits the triangle
	*/
}
