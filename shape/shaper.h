#pragma once

#include <glad.h>
#include "csc/csc_debug.h"
#include "csc/csc_math.h"



struct shaper
{
	void * memory;
	uint32_t capacity;
	uint32_t last;
	GLenum mode;
};


struct shape_square
{
	float p[4];
	float q[4];
};

void shape_square_make(struct shape_square * square, float v[])
{
	float s[] =
	{
	-1.0f,    -1.0f,     0.0f,
	 1.0f,    -1.0f,     0.0f,
	 1.0f,     1.0f,     0.0f,

	-1.0f,    -1.0f,     0.0f,
	 1.0f,     1.0f,     0.0f,
	-1.0f,     1.0f,     0.0f
	};
	qf32_rotate_vector1 (square->q, s+3*0);
	qf32_rotate_vector1 (square->q, s+3*1);
	qf32_rotate_vector1 (square->q, s+3*2);
	qf32_rotate_vector1 (square->q, s+3*3);
	qf32_rotate_vector1 (square->q, s+3*4);
	qf32_rotate_vector1 (square->q, s+3*5);
	vvf32_add (3, s+3*0, s+3*0, square->p);
	vvf32_add (3, s+3*1, s+3*1, square->p);
	vvf32_add (3, s+3*2, s+3*2, square->p);
	vvf32_add (3, s+3*3, s+3*3, square->p);
	vvf32_add (3, s+3*4, s+3*4, square->p);
	vvf32_add (3, s+3*5, s+3*5, square->p);
	vf32_cpy (3*6, v, s);
}



void shaper_init (struct shaper * s)
{
	ASSERT (sizeof (float) == sizeof (GLfloat));
	s->memory = malloc (s->capacity * sizeof (float) * 3);
	s->last = 0;
}

void shaper_add_square (struct shaper * s, float const t[])
{
	uint32_t n = 6;
	ASSERT (s->capacity >= s->last);
	if ((s->capacity - s->last) < n)
	{
		fprintf (stderr, "No capacity to add more vertices. Capacity = %i, Last = %i, n = %i. %s:%i\n", s->capacity, n, s->last, __FILE__, __LINE__);
		return;
	}
	float square[] =
	{
	0.0f,     0.0f,     0.0f,
	1.0f,     0.0f,     0.0f,
	1.0f,     1.0f,     0.0f,

	0.0f,     0.0f,     0.0f,
	1.0f,     1.0f,     0.0f,
	0.0f,     1.0f,     0.0f
	};
	if (t)
	{
		for (int i = 0; i < 6*3; i += 3)
		{
			vvf32_add (3, square + i, square + i, t);
		}
	}
	void * memory = s->memory + (s->last * sizeof (float) * 3);
	memcpy (memory, square, sizeof (square));
	s->last += n;
}


