#pragma once

#include <glad.h>
#include "csc/csc_debug.h"


struct shaper
{
	void * memory;
	uint32_t capacity;
	uint32_t last;
	GLenum mode;
};


void shaper_init (struct shaper * s)
{
	ASSERT (sizeof (float) == sizeof (GLfloat));
	s->memory = malloc (s->capacity * sizeof (float) * 3 * 3);
	s->last = 0;
}

void shaper_add_square (struct shaper * s)
{
	uint32_t n = 2;
	ASSERT (s->capacity >= s->last);
	if ((s->capacity - s->last) < n)
	{
		fprintf (stderr, "No capacity to add more vertices. Capacity = %i, Last = %i, n = %i. %s:%i\n", s->capacity, n, s->last, __FILE__, __LINE__);
		return;
	}
	const float square[] =
	{
	0.0f,     0.0f,     0.0f,
	1.0f,     0.0f,     0.0f,
	1.0f,     1.0f,     0.0f,

	0.0f,     0.0f,     0.0f,
	1.0f,     1.0f,     0.0f,
	0.0f,     1.0f,     0.0f
	};
	void * memory = s->memory + (s->last * sizeof (float) * 3 * 3);
	memcpy (memory, square, sizeof (square));
	s->last += n;
}
