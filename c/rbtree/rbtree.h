/* 4_22/rbtree.h */
#ifndef RBTREE_SENTRY
#define RBTREE_SENTRY

/* inteface for the red-black tree-based dictionary with string keys and
 * anything in data. 
 *
 * When adding elements, the key strings will be copied, but the data
 * will just be passed by reference.
 *
 * Manual control over the tree nodes is possible, but not advisable. */

typedef enum tag_node_color { red, black } node_color;

typedef struct tag_tree_node {
    char *key;
    void *data;
    node_color color;
    struct tag_tree_node *left, *right, *parent;
} tree_node;

const tree_node *rbtree_get_element(const tree_node *root, const char *key);
int rbtree_add_element(tree_node **root, const char *key, void *data);
void rbtree_print(const tree_node *root);
void rbtree_destroy(tree_node *root);

#endif
