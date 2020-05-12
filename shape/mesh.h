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

#define MESH_SHOW      UINT32_C(0x00000001)
#define MESH_ALLOCATED UINT32_C(0x00000002)


struct gmeshes
{
	unsigned capacity;
	GLuint * vao;//OpenGL Vertex array object
	GLuint * vbo;//OpenGL Vertex buffer object
	GLenum * mode;//OpenGL draw mode, e.g. GL_TRIANGLES
	uint32_t * flags;//OpenGL draw mode, e.g. GL_TRIANGLES
	unsigned * vcount;
};



static void gmeshes_init (struct gmeshes * m, unsigned count)
{
	ASSERT_PARAM_NOTNULL (m);
	m->vao = calloc (count, sizeof (GLuint));
	m->vbo = calloc (count, sizeof (GLuint));
	m->mode = calloc (count, sizeof (GLenum));
	m->vcount = calloc (count, sizeof (unsigned));
	m->flags = calloc (count, sizeof (uint32_t));
	ASSERT (m->vao);
	ASSERT (m->vbo);
	ASSERT (m->mode);
	ASSERT (m->vcount);
	ASSERT (m->flags);
	//generate names for buffers:
	glGenVertexArrays (count, m->vao);
	glGenBuffers (count, m->vbo);
	m->capacity = count;
}


static GLint gmeshes_size (struct gmeshes * m, unsigned index)
{
	ASSERT_PARAM_NOTNULL (m);
	ASSERT (index < m->capacity);
	ASSERT (glIsBuffer (m->vbo[index]));
	GLint size = 0;
	glBindBuffer (GL_ARRAY_BUFFER, m->vbo[index]);
	glGetBufferParameteriv (GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	return size;
}


static void gmeshes_allocate (struct gmeshes * m, unsigned index, struct vertex * vertex, unsigned vcount, GLenum mode)
{
	ASSERT_PARAM_NOTNULL (m);
	ASSERT (index < m->capacity);
	glBindVertexArray (m->vao[index]);
	glBindBuffer (GL_ARRAY_BUFFER, m->vbo[index]);
	GLsizeiptr size = vcount * sizeof (struct vertex);
	glEnableVertexAttribArray (main_glattr_pos);
	glEnableVertexAttribArray (main_glattr_col);
	glVertexAttribPointer (main_glattr_pos, 4, GL_FLOAT, GL_FALSE, sizeof (struct vertex), (void*)offsetof (struct vertex, pos));
	glVertexAttribPointer (main_glattr_col, 4, GL_FLOAT, GL_FALSE, sizeof (struct vertex), (void*)offsetof (struct vertex, col));
	glBufferData (GL_ARRAY_BUFFER, size, vertex, GL_STATIC_DRAW);
	m->vcount[index] = vcount;
	m->mode[index] = mode;
	m->flags[index] |= MESH_SHOW | MESH_ALLOCATED;
}


static void gmeshes_update (struct gmeshes * m, unsigned index, struct vertex * vertex, unsigned vcount)
{
	ASSERT_PARAM_NOTNULL (m);
	ASSERT (index < m->capacity);
	GLuint vbo = m->vbo[index];
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	GLintptr offset = 0;
	GLsizeiptr length = vcount * sizeof(struct vertex);
	ASSERT (glGetError() == GL_NO_ERROR);
	struct vertex * vg = glMapBufferRange (GL_ARRAY_BUFFER, offset, length, GL_MAP_WRITE_BIT);
	ASSERT (glGetError() == GL_NO_ERROR);
	ASSERT (vg);
	memcpy (vg, vertex, length);
	glUnmapBuffer (GL_ARRAY_BUFFER);
}


static void gmeshes_draw (struct gmeshes * m)
{
	ASSERT_PARAM_NOTNULL (m);
	for (unsigned i = 0; i < m->capacity; ++i)
	{
		GLuint vao = m->vao[i];
		GLenum mode = m->mode[i];
		unsigned vcount = m->vcount[i];
		if (m->flags[i] & (MESH_ALLOCATED | MESH_SHOW))
		{
			glBindVertexArray (vao);
			glDrawArrays (mode, 0, vcount);
		}
	}
}



struct cmesh
{
	unsigned capacity;
	unsigned last;
	struct vertex * v0;
	struct vertex * v1;
	float p[4];//Position
	float q[4];//Quaternion
};

static void cvertices_init (struct cmesh * m, unsigned vcount)
{
	m->capacity = vcount;
	m->v0 = calloc (vcount, sizeof (struct vertex));
	m->v1 = calloc (vcount, sizeof (struct vertex));
	qf32_identity (m->q);
	v4f32_set_xyzw (m->p, 0.0f, 0.0f, 0.0f, 1.0f);
}

static void cvertices_add (struct cmesh * m, struct vertex * v, unsigned vcount)
{
	unsigned last = m->last + vcount;
	ASSERT (last <= m->capacity);
	m->last += vcount;
	memcpy (m->v0, v, vcount * sizeof (struct vertex));
}

static void cvertices_add_square (struct cmesh * m)
{
	struct vertex s[] =
	{
	{{-1.0f, -1.0f, 0.0f, 1.0f}, {1, 1, 0, 1}},
	{{ 1.0f, -1.0f, 0.0f, 1.0f}, {0, 1, 0, 1}},
	{{ 1.0f,  1.0f, 0.0f, 1.0f}, {0, 0, 1, 1}},
	{{-1.0f, -1.0f, 0.0f, 1.0f}, {1, 1, 0, 1}},
	{{ 1.0f,  1.0f, 0.0f, 1.0f}, {0, 0, 1, 1}},
	{{-1.0f,  1.0f, 0.0f, 1.0f}, {0, 1, 1, 1}}};
	cvertices_add (m, s, 6);
}


static void cvertices_update (struct cmesh * m)
{
	ASSERT_PARAM_NOTNULL (m);
	struct vertex * v0 = m->v0;
	struct vertex * v1 = m->v1;
	for (uint32_t i = 0; i < m->last; ++i)
	{
		vf32_cpy (4, v1->col, v0->col);
		qf32_rotate_vector (m->q, v0->pos, v1->pos);
		vf32_acc (4, v1->pos, m->p);
		v0++;
		v1++;
	}
}



