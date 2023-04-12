/* rbtree/test_generation/generate_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* This program generates and prints to stdout a sequence of addition
 * commands to the test engine, with number of commands and
 * commands picked randomly. 
 *
 * This program also works with one cli arg, the int offset from the current
 * time for the random seed.
 * (required for generating different tests within one second) */

/* constants */
enum { min_commands = 10, max_commands = 150, min_char = 33, max_char = 126 };

/* function for generating random integers */
int random_in_range(int min, int max)
{
    return min + (int)(((double)max + 1) * rand() / (RAND_MAX + 1.0));
}

int main(int argc, char **argv) 
{
    int seed_param, num_commands;

    if (argc > 2) {
        fprintf(stderr, "Provide one arg, a random seed parameter, "
                        "or no args\n");
        return 1;
    }

    /* if provided offset in arg, set the seed equal to cur time + offset */
    seed_param = argc == 2 ? atoi(argv[1]) : 0;
    srand(time(NULL) + seed_param);

    /* generate random number of commands */
    num_commands = random_in_range(min_commands, max_commands);

    /* generate n commands of type "aX" or "dX", where X is the one-chr string,
     * identifier of an element, and print them to stdout */
    for (; num_commands > 0; num_commands--) {
        int command;

        command = random_in_range(0, 1) ? 'a' : 'd';
        printf("%c%c\n", command, random_in_range(min_char, max_char));
    }

    /* print the "check tree validity" command to complete the test input */
    printf("c_\n");

    return 0;
}
