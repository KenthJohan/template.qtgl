#pragma once

#include <glad.h>
#include "csc/csc_debug.h"
#include "csc/csc_math.h"

struct vertex
{
	float pos[4];
	float col[4];
};


struct vertex_handler
{
	uint32_t vcapacity;
	uint32_t vlast;
	uint32_t scapacity;
	uint32_t slast;
	uint32_t * soffset;
	uint32_t * scount;
};


struct vertex_transformation
{
	float p[4];
	float q[4];
};


static void vertexbatch_init (struct vertex_handler * h)
{
	h->slast = 0;
	h->vlast = 0;
	h->soffset = malloc (h->scapacity * sizeof (uint32_t));
	h->scount = malloc (h->scapacity * sizeof (uint32_t));
	h->soffset[h->slast] = 0;
	h->scount[h->slast] = 0;
	h->slast++;
}


static uint32_t vertexbatch_push (struct vertex_handler * h, uint32_t vcount)
{
	uint32_t slast = h->slast;
	if (slast >= h->scapacity)
	{
		fprintf (stderr, "Vertex handler is full\n");
		return 0;
	}
	h->soffset[slast] = h->scount[slast-1];
	h->scount[slast] = vcount;
	h->vlast += vcount;
	h->slast++;
	return slast;
}


struct vertex * vertex_map_buffer (struct vertex_handler * h, uint32_t vcount, uint32_t sindex)
{
	if (sindex == 0)
	{
		sindex = vertexbatch_push (h, vcount);
	}
	uint32_t vcount0 = h->scount[sindex];
	if (vcount > vcount0)
	{
		return NULL;
	}
	GLintptr offset = h->soffset[sindex] * sizeof(struct vertex);
	GLsizeiptr length = h->scount[sindex] * sizeof(struct vertex);
	ASSERT (glGetError() == GL_NO_ERROR);
	void * ptr = glMapBufferRange (GL_ARRAY_BUFFER, offset, length, GL_MAP_WRITE_BIT);
	ASSERT (glGetError() == GL_NO_ERROR);
	return ptr;
}


static void vertex_cpy (struct vertex_handler * vbatch, struct vertex * vdata, uint32_t vcount, uint32_t vindex)
{
	struct vertex * ptr = vertex_map_buffer (vbatch, vcount, vindex);
	if (ptr)
	{
		memcpy (ptr, vdata, vcount * sizeof (struct vertex));
		glUnmapBuffer (GL_ARRAY_BUFFER);
	}
}


static void shape_square_make (struct vertex_transformation * transformation, struct vertex_handler * h, uint32_t vindex)
{
	struct vertex s[] =
	{
	{{-1.0f, -1.0f, 0.0f, 1.0f}, {1, 1, 0, 1}},
	{{ 1.0f, -1.0f, 0.0f, 1.0f}, {0, 1, 0, 1}},
	{{ 1.0f,  1.0f, 0.0f, 1.0f}, {0, 0, 1, 1}},
	{{-1.0f, -1.0f, 0.0f, 1.0f}, {1, 1, 0, 1}},
	{{ 1.0f,  1.0f, 0.0f, 1.0f}, {0, 0, 1, 1}},
	{{-1.0f,  1.0f, 0.0f, 1.0f}, {1, 1, 1, 1}},
	};
	qf32_rotate_vector1 (transformation->q, s[0].pos);
	qf32_rotate_vector1 (transformation->q, s[1].pos);
	qf32_rotate_vector1 (transformation->q, s[2].pos);
	qf32_rotate_vector1 (transformation->q, s[3].pos);
	qf32_rotate_vector1 (transformation->q, s[4].pos);
	qf32_rotate_vector1 (transformation->q, s[5].pos);
	vf32_acc (4, s[0].pos, transformation->p);
	vf32_acc (4, s[1].pos, transformation->p);
	vf32_acc (4, s[2].pos, transformation->p);
	vf32_acc (4, s[3].pos, transformation->p);
	vf32_acc (4, s[4].pos, transformation->p);
	vf32_acc (4, s[5].pos, transformation->p);
	vertex_cpy (h, s, 6, vindex);
}

