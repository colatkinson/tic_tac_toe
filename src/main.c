#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <uthash.h>
#include <hpdf.h>
#include <pdf.h>
#include <board.h>

int main(int argc, char **argv) {
    char state[BOARD_SIZE];
    memset(state, ' ', BOARD_SIZE);

    printf("Generating boards... ");
    fflush(stdout);

    board_inf_t *mm = gen_child_boards(state, 'X', true, NULL);

    printf("DONE\n");

    size_t count = 0;
    board_inf_t *s, *tmp;
    HASH_ITER(hh, mm, s, tmp) {
        ++count;
    }

    printf("Generating PDF... ");
    fflush(stdout);

    HPDF_Doc pdf = NULL;
    gen_pdf(mm, &pdf);

    printf("DONE\n");

    HPDF_SaveToFile(pdf, "out.pdf");
    HPDF_Free(pdf);

    HASH_ITER(hh, mm, s, tmp) {
        HASH_DEL(mm, s);
        free_move_list(&(s->moves_head));
        free(s);
    }

    return EXIT_SUCCESS;
}
