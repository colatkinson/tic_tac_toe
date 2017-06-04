#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include <uthash.h>
#include <hpdf.h>
#include <pdf.h>
#include <board.h>

void print_usage(char *prog) {
    printf("Usage: %s [-2p] [-h]\n", prog);

    printf("\t2: 2-player mode. If set, will allow playing as both X and O.\n");
    printf("\tp: Print mode. If set, will print line numbers.\n");
    printf("\th: Print this help message.\n");
}

int main(int argc, char **argv) {
    bool two_player = false;
    bool print = false;
    char c;

    while((c = getopt(argc, argv, "2ph")) != -1) {
        switch(c) {
            case '2':
                two_player = true;
                break;
            case 'p':
                print = true;
                break;
            default: // Also -h
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    char state[BOARD_SIZE];
    memset(state, ' ', BOARD_SIZE);

    printf("Generating boards... ");
    fflush(stdout);

    board_inf_t *mm = gen_child_boards(state, 'X', !two_player, NULL);

    printf("DONE\n");

    size_t count = 0;
    board_inf_t *s, *tmp;
    HASH_ITER(hh, mm, s, tmp) {
        ++count;
    }

    printf("Generating PDF... ");
    fflush(stdout);

    HPDF_Doc pdf = NULL;
    gen_pdf(mm, print, &pdf);

    printf("DONE\n");

    HPDF_SaveToFile(pdf, "tic_tac_toe.pdf");
    HPDF_Free(pdf);

    HASH_ITER(hh, mm, s, tmp) {
        HASH_DEL(mm, s);
        free_board(s);
    }

    return EXIT_SUCCESS;
}
