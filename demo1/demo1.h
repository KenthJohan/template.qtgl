#pragma once

#include "csc/csc_crossos.h"
#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_sdlcam.h"
#include "csc/csc_gl.h"
#include "csc/csc_math.h"
#include "csc/csc_debug_nng.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>

#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>
#include <nng/supplemental/util/platform.h>


static void APIENTRY openglCallbackFunction(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam)
{
	(void)source;
	(void)type;
	(void)id;
	(void)severity;
	(void)length;
	(void)userParam;
	fprintf (stderr, "%s\n", message);
	if (severity == GL_DEBUG_SEVERITY_HIGH)
	{
		fprintf (stderr, "Aborting...\n");
		abort();
	}
}


#define main_glattr_pos 0
#define main_glattr_col 1
#define main_glattr_tex 2


struct mesh_rectangle
{
	unsigned cap;
	GLuint vao;
	GLuint vbop;
	GLuint vboc;
	GLuint vbot;
	GLuint texture;
	GLuint program;
	GLuint uniform_mvp;
};

struct mesh_pointcloud
{
	unsigned cap;
	GLuint vao;
	GLuint vbop;
	GLuint vboc;
	GLuint program;
	GLuint uniform_mvp;
};

static void mesh_pointcloud_init (struct mesh_pointcloud * m)
{
	glGenVertexArrays(1, &m->vao);
	glGenBuffers(1, &m->vbop);
	glGenBuffers(1, &m->vboc);


	glBindVertexArray (m->vao);

	glBindBuffer(GL_ARRAY_BUFFER, m->vbop);
	glBufferData(GL_ARRAY_BUFFER, m->cap*sizeof(float)*4, NULL, GL_STATIC_DRAW);
	glVertexAttribPointer (main_glattr_pos, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (main_glattr_pos);

	glBindBuffer(GL_ARRAY_BUFFER, m->vboc);
	glBufferData(GL_ARRAY_BUFFER, m->cap*sizeof(float)*4, NULL, GL_STATIC_DRAW);
	glVertexAttribPointer (main_glattr_col, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (main_glattr_col);

	float * v;


	glBindBuffer (GL_ARRAY_BUFFER, m->vbop);
	v = glMapBufferRange (GL_ARRAY_BUFFER, 0, m->cap*sizeof(float)*4, GL_MAP_WRITE_BIT);
	for (unsigned i = 0; i < m->cap; ++i)
	{
		v[0] = ((float)rand() / (float)RAND_MAX) - 0.5f;
		v[1] = ((float)rand() / (float)RAND_MAX) - 0.5f;
		v[2] = ((float)rand() / (float)RAND_MAX) - 0.5f;
		v[3] = 1.0f;
		v += 4;
	}
	glUnmapBuffer (GL_ARRAY_BUFFER);


	glBindBuffer (GL_ARRAY_BUFFER, m->vboc);
	v = glMapBufferRange (GL_ARRAY_BUFFER, 0, m->cap*sizeof(float)*4, GL_MAP_WRITE_BIT);
	for (unsigned i = 0; i < m->cap; ++i)
	{
		v[0] = 1.0f;
		v[1] = 1.0f;
		v[2] = 1.0f;
		v[3] = 1.0f;
		v += 4;
	}
	glUnmapBuffer (GL_ARRAY_BUFFER);
}


static void mesh_pointcloud_draw (struct mesh_pointcloud * m, float * mvp)
{
	glUseProgram (m->program);
	glUniformMatrix4fv (m->uniform_mvp, 1, GL_FALSE, (const GLfloat *) mvp);
	glBindVertexArray (m->vao);
	glDrawArrays (GL_POINTS, 0, m->cap);
}


static void mesh_rectangle_init (struct mesh_rectangle * m)
{
	glGenVertexArrays(1, &m->vao);
	glGenBuffers(1, &m->vbop);
	glGenBuffers(1, &m->vboc);
	glGenBuffers(1, &m->vbot);

	float const p[] =
	{
	-1.0f, -1.0f, 0.0f, 1.0f, // left, bottom
	 1.0f, -1.0f, 0.0f, 1.0f, // right, bottom
	 1.0f,  1.0f, 0.0f, 1.0f, // right, top
	-1.0f, -1.0f, 0.0f, 1.0f, // left, bottom
	 1.0f,  1.0f, 0.0f, 1.0f, // right, top
	-1.0f,  1.0f, 0.0f, 1.0f  // left, top
	};
	float const c[] =
	{
	1.0f, 1.0f, 1.0f, 1.0f, // left, bottom
	1.0f, 1.0f, 1.0f, 1.0f, // right, bottom
	1.0f, 1.0f, 1.0f, 1.0f, // right, top
	1.0f, 1.0f, 1.0f, 1.0f, // left, bottom
	1.0f, 1.0f, 1.0f, 1.0f, // right, top
	1.0f, 1.0f, 1.0f, 1.0f  // left, top
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

	glBindVertexArray (m->vao);

	glBindBuffer (GL_ARRAY_BUFFER, m->vbop);
	glBufferData (GL_ARRAY_BUFFER, m->cap*sizeof(float)*4, p, GL_STATIC_DRAW);
	glVertexAttribPointer (main_glattr_pos, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (main_glattr_pos);

	glBindBuffer (GL_ARRAY_BUFFER, m->vboc);
	glBufferData (GL_ARRAY_BUFFER, m->cap*sizeof(float)*4, c, GL_STATIC_DRAW);
	glVertexAttribPointer (main_glattr_col, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (main_glattr_col);

	glBindBuffer (GL_ARRAY_BUFFER, m->vbot);
	glBufferData (GL_ARRAY_BUFFER, m->cap*sizeof(float)*2, t, GL_STATIC_DRAW);
	glVertexAttribPointer (main_glattr_tex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (main_glattr_tex);
}


static void mesh_rectangle_draw (struct mesh_rectangle * m, float * mvp)
{
	glUseProgram (m->program);
	glUniform1i (glGetUniformLocation (m->program, "texture1"), 0);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, m->texture);
	glUniformMatrix4fv (m->uniform_mvp, 1, GL_FALSE, (const GLfloat *) mvp);
	glBindVertexArray (m->vao);
	glDrawArrays (GL_TRIANGLES, 0, m->cap);
}



void pair_listen (nng_socket * sock, char const * address)
{
	int r;
	r = nng_pair0_open (sock);
	NNG_EXIT_ON_ERROR (r);
	r = nng_listen (*sock, address, NULL, 0);
	NNG_EXIT_ON_ERROR (r);
}



void net_recv (nng_socket sock, GLuint vbo, unsigned cap)
{
	int rv;
	size_t sz;
	float * val = NULL;
	rv = nng_recv (sock, &val, &sz, NNG_FLAG_ALLOC | NNG_FLAG_NONBLOCK);
	if (rv == NNG_EAGAIN)
	{
		return;
	}
	else if (rv != 0)
	{
		NNG_EXIT_ON_ERROR (rv);
	}
	//printf ("New message %i\n", sz);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	GLsizeiptr length = MIN (cap * sizeof(float) * 4, sz);
	float * v = glMapBufferRange (GL_ARRAY_BUFFER, 0, length, GL_MAP_WRITE_BIT);
	memcpy (v, val, length);
	glUnmapBuffer (GL_ARRAY_BUFFER);
	nng_free (val, sz);
}

