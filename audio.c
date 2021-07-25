#include <stdio.h>
#include <stdlib.h>
#define POCKETMOD_IMPLEMENTATION
#include <pocketmod.h>

#include "audio.h"

char *
init_data(char const * fp, size_t * mod_size, int rate)
{
	FILE * mod_file = fopen(fp, "rb");
	if (!mod_file) {return NULL;}
	fseek(mod_file, 0L, SEEK_END);
	*mod_size = ftell(mod_file);
	rewind(mod_file);
	char * mod_data = malloc(*mod_size);
	if (!mod_data) {
		fclose(mod_file);
		return NULL;
	}
	size_t i = 0;
	while (i < *mod_size && !ferror(mod_file))
	{
		i += fread(mod_data + i, 1, (*mod_size) - i, mod_file);
	}
	if (ferror(mod_file))
	{
		fclose(mod_file);
		free(mod_data);
		return NULL;
	}
	fclose(mod_file);
	return mod_data;
}

char *
init_context(struct pocketmod_context * context, char const * fp, int rate)
{
	size_t mod_size;
	char * mod_data = init_data(fp, &mod_size, rate);
	if (!mod_data) {return NULL;}
	if (!pocketmod_init(context, mod_data, mod_size, rate))
	{
		free(mod_data);
		return NULL;
	}
	return mod_data;
}

void
mix(struct SoundManager * manager,
    float * restrict buffer, float * restrict * scratch, int bytes)
{
	if (!scratch || !buffer || !manager || !manager->bgm) {return;}
	if (!*scratch) {*scratch = malloc(bytes);}
	if (!*scratch) {return;}
	int i = 0;
	while (i < bytes)
	{
		i += pocketmod_render(manager->bgm,
		                      (char*)(buffer) + i, bytes - i);
	}
	struct sfx_list * p = NULL;
	struct sfx_list * s = manager->sfx;
	while (s)
	{
		i = pocketmod_render(s->data, (char*)(* scratch), bytes);
		for (;i < bytes; ++i)
		{
			((char*)(*scratch))[i] = 0;
		}
		for (i = 0; i < bytes / sizeof(float); ++i)
		{
			buffer[i] += (*scratch)[i];
		}
		if (!pocketmod_loop_count(s->data))
		{
			p = s;
			s = s->next;
			continue;
		}
		if (p)
		{
			p->next = s->next;
			free(s->data);
			free(s);
			s = p->next;
			continue;
		}
		manager->sfx = s->next;
		free(s->data);
		free(s);
		s = manager->sfx;
	}
}

void
enqueue_sfx(struct SoundManager * manager,
            char * data, size_t size, int rate)
{
	struct sfx_list * s = malloc(sizeof(struct sfx_list));
	if (!s) {return;}
	s->data = malloc(sizeof(struct pocketmod_context));
	if (!s->data)
	{
		free(s);
		return;
	}
	if (!pocketmod_init(s->data, data, size, rate))
	{
		free(s);
		return;
	}
	s->next = manager->sfx;
	manager->sfx = s;
}
