#pragma once

#include "csc/csc_crossos.h"
#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_gl.h"

#include "csc/experiment/csc_glimage.h"
#include "csc/experiment/csc_glpointcloud.h"
#include "csc/experiment/csc_gltex.h"
#include "csc/experiment/netgl.h"
#include "components.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <stdio.h>
#include <flecs.h>

#define NETGL_OPENGL_MAXENTS 100
struct netgl_opengl
{
	ecs_entity_t ents[NETGL_OPENGL_MAXENTS];
};

static void netgl_opengl_init (ecs_world_t * world, struct netgl_opengl * item)
{
}


static void netgl_recv_img_content (ecs_world_t * world, struct netgl_opengl * glctx, struct myapi_img_content * content)
{

};


static void netgl_recv_img_layout (ecs_world_t * world, struct netgl_opengl * glctx, struct myapi_img_layout * layout)
{

};




void netgl_recv
(
	ecs_world_t * world,
	struct netgl_opengl * glctx,
	struct myapi_pkg * pkg,
	struct csc_glpointcloud * g_pointcloud,
	struct csc_glimgcontext * g_img
)
{
	struct myapi_pointcloud * pkg_pointcloud;
	switch (pkg->type)
	{
	case MYAPI_POINTCLOUD:
		pkg_pointcloud = (void*)pkg;
		csc_glpointcloud_update (g_pointcloud, pkg_pointcloud->from, pkg_pointcloud->count, pkg_pointcloud->data);
		break;
	case MYAPI_IMG_CONTENT:
		netgl_recv_img_content (world, glctx, (void*)pkg);
		break;
	}
}












