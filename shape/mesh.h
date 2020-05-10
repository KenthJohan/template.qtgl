#pragma once

#include <glad.h>
#include "csc/csc_debug.h"
#include "csc/csc_math.h"


enum main_glattr
{
	main_glattr_pos,
	main_glattr_col
};


struct vertex
{
	float pos[4];
	float col[4];
};

#define MESH_SHOW UINT32_C(0x00000001)
#define MESH_ALLOCATED UINT32_C(0x00000001)

struct mesh
{
	uint32_t flags;
	uint32_t vcapacity;
	uint32_t vlast;
	GLuint vao;
	GLuint vbo;
	GLenum mode;
	float p[4];
	float q[4];
	struct vertex * vertices0;
	struct vertex * vertices1;
};

struct mesharray
{
	uint32_t capacity;
	uint32_t last;
	struct mesh * meshes;
};


static void mesh_init (struct mesh * m)
{
	glGenVertexArrays (1, &m->vao);
	glGenBuffers (1, &m->vbo);
	glBindVertexArray (m->vao);
	glBindBuffer (GL_ARRAY_BUFFER, m->vbo);
	GLsizeiptr size = m->vcapacity * sizeof (struct vertex);
	glEnableVertexAttribArray (main_glattr_pos);
	glEnableVertexAttribArray (main_glattr_col);
	glVertexAttribPointer (main_glattr_pos, 4, GL_FLOAT, GL_FALSE, sizeof (struct vertex), (void*)offsetof (struct vertex, pos));
	glVertexAttribPointer (main_glattr_col, 4, GL_FLOAT, GL_FALSE, sizeof (struct vertex), (void*)offsetof (struct vertex, col));
	glBufferData (GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
	m->vertices0 = malloc (size);
	m->vertices1 = malloc (size);
	m->flags |= MESH_ALLOCATED;
	m->vlast = 0;
	v4f32_set_xyzw (m->p, 0.0f, 0.0f, 0.0f, 1.0f);
	qf32_identity (m->q);
}


static void mesh_upload (struct mesh * m)
{
	glBindBuffer (GL_ARRAY_BUFFER, m->vbo);
	GLintptr offset = 0;
	GLsizeiptr length = m->vlast * sizeof(struct vertex);
	ASSERT (glGetError() == GL_NO_ERROR);
	struct vertex * vg = glMapBufferRange (GL_ARRAY_BUFFER, offset, length, GL_MAP_WRITE_BIT);
	ASSERT (glGetError() == GL_NO_ERROR);
	ASSERT (vg);
	memcpy (vg, m->vertices1, length);
	glUnmapBuffer (GL_ARRAY_BUFFER);
}


static void mesh_update_transformation (struct mesh * m)
{
	struct vertex * v0 = m->vertices0;
	struct vertex * v1 = m->vertices1;
	for (uint32_t i = 0; i < m->vlast; ++i)
	{
		vf32_cpy (4, v1->col, v0->col);
		qf32_rotate_vector (m->q, v0->pos, v1->pos);
		vf32_acc (4, v1->pos, m->p);
		v0++;
		v1++;
	}
}


static void mesh_push (struct mesh * m, struct vertex vertices[], uint32_t vcount)
{
	ASSERT ((m->vlast + vcount) <= m->vcapacity);
	struct vertex * v = m->vertices0 + m->vlast;
	memcpy (v, vertices, 6 * sizeof (struct vertex));
	m->vlast += vcount;
	m->flags |= MESH_SHOW;
}


static void mesharray_init (struct mesharray * m)
{
	m->meshes = calloc (m->capacity, sizeof (struct mesh));
	m->last = 0;
}


static void mesharray_draw (struct mesharray * marr)
{
	struct mesh * m = marr->meshes;
	for (uint32_t i = 0; i < marr->last; ++i)
	{
		if (m->flags & (MESH_ALLOCATED | MESH_SHOW))
		{
			glBindVertexArray (m->vao);
			glDrawArrays (m->mode, 0, m->vlast);
		}
		m++;
	}
}


static struct mesh * mesharray_allocate_mesh (struct mesharray * marr, uint32_t vcapacity, GLenum mode)
{
	if (marr->last > marr->capacity)
	{
		return NULL;
	}
	struct mesh * m = marr->meshes + marr->last;
	m->vcapacity = vcapacity;
	m->mode = mode;
	mesh_init (m);
	marr->last++;
	return m;
}



static void shape_square_make (struct mesh * m)
{
	struct vertex s[] =
	{
	{{-1.0f, -1.0f, 0.0f, 1.0f}, {1, 1, 0, 1}},
	{{ 1.0f, -1.0f, 0.0f, 1.0f}, {0, 1, 0, 1}},
	{{ 1.0f,  1.0f, 0.0f, 1.0f}, {0, 0, 1, 1}},
	{{-1.0f, -1.0f, 0.0f, 1.0f}, {1, 1, 0, 1}},
	{{ 1.0f,  1.0f, 0.0f, 1.0f}, {0, 0, 1, 1}},
	{{-1.0f,  1.0f, 0.0f, 1.0f}, {0, 1, 1, 1}},
	};
	mesh_push (m, s, 6);
}





