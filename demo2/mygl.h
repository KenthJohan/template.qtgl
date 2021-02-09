#pragma once

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>
#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_math.h"


/*
#define MYGL_POINTS 1
#define MYGL_LINES 2
#define MYGL_RECTANGLES 3
#define MYGL_TRIANGLES 4
#define MYGL_CUBES 5
#define MYGL_IMGS 5



struct mygl_va_img
{
	float x;
	float y;
	float z;
	float layer;
	float u;
	float v;
};

struct mygl_va_point
{
	float x;
	float y;
	float z;
	float layer;
};


struct mygl_vertices
{
	uint32_t n;
	uint32_t l;
	uint32_t type;
	uint32_t * mesh_id;
	union
	{
		uint8_t * data;
		struct mygl_va_img * vimg;
		struct mygl_va_point * vpoint;
	};
	uint32_t vao[1];
	uint32_t vbo[3];
};

uint32_t mygl_type_size (uint32_t type)
{
	switch (type)
	{
	case MYGL_IMGS: return sizeof(struct mygl_va_img);
	case MYGL_POINTS: return sizeof(struct mygl_va_point);
	}
	return 0;
}

void mygl_vertices_calloc (struct mygl_vertices * item)
{
	void * mesh_id = calloc (item->n * sizeof (uint32_t), 1);
	void * data = calloc (item->n * mygl_type_size (item->type), 1);
	ASSERT_NOTNULL (mesh_id);
	ASSERT_NOTNULL (data);
	item->mesh_id = mesh_id;
	item->data = data;
}


void mygl_vertices_realloc (struct mygl_vertices * item)
{
	void * mesh_id = realloc (item->mesh_id, item->n * sizeof (uint32_t));
	void * data = realloc (item->data, item->n * mygl_type_size (item->type));
	ASSERT_NOTNULL (mesh_id);
	ASSERT_NOTNULL (data);
	item->mesh_id = mesh_id;
	item->data = data;
}


void mygl_vertices_touch (struct mygl_vertices * item, uint32_t id)
{
	if (id >= item->n)
	{
		item->n = id+1;
		mygl_vertices_realloc (item);
	}
}


void mygl_vertices_reserve (struct mygl_vertices * item, uint32_t n)
{
	mygl_vertices_touch (item, item->l + n);
}


void mygl_vertices_add (struct mygl_vertices * item, uint32_t mesh_id, void * value, uint32_t n)
{
	mygl_vertices_reserve (item, n);
	uint8_t * data = item->data + item->l * mygl_type_size (item->type);
	uint8_t * mesh_ids = item->mesh_id + item->l;
	memcpy (data, value, mygl_type_size (item->type) * n);
	for (uint32_t i = 0; i < n; ++i)
	{
		mesh_ids[i] = mesh_id;
	}
	item->l += n;
}


void mygl_vertices_opengl (struct mygl_vertices * item)
{
	glGenVertexArrays (1, item->vao);
	glGenBuffers (1, item->vbo);

	glBindVertexArray (img->vao);

	glBindBuffer (GL_ARRAY_BUFFER, img->vbop);
	glBufferData (GL_ARRAY_BUFFER, img->cap * CSC_GLIMAGE_VERTS_COUNT * CSC_GLIMAGE_POS_DIM * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer (0, CSC_GLIMAGE_POS_DIM, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (0);

	glBindBuffer (GL_ARRAY_BUFFER, img->vbot);
	csc_rectangle_setuv_vf32 (img->vtex, img->cap);
	glBufferData (GL_ARRAY_BUFFER, img->cap * CSC_GLIMAGE_VERTS_COUNT * CSC_GLIMAGE_TEX_DIM * sizeof(float), img->vtex, GL_DYNAMIC_DRAW);
	glVertexAttribPointer (1, CSC_GLIMAGE_TEX_DIM, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (1);
}





struct mygl_mesh
{
	uint32_t enabled;
};


struct mygl_meshes
{
	uint32_t n;
	struct mygl_mesh * meshes;
};

void mygl_meshes_calloc (struct mygl_meshes * item)
{
	item->meshes = calloc (item->n * sizeof(struct mygl_mesh), 1);
	ASSERT_NOTNULL (item->meshes);
}

void mygl_meshes_touch (struct mygl_meshes * item, uint32_t id)
{
	if (id >= item->n)
	{
		item->n = id+1;
		void * meshes = realloc (item->meshes, sizeof(struct mygl_mesh) * item->n);
		ASSERT_NOTNULL (meshes);
		item->meshes = meshes;
	}
	item->meshes[id].enabled = 1;
}





struct mygl_context
{
	struct mygl_vertices vimgs;
	struct mygl_vertices vpoints;
	struct mygl_meshes meshes;
};



void mygl_context_add (struct mygl_context * ctx, uint32_t mesh_id, uint32_t type, void * value, uint32_t n)
{
	mygl_meshes_touch (ctx, mesh_id);
	switch (type)
	{
	case MYGL_POINTS:
		mygl_vertices_add (&ctx->vpoints, mesh_id, value, n);
		break;
	case MYGL_IMGS:
		mygl_vertices_add (&ctx->vimgs, mesh_id, value, n);
		break;
	}
}
*/


void mygl_make_rectangle (struct mygl_context * ctx, float v[], float x, float y, float z, float w, float width, float height, uint32_t stride)
{
	float wr = width / 2.0f;
	float hr = height / 2.0f;

	v[0] = -wr + x;
	v[1] = -hr + y;
	v[2] = z;
	v[3] = w;
	v += stride;

	v[0] = wr + x;
	v[1] = -hr + y;
	v[2] = z;
	v[3] = w;
	v += stride;

	v[0] = wr + x;
	v[1] = hr + y;
	v[2] = z;
	v[3] = w;
	v += stride;

	v[0] = -wr + x;
	v[1] = -hr + y;
	v[2] = z;
	v[3] = w;
	v += stride;

	v[0] = wr + x;
	v[1] = hr + y;
	v[2] = z;
	v[3] = w;
	v += stride;

	v[0] = -wr + x;
	v[1] = hr + y;
	v[2] = z;
	v[3] = w;
	v += stride;
}


void mygl_make_rectangle_uv (float uv[], uint32_t n, uint32_t stride)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		uv[0] = 0.0f;
		uv[1] = 0.0f;
		uv += stride;

		uv[0] = 1.0f;
		uv[1] = 0.0f;
		uv += stride;

		uv[0] = 1.0f;
		uv[1] = 1.0f;
		uv += stride;

		uv[0] = 0.0f;
		uv[1] = 0.0f;
		uv += stride;

		uv[0] = 1.0f;
		uv[1] = 1.0f;
		uv += stride;

		uv[0] = 0.0f;
		uv[1] = 1.0f;
		uv += stride;
	}
}













