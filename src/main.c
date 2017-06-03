#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <uthash.h>
#include <hpdf.h>
#include <pdf.h>
#include <board.h>

void free_move_list(move_list_t **moves) {
    if(moves == NULL || *moves == NULL) return;

    move_list_t *head = *moves;
    move_list_t *tmp = NULL;

    while(head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }

    *moves = NULL;
}

void print_board(char *state) {
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
            if(state[i * ROW + j] == 'X') {
                consec_x += 1;
            } else if(state[i * ROW + j] == 'O') {
                consec_o += 1;
            }
        }

        if(consec_x == ROW || consec_o == ROW) return true;
    }

    // Check columns
    for(size_t i = 0; i < ROW; ++i) {
        uint8_t consec_x = 0, consec_o = 0;

        for(size_t j = 0; j < ROW; ++j) {
            if(state[i + j * ROW] == 'X') {
                consec_x += 1;
            } else if(state[i + j * ROW] == 'O') {
                consec_o += 1;
            }
        }

        if(consec_x == ROW || consec_o == ROW) return true;
    }

    // Check top-left to bottom-right diagonal
    // uint16_t tl_diag = state[0] + state[4] + state[8];
    uint16_t tl_diag = 0;
    for(size_t i = 0; i < BOARD_SIZE; i += ROW + 1) {
        tl_diag += state[i];
    }
    if(tl_diag == 'X' * ROW || tl_diag == 'O' * ROW) return true;

    // Check top-right to bottom-left diagonal
    // uint16_t tr_diag = state[2] + state[4] + state[6];
    uint16_t tr_diag = 0;
    for(size_t i = ROW - 1; i < BOARD_SIZE - ROW + 1; i += ROW - 1) {
        tr_diag += state[i];
    }
    if(tr_diag == 'X' * ROW || tr_diag == 'O' * ROW) return true;

    return false;
}

bool is_draw(char *state) {
    for(size_t i = 0; i < BOARD_SIZE; ++i) {
        if(state[i] == ' ') return false;
    }

    return true;
}

int32_t minimax(char *state, char player, int32_t *out) {
    char other_player = (player == 'X') ? 'O' : 'X';

    // If the game is over, you've reached the base case
    if(is_won(state)) {
        if(other_player == 'O')
            return +1;
        else
            return -1;
    } else if(is_draw(state)) {
        return 0;
    }

    int32_t best = 0;
    int32_t best_ind = -1;

    // If it's the AI's turn, maximize score
    // If it's the human's turn, minimize score
    if(player == 'O') {
        best = INT32_MIN;

        // Iterate over possible moves
        for(int32_t i = 0; i < BOARD_SIZE; ++i) {
            if(state[i] != ' ') continue;

            char new_state[BOARD_SIZE] = {0};
            memcpy(new_state, state, BOARD_SIZE);
            new_state[i] = player;

            // Recursion!
            int32_t val = minimax(new_state, other_player, NULL);

            if(val > best) {
                best = val;
                best_ind = i;
            }
        }
    } else {
        best = INT32_MAX;

        // Iterate over possible moves
        for(int32_t i = 0; i < BOARD_SIZE; ++i) {
            if(state[i] != ' ') continue;

            char new_state[BOARD_SIZE] = {0};
            memcpy(new_state, state, BOARD_SIZE);
            new_state[i] = player;

            // Recursion!
            int32_t val = minimax(new_state, other_player, NULL);

            if(val < best) {
                best = val;
                best_ind = i;
            }
        }
    }

    // If it's the top level, return the move to make
    if(out != NULL) *out = best_ind;

    return best;
}

void print_move_list(move_list_t *head) {
    move_list_t *cur = head;

    while(cur != NULL) {
        printf("\t");
        print_key(((board_inf_t *) cur->data)->state);

        cur = cur->next;
    }
}

board_inf_t *gen_child_boards(char *state, char player, bool ai, board_inf_t *boards) {
    // Check if already exists
    board_inf_t *tmp;
    HASH_FIND(hh, boards, state, BOARD_SIZE, tmp);
    if(tmp != NULL) return tmp;

    char other_player = (player == 'X') ? 'O' : 'X';

    // If it's the AI's turn, run minimax
    if(ai && player == 'O') {
        // Determine the best move to make
        int32_t move_ind = -1;
        minimax(state, 'O', &move_ind);

        // Make that move
        char new_state[BOARD_SIZE];
        memcpy(new_state, state, BOARD_SIZE);
        new_state[move_ind] = player;

        // Only add the state with the move already made to the board
        return gen_child_boards(new_state, other_player, ai, boards);
    }

    // Make a new copy and put in hash table
    board_inf_t *board = (board_inf_t *) malloc(sizeof(board_inf_t));
    memcpy(board->state, state, BOARD_SIZE);
    board->next_move = player;
    board->moves_head = NULL;
    board->moves_tail = NULL;

    board->winner = 0;

    HASH_ADD(hh, boards, state, BOARD_SIZE, board);

    // Check if game is over
    if(is_won(state)) {
        board->winner = other_player;
        return board;
    } else if(is_draw(state)) {
        board->winner = ' ';
        return board;
    }

    // Recurse over possible moves
    for(size_t i = 0; i < BOARD_SIZE; ++i) {
        if(state[i] != ' ') continue;

        char new_state[BOARD_SIZE];
        memcpy(new_state, state, BOARD_SIZE);
        new_state[i] = player;

        board_inf_t *child_board = gen_child_boards(new_state, other_player, ai, boards);
        if(child_board == NULL) continue;

        move_list_t *move = (move_list_t *) malloc(sizeof(move_list_t));
        move->next = NULL;
        move->data = child_board;

        if(board->moves_head == NULL) {
            board->moves_head = move;
        } else {
            board->moves_tail->next = move;
        }

        board->moves_tail = move;
    }

    return board;
}

int main(int argc, char **argv) {
    // board_inf_t *boards = NULL;

    // char state[10] = "         ";
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
