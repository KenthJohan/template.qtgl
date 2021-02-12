#pragma once

#include "csc/csc_math.h"
#include <flecs.h>
#include <GL/glew.h>

typedef GLuint component_tbo;

typedef struct component_texture
{
	uint32_t unit;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
} component_texture;


ECS_COMPONENT_DECLARE (component_tbo);
ECS_COMPONENT_DECLARE (component_texture);


static void component_tbo_onadd (ecs_iter_t *it)
{
	printf ("component_tbo_onadd\n");
	ECS_COLUMN (it, component_tbo, tbo, 1);
	glGenTextures (it->count, tbo);
	for (int32_t i = 0; i < it->count; ++i){}
}



static void component_texture_onset (ecs_iter_t *it)
{
	printf ("component_texture_onadd\n");
	ECS_COLUMN (it, component_texture, tex, 1);
	ECS_COLUMN (it, component_tbo, tbo, 2);
	for (int32_t i = 0; i < it->count; ++i)
	{
		srand (i);
		uint32_t width = tex[i].width;
		uint32_t height = tex[i].height;
		uint32_t depth = tex[i].depth;
		uint32_t channels = 4;
		unsigned size = width * height * depth * channels * sizeof(uint8_t);
		uint8_t * data = calloc (size, 1);
		glActiveTexture (GL_TEXTURE0 + tex[i].unit);
		glBindTexture (GL_TEXTURE_2D_ARRAY, tbo[i]);//Depends on glActiveTexture()
		glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//Depends on glBindTexture()
		glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//Depends on glBindTexture()
		glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//Depends on glBindTexture()
		glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//Depends on glBindTexture()
		glTexStorage3D (GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, depth);//Depends on glBindTexture()
		//fill_texture (data, width, height, channels, 0);
		//fill_texture2 (data, width, height, channels, 255);
		for (uint32_t j = 0; j < depth; ++j)
		{
			//fill_texture (data + (j%3), width, height, channels, 200);
			glTexSubImage3D (GL_TEXTURE_2D_ARRAY, 0, 0, 0, j, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);//Depends on glBindTexture()
		}
	}
}


static void component_tbo_init (ecs_world_t * world)
{
	printf ("component_tbo_init\n");
	ECS_COMPONENT_DEFINE (world, component_tbo);
	ECS_COMPONENT_DEFINE (world, component_texture);
	ECS_TRIGGER (world, component_tbo_onadd, EcsOnAdd, component_tbo);
	//ECS_SYSTEM (world, component_tbo_onadd, EcsMonitor, component_tbo);
	ECS_SYSTEM (world, component_texture_onset, EcsOnSet, component_texture, component_tbo);
}



