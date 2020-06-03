#pragma once


#include <GL/glew.h>
#include "csc/csc_debug.h"
#include "csc/csc_math.h"



struct gtextures
{
	unsigned cap;//Maximum amount of meshes
	GLuint * textures;
	uint32_t * flags;
};


static void gtextures_init (struct gtextures * t, unsigned count)
{
	t->textures = calloc (count, sizeof (GLuint));
	t->flags = calloc (count, sizeof (uint32_t));
	glGenTextures (count, t->textures);
	t->cap = count;
};

static void gtextures_allocate (struct gtextures * t, unsigned index, GLsizei width, GLsizei height)
{
	GLuint texture = t->textures[index];
	glBindTexture (GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	uint8_t * data = NULL;
	unsigned size = width*height*4*sizeof(uint8_t);
	data = malloc (size);
	for (unsigned i = 0; i < size; ++i)
	{
		data[i] = rand();
	}
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap (GL_TEXTURE_2D);
}


enum main_glattr
{
	main_glattr_pos,
	main_glattr_col,
	main_glattr_tex
};

#define MESH_SHOW        UINT32_C(0x00000001)
#define MESH_ALLOCATED   UINT32_C(0x00000002)
#define MESH_POSITIONED  UINT32_C(0x00000010)
#define MESH_COLORED     UINT32_C(0x00000020)
#define MESH_TEXTURED    UINT32_C(0x00000040)


struct gmeshes
{
	unsigned cap;//Maximum amount of meshes
	GLuint * vao;//Array of OpenGL Vertex array object
	GLuint * vbop;//Array of OpenGL Vertex buffer object position
	GLuint * vboc;//Array of OpenGL Vertex buffer object color
	GLuint * vbot;//Array of OpenGL Vertex buffer object texture
	GLuint * textures;
	GLuint * program;
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
	m->vbot = calloc (count, sizeof (GLuint));
	m->textures = calloc (count, sizeof (GLuint));
	m->program = calloc (count, sizeof (GLuint));
	m->mode = calloc (count, sizeof (GLenum));
	m->vcap = calloc (count, sizeof (unsigned));
	m->flags = calloc (count, sizeof (uint32_t));
	m->q = calloc (count, sizeof (float) * 4);
	m->p = calloc (count, sizeof (float) * 4);
	ASSERT (m->vao);
	ASSERT (m->vbop);
	ASSERT (m->vboc);
	ASSERT (m->vbot);
	ASSERT (m->textures);
	ASSERT (m->program);
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
	glGenBuffers (count, m->vbot);
	m->cap = count;
}



static void gmeshes_allocate (struct gmeshes * m, unsigned index, unsigned vcount, GLenum mode)
{
	//Do not use index outside limit:
	//Do not allocate an already allocated mesh:
	ASSERT_PARAM_NOTNULL (m);
	ASSERT (index < m->cap);
	ASSERT ((m->flags[index] & MESH_ALLOCATED) == 0);
	glBindVertexArray (m->vao[index]);

	if (m->flags[index] & MESH_POSITIONED)
	{
		//Convert vertex count to size in bytes:
		unsigned dim = 4;
		GLsizeiptr size = vcount * sizeof (float) * dim;
		glBindBuffer (GL_ARRAY_BUFFER, m->vbop[index]);
		glEnableVertexAttribArray (main_glattr_pos);
		glVertexAttribPointer (main_glattr_pos, dim, GL_FLOAT, GL_FALSE, sizeof (float) * dim, 0);
		glBufferData (GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
	}

	if (m->flags[index] & MESH_COLORED)
	{
		//Convert vertex count to size in bytes:
		unsigned dim = 4;
		GLsizeiptr size = vcount * sizeof (float) * dim;
		glBindBuffer (GL_ARRAY_BUFFER, m->vboc[index]);
		glEnableVertexAttribArray (main_glattr_col);
		glVertexAttribPointer (main_glattr_col, dim, GL_FLOAT, GL_FALSE, sizeof (float) * dim, 0);
		glBufferData (GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
	}


	if (m->flags[index] & MESH_TEXTURED)
	{
		unsigned dim = 2;
		GLsizeiptr size = vcount * sizeof (float) * dim;
		glBindBuffer (GL_ARRAY_BUFFER, m->vbot[index]);
		glEnableVertexAttribArray (main_glattr_tex);
		glVertexAttribPointer (main_glattr_tex, dim, GL_FLOAT, GL_FALSE, sizeof (float) * dim, 0);
		glBufferData (GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
	}


	m->vcap[index] = vcount;
	m->mode[index] = mode;
	m->flags[index] |= MESH_ALLOCATED;
}



static void gmeshes_draw (struct gmeshes * m, GLuint uniform_mvp, float mvp[4*4])
{
	ASSERT_PARAM_NOTNULL (m);
	glClearColor (0.1f, 0.1f, 0.1f, 0.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	for (unsigned i = 0; i < m->cap; ++i)
	{
		GLuint vao = m->vao[i];//Mesh Vertex array object
		GLenum mode = m->mode[i];//Mesh mode, Specifies what kind of primitives to render
		float * p = m->p + (i*4);//Mesh Position
		float * q = m->q + (i*4);//Mesh Quaternion
		unsigned vcount = m->vcap[i];

		GLuint program = m->program[i];
		GLuint texture = m->textures[i];
		uint32_t flags = m->flags[i];

		if ((flags & (MESH_SHOW)) == 0) {continue;}

		glUseProgram (program);

		if (flags & (MESH_TEXTURED))
		{
			glUniform1i (glGetUniformLocation (program, "texture1"), 0);
			glActiveTexture (GL_TEXTURE0);
			glBindTexture (GL_TEXTURE_2D, texture);
		}

		if (flags & (MESH_POSITIONED))
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
	GLsizeiptr length;
	switch (attr)
	{
	case main_glattr_pos:
		vbo = m->vbop[index];
		length = vcount * sizeof (float) * 4;
		break;
	case main_glattr_col:
		vbo = m->vboc[index];
		length = vcount * sizeof (float) * 4;
		break;
	case main_glattr_tex:
		vbo = m->vbot[index];
		length = vcount * sizeof (float) * 2;
		break;
	default:
		ASSERT (0);
		break;
	}
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	GLintptr offset = 0;
	ASSERT (vcount <= m->vcap[index]);
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
	-1.0f, -1.0f, 0.0f, 1.0f, // left, bottom
	 1.0f, -1.0f, 0.0f, 1.0f, // right, bottom
	 1.0f,  1.0f, 0.0f, 1.0f, // right, top
	-1.0f, -1.0f, 0.0f, 1.0f, // left, bottom
	 1.0f,  1.0f, 0.0f, 1.0f, // right, top
	-1.0f,  1.0f, 0.0f, 1.0f  // left, top
	};
	float const t[] =
	{
	0.0f, 0.0f, // left, bottom
	1.0f, 0.0f, // right, bottom
	1.0f, 1.0f, // right, top
	0.0f, 0.0f, // left, bottom
	1.0f, 1.0f, // right, top
	0.0f, 1.0f  // left, top
	};
	if (m->flags[index] & MESH_POSITIONED)
	{
		gmeshes_update (m, index, main_glattr_pos, s, 6);
	}
	if (m->flags[index] & MESH_TEXTURED)
	{
		gmeshes_update (m, index, main_glattr_tex, t, 6);
	}
}

static void gmeshes_color (struct gmeshes * m, unsigned index, float r, float g, float b)
{
	GLintptr offset = 0;
	unsigned n = m->vcap[index];
	GLsizeiptr length = n * sizeof(float) * 4;
	float * v;
	GLuint vbo = m->vboc[index];
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	v = glMapBufferRange (GL_ARRAY_BUFFER, offset, length, GL_MAP_WRITE_BIT);
	for (unsigned i = 0; i < n; ++i)
	{
		v[0] = r;
		v[1] = g;
		v[2] = b;
		v[3] = 1.0f;
		v += 4;
	}
	glUnmapBuffer (GL_ARRAY_BUFFER);
}

static void gmeshes_color1 (struct gmeshes * m, unsigned index, unsigned offset, unsigned n, float r, float g, float b)
{
	GLintptr offsetbyte = offset * sizeof(float) * 4;
	GLsizeiptr length = n * sizeof(float) * 4;
	float * v;
	glBindBuffer (GL_ARRAY_BUFFER, m->vboc[index]);
	v = glMapBufferRange (GL_ARRAY_BUFFER, offsetbyte, length, GL_MAP_WRITE_BIT);
	for (unsigned i = 0; i < n; ++i)
	{
		v[0] = r;
		v[1] = g;
		v[2] = b;
		v[3] = 1.0f;
		v += 4;
	}
	glUnmapBuffer (GL_ARRAY_BUFFER);
}


static void plane_parametric_form (float a, float b, float c, float d, float r, float s, float x[3])
{
	x[0] = (d/a) + r * (-b/a) + s * (-c/a);
	x[1] = r;
	x[2] = s;
}


static void gmeshes_line (struct gmeshes * m, unsigned index, float a[], float b[])
{
	float s[4*2];
	memcpy(s+0*4, a, sizeof (float) * 4);
	memcpy(s+1*4, b, sizeof (float) * 4);
	gmeshes_update (m, index, main_glattr_pos, s, 2);
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
}








