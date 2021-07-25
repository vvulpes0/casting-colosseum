/* rbt/rotate_right.c
 * Copyright (c) 2021, Dakotah Lambert
 */

#include "rbt.h"

struct rbt_tree *
rbt_rotate_right(struct rbt_tree * const this)
{
	if (!this) {return this;}
	struct rbt_tree * child = this->left;
	if (!child) {return this;}
	this->left = child->right;
	child->right = this;
	return child;
}
