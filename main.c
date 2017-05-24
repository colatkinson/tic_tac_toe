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
    board_inf_t *tmp;
    HASH_FIND(hh, boards, state, BOARD_SIZE, tmp);
    if(tmp != NULL) return;

    // Make a new copy and put in hash table
    board_inf_t *board = (board_inf_t *) malloc(sizeof(board_inf_t));
    memcpy(board->state, state, BOARD_SIZE);
    board->next_move = player;

    board->winner = 0;

    HASH_ADD(hh, boards, state, BOARD_SIZE, board);

    char other_player = (player == 'X') ? 'O' : 'X';

    // Check if game is over
    if(is_won(state)) {
        board->winner = other_player;
        return;
    } else if(is_draw(state)) {
        board->winner = ' ';
        return;
    }

    // Recurse over possible moves
    for(size_t i = 0; i < BOARD_SIZE; ++i) {
        if(state[i] != ' ') {
            board->moves[i] = 0;
            continue;
        }

        board->moves[i] = 1;

        char new_state[BOARD_SIZE];
        memcpy(new_state, state, BOARD_SIZE);
        new_state[i] = player;

        gen_child_boards(new_state, other_player);
    }
}

void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data) {
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
}

int main(int argc, char **argv) {
    char state[10] = "         ";

    // Build board tree
    gen_child_boards(state, 'X');

    HPDF_Doc pdf = HPDF_New(error_handler, NULL);
    if(!pdf) {
        perror("Cold not create PdfDoc object");
        return EXIT_FAILURE;
    }

    // Compress PDF
    HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);

    HPDF_Destination dst;
    HPDF_Rect rect;
    HPDF_Annotation annot;

    board_inf_t *s, *tmp;
    HASH_ITER(hh, boards, s, tmp) {
        // Create new page
        s->page = HPDF_AddPage(pdf);
        HPDF_Page_SetWidth(s->page, ROW*100);
        HPDF_Page_SetHeight(s->page, ROW*100);

        // Alternate colors
        bool odd = false;

        // Draw background
        for(size_t i = 0; i < ROW; ++i) {
            for(size_t j = 0; j < ROW; ++j) {
                if(odd) {
                    HPDF_Page_SetRGBFill(s->page, 1, 1, 1);
                } else {
                    HPDF_Page_SetRGBFill(s->page, 0.95, 0.95, 0.95);
                }
                odd = !odd;

                HPDF_Page_Rectangle(s->page, i * 100, j * 100, 100, 100);
                HPDF_Page_Fill(s->page);
            }
        }

        // Now to draw the marks
        HPDF_Page_SetRGBStroke(s->page, 0.25, 0.25, 0.25);
        HPDF_Page_SetLineWidth(s->page, 5);

        for(size_t i = 0; i < ROW; ++i) {
            for(size_t j = 0; j < ROW; ++j) {
                char cur = s->state[i * ROW + j];

                if(cur == 'X') {
                    HPDF_Page_MoveTo(s->page, i * 100 + 25, j * 100 + 25);
                    HPDF_Page_LineTo(s->page, i * 100 + 75, j * 100 + 75);

                    HPDF_Page_MoveTo(s->page, i * 100 + 75, j * 100 + 25);
                    HPDF_Page_LineTo(s->page, i * 100 + 25, j * 100 + 75);
                } else if(cur == 'O') {
                    HPDF_Page_Circle(s->page, i * 100 + 50, j * 100 + 50, 25);
                } else {
                    continue;
                }

                HPDF_Page_Stroke(s->page);
            }
        }
    }

    HASH_ITER(hh, boards, s, tmp) {
        // For now, don't add any links for won games
        if(s->winner != 0) continue;

        for(size_t i = 0; i < ROW; ++i) {
            for(size_t j = 0; j < ROW; ++j) {
                char cur = s->state[i * ROW + j];

                if(cur != ' ') continue;

                // Generate a rectangular area
                rect.left = i * 100;
                rect.right = rect.left + 100;
                rect.top = j * 100;
                rect.bottom = rect.top + 100;

                board_inf_t *tmp = NULL;

                // Find the state to which to link
                char id[BOARD_SIZE] = {0};
                memcpy(id, s->state, BOARD_SIZE);
                id[i * ROW + j] = s->next_move;
                HASH_FIND(hh, boards, id, BOARD_SIZE, tmp);

                // This shouldn't happen (TM)
                if(tmp == NULL) {
                    printf("ERROR: Could not find id: ");
                    print_key(id);

                    return EXIT_FAILURE;
                }

                // Add a link to the page
                dst = HPDF_Page_CreateDestination(tmp->page);
                annot = HPDF_Page_CreateLinkAnnot(s->page, rect, dst);
                HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_NO_HIGHTLIGHT);
            }
        }
    }

    HPDF_SaveToFile(pdf, "out.pdf");
    HPDF_Free(pdf);

    HASH_ITER(hh, boards, s, tmp) {
        HASH_DEL(boards, s);
        free(s);
    }

    return EXIT_SUCCESS;
}
