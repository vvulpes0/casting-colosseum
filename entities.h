#ifndef ENTITIES_H
#define ENTITIES_H

enum component {
	COMP_X,
	COMP_Y,
	COMP_HP,
	COMP_MAX_HP,
	COMP_SPR,
	COMP_SPR_WIDTH,
	COMP_SPR_HEIGHT,
	COMP_TYPE,
	COMP_HATCH_TIME
};

enum entity_types {
	ENTITY_PHOENIX,
	ENTITY_EGG,
	ENTITY_SLIME,
};

struct rbt_tree * add_component(struct rbt_tree *, enum component, int);
struct rbt_tree * phoenix(int x, int y);
struct rbt_tree * egg(int x, int y);
struct rbt_tree * slime(int x, int y);

#endif
