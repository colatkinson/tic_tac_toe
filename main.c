#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <uthash.h>
#include <hpdf.h>

#define ROW 3
#define BOARD_SIZE ROW * ROW
// #define KEY_SIZE BOARD_SIZE + 1

typedef struct board_inf {
    char moves[BOARD_SIZE];
    char state[BOARD_SIZE];
    char next_move;
    char winner;

    HPDF_Page page;

    UT_hash_handle hh;
} board_inf_t;

// Hash table
board_inf_t *boards = NULL;

void print_board(char *state) {
    // printf(" ");
    for(size_t i = 0; i < ROW + 4; ++i) {
        printf("_");
    }

    printf("\n|");
    for(size_t i = 0; i < ROW; ++i) {
        for(size_t j = 0; j < ROW; ++j) {
            printf("%c", state[i * ROW + j]);

            if(j < ROW - 1) {
                printf(" ");
            }
        }
        printf("|\n");

        if(i < ROW - 1) {
            printf("|");
        }
    }

    for(size_t i = 0; i < ROW + 4; ++i) {
        printf("^");
    }
    printf("\n");
}

void print_key(char *state) {
    printf("[");
    for(size_t i = 0; i < BOARD_SIZE; ++i) {
        printf("%c", state[i]);
    }
    printf("]\n");
}

bool is_won(char *state) {
    // Check rows
    for(size_t i = 0; i < ROW; ++i) {
        uint8_t consec_x = 0, consec_o = 0;

        for(size_t j = 0; j < ROW; ++j) {
            if(state[i * 3 + j] == 'X') {
                consec_x += 1;
            } else if(state[i * 3 + j] == 'O') {
                consec_o += 1;
            }
        }

        if(consec_x == ROW || consec_o == ROW) return true;
    }

    // Check columns
    for(size_t i = 0; i < ROW; ++i) {
        uint8_t consec_x = 0, consec_o = 0;

        for(size_t j = 0; j < ROW; ++j) {
            if(state[i + j * 3] == 'X') {
                consec_x += 1;
            } else if(state[i + j * 3] == 'O') {
                consec_o += 1;
            }
        }

        if(consec_x == ROW || consec_o == ROW) return true;
    }

    // Check top-left to bottom-right diagonal
    uint16_t tl_diag = state[0] + state[4] + state[8];
    if(tl_diag == 'X' * ROW || tl_diag == 'O' * ROW) return true;

    // Check top-right to bottom-left diagonal
    uint16_t tr_diag = state[2] + state[4] + state[6];
    if(tr_diag == 'X' * ROW || tr_diag == 'O' * ROW) return true;

    return false;
}

bool is_draw(char *state) {
    for(size_t i = 0; i < BOARD_SIZE; ++i) {
        if(state[i] == ' ') return false;
    }

    return true;
}

void gen_child_boards(char *state, char player) {
    // Check if already exists
    board_inf_t *tmp = (board_inf_t *) malloc(sizeof(board_inf_t));
    HASH_FIND(hh, boards, state, BOARD_SIZE, tmp);
    // bool already_exists = (tmp != NULL);
    //
    // if(already_exists) return;
    if(tmp != NULL) return;

    // print_key(state);

    // Make a new copy
    board_inf_t *board = (board_inf_t *) malloc(sizeof(board_inf_t));
    memcpy(board->state, state, BOARD_SIZE);
    // board->state[BOARD_SIZE] = player;
    board->next_move = player;

    for(size_t i = 0; i < BOARD_SIZE; ++i) {
        board->moves[i] = 0;
    }

    HASH_ADD(hh, boards, state, BOARD_SIZE, board);

    char other_player = (player == 'X') ? 'O' : 'X';

    if(is_won(state)) {
        board->winner = other_player;
        return;
    } else if(is_draw(state)) {
        board->winner = ' ';
        return;
    }

    for(size_t i = 0; i < BOARD_SIZE; ++i) {
        if(state[i] != ' ') continue;

        board->moves[i] = 1;
        // memcpy(board->moves[i], state, BOARD_SIZE);
        // board->moves[i][i] = player;
        char new_state[BOARD_SIZE];
        memcpy(new_state, state, BOARD_SIZE);
        new_state[i] = player;

        // print_key(new_state);

        gen_child_boards(new_state, other_player);
    }
}

void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data) {
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
}

int main(int argc, char **argv) {
    char state[10] = "         ";

    gen_child_boards(state, 'X');

    HPDF_Doc pdf = HPDF_New(error_handler, NULL);
    if(!pdf) {
        perror("Cold not create PdfDoc object");
        return EXIT_FAILURE;
    }
    // HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);

    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);

    // size_t page_count = 1024;
    // HPDF_Page *pages = (HPDF_Page *) malloc(page_count * sizeof(HPDF_Page));
    //
    // for(size_t i = 0; i < page_count; ++i) {
    //     pages[i] = HPDF_AddPage(pdf);
    //
    //     HPDF_Page_SetWidth(pages[i], 100);
    //     HPDF_Page_SetHeight(pages[i], 100);
    //
    //     HPDF_Page_BeginText(pages[i]);
    //     HPDF_Page_SetFontAndSize(pages[i], font, 10);
    //     HPDF_Page_ShowText(pages[i], "ABCDEFGHI");
    //     HPDF_Page_EndText(pages[i]);
    // }

    // HPDF_Destination dst;

    // unsigned int count = 0;
    //
    board_inf_t *s, *tmp;
    // HASH_ITER(hh, boards, s, tmp) {
    //     ++count;
    // }
    //
    HASH_ITER(hh, boards, s, tmp) {
        // print_board(s->state);
        // print_key(s->state);
        // ++count;
        // print_board(s->state);
        s->page = HPDF_AddPage(pdf);
        HPDF_Page_SetWidth(s->page, 100);
        HPDF_Page_SetHeight(s->page, 100);

        HPDF_Page_BeginText(s->page);
        HPDF_Page_SetFontAndSize(s->page, font, 10);
        char state_str[BOARD_SIZE + 1];
        memcpy(state_str, s->state, BOARD_SIZE);
        state_str[BOARD_SIZE] = '\0';
        HPDF_Page_ShowText(s->page, state_str);
        HPDF_Page_EndText(s->page);
    }

    // printf("New\n");
    // board_inf_t *s;
    // for(s = boards; s != NULL; s=s->hh.next) {
    //     print_key(s->state);
    // }

    HPDF_SaveToFile(pdf, "out.pdf");
    HPDF_Free(pdf);

    // printf("Generated %u boards\n", count);

    return EXIT_SUCCESS;
}
