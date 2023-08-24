#include "unit.h"
#include "../small/rb.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "assert.h"
#include <string.h>


#define NUMBER_NODES 15
#define RB_COMPACT 1

typedef struct node_s node_t;

struct node_s {
	rb_node(node_t) node;
	int key;
	int data;
};

typedef rb_tree(node_t) tree_t;

static inline int
key_cmp(const int a, const int b)
{
	return (a > b) ? 1 : (a == b) ? 0 : -1;
}
static inline int
key_node_cmp(const int a, const node_t *b)
{
	return key_cmp(a, b->key);
}
static inline int
node_cmp(const node_t *a, const node_t *b)
{
	return key_cmp(a->key, b->key);
}

rb_gen_ext_key(MAYBE_UNUSED static inline, test_, tree_t, node_t, node,
	       node_cmp, int, key_node_cmp);

node_t *
check_simple(tree_t *tree)
{
	header();
	test_new(tree);
	fail_unless(test_empty(tree));
	node_t *nodes = (node_t *)calloc(NUMBER_NODES, sizeof(*nodes));
	if (!nodes) {
		printf("can't allocate nodes\n");
		exit(1);
	}
	for (int i = 0; i < NUMBER_NODES; i++) {
		nodes[i].key = i;
		nodes[i].data = 2 * i;
		test_insert(tree, nodes + i);
	}
	fail_if(test_empty(tree));
	for (int i = 0; i < NUMBER_NODES; i++) {
		node_t *node = test_search(tree, i);
		fail_if(node == NULL);
		fail_unless(node->data == 2 * i && node->key == i);
		if (i + 1 < NUMBER_NODES) {
			fail_unless(test_next(tree, node)->key == i + 1 &&
				    test_next(tree, node)->data == 2 *(i + 1));
		} else {
			fail_unless(test_next(tree, node) == NULL);
		}
		if (i > 0) {
			fail_unless(test_prev(tree, node)->key == i - 1 &&
				    test_prev(tree, node)->data == 2 *(i - 1));
		} else {
			fail_unless(test_prev(tree, node) == NULL);
		}
	}
	fail_if(test_search(tree, NUMBER_NODES) != NULL);
	fail_unless(test_first(tree)->key == 0);
	fail_unless(test_last(tree)->key == NUMBER_NODES - 1);
	footer();
	return nodes;
}

static node_t *
print_cb(tree_t *t, node_t *node, void* arg)
{
	(void)t;
	(void)arg;
	printf(" %i->%i", node->key, node->data);
	return NULL;
}

void
check_old_iter(tree_t *tree, node_t* nodes)
{
	header();
	node_t *node = test_psearch(tree, 6);
	fail_unless(node->key == 6);
	node = test_psearch(tree, -1);
	fail_unless(node == NULL);

	node = test_nsearch(tree, 6);
	fail_unless(node->key == 6);
	node = test_nsearch(tree, NUMBER_NODES);
	fail_unless(node == NULL);

	test_iter(tree, NULL, print_cb, NULL);
	printf("\n");
	test_reverse_iter(tree, NULL, print_cb, NULL);
	printf("\n");
	test_iter(tree, nodes + 3, print_cb, NULL);
	printf("\n");
	test_reverse_iter(tree, nodes + 3, print_cb, NULL);
	printf("\n");
	footer();
}

void check_new_iter(tree_t *tree, node_t* nodes)
{
	header();
	struct test_iterator it;
	test_ifirst(tree, &it);
	(void) nodes;
	int count = 0;
	node_t *node = test_inext(&it);
	while (node) {
		fail_unless(node->key == count++);
		node = test_inext(&it);
	}
	test_icreate(tree, nodes + 3, &it);
	node = test_inext(&it);
	fail_unless(node);
	count = 3;
	while (node) {
		fail_unless(node->key == count++);
		node = test_inext(&it);
	}
	test_isearch(tree, 6, &it);
	node = test_inext(&it);
	fail_unless(node);
	count = 6;
	while (node) {
		fail_unless(node->key == count++);
		node = test_inext(&it);
	}
	test_isearch(tree, NUMBER_NODES - 1, &it);
	node = test_iprev(&it);
	fail_unless(node);
	count = NUMBER_NODES - 1;
	while (node) {
		fail_unless(node->key == count--);
		node = test_iprev(&it);
	}
	test_isearch_lt(tree, 6, &it);
	node = test_inext(&it);
	fail_unless(node->key == 5);
	test_isearch_gt(tree, 6, &it);
	node = test_inext(&it);
	fail_unless(node->key == 7);
	test_isearch_ge(tree, 6, &it);
	node = test_inext(&it);
	fail_unless(node->key == 6);
	test_isearch_le(tree, 6, &it);
	node = test_inext(&it);
	fail_unless(node->key == 6);

	test_isearch_le(tree, -1, &it);
	node = test_inext(&it);
	fail_unless(node == NULL);
	test_isearch_ge(tree, NUMBER_NODES, &it);
	node = test_inext(&it);
	fail_unless(node == NULL);

	test_isearch_lt(tree, 0, &it);
	node = test_inext(&it);
	fail_unless(node == NULL);

	test_isearch_gt(tree, NUMBER_NODES - 1, &it);
	node = test_inext(&it);
	fail_unless(node == NULL);

	footer();
}

int main()
{
	tree_t tree;
	node_t *nodes = check_simple(&tree);
	check_old_iter(&tree, nodes);
	check_new_iter(&tree, nodes);
	free(nodes);
}
