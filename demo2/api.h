#include "csc/csc_crossos.h"
#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_sdl_motion.h"
#include "csc/csc_gcam.h"
#include "csc/csc_gl.h"
#include "csc/csc_math.h"
#include "csc/csc_sdlglew.h"
#include "csc/csc_dod.h"
#include "csc/csc_glimage.h"
#include "csc/csc_glpointcloud.h"
#include "csc/csc_gltex.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>



enum mytype
{
	MYTYPE_TEXSTORAGE,
	MYTYPE_GLIMG,
	MYTYPE_V4F32
};


struct myapi_package
{
	uint32_t type;
	union
	{
		struct
		{
			uint32_t tbo;
			uint32_t width;
			uint32_t height;
			uint32_t layers;
			uint32_t unit;
		} texstorage;
		struct
		{
			uint32_t tbo;
			uint32_t layer;
			uint32_t size;
		} texdata;
		struct csc_glimg glimg;
	};
	uint8_t data[0];
};

void myapi_send(struct myapi_package * pkg)
{

}


void myapi_recv
(
		struct myapi_package * pkg,
		struct csc_gltexcontext * texctx,
		struct csc_glimgcontext * imgctx
)
{
	switch (pkg->type)
	{
	case MYTYPE_TEXSTORAGE:
		texctx->tex[pkg->texstorage.tbo].width = pkg->texstorage.width;
		texctx->tex[pkg->texstorage.tbo].height = pkg->texstorage.height;
		texctx->tex[pkg->texstorage.tbo].layers = pkg->texstorage.layers;
		texctx->tex[pkg->texstorage.tbo].unit = pkg->texstorage.unit;
		break;
	case MYTYPE_GLIMG:
		csc_glimage_update (imgctx, &pkg->glimg, 1);
		break;
	}
}


void myapi_test()
{
	struct myapi_package p1;
	p1.type = MYTYPE_TEXSTORAGE;
	p1.texstorage.tbo = 0;
	p1.texstorage.layers = 4;
	p1.texstorage.width = 256;
	p1.texstorage.height = 256;
	myapi_send (&p1);


	struct myapi_package p2;
	p2.texdata.tbo = 0;
	p2.texdata.layer = 0;
	p2.texdata.size = 256*256*4;
	myapi_send (&p2);
}

