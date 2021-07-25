#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "rbt.h"

static void init_tree(struct rbt_tree * const, uint8_t const);
static void free_tree(struct rbt_tree *);
static void print_tree(struct rbt_tree const * const);
static int  print_tree_data(struct rbt_tree const * const,
                            struct rbt_tree const * const,
                            int const, int const);

int
main(int argc, char ** argv)
{
	struct rbt_tree * t = NULL;
	for (int i = 1; i < argc; ++i)
	{
		struct rbt_tree * x = malloc(sizeof(struct rbt_tree));
		init_tree(x, (int)strtol(argv[i], NULL, 10));
		t = rbt_insert(t, x);
	}
	print_tree(t);
	free_tree(t);
	return EXIT_SUCCESS;
}

static void
init_tree(struct rbt_tree * const this, uint8_t const tag)
{
	this->left = NULL;
	this->right = NULL;
	this->data = NULL;
	this->type = tag;
}

static void
free_tree(struct rbt_tree * this)
{
	if (!this) {return;}
	free_tree(this->left);
	free_tree(this->right);
	free(this);
}

static void
print_tree(struct rbt_tree const * const this)
{
	printf("digraph {\n");
	printf("node [style=\"filled\",fontcolor=\"white\"];\n");
	print_tree_data(this, NULL, 0, 0);
	printf("}\n");
}

static int
print_tree_data(struct rbt_tree const * const this,
                struct rbt_tree const * const p,
                int const n, int const pn)
{
	if (!this) {return n;}
	char const * const c = this->is_red ? "#DE7D2D" : "#000000";
	printf("%d [fillcolor=\"%s\",label=\"%d\"];\n",
	       n, c, this->type);
	if (p)
	{
		printf("%d -> %d;\n", pn, n);
	}
	int const next = print_tree_data(this->left, this, n + 1, n);
	return print_tree_data(this->right, this, next, n);
}
