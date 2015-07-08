#include <stdio.h>
#include <stdlib.h>
#include "../src/include/util.h"

int
main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return -1;
    }

    FILE *file;
    if ((file = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Error while reading %s\n", argv[1]);
        return -2;
    }

    /* Test case #1 */
    char *text = NULL;
    size_t size;

    size = get_str_from_path(&text, 0, argv[1]);
    printf("\x1b[31;1mTest #1\x1b[0m\nFile contents:\n%s\nSize: %ld\n", text, size);
    free(text);
    text = NULL;

    /* Test case #2 */
    text = (char *) malloc(10);
    size = get_str_from_stream(&text, 10, file);
    printf("\x1b[31;1mTest #2\x1b[0m\nFile contents:\n%s\nSize: %ld\n", text, size);
    free(text);

    fclose(file);
    return 0;
}
