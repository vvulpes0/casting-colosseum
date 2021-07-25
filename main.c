#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <pocketmod.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "audio.h"

#define TILE_SIZE 16
#define FONT_WIDTH 10
#define SCR_WIDTH 160
#define SCR_HEIGHT 144

enum {CD_RIGHT=1,CD_UP=2,CD_LEFT=3,CD_DOWN=4};

static char const * song_path="songs/untitled.mod";
static char const * sprite_path="images/spritesheet.png";
static float * scratch;

static char const * sfx_paths[] = {
	"sfx/bweeop.mod",
	"sfx/fssssh.mod"
};
static char * sfx_datas[sizeof(sfx_paths)/sizeof(sfx_paths[0])];
static size_t sfx_sizes[sizeof(sfx_paths)/sizeof(sfx_paths[0])];

static void audio_callback(void *, unsigned char *, int);
static SDL_Texture *
load_spritesheet(char const * restrict, int * restrict, int * restrict,
                 unsigned char **, SDL_Renderer*);
static void sfx(struct SoundManager *, int, int);
static void
spr(SDL_Renderer *, SDL_Texture *, int, int, int, int, int);
static void
text(SDL_Renderer *, SDL_Texture *, char * restrict, int, int);


int
main(int argc, char **argv)
{
	struct SoundManager manager;
	SDL_RWops *mod_file;
	char* mod_data;
	size_t mod_size;
	SDL_AudioSpec format;
	SDL_AudioDeviceID device;
	SDL_Window * window;
	SDL_Renderer * renderer;
	int i;

	if (argc > 1) song_path = argv[1];

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO))
	{
		printf("error: SDL_Init() failed: %s\n", SDL_GetError());
		return -1;
	}
	atexit(SDL_Quit);
	struct pocketmod_context * context
		= malloc(sizeof(struct pocketmod_context));
	if (!context)
	{
		printf("error: can't allocate memory for context");
	}
	manager.bgm = context;
	manager.sfx = NULL;

	format.freq = 44100;
	format.format = AUDIO_F32;
	format.channels = 2;
	format.samples = 2048;
	format.callback = audio_callback;
	format.userdata = &manager;
	device = SDL_OpenAudioDevice(NULL, 0, &format, &format,
	                             SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
	if (!device)
	{
		printf("error: SDL_OpenAudioDevice() failed: %s\n",
		       SDL_GetError());
		return -1;
	}
	window = SDL_CreateWindow("Game!",
	                          SDL_WINDOWPOS_UNDEFINED,
	                          SDL_WINDOWPOS_UNDEFINED,
	                          SCR_WIDTH, SCR_HEIGHT,
	                          SDL_WINDOW_RESIZABLE);
	if (window)
	{
		renderer = SDL_CreateRenderer(window, -1,
		                              SDL_RENDERER_PRESENTVSYNC);
	}
	if (!window || !renderer)
	{
		printf("error: Could not create window: %s\n", SDL_GetError());
		return -1;
	}
	SDL_SetWindowMinimumSize(window, SCR_WIDTH, SCR_HEIGHT);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
	SDL_RenderSetLogicalSize(renderer, SCR_WIDTH, SCR_HEIGHT);
	SDL_RenderSetIntegerScale(renderer, 1);

	/* load audio */
	mod_data = init_context(manager.bgm, song_path, format.freq);
	for (i = 0; i < sizeof(sfx_paths)/sizeof(sfx_paths[0]); ++i)
	{
		sfx_datas[i] = init_data(sfx_paths[i],
		                         sfx_sizes + i,
		                         format.freq);
	}
	/* load graphics */
	int width;
	int height;
	unsigned char * sprites;
	SDL_Texture * texture
		= load_spritesheet(sprite_path, &width, &height,
		                   &sprites, renderer);
	if (!sprites)
	{
		printf("Failed to load sprites\n");
		free(context);
		context = NULL;
		free(mod_data);
		mod_data = NULL;
		free(scratch);
		scratch = NULL;
		SDL_CloseAudioDevice(device);
		return -1;
	}

	SDL_PauseAudioDevice(device, 0);
	SDL_Event event;
	int proceed = 1;
	i = 0;
	int cdir = 0;
	while (proceed)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 77, 190, 255, 255);
		SDL_RenderFillRect(renderer, NULL);
		text(renderer, texture, "HELLO!", 0, 0);
		spr(renderer, texture, 14, 2, 2, 64, 64);
		spr(renderer, texture, cdir, 1, 1, 24, 96);
		SDL_RenderPresent(renderer);
		if (SDL_WaitEventTimeout(&event, 16))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				proceed = 0;
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.scancode)
				{
				case SDL_SCANCODE_W:
					if (cdir == CD_UP) {cdir=0;}
					break;
				case SDL_SCANCODE_A:
					if (cdir == CD_LEFT) {cdir=0;}
					break;
				case SDL_SCANCODE_S:
					if (cdir == CD_DOWN) {cdir=0;}
					break;
				case SDL_SCANCODE_D:
					if (cdir == CD_RIGHT) {cdir=0;}
					break;
				default:
					break;
				}
				break;
			case SDL_KEYDOWN:
				if (event.key.repeat) {break;}
				switch (event.key.keysym.scancode)
				{
				case SDL_SCANCODE_W:
					cdir=CD_UP;
					sfx(&manager, format.freq, 1);
					break;
				case SDL_SCANCODE_A:
					cdir=CD_LEFT;
					sfx(&manager, format.freq, 1);
					break;
				case SDL_SCANCODE_S:
					cdir=CD_DOWN;
					sfx(&manager, format.freq, 1);
					break;
				case SDL_SCANCODE_D:
					cdir=CD_RIGHT;
					sfx(&manager, format.freq, 1);
					break;
				case SDL_SCANCODE_ESCAPE:
					proceed = 0;
					break;
				case SDL_SCANCODE_SPACE:
					sfx(&manager, format.freq, 0);
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}
	}

	struct sfx_list * s = manager.sfx;
	while (s)
	{
		manager.sfx = s->next;
		free(s);
		s = manager.sfx;
	}
	free(context);
	context = NULL;
	free(mod_data);
	mod_data = NULL;
	free(scratch);
	scratch = NULL;

	SDL_CloseAudioDevice(device);
	return 0;
}

static void
sfx(struct SoundManager * manager, int rate, int n)
{
	if (!manager || !sfx_datas[n]) {return;}
	enqueue_sfx(manager, sfx_datas[n], sfx_sizes[n], rate);
}

static void
audio_callback(void * userdata, unsigned char * buffer, int bytes)
{
	mix(userdata, (float *)(buffer), &scratch, bytes);
}

static SDL_Texture *
load_spritesheet(char const * restrict fp,
                 int * restrict w, int * restrict h,
                 unsigned char **data,
                 SDL_Renderer * renderer)
{
	int n;
	*data = stbi_load(fp, w, h, &n, 4);
	if (!data)
	{
		return NULL;
	}
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	Uint32 rmask = 0xFF000000;
	Uint32 gmask = 0x00FF0000;
	Uint32 bmask = 0x0000FF00;
	Uint32 amask = 0x000000FF;
	#else
	Uint32 rmask = 0x000000FF;
	Uint32 gmask = 0x0000FF00;
	Uint32 bmask = 0x00FF0000;
	Uint32 amask = 0xFF000000;
	#endif
	int depth = 32;
	int pitch = 4 * (*w);
	SDL_Surface * surf =
		SDL_CreateRGBSurfaceFrom((void*)(*data), (*w), (*h),
		                         depth, pitch,
		                         rmask, gmask, bmask, amask);
	if (!surf)
	{
		stbi_image_free(data);
		return NULL;
	}
	SDL_Texture * texture =
		SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);
	return texture;
}

static void
spr(SDL_Renderer * renderer, SDL_Texture * spritesheet,
    int n, int w, int h, int x, int y)
{
	int tw;
	int npr;
	SDL_Rect source;
	SDL_Rect dest;
	SDL_QueryTexture(spritesheet, NULL, NULL, &tw, NULL);
	npr = tw / TILE_SIZE;
	source.y = (n / npr) * TILE_SIZE;
	source.x = (n % npr) * TILE_SIZE;
	source.w = TILE_SIZE * w;
	source.h = TILE_SIZE * h;
	dest.x = x;
	dest.y = y;
	dest.w = source.w;
	dest.h = source.h;
	SDL_RenderCopy(renderer, spritesheet, &source, &dest);
}

static void
text(SDL_Renderer * renderer, SDL_Texture * spritesheet,
     char * restrict string, int x, int y)
{
	while (*string)
	{
		spr(renderer, spritesheet, *(string++), 1, 1, x, y);
		x += FONT_WIDTH;
	}
}
