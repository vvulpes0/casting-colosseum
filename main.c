#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <pocketmod.h>
#include <rbt.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "audio.h"
#include "entities.h"
#include "queue.h"

#define TILE_SIZE 16
#define FONT_WIDTH 10
#define SCR_WIDTH 160
#define SCR_HEIGHT 144

enum {CD_NONE=0,CD_RIGHT=1,CD_UP=2,CD_LEFT=3,CD_DOWN=4};

static char const * song_path="songs/untitled.mod";
static char const * sprite_path="images/spritesheet.png";
static float * scratch;

static char const * sfx_paths[] = {
	"sfx/bweeop.mod",
	"sfx/fssssh.mod"
};

enum {
	SPELL_NONE = 0,
	SPELL_WATER = 1,
	SPELL_FIRE = 2,
	SPELL_HEAL = 3,
	SPELL_GUARD = 4
};
static int spell_seqs[][10] = {
	{-1},
	{CD_RIGHT, CD_RIGHT, CD_LEFT,  CD_LEFT, CD_RIGHT, CD_RIGHT, CD_UP, -1},
	{CD_RIGHT, CD_LEFT,  CD_RIGHT, CD_UP,   -1},
	{CD_DOWN,  CD_DOWN,  CD_UP,    -1},
	{CD_UP,    CD_NONE,  CD_UP,    CD_NONE, -1}
};

struct trie {
	struct trie * dir[5];
	int type;
};
static void trie_add(struct trie *, int const *, int);
static void free_trie(struct trie *);
static void free_tree(struct rbt_tree *);

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
static void show_cast(SDL_Renderer *, SDL_Texture *, struct queue *);
static void show_spellbook(SDL_Renderer *, SDL_Texture *);

static void init_spells(struct trie *);

static void draw_entity(SDL_Renderer *, SDL_Texture *, struct rbt_tree *);
static void draw_player(SDL_Renderer *, SDL_Texture *,
                        int cdir, int hp, int max_hp, int x, int y);
static void draw_hp_bar(SDL_Renderer * renderer, SDL_Texture * spritesheet,
                        int hp, int max_hp, int x, int y);

static int parse_cast(struct trie *, struct queue *, int *, int *, int *);

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
	window = SDL_CreateWindow("Casting Colosseum",
	                          SDL_WINDOWPOS_UNDEFINED,
	                          SDL_WINDOWPOS_UNDEFINED,
	                          3 * SCR_WIDTH, 3 * SCR_HEIGHT,
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

	struct trie * spells = malloc(sizeof(struct trie));
	if (!sprites || !spells)
	{
		printf("Failed to load sprites or allocate spells\n");
		free(context);
		context = NULL;
		free(mod_data);
		mod_data = NULL;
		free(scratch);
		scratch = NULL;
		SDL_CloseAudioDevice(device);
		return -1;
	}
	init_spells(spells);

	SDL_PauseAudioDevice(device, 0);
	SDL_Event event;
	i = 0;
	int cdir = 0;
	char eighths[5];
	int es = 0;
	int upgraded = 0;
	snprintf(eighths,5,"%4d",es);
	int modulate = 2;
	if (manager.bgm->ticks_per_line <= 3)
	{
		modulate = 4;
	}
	struct queue cast = {.head = NULL, .tail = NULL};
	struct queue entities = {.head = NULL, .tail = NULL};
	int max_hp = 30;
	int hp = max_hp;
	queue_add(&entities, phoenix(96, 16));
	int proceed = 1;
	while (proceed)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 77, 190, 255, 255);
		SDL_RenderFillRect(renderer, NULL);
		if (0 == (manager.bgm->line & modulate) && !upgraded)
		{
			upgraded = 1;
			es += 1;
			es %= 10000;
			snprintf(eighths,5,"%4d",es);
			int qs = queue_size(&cast);
			if ((qs > 0 || cdir) && qs < 11)
			{
				int * x = malloc(sizeof(int));
				if (x) {
					*x = cdir;
					queue_add(&cast, x);
				}
			}
		}
		else if (0 != (manager.bgm->line & modulate))
		{
			upgraded = 0;
		}
		else
		{
			if (cast.tail && cast.tail->data)
			{
				if (cast.tail != cast.head || cdir)
				{
					*(int*)(cast.tail->data) = cdir;
				}
			}
			else if (cast.tail != cast.head || cdir)
			{
				int * x = malloc(sizeof(int));
				if (x) {
					*x = cdir;
					queue_add(&cast,x);
				}
			}
		}
		struct queue_list * entity = entities.head;
		while (entity)
		{
			draw_entity(renderer, texture, entity->data);
			entity = entity->next;
		}
		draw_player(renderer, texture, cdir, hp, max_hp, 24, 64);
		show_cast(renderer, texture, &cast);
		show_spellbook(renderer, texture);
		SDL_RenderPresent(renderer);
		int do_cast = 0;
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
					do_cast = 1;
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
		if (do_cast)
		{
			int dmg_f = 0;
			int dmg_w = 0;
			int heal = 0;
			int block =
				parse_cast(spells, &cast,
				           &dmg_f,
				           &dmg_w,
				           &heal);
			struct queue_list * entity_n = entities.head;
			while (entity_n)
			{
				int destroy = 0;
				struct rbt_tree * entity
					= (struct rbt_tree*)(entity_n->data);
				struct rbt_tree * c;
				c = rbt_find(entity, COMP_TYPE);
				int t = -1;
				if (c && c->data) {t = *(int*)(c->data);}
				if (t == ENTITY_PHOENIX && dmg_f != 0)
				{
					int entity_x = 0;
					c = rbt_find(entity, COMP_X);
					if (c && c->data)
					{
						entity_x = *(int*)(c->data);
					}
					struct queue_list * t = entities.tail;
					struct rbt_tree * tx =
						(struct rbt_tree *)(t->data);
					tx = rbt_find(tx, COMP_X);
					if (tx && tx->data)
					{
						entity_x = *(int*)(tx->data);
					}
					queue_add(&entities,
					          egg(entity_x + TILE_SIZE,
					              60));
				}
				int emhp = 0;
				c = rbt_find(entity, COMP_MAX_HP);
				if (c && c->data) {emhp = *(int*)(c->data);}
				int ehp = 1;
				c = rbt_find(entity, COMP_HP);
				if (c && c->data) {ehp = *(int*)(c->data);}
				switch (t)
				{
				case ENTITY_PHOENIX:
					ehp -= dmg_w;
					if (ehp < 0) {ehp = 0;}
					if (!block){hp -= 8;}
					break;
				case ENTITY_SLIME:
					ehp -= dmg_f;
					ehp += dmg_w;
					if (ehp < 0) {ehp = 0;}
					if (ehp > emhp) {ehp = emhp;}
					if (!block){hp -= 4;}
					break;
				default:
					break;
				}
				if (c && c->data)
				{
					*(int*)(c->data) = ehp;
				}
				if (ehp < 1) {destroy = 1;}
				c = rbt_find(entity, COMP_HATCH_TIME);
				if (c && c->data)
				{
					*(int*)(c->data) -= 1;
					if (*(int*)(c->data) < 1)
					{
						int ecx;
						int ecy;
						struct rbt_tree * ec;
						ec = rbt_find(entity, COMP_X);
						if (ec && ec->data)
						{
							ecx = *(int*)ec->data;
						}
						ec = rbt_find(entity, COMP_Y);
						if (ec && ec->data)
						{
							ecy = *(int*)ec->data;
						}
						queue_add(&entities,
						          (rand() % 2)
						          ? phoenix(ecx, 16)
						          : slime(ecx, ecy));
						destroy = 1;
					}
				}
				struct queue_list * nxt = entity_n->next;
				if (destroy)
				{
					if (entities.head == entity_n)
					{
						entities.head
							= entity_n->next;
					}
					else
					{
						entity_n->prev->next
							= entity_n->next;
					}
					if (entities.tail == entity_n)
					{
						entities.tail
							= entity_n->prev;
					}
					else
					{
						entity_n->next->prev
							= entity_n->prev;
					}
					free_tree(entity);
					free(entity_n);
				}
				entity_n = nxt;
			}
			hp += heal;
			if (hp <= 0) {proceed = 0;}
			free_queue_and_data(&cast);
			cdir=0;
		}
	}

	struct sfx_list * s = manager.sfx;
	while (s)
	{
		manager.sfx = s->next;
		free(s);
		s = manager.sfx;
	}
	free_queue_and_data(&cast);
	free_queue_and_data(&entities);
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

static void
show_cast(SDL_Renderer * renderer, SDL_Texture * spritesheet,
          struct queue * cast)
{
	if (!renderer || !spritesheet || !cast) {return;}
	SDL_Rect r = {.x = 0, .y = 0, .w = SCR_WIDTH, .h = TILE_SIZE};
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &r);
	struct queue_list * node = cast->head;
	int dir = -1;
	int pdir = -1;
	int n = 0;
	int y = 0;//SCR_HEIGHT * 3 / 4;
	for (int i = 0; i < 10; ++i)
	{
		int x = i * TILE_SIZE;
		pdir = dir;
		dir = -1;
		if (node && node->data)
		{
			dir = *(int*)(node->data);
		}
		if (dir < 0)
		{
			n = 22;
		}
		else if (dir != pdir)
		{
			n = 16 + dir;
		}
		else
		{
			n = 21;
		}
		spr(renderer, spritesheet, n, 1, 1, x, y);
		if (node) {node = node->next;}
	}
}

static void
init_spell_trie(struct trie * t)
{
	if (!t) {return;}
	for(int i = 0; i < 5; ++i)
	{
		(t->dir)[i] = NULL;
		t->type = SPELL_NONE;
	}
}

static void
trie_add(struct trie * spells, int const * seq, int type)
{
	if (!spells || !seq) {return;}
	while (*seq >= 0)
	{
		struct trie * x = (spells->dir)[*seq];
		if (!x)
		{
			x = malloc(sizeof(struct trie));
			if (!x)
			{
				return;
			}
			init_spell_trie(x);
			(spells->dir)[*seq] = x;
		}
		spells = x;
		++seq;
	}
	spells->type = type;
}

static void
init_spells(struct trie * spells)
{
	init_spell_trie(spells);
	for (int i = 0; i < sizeof(spell_seqs)/sizeof(spell_seqs[0]); ++i)
	{
		trie_add(spells, spell_seqs[i], i);
	}
}

static void
free_trie(struct trie * this)
{
	for (int i = 0; i < 5; ++i)
	{
		free_trie(this->dir[i]);
	}
	free(this);
}

static void
show_spellbook(SDL_Renderer * renderer, SDL_Texture * spritesheet)
{
	SDL_SetRenderDrawColor(renderer, 0, 64, 0, 255);
	SDL_Rect r = {
		.x = 0,
		.y = SCR_HEIGHT - 4 * TILE_SIZE,
		.w = SCR_WIDTH,
		.h = 4 * TILE_SIZE
	};
	SDL_RenderFillRect(renderer, &r);
	for (int i = 0; i < 4; ++i)
	{
		int y = r.y + i * TILE_SIZE;
		spr(renderer, spritesheet, 5+i, 1, 1, 0, y);
		int j = 0;
		while (spell_seqs[i+1][j] >= 0)
		{
			int n = spell_seqs[i+1][j];
			if (j != 0 && n == spell_seqs[i+1][j-1])
			{
				n = 5;
			}
			spr(renderer, spritesheet,
			    16+n, 1, 1,
			    (j+1) * TILE_SIZE, y);
			++j;
		}
	}
}

static void
free_tree(struct rbt_tree * this)
{
	if (!this) {return;}
	if (this->data) {free(this->data); this->data = NULL;}
	free_tree(this->left);
	this->left = NULL;
	free_tree(this->right);
	this->right = NULL;
	free(this);
}

static void
draw_entity(SDL_Renderer * renderer, SDL_Texture * spritesheet,
            struct rbt_tree * this)
{
	struct rbt_tree * c;
	int x = 0;
	int y = 0;
	int w = 1;
	int h = 1;
	int s = 0;
	int hp = 0;
	int mhp = 0;
	c = rbt_find(this, COMP_X);
	if (c && c->data) {x = *(int*)(c->data);}
	c = rbt_find(this, COMP_Y);
	if (c && c->data) {y = *(int*)(c->data);}
	c = rbt_find(this, COMP_HP);
	if (c && c->data) {hp = *(int*)(c->data);}
	c = rbt_find(this, COMP_MAX_HP);
	if (c && c->data) {mhp = *(int*)(c->data);}
	c = rbt_find(this, COMP_SPR);
	if (c && c->data) {s = *(int*)(c->data);}
	c = rbt_find(this, COMP_SPR_WIDTH);
	if (c && c->data) {w = *(int*)(c->data);}
	c = rbt_find(this, COMP_SPR_HEIGHT);
	if (c && c->data) {h = *(int*)(c->data);}
	spr(renderer, spritesheet, s, w, h, x, y);
	if (mhp)
	{
		draw_hp_bar(renderer, spritesheet, hp, mhp,
		            x + (TILE_SIZE * (w - 1)) / 2,
		            y + TILE_SIZE * h - 7);
	}
}

static void
draw_player(SDL_Renderer * renderer, SDL_Texture * spritesheet,
            int cdir, int hp, int max_hp, int x, int y)
{
	spr(renderer, spritesheet, cdir, 1, 1, x, y);
	draw_hp_bar(renderer, spritesheet, hp, max_hp, x, y-TILE_SIZE);
}

static void
draw_hp_bar(SDL_Renderer * renderer, SDL_Texture * spritesheet,
            int hp, int max_hp, int x, int y)
{
	spr(renderer, spritesheet, 23, 1, 1, x, y);
	if (hp < 1) {return;}
	int w = hp * (TILE_SIZE - 2) / max_hp;
	SDL_Rect f = {.x = x+1, .y = y+7, .w = w, .h = 1};
	int r;
	int g;
	int b;
	if (max_hp / hp >= 6)
	{
		r = 222; g = 125; b = 45;
	}
	else if (max_hp / hp >= 3)
	{
		r = 255; g = 198; b = 75;
	}
	else
	{
		r = 0; g = 179; b = 131;
	}
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
	SDL_RenderFillRect(renderer, &f);
}

static int
parse_cast(struct trie * spells, struct queue * cast,
           int * dmg_fire, int * dmg_water, int * healed)
{
	int out = 0;
	if (!cast || !spells) {return 0;}
	if (!dmg_fire || !dmg_water || !healed) {return 0;}
	struct queue_list * h = cast->head;
	struct trie * s = spells;
	while (h)
	{
		int x = 0;
		if (h && h->data) x = *(int*)(h->data);
		s = s->dir[x];
		if (s)
		{
			switch (s->type)
			{
			case SPELL_WATER:
				*dmg_water += 5;
				break;
			case SPELL_FIRE:
				*dmg_fire += 5;
				break;
			case SPELL_HEAL:
				*healed += 5;
				break;
			case SPELL_GUARD:
				out = 1;
				break;
			default:
				break;
			}
			if (s->type != SPELL_NONE)
			{
				s = spells;
			}
		}
		else /* if (s) */
		{
			s = spells;
		}
		h = h->next;
	}
	return out;
}
