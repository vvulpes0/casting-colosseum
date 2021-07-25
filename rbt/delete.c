/* rbt/delete.c
 * Copyright (c) 2021, Dakotah Lambert
 */
#include <stddef.h>

#include "rbt.h"

/* for now, an inefficent deletion */
struct rbt_tree *
rbt_delete(struct rbt_tree * const this, struct rbt_tree * const data)
{
	struct rbt_tree * out = NULL;
	struct rbt_tree * t = this;
	while (t)
	{
		struct rbt_tree * p = NULL;
		struct rbt_tree * m = t;
		while (m->left)
		{
			p = m;
			m = m->left;
		}
		if (p)
		{
			p->left = m->right;
		}
		else
		{
			t = m->right;
		}
		m->left = NULL;
		m->right = NULL;
		if (m != data)
		{
			out = rbt_insert(out, m);
		}
	}
	return out;
}
