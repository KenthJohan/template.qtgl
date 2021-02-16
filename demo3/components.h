#pragma once

#include <flecs.h>

#include "csc/csc_math.h"
#include "csc/csc_gl.h"
#include "csc/csc_gcam.h"
#include "csc/csc_qf32.h"


typedef uint32_t component_tbo;
typedef uint32_t component_color;
typedef v4f32 component_position;
typedef v4f32 component_scale;
typedef qf32 component_quaternion;
typedef v4f32 component_applyrotation;
typedef v2f32 component_uv;
typedef v2f32 component_rectangle;
typedef struct component_controller
{
	const uint8_t * keyboard; //SDL Keyboard
} component_controller;
typedef struct component_texture
{
	uint32_t unit;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
} component_texture;
typedef struct component_pointcloud
{
	uint32_t vao;
	uint32_t vboc;
	uint32_t vbop;
} component_pointcloud;
typedef struct component_mesh
{
	uint32_t vbop;
	uint32_t vbot;
} component_mesh;
typedef uint32_t component_vbo;
typedef uint32_t component_va;
typedef uint32_t component_stride;
typedef uint32_t component_count;
typedef uint32_t component_vao;


ECS_COMPONENT_DECLARE (component_tbo);
ECS_COMPONENT_DECLARE (component_texture);
ECS_COMPONENT_DECLARE (component_color);
ECS_COMPONENT_DECLARE (component_position);
ECS_COMPONENT_DECLARE (component_scale);
ECS_COMPONENT_DECLARE (component_quaternion);
ECS_COMPONENT_DECLARE (component_applyrotation);
ECS_COMPONENT_DECLARE (component_uv);
ECS_COMPONENT_DECLARE (component_rectangle);
ECS_COMPONENT_DECLARE (component_controller);
ECS_COMPONENT_DECLARE (component_vbo);
ECS_COMPONENT_DECLARE (component_va);
ECS_COMPONENT_DECLARE (component_stride);
ECS_COMPONENT_DECLARE (component_count);
ECS_COMPONENT_DECLARE (component_vao);
ECS_COMPONENT_DECLARE (component_pointcloud);
ECS_COMPONENT_DECLARE (component_mesh);



enum glprogram_type
{
	GLPROGRAM_TRIANGLES,
	GLPROGRAM_POINTS,
	GLPROGRAM_IMGS,
	GLPROGRAM_COUNT,
};

enum gluniform_type
{
	GLUNIFORM_TRIANGLES_MVP,
	GLUNIFORM_POINTS_MVP,
	GLUNIFORM_IMGS_MVP,
	GLUNIFORM_IMGS_TEX0,
	GLUNIFORM_COUNT,
};

#define MYGL_MAX_POINTS 10000
#define MYGL_MAX_TRIANGLES 10000
#define MYGL_MAX_IMGS 100





enum myattr
{
	ATTR_COUNT,
	ATTR_STRIDE,
	ATTR_POINTCLOUD,
};

struct mynet_eav
{
	uint32_t entity;
	uint32_t attribute;
	uint8_t value[0];
};

struct mynet_eav_u32
{
	uint32_t entity;
	uint32_t attribute;
	uint32_t value;
};



static void receiver (ecs_world_t * world, ecs_entity_t const e[], void * ptr)
{
	struct mynet_eav * eav = ptr;
	switch (eav->attribute)
	{
	case ATTR_COUNT:{
		struct mynet_eav_u32 * eav32 = ptr;
		ecs_set (world, e[eav->entity], component_count, {eav32->value});
		break;}
	case ATTR_STRIDE:{
		struct mynet_eav_u32 * eav32 = ptr;
		ecs_set (world, e[eav->entity], component_stride, {eav32->value});
		break;}
	case ATTR_POINTCLOUD:
		ecs_add (world, e[eav->entity], component_pointcloud);
		break;
	}
}


void mynet_test (ecs_world_t * world)
{
	ecs_entity_t const * e = ecs_bulk_new (world, 0, 4);


	receiver(world, e, &(struct mynet_eav){0, ATTR_POINTCLOUD});
	receiver(world, e, &(struct mynet_eav_u32){0, ATTR_COUNT, 10000});

}














