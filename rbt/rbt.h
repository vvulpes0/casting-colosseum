/* rbt.h
 * Copyright (c) 2021 Dakotah Lambert
 */

/* Red-Black Trees
 * A red-black tree is a binary search tree
 * where nodes store an extra "color" bit.
 * The following properties must be satisfied:
 * + All nodes are red or black
 * + The root is black
 * + A red node has no red children
 * + Every path from the root to NULL
 *   has the same number of black nodes
 * The following conclusions can be drawn:
 * + Shortest and longest paths
 *   from the root to NULL differ
 *   by no more than a factor of 2
 * + If a node has but a single child:
 *   - That node is black
 *   - Its child is red
 *   - It has no grandchildren
 */

#ifndef RBT_RBT_H
#define RBT_RBT_H

#ifdef __cplusplus
extern "C" {
#endif

struct rbt_tree {
	struct rbt_tree * left;
	struct rbt_tree * right;
	void* data;
	unsigned int type;
	unsigned char is_red;
};

struct rbt_tree * rbt_rotate_left(struct rbt_tree * const);
struct rbt_tree * rbt_rotate_right(struct rbt_tree * const);

struct rbt_tree * rbt_find(struct rbt_tree * const, unsigned int const);
struct rbt_tree * rbt_insert(struct rbt_tree * const, struct rbt_tree * const);
struct rbt_tree * rbt_delete(struct rbt_tree * const, struct rbt_tree * const);

#ifdef __cplusplus
}
#endif

#endif /* RBT_RBT_H */
