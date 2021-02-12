#pragma once

#include "csc/csc_math.h"
#include <flecs.h>

typedef v4f32 component_position;
typedef v4f32 component_quaternion;
typedef v2f32 component_wh;





ECS_COMPONENT_DECLARE (component_position);
ECS_COMPONENT_DECLARE (component_quaternion);
ECS_COMPONENT_DECLARE (component_wh);
ECS_TAG_DECLARE (tag_imgs);
ECS_TAG_DECLARE (tag_points);


static void components_init (ecs_world_t * world)
{
	ECS_COMPONENT_DEFINE(world, component_position);
	ECS_COMPONENT_DEFINE(world, component_quaternion);
	ECS_COMPONENT_DEFINE(world, component_wh);
	ECS_TAG_DEFINE(world, tag_imgs);
	ECS_TAG_DEFINE(world, tag_points);
	//ECS_SYSTEM(world, bounce, EcsOnUpdate, type_flatimage);
}
