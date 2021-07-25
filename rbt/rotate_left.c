/* rbt/rotate_left.c
 * Copyright (c) 2021, Dakotah Lambert
 */

#include "rbt.h"

struct rbt_tree *
rbt_rotate_left(struct rbt_tree * const this)
{
	if (!this) {return this;}
	struct rbt_tree * child = this->right;
	if (!child) {return this;}
	this->right = child->left;
	child->left = this;
	return child;
}
