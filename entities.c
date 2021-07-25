#include <stdlib.h>

#include <rbt.h>

#include "entities.h"

struct rbt_tree *
add_component(struct rbt_tree * this, enum component type, int value)
{
	int * x = malloc(sizeof(value));
	if (!x) {return this;}
	*x = value;
	struct rbt_tree * t = malloc(sizeof(struct rbt_tree));
	if (!t) {free(x); return this;}
	t->left = NULL;
	t->right = NULL;
	t->data = x;
	t->type = type;
	t->is_red = 0;
	return rbt_insert(this, t);
}

struct rbt_tree *
phoenix(int x, int y)
{
	struct rbt_tree * t = NULL;
	t = add_component(t, COMP_X, x);
	t = add_component(t, COMP_Y, y);
	t = add_component(t, COMP_HP, 30);
	t = add_component(t, COMP_MAX_HP, 30);
	t = add_component(t, COMP_SPR, 14);
	t = add_component(t, COMP_SPR_WIDTH, 2);
	t = add_component(t, COMP_SPR_HEIGHT, 2);
	t = add_component(t, COMP_TYPE, ENTITY_PHOENIX);
	return t;
}

struct rbt_tree *
egg(int x, int y)
{
	struct rbt_tree * t = NULL;
	t = add_component(t, COMP_X, x);
	t = add_component(t, COMP_Y, y);
	t = add_component(t, COMP_SPR, 13);
	t = add_component(t, COMP_SPR_WIDTH, 1);
	t = add_component(t, COMP_SPR_HEIGHT, 1);
	t = add_component(t, COMP_TYPE, ENTITY_EGG);
	t = add_component(t, COMP_HATCH_TIME, 2);

	return t;
}

struct rbt_tree *
slime(int x, int y)
{
	struct rbt_tree * t = NULL;
	t = add_component(t, COMP_X, x);
	t = add_component(t, COMP_Y, y);
	t = add_component(t, COMP_HP, 10);
	t = add_component(t, COMP_MAX_HP, 10);
	t = add_component(t, COMP_SPR, 29);
	t = add_component(t, COMP_SPR_WIDTH, 1);
	t = add_component(t, COMP_SPR_HEIGHT, 1);
	t = add_component(t, COMP_TYPE, ENTITY_SLIME);
	return t;
}
