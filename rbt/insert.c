/* rbt/insert.c
 * Copyright (c) 2021, Dakotah Lambert
 */
#include <stddef.h>
#include <stdint.h>

#include "rbt.h"

struct rbt_tree *
rbt_insert(struct rbt_tree * const this, struct rbt_tree * const data)
{
	if (!data) {return this;}
	if (!this) {
		data->is_red = 0;
		return data;
	}
	data->is_red = 1;
	struct rbt_tree * gg = NULL;
	struct rbt_tree * g = NULL;
	struct rbt_tree * p = this;
	unsigned int tag = data->type;
	while (1)
	{
		unsigned int ptag = p->type;
		if (tag <= ptag)
		{
			if (p->left)
			{
				gg = g;
				g = p;
				p = p->left;
			}
			else
			{
				p->left = data;
				break;
			}
		}
		else if (p->right)
		{
			gg = g;
			g = p;
			p = p->right;
		}
		else
		{
			p->right = data;
			break;
		}
	}
	/* data has been inserted somewhere;
	 * p is data's parent, g is p's parent, and gg is g's parent
	 */
	if (!(p->is_red)) {return this;} /* all properties preserved */
	/* data and p are both red; a violation
	 * since the root is always black, g is set
	 * gg may be NULL
	 */
	struct rbt_tree * const u = (g->left == p) ? g->right : g->left;
	if (u && u->is_red)
	{
		/* parent and uncle both red; flip both, and grandparent */
		p->is_red = 0;
		u->is_red = 0;
		if (g != this) /* leave root black */
		{
			g->is_red = 1;
			/* this might incur a red-violation; fix that.
			 * the slow way: remove g and re-add
			 */
			if (!gg) {return this;}
			if (gg->left == g) {gg->left = NULL;}
			else {gg->right = NULL;}
			return rbt_insert(this, g);
		}
		return this;
	}
	g->is_red = 1;
	if (p == g->right)
	{
		if (data == p->right)
		{
			p->is_red = 0;
			if (gg)
			{
				if (gg->left == g)
				{
					gg->left = rbt_rotate_left(g);
				}
				else
				{
					gg->right = rbt_rotate_left(g);
				}
				return this;
			}
			return rbt_rotate_left(g);
		}
		data->is_red = 0;
		g->right = rbt_rotate_right(p);
		if (gg)
		{
			if (gg->left == g)
			{
				gg->left = rbt_rotate_left(g);
			}
			else
			{
				gg->right = rbt_rotate_left(g);
			}
			return this;
		}
		return rbt_rotate_left(g);
	}
	if (data == p->left)
	{
		p->is_red = 0;
		if (gg)
		{
			if (gg->left == g)
			{
				gg->left = rbt_rotate_right(g);
			}
			else
			{
				gg->right = rbt_rotate_right(g);
			}
			return this;
		}
		return rbt_rotate_right(g);
	}
	data->is_red = 0;
	g->left = rbt_rotate_left(p);
	if (gg)
	{
		if (gg->left == g)
		{
			gg->left = rbt_rotate_right(g);
		}
		else
		{
			gg->right = rbt_rotate_right(g);
		}
		return this;
	}
	return rbt_rotate_right(g);
}
