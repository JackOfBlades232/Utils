/* 4_22/prog.c */
#include "../rbtree.h"
#include <stdio.h>

/* uncomment this if you want to be able to run the test engine in full 
 * interactive mode with feedback */

/* #define INTERACTIVE */

/* This program provides a cli interface for printing, checking, adding and
 * removing elements to\from a red-black tree, with all the functionality for
 * checking it's validity. This program can be used by the user to interact
 * with the tree, or it can be fed pre-generated input for testing. 
 *
 * For simplicity, no data is stored in tree nodes, all keys must be 
 * of length 1, and all input strings must contain 2 characters: 
 * the command symbol and the 1-char key. 
 * 
 * When the program is asked to check the tree, it will
 * terminate with code 0 if the tree was valid and 1, else. 
 *
 * Commands:
 * -- 'aX' -- add element with key "X"
 * -- 'dX' -- delete element with key "X"
 * -- 'p_' -- print tree (needs second char for simplicity, can be any)
 * -- 'c_' -- check tree and terminate (second char as on print */

/* function for parsing a line of input 
 * (return: 1 if input was correct, 0 else) */
int read_line(char *command, char *key) {
    *command = getchar();
    *key = getchar();

    return getchar() == 10;
}

/* function for checking the first rbtree property: the root must be black */
int check_black_root_property(const tree_node *root) 
{
    return !root || root->color == black;
}

/* function for checking the second rbtree property: 
 * all red nodes must have black children */
int check_red_node_children_property(const tree_node *root)
{
    if (!root)
        return 1;

    if (root->parent && root->color == red && root->parent->color == red)
        return 0;

    return check_red_node_children_property(root->left) && 
        check_red_node_children_property(root->right);
}

/* recursive function for checking leaf depths */
int check_leaf_depth_property_iteratively(const tree_node *root,
        int cur_depth, int *req_depth)
{
    /* if reached leaf, and required depth already set, then return false if
     * current leaf depth does not match it, else true.
     * If req depth was not set, then set it with current leaf depth, and
     * return true. */
    if (!root) {
        if (*req_depth >= 0 && cur_depth != *req_depth)
            return 0;
        else if (*req_depth < 0)
            *req_depth = cur_depth;

        return 1;
    }

    /* only increase the depth if the node was black */
    if (root->color == black)
        cur_depth++;

    /* recursively traverse the whole tree while checking leaves */
    return check_leaf_depth_property_iteratively(
            root->left, cur_depth, req_depth) &&
        check_leaf_depth_property_iteratively(
                root->right, cur_depth, req_depth);
}

/* function for checking the third rbtree property: 
 * all paths from the root to the leaves must contain the same number of
 * black nodes */
int check_leaf_depth_property(const tree_node *root)
{
    int req_depth = -1;

    return check_leaf_depth_property_iteratively(root, 0, &req_depth);
}

/* function checks the given tree for all three properties */
int check_tree(const tree_node *root) 
{
    return check_black_root_property(root) &&
        check_red_node_children_property(root) &&
        check_leaf_depth_property(root);
}

int main() {
    tree_node *root = NULL;
    char command, key[2];
    int prog_res = 1;
#ifdef INTERACTIVE
    const tree_node *n;
    int op_res; /* addition operation result, bool */
#endif

    /* main cycle, messages, printing the tree and retrieving elements only
     * available in interactive mode */
    while (read_line(&command, key)) {
        switch (command) {
            case 'a':
#ifdef INTERACTIVE
                op_res = rbtree_add_element(&root, key, NULL);
                printf(op_res ? "Added\n" : "Was in tree\n");
#else
                rbtree_add_element(&root, key, NULL);
#endif
                break;
            case 'd':
#ifdef INTERACTIVE
                op_res = rbtree_remove_element(&root, key);
                printf(op_res ? "Removed\n" : "Was not in tree\n");
#else
                rbtree_remove_element(&root, key);
#endif
                break;
#ifdef INTERACTIVE
            case 'p':
                rbtree_print(root);
                break;
            case 'g':
                n = rbtree_get_element(root, key);
                printf(n ? "Found!\n" : "Not found\n");
                break;
#endif
            case 'c':
                /* check tree returns standard integer bool (0 false, 1 true),
                 * must be converted to program result code (0 ok, 1 error) */
                prog_res = !check_tree(root);
                printf(prog_res ? "Not valid\n" : "Valid\n");
            default:
                goto cleanup;
        }
    }

cleanup: /* memory management */
    rbtree_destroy(root);

    return prog_res;
}
