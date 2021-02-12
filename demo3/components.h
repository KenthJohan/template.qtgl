#pragma once

#include <flecs.h>
#include <GL/glew.h>

#include "csc/csc_math.h"
#include "csc/csc_gl.h"
#include "csc/csc_gcam.h"

typedef uint32_t component_color;
typedef v4f32 component_position;
typedef v4f32 component_scale;
typedef v4f32 component_quaternion;
typedef v2f32 component_uv;
typedef v2f32 component_wh;





ECS_COMPONENT_DECLARE (component_color);
ECS_COMPONENT_DECLARE (component_position);
ECS_COMPONENT_DECLARE (component_scale);
ECS_COMPONENT_DECLARE (component_quaternion);
ECS_COMPONENT_DECLARE (component_uv);
ECS_COMPONENT_DECLARE (component_wh);
ECS_TAG_DECLARE (tag_glpoints);
ECS_TAG_DECLARE (tag_glimgs);
ECS_TAG_DECLARE (tag_gltriangles);

enum vao_type
{
	VAO_TRIANGLES,
	VAO_POINTS,
	VAO_IMGS,
	VAO_COUNT
};

enum vbo_type
{
	VBO_TRIANGLES_POS,
	VBO_POINTS_POS,
	VBO_POINTS_COL,
	VBO_IMGS_POS,
	VBO_IMGS_UV,
	VBO_COUNT
};

enum vto_type
{
	VTO1,
	VTO2,
	VTO_COUNT
};

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



static GLuint global_vao[VAO_COUNT];
static GLuint global_vbo[VBO_COUNT];
static GLuint global_vto[VTO_COUNT];
static GLint global_glprogram[GLPROGRAM_COUNT];
static GLint global_gluniform[GLUNIFORM_COUNT];
static struct csc_gcam global_gcam;

static void system_render_points(ecs_iter_t *it)
{
	ECS_COLUMN (it, component_position, position, 1);
	glBindVertexArray (global_vao[VAO_POINTS]);
	for (int32_t i = 0; i < it->count; ++i)
	{

	}
	glBindBuffer (GL_ARRAY_BUFFER, global_vbo[VBO_POINTS_POS]);
	glBufferSubData (GL_ARRAY_BUFFER, 0, it->count * sizeof (component_position), position);
	glUseProgram (global_glprogram[GLPROGRAM_POINTS]);
	glUniformMatrix4fv (global_gluniform[GLUNIFORM_POINTS_MVP], 1, GL_FALSE, (const GLfloat *) global_gcam.mvp);
	glDrawArrays (GL_POINTS, 0, it->count);
}


static void system_render_imgs(ecs_iter_t *it)
{
	ECS_COLUMN (it, component_position, p, 1);
	ECS_COLUMN (it, component_scale, s, 2);
	ECS_COLUMN (it, component_quaternion, q, 3);
	glBindVertexArray (global_vao[VAO_IMGS]);
	glUseProgram (global_glprogram[GLPROGRAM_IMGS]);
	glUniform1i (global_gluniform[GLUNIFORM_IMGS_TEX0], 0);
	for (int32_t i = 0; i < it->count; ++i)
	{
		float m[4*4];
		float mt[4*4];
		float mr[4*4];
		m4f32_identity (m);
		m4f32_identity (mr);
		m4f32_identity (mt);
		m4f32_translation (mt, p[i]);
		m4f32_scale_xyz (mt, s[i][0], s[i][1], s[i][2]);
		qf32_m4 (mr, q[i]);
		m4f32_mul (m, mt, m); //Apply translation
		m4f32_mul (m, mr, m); //Apply rotation
		m4f32_mul (m, global_gcam.mvp, m);
		//m4f32_print (global_gcam.mvp, stdout);
		glUniformMatrix4fv (global_gluniform[GLUNIFORM_IMGS_MVP], 1, GL_FALSE, (const GLfloat *) m);
		glDrawArrays (GL_TRIANGLES, i * 6, 6);
	}
}



static void components_init (ecs_world_t * world)
{
	ECS_COMPONENT_DEFINE(world, component_color);
	ECS_COMPONENT_DEFINE(world, component_position);
	ECS_COMPONENT_DEFINE(world, component_scale);
	ECS_COMPONENT_DEFINE(world, component_quaternion);
	ECS_COMPONENT_DEFINE(world, component_uv);
	ECS_COMPONENT_DEFINE(world, component_wh);
	ECS_TAG_DEFINE(world, tag_glpoints);
	ECS_TAG_DEFINE(world, tag_glimgs);
	ECS_TAG_DEFINE(world, tag_gltriangles);
	ECS_SYSTEM(world, system_render_points, EcsOnUpdate, component_position, tag_glpoints);
	ECS_SYSTEM(world, system_render_imgs, EcsOnUpdate, component_position, component_scale, component_quaternion, tag_glimgs);

	global_glprogram[GLPROGRAM_TRIANGLES] = csc_gl_program_from_files1 (CSC_SRCDIR"shader_pointcloud.glvs;"CSC_SRCDIR"shader_pointcloud.glfs");
	global_glprogram[GLPROGRAM_POINTS] = csc_gl_program_from_files1 (CSC_SRCDIR"shader_pointcloud.glvs;"CSC_SRCDIR"shader_pointcloud.glfs");
	global_glprogram[GLPROGRAM_IMGS] = csc_gl_program_from_files1 (CSC_SRCDIR"shader_image.glvs;"CSC_SRCDIR"shader_image.glfs");
	glLinkProgram (global_glprogram[GLPROGRAM_TRIANGLES]);
	glLinkProgram (global_glprogram[GLPROGRAM_POINTS]);
	glLinkProgram (global_glprogram[GLPROGRAM_IMGS]);

	global_gluniform[GLUNIFORM_TRIANGLES_MVP] = glGetUniformLocation (global_glprogram[GLPROGRAM_TRIANGLES], "mvp");
	global_gluniform[GLUNIFORM_POINTS_MVP] = glGetUniformLocation (global_glprogram[GLPROGRAM_POINTS], "mvp");
	global_gluniform[GLUNIFORM_IMGS_MVP] = glGetUniformLocation (global_glprogram[GLPROGRAM_IMGS], "mvp");
	global_gluniform[GLUNIFORM_IMGS_TEX0] = glGetUniformLocation (global_glprogram[GLPROGRAM_IMGS], "tex0");

	glGenVertexArrays (VAO_COUNT, global_vao);
	glGenBuffers (VBO_COUNT, global_vbo);
	glGenTextures (VAO_COUNT, global_vao);

	{
		glBindVertexArray (global_vao[VAO_POINTS]);
		glBindBuffer (GL_ARRAY_BUFFER, global_vbo[VBO_POINTS_POS]);
		glBufferData (GL_ARRAY_BUFFER, MYGL_MAX_POINTS * sizeof (component_position), NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray (0);
		glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, (void*)(intptr_t)0);
		glBindBuffer (GL_ARRAY_BUFFER, global_vbo[VBO_POINTS_COL]);
		glBufferData (GL_ARRAY_BUFFER, MYGL_MAX_POINTS * sizeof (component_color), NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray (1);
		glVertexAttribPointer (1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)(intptr_t)0);
	}

	{
		glBindVertexArray (global_vao[VAO_IMGS]);
		glBindBuffer (GL_ARRAY_BUFFER, global_vbo[VBO_IMGS_POS]);
		glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, (void*)(intptr_t)0);
		glEnableVertexAttribArray (0);
		component_position * p = malloc (MYGL_MAX_IMGS * 6 * sizeof (component_position));
		for (int32_t i = 0; i < MYGL_MAX_IMGS; ++i)
		{
			csc_gl_make_rectangle_pos ((void*)(p + i*6), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 4);
		}
		glBufferData (GL_ARRAY_BUFFER, MYGL_MAX_IMGS * 6 * sizeof (component_position), p, GL_DYNAMIC_DRAW);
		free (p);
		glBindBuffer (GL_ARRAY_BUFFER, global_vbo[VBO_IMGS_UV]);
		glEnableVertexAttribArray (1);
		glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(intptr_t)0);
		component_uv * uv = malloc (MYGL_MAX_IMGS * 6 * sizeof (component_uv));
		csc_gl_make_rectangle_uv ((void*)uv, MYGL_MAX_IMGS, 2);
		glBufferData (GL_ARRAY_BUFFER, MYGL_MAX_IMGS * 6 * sizeof (component_uv), NULL, GL_DYNAMIC_DRAW);
		free (uv);
	}

	{
		glBindVertexArray (global_vao[VAO_TRIANGLES]);
		glBindBuffer (GL_ARRAY_BUFFER, global_vbo[VBO_TRIANGLES_POS]);
		glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, (void*)(intptr_t)0);
		glBufferData (GL_ARRAY_BUFFER, MYGL_MAX_TRIANGLES * 3 * sizeof (component_position), NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray (0);
	}

	csc_gcam_init (&global_gcam);
	v4f32_set_xyzw (global_gcam.p, 0.0f, 0.0f, -1.0f, 1.0f);
}
