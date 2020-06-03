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


#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Texture Demo"

#define MAIN_RUNNING    UINT32_C (0x00000001)
#define MAIN_FULLSCREEN UINT32_C (0x00000002)

#define main_glattr_pos 0
#define main_glattr_col 1
#define main_glattr_tex 2


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



int main (int argc, char * argv[])
{
	csc_crossos_enable_ansi_color ();
	ASSERT (argc);
	ASSERT (argv);


	uint32_t main_flags = MAIN_RUNNING;
	SDL_Window * window;
	SDL_Init (SDL_INIT_VIDEO);
	window = SDL_CreateWindow (WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL);
	if (window == NULL)
	{
		fprintf (stderr, "Could not create SDL_Window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GLContext context = SDL_GL_CreateContext (window);
	if (context == NULL)
	{
		fprintf (stderr, "Could not create SDL_GLContext: %s\n", SDL_GetError());
		return 1;
	}
	glewExperimental = 1;
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to setup GLEW\n");
		exit(1);
	}

	// Enable the debug callback
	glEnable (GL_DEBUG_OUTPUT);
	glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback (openglCallbackFunction, NULL);
	//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
	glDebugMessageControl (GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);//Disable all messages
	glDebugMessageControl (GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);//Enable error messages


	//glEnable (GL_DEPTH_TEST);
	//glEnable (GL_PROGRAM_POINT_SIZE_EXT);
	//glEnable (GL_VERTEX_PROGRAM_POINT_SIZE);
	//glEnable (GL_TEXTURE_2D);
	//glPointSize (50.0f);
	//glLineWidth (20.0f);


	char const * shaderfiles[] = {"../texture/shader2.glvs", "../texture/shader2.glfs", NULL};
	GLuint shader_program = csc_gl_program_from_files (shaderfiles);
	glBindAttribLocation (shader_program, main_glattr_pos, "pos" );
	glBindAttribLocation (shader_program, main_glattr_col, "col" );
	glBindAttribLocation (shader_program, main_glattr_tex, "tex" );
	glLinkProgram (shader_program);
	glUseProgram (shader_program);
	glUniform1i (glGetUniformLocation (shader_program, "texture1"), 0);




	float vertices[] = {
	// positions          // colors           // texture coords
	0.5f,  0.5f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // top right
	0.5f, -0.5f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f,   1.0f, 0.0f, // bottom right
	-0.5f, -0.5f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f, 1.0f,   0.0f, 0.0f, // bottom left
	-0.5f,  0.5f, 0.0f, 1.0f,   1.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f  // top left
	};
	unsigned int indices[] = {
	0, 1, 3, // first triangle
	1, 2, 3  // second triangle
	};
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(main_glattr_pos, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(main_glattr_pos);
	// color attribute
	glVertexAttribPointer(main_glattr_col, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
	glEnableVertexAttribArray(main_glattr_col);
	// texture coord attribute
	glVertexAttribPointer(main_glattr_tex, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(main_glattr_tex);

	unsigned int texture1;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	{
		int width = 100;
		int height = 100;
		uint8_t * data = NULL;
		unsigned size = width*height*4*sizeof(uint8_t);
		data = malloc (size);
		for (unsigned i = 0; i < size; ++i)
		{
			data[i] = rand();
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	const Uint8 * keyboard = SDL_GetKeyboardState (NULL);
	while (main_flags & MAIN_RUNNING)
	{
		SDL_Event Event;
		while (SDL_PollEvent (&Event))
		{
			switch (Event.type)
			{
			case SDL_QUIT:
				main_flags &= ~MAIN_RUNNING;
				break;
			}

		}


		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram (shader_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		SDL_Delay (10);
		SDL_GL_SwapWindow (window);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
