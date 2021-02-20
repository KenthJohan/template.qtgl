#pragma once

#include <flecs.h>
#include <GL/glew.h>
#include <SDL2/SDL_net.h>

#include "csc/csc_math.h"
#include "csc/csc_gl.h"
#include "csc/csc_gcam.h"
#include "csc/csc_qf32.h"


#include "components.h"

#define EAVNET_ENTITY_MAX 1000

struct eavnet_context
{
	ecs_entity_t entities[EAVNET_ENTITY_MAX];
};

static void eavnet_context_init (struct eavnet_context * ctx, ecs_world_t * world)
{
	ecs_entity_t const * e = ecs_bulk_new (world, 0, EAVNET_ENTITY_MAX);
	memcpy (ctx->entities, e, sizeof (ecs_entity_t) * EAVNET_ENTITY_MAX);
}


static void eavnet_receiver (ecs_world_t * world, ecs_entity_t const e[], void * ptr, uint32_t value_size)
{
	struct mynet_eav * eav = ptr;
	switch (eav->attribute)
	{
	case ATTR_COUNT:{
		struct mynet_eav_u32 * eav32 = ptr;
		ecs_set (world, e[eav->entity], component_count, {eav32->value});
		break;}
	case ATTR_POINTCLOUD:
		ecs_add (world, e[eav->entity], component_pointcloud);
		break;
	case ATTR_POINTCLOUD_POS:{
		component_pointcloud const * cloud = ecs_get (world, e[eav->entity], component_pointcloud);
		component_count const * c = ecs_get (world, e[eav->entity], component_count);
		uint32_t value_count = value_size / sizeof (component_position);
		ASSERT (value_count <= *c);
		glBindBuffer (GL_ARRAY_BUFFER, cloud->vbop);
		component_position * pos = (void*)eav->value;
		glBufferSubData (GL_ARRAY_BUFFER, 0, value_size, pos);
		break;}
	case ATTR_POINTCLOUD_COL:{
		component_pointcloud const * cloud = ecs_get (world, e[eav->entity], component_pointcloud);
		component_count const * c = ecs_get (world, e[eav->entity], component_count);
		uint32_t value_count = value_size / sizeof (component_color);
		ASSERT (value_count <= *c);
		glBindBuffer (GL_ARRAY_BUFFER, cloud->vboc);
		component_color * color = (void*)eav->value;
		glBufferSubData (GL_ARRAY_BUFFER, 0, value_size, color);
		break;}
	case ATTR_MESH:
		ecs_add (world, e[eav->entity], component_mesh);
		ecs_add (world, e[eav->entity], component_vao);
		break;
	case ATTR_TEXTURE:{
		struct mynet_eav_component_texture * tex = ptr;
		ecs_add (world, e[tex->entity], component_tbo);
		ecs_set_ptr (world, e[tex->entity], component_texture, &tex->value);
		break;}
	case ATTR_POSITION:{
		struct mynet_eav_component_position * pos = ptr;
		ecs_set_ptr (world, e[pos->entity], component_position, &pos->value);
		break;}
	case ATTR_SCALE:{
		struct mynet_eav_component_scale * scale = ptr;
		ecs_set_ptr (world, e[scale->entity], component_scale, &scale->value);
		break;}
	case ATTR_QUATERNION:{
		struct mynet_eav_component_quaternion * q = ptr;
		ecs_set_ptr (world, e[q->entity], component_quaternion, &q->value);
		break;}
	case ATTR_ADD_INSTANCEOF:{
		struct mynet_eav_u32 * a = ptr;
		ecs_add_entity (world, e[a->entity], ECS_INSTANCEOF | e[a->value]);
		break;}
	case ATTR_RECTANGLE:{
		struct mynet_eav_component_rectangle * a = ptr;
		ecs_set_ptr (world, e[a->entity], component_rectangle, &a->value);
		break;}
	}

}



static int eavnet_send_u32 (TCPsocket client, uint32_t entity, uint32_t attribute, uint32_t value)
{
	struct mynet_eav_u32 msg = {entity, attribute, value};
	int result = SDLNet_TCP_Send (client, &msg, sizeof (msg));
	return result;
}


static int eavnet_send_ptr (TCPsocket client, uint32_t entity, uint32_t attribute, void * value, uint32_t size)
{
	struct
	{
		uint32_t entity;
		uint32_t attribute;
		uint8_t value[100];
	} msg;
	msg.entity = entity;
	msg.attribute = attribute;
	memcpy (msg.value, value, size);
	int result = SDLNet_TCP_Send (client, &msg, sizeof (struct mynet_eav) + size);
	return result;
}


static void eavnet_test (struct ecs_world_t * world, ecs_entity_t entities[])
{

	enum myent
	{
		MYENT_MESH_RECTANGLE,
		MYENT_TEXTURE1,
		MYENT_TEXTURE2,

		MYENT_DRAW_CLOUD,
		MYENT_DRAW_IMG1,
		MYENT_DRAW_IMG2,
	};




	{
		uint32_t count = 1000;
		eavnet_receiver (world, entities, &(struct mynet_eav){MYENT_DRAW_CLOUD, ATTR_POINTCLOUD}, 0);
		eavnet_receiver (world, entities, &(struct mynet_eav_u32){MYENT_DRAW_CLOUD, ATTR_COUNT, count}, 0);
		printf ("sizeof (struct mynet_eav): %i\n", sizeof (struct mynet_eav));
		uint32_t size = count * sizeof (component_position);
		struct mynet_eav * pc = malloc (sizeof (struct mynet_eav) + size);
		pc->entity = MYENT_DRAW_CLOUD;
		pc->attribute = ATTR_POINTCLOUD_POS;
		component_position * p = (void*)pc->value;
		for (uint32_t i = 0; i < count; ++i)
		{
			p[i][0] = 10.0f * (float)i / rand();
			p[i][1] = 1.0f * (float)i / rand();
			p[i][2] = 10.0f * (float)i / rand();
			p[i][3] = 100.0f;
		}
		eavnet_receiver (world, entities, pc, size);
	}


	eavnet_receiver (world, entities, &(struct mynet_eav_component_texture){MYENT_TEXTURE1, ATTR_TEXTURE, {0, 100, 100, 1}}, 0);
	eavnet_receiver (world, entities, &(struct mynet_eav_component_texture){MYENT_TEXTURE2, ATTR_TEXTURE, {0, 300, 300, 1}}, 0);

	eavnet_receiver (world, entities, &(struct mynet_eav){MYENT_MESH_RECTANGLE, ATTR_MESH}, 0);
	eavnet_receiver (world, entities, &(struct mynet_eav_u32){MYENT_MESH_RECTANGLE, ATTR_COUNT, 6}, 0);
	eavnet_receiver (world, entities, &(struct mynet_eav_component_rectangle){MYENT_MESH_RECTANGLE, ATTR_RECTANGLE, {1.0f, 1.0f}}, 0);

	eavnet_receiver (world, entities, &(struct mynet_eav_component_position){MYENT_DRAW_IMG1, ATTR_POSITION, {3.0f, 1.0f, 0.0f, 1.0f}}, 0);
	eavnet_receiver (world, entities, &(struct mynet_eav_component_position){MYENT_DRAW_IMG1, ATTR_SCALE, {0.3f, 0.3f, 0.0f, 1.0f}}, 0);
	eavnet_receiver (world, entities, &(struct mynet_eav_component_position){MYENT_DRAW_IMG1, ATTR_QUATERNION, {0.0f, 0.0f, 0.0f, 1.0f}}, 0);
	eavnet_receiver (world, entities, &(struct mynet_eav_u32){MYENT_DRAW_IMG1, ATTR_ADD_INSTANCEOF, MYENT_MESH_RECTANGLE}, 0);
	eavnet_receiver (world, entities, &(struct mynet_eav_u32){MYENT_DRAW_IMG1, ATTR_ADD_INSTANCEOF, MYENT_TEXTURE1}, 0);

	eavnet_receiver (world, entities, &(struct mynet_eav_component_position){MYENT_DRAW_IMG2, ATTR_POSITION, {4.0f, 1.0f, 0.0f, 1.0f}}, 0);
	eavnet_receiver (world, entities, &(struct mynet_eav_component_position){MYENT_DRAW_IMG2, ATTR_SCALE, {0.3f, 0.3f, 0.0f, 1.0f}}, 0);
	eavnet_receiver (world, entities, &(struct mynet_eav_component_position){MYENT_DRAW_IMG2, ATTR_QUATERNION, {0.0f, 0.0f, 0.0f, 1.0f}}, 0);
	eavnet_receiver (world, entities, &(struct mynet_eav_u32){MYENT_DRAW_IMG2, ATTR_ADD_INSTANCEOF, MYENT_MESH_RECTANGLE}, 0);
	eavnet_receiver (world, entities, &(struct mynet_eav_u32){MYENT_DRAW_IMG2, ATTR_ADD_INSTANCEOF, MYENT_TEXTURE2}, 0);


	//mynet_send_ptr(NULL, 0, 0, &(component_position){1.0f, 2.0f, 3.0f, 1.0f}, sizeof (component_position));

}




