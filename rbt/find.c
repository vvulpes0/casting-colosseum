/* rbt/find.c
 * Copyright (c) 2021, Dakotah Lambert
 */

#include "rbt.h"

struct rbt_tree *
rbt_find(struct rbt_tree * const this, unsigned int const value)
{
	struct rbt_tree * t = this;
	while (t)
	{
		unsigned int const v = t->type;
		if (value == v)
		{
			return t;
		}
		else if (value < v)
		{
			t = t->left;
		}
		else
		{
			t = t->right;
		}
	}
	return t;
}
