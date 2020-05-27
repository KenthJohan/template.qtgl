#pragma once

#include <SDL2/SDL.h>
#include "csc/csc_sdlcam.h"




/*
glm::vec3 CFreeCamera::CreateRay() {
	// these positions must be in range [-1, 1] (!!!), not [0, width] and [0, height]
	float mouseX = getMousePositionX() / (getWindowWidth()  * 0.5f) - 1.0f;
	float mouseY = getMousePositionY() / (getWindowHeight() * 0.5f) - 1.0f;

	glm::mat4 proj = glm::perspective(FoV, AspectRatio, Near, Far);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f), CameraDirection, CameraUpVector);

	glm::mat4 invVP = glm::inverse(proj * view);
	glm::vec4 screenPos = glm::vec4(mouseX, -mouseY, 1.0f, 1.0f);
	glm::vec4 worldPos = invVP * screenPos;

	glm::vec3 dir = glm::normalize(glm::vec3(worldPos));

	return dir;
}
*/

static void proj1 (SDL_Window * window, struct csc_sdlcam * cam, float world[4])
{
	int x;
	int y;
	SDL_GetMouseState (&x, &y);
	int w;
	int h;
	SDL_GetWindowSize (window, &w, &h);

	float clip[4];
	clip[0] = 2.0f * ((float)x / (float)w) - 1.0f;
	clip[1] = 1.0f - 2.0f * ((float)y / (float)h);
	clip[2] = -1.0f;
	clip[3] = 1.0f;
	//printf ("(%i %i) (%f %f)\n", x, y, clip[0], clip[1]);


	// 4D Eye (Camera) Coordinates
	float p[4*4];
	float eye[4] = {0.0f};
	m4f32_perspective1 (p, cam->fov, cam->w/cam->h, cam->n, cam->f); //Create perspective matrix
	m4f32_glu_inv (p, p);
	mv4f32_macc (eye, p, clip);
	eye[2] = -1.0f;
	eye[3] = 0.0f;

	// 4D World Coordinates
	float view[4*4];
	m4f32_mul (view, cam->mr, cam->mt);
	m4f32_glu_inv (view, view);
	v4f32_set1 (world, 0.0f);
	mv4f32_macc (world, view, eye);
	v4f32_normalize (world, world);
}



static void proj2 (SDL_Window * window, struct csc_sdlcam * cam, float world[4])
{
	int x;
	int y;
	SDL_GetMouseState (&x, &y);
	int w;
	int h;
	SDL_GetWindowSize (window, &w, &h);

	float clip[4];
	clip[0] = 2.0f * ((float)x / (float)w) - 1.0f;
	clip[1] = 2.0f * ((float)y / (float)h) - 1.0f;
	clip[1] *= -1;
	clip[2] = 1.0f;
	clip[3] = 1.0f;
	//printf ("(%i %i) (%f %f)\n", x, y, clip[0], clip[1]);
	//printf (V4F32_FORMAT, V4F32_ARGS (clip));
	m4f32_glu_inv (cam->mvp, cam->mvp);
	mv4f32_mul (world, cam->mvp, clip);
	v4f32_normalize (world, world);
}
