#pragma once


#include <GL/glew.h>
#include "csc/csc_debug.h"
#include "csc/csc_math.h"



enum main_glattr
{
	main_glattr_pos,
	main_glattr_col
};

#define MESH_SHOW      UINT32_C(0x00000001)
#define MESH_ALLOCATED UINT32_C(0x00000002)

struct gmeshes
{
	unsigned cap;//Maximum amount of meshes
	GLuint * vao;//Array of OpenGL Vertex array object
	GLuint * vbop;//Array of OpenGL Vertex buffer object position
	GLuint * vboc;//Array of OpenGL Vertex buffer object color
	GLenum * mode;//Array of OpenGL draw mode, e.g. GL_TRIANGLES
	uint32_t * flags;//Array of Misc info
	unsigned * vcap;//Array of Maximum amount of vertices
	float * q;
	float * p;
};


static void gmeshes_init (struct gmeshes * m, unsigned count)
{
	ASSERT_PARAM_NOTNULL (m);
	m->vao = calloc (count, sizeof (GLuint));
	m->vbop = calloc (count, sizeof (GLuint));
	m->vboc = calloc (count, sizeof (GLuint));
	m->mode = calloc (count, sizeof (GLenum));
	m->vcap = calloc (count, sizeof (unsigned));
	m->flags = calloc (count, sizeof (uint32_t));
	m->q = calloc (count, sizeof (float) * 4);
	m->p = calloc (count, sizeof (float) * 4);
	ASSERT (m->vao);
	ASSERT (m->vbop);
	ASSERT (m->vboc);
	ASSERT (m->mode);
	ASSERT (m->vcap);
	ASSERT (m->flags);
	ASSERT (m->q);
	ASSERT (m->p);
	for (unsigned i = 0; i < count; ++i)
	{
		qf32_identity (m->q + i*4);
	}
	//generate names for buffers:
	glGenVertexArrays (count, m->vao);
	glGenBuffers (count, m->vbop);
	glGenBuffers (count, m->vboc);
	m->cap = count;
}



static void gmeshes_allocate (struct gmeshes * m, unsigned index, unsigned vcount, GLenum mode)
{
	ASSERT_PARAM_NOTNULL (m);
	ASSERT (index < m->cap);
	GLsizeiptr size = vcount * sizeof (float) * 4;
	glBindVertexArray (m->vao[index]);

	glBindBuffer (GL_ARRAY_BUFFER, m->vbop[index]);
	glEnableVertexAttribArray (main_glattr_pos);
	glVertexAttribPointer (main_glattr_pos, 4, GL_FLOAT, GL_FALSE, sizeof (float) * 4, 0);
	glBufferData (GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);

	glBindBuffer (GL_ARRAY_BUFFER, m->vboc[index]);
	glEnableVertexAttribArray (main_glattr_col);
	glVertexAttribPointer (main_glattr_col, 4, GL_FLOAT, GL_FALSE, sizeof (float) * 4, 0);
	glBufferData (GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);

	m->vcap[index] = vcount;
	m->mode[index] = mode;
	m->flags[index] |= MESH_ALLOCATED;
}



static void gmeshes_draw (struct gmeshes * m, GLuint uniform_mvp, float mvp[4*4])
{
	ASSERT_PARAM_NOTNULL (m);
	glClearColor (0.1f, 0.1f, 0.1f, 0.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (unsigned i = 0; i < m->cap; ++i)
	{
		GLuint vao = m->vao[i];//Mesh Vertex array object
		GLenum mode = m->mode[i];//Mesh mode, Specifies what kind of primitives to render
		float * p = m->p + (i*4);//Mesh Position
		float * q = m->q + (i*4);//Mesh Quaternion
		unsigned vcount = m->vcap[i];
		if (m->flags[i] & (MESH_SHOW))
		{
			float mmvp[4*4];//Matrix Model View Projection
			m4f32_identity (mmvp);//Init matrix
			m4f32_translation (mmvp, p);//Set translation values from position
			qf32_m4 (mmvp, q);//Set matrix values from quaternion
			m4f32_mul (mmvp, mvp, mmvp);//Apply model transformation to the Matrix View Projection
			glUniformMatrix4fv (uniform_mvp, 1, GL_FALSE, (const GLfloat *) mmvp);
			glBindVertexArray (vao);
			glDrawArrays (mode, 0, vcount);
		}
	}
}



static void gmeshes_update (struct gmeshes * m, unsigned index, enum main_glattr attr, float const v[], unsigned vcount)
{
	ASSERT_PARAM_NOTNULL (m);
	ASSERT (index < m->cap);
	GLuint vbo;
	switch (attr)
	{
	case main_glattr_pos:
		vbo = m->vbop[index];
		break;
	case main_glattr_col:
		vbo = m->vboc[index];
		break;
	}
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	GLintptr offset = 0;
	ASSERT (vcount <= m->vcap[index]);
	GLsizeiptr length = vcount * sizeof (float) * 4;
	ASSERT (glGetError() == GL_NO_ERROR);
	float * mem = glMapBufferRange (GL_ARRAY_BUFFER, offset, length, GL_MAP_WRITE_BIT);
	ASSERT (glGetError() == GL_NO_ERROR);
	ASSERT (mem);
	memcpy (mem, v, length);
	glUnmapBuffer (GL_ARRAY_BUFFER);
}



static void gmeshes_square (struct gmeshes * m, unsigned index)
{
	float const s[] =
	{
	-1.0f, -1.0f, 0.0f, 1.0f,
	 1.0f, -1.0f, 0.0f, 1.0f,
	 1.0f,  1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 1.0f,
	 1.0f,  1.0f, 0.0f, 1.0f,
	-1.0f,  1.0f, 0.0f, 1.0f
	};
	gmeshes_update (m, index, main_glattr_pos, s, 6);
	float const c[] =
	{
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f
	};
	gmeshes_update (m, index, main_glattr_col, c, 6);
}


static void gmeshes_line (struct gmeshes * m, unsigned index, float a[], float b[])
{
	float s[4*2];
	memcpy(s+0*4, a, sizeof (float) * 4);
	memcpy(s+1*4, b, sizeof (float) * 4);
	gmeshes_update (m, index, main_glattr_pos, s, 2);
	float const c[] =
	{
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f
	};
	gmeshes_update (m, index, main_glattr_col, c, 2);
}


static void gmeshes_points (struct gmeshes * m, unsigned index)
{
	GLintptr offset = 0;
	unsigned n = m->vcap[index];
	GLsizeiptr length = n * sizeof(float) * 4;
	float * v;

	glBindBuffer (GL_ARRAY_BUFFER, m->vbop[index]);
	v = glMapBufferRange (GL_ARRAY_BUFFER, offset, length, GL_MAP_WRITE_BIT);
	for (unsigned i = 0; i < n; ++i)
	{
		v[0] = (float)rand() / (float)RAND_MAX;
		v[1] = (float)rand() / (float)RAND_MAX;
		v[2] = (float)rand() / (float)RAND_MAX;
		v[3] = 1.0f;
		v += 4;
	}
	glUnmapBuffer (GL_ARRAY_BUFFER);

	glBindBuffer (GL_ARRAY_BUFFER, m->vboc[index]);
	v = glMapBufferRange (GL_ARRAY_BUFFER, offset, length, GL_MAP_WRITE_BIT);
	for (unsigned i = 0; i < n; ++i)
	{
		v[0] = 1.0f;
		v[1] = 1.0f;
		v[2] = 1.0f;
		v[3] = 1.0f;
		v += 4;
	}
	glUnmapBuffer (GL_ARRAY_BUFFER);
}








