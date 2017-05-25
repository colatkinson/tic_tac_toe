#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <uthash.h>
#include <hpdf.h>

#define ROW 3
#define BOARD_SIZE ROW * ROW

typedef struct move_list {
    struct move_list *next;

    struct board_inf *data;
} move_list_t;

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

typedef struct board_inf {
    move_list_t *moves_head;
    move_list_t *moves_tail;

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

board_inf_t *gen_child_boards(char *state, char player) {
    // Check if already exists
    board_inf_t *tmp;
    HASH_FIND(hh, boards, state, BOARD_SIZE, tmp);
    if(tmp != NULL) return tmp;

    // Make a new copy and put in hash table
    board_inf_t *board = (board_inf_t *) malloc(sizeof(board_inf_t));
    memcpy(board->state, state, BOARD_SIZE);
    board->next_move = player;
    board->moves_head = NULL;
    board->moves_tail = NULL;

    board->winner = 0;

    HASH_ADD(hh, boards, state, BOARD_SIZE, board);

    char other_player = (player == 'X') ? 'O' : 'X';

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

        board_inf_t *child_board = gen_child_boards(new_state, other_player);
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

void print_move_list(move_list_t *head) {
    move_list_t *cur = head;

    while(cur != NULL) {
        printf("\t");
        print_key(cur->data->state);

        cur = cur->next;
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
    HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);

    // Open in full screen mode
    HPDF_SetPageMode(pdf, HPDF_PAGE_MODE_FULL_SCREEN);

    HPDF_Font font = HPDF_GetFont(pdf, "Times-Roman", NULL);

    // Generate cover
    HPDF_Page cover = HPDF_AddPage(pdf);
    HPDF_Page_SetWidth(cover, ROW*100);
    HPDF_Page_SetHeight(cover, ROW*100);

    HPDF_Page_BeginText(cover);

    HPDF_Page_SetFontAndSize(cover, font, ROW*16);
    HPDF_Page_TextRect(cover, 0, ROW*75, ROW*100, 75, "The Complete Tic-Tac-Toe", HPDF_TALIGN_CENTER, NULL);

    HPDF_Page_SetFontAndSize(cover, font, ROW*8);
    HPDF_Page_TextRect(cover, 0, 75, ROW*100, 0, "Click to Start", HPDF_TALIGN_CENTER, NULL);
    HPDF_Page_EndText(cover);

    HPDF_Destination cover_dst = HPDF_Page_CreateDestination(cover);
    HPDF_SetOpenAction(pdf, cover_dst);

    // Link annotation variables
    HPDF_Rect rect;
    rect.left = 0;
    rect.right = ROW * 100;
    rect.top = ROW * 100;
    rect.bottom = 0;

    HPDF_Annotation annot;

    // Generate X won page
    HPDF_Page x_win = HPDF_AddPage(pdf);
    HPDF_Page_SetWidth(x_win, ROW*100);
    HPDF_Page_SetHeight(x_win, ROW*100);

    HPDF_Page_BeginText(x_win);

    HPDF_Page_SetFontAndSize(x_win, font, ROW*16);
    HPDF_Page_TextRect(x_win, 0, ROW*75, ROW*100, 75, "X Won!", HPDF_TALIGN_CENTER, NULL);

    HPDF_Page_SetFontAndSize(x_win, font, ROW*8);
    HPDF_Page_TextRect(x_win, 0, 75, ROW*100, 0, "Click to Restart", HPDF_TALIGN_CENTER, NULL);
    HPDF_Page_EndText(x_win);

    HPDF_Destination x_win_dst = HPDF_Page_CreateDestination(x_win);

    annot = HPDF_Page_CreateLinkAnnot(x_win, rect, cover_dst);
    HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_NO_HIGHTLIGHT);
    HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);

    // Generate O won page
    HPDF_Page o_win = HPDF_AddPage(pdf);
    HPDF_Page_SetWidth(o_win, ROW*100);
    HPDF_Page_SetHeight(o_win, ROW*100);

    HPDF_Page_BeginText(o_win);

    HPDF_Page_SetFontAndSize(o_win, font, ROW*16);
    HPDF_Page_TextRect(o_win, 0, ROW*75, ROW*100, 75, "O Won!", HPDF_TALIGN_CENTER, NULL);

    HPDF_Page_SetFontAndSize(o_win, font, ROW*8);
    HPDF_Page_TextRect(o_win, 0, 75, ROW*100, 0, "Click to Restart", HPDF_TALIGN_CENTER, NULL);
    HPDF_Page_EndText(o_win);

    HPDF_Destination o_win_dst = HPDF_Page_CreateDestination(o_win);

    annot = HPDF_Page_CreateLinkAnnot(o_win, rect, cover_dst);
    HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_NO_HIGHTLIGHT);
    HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);

    // Generate draw page
    HPDF_Page draw = HPDF_AddPage(pdf);
    HPDF_Page_SetWidth(draw, ROW*100);
    HPDF_Page_SetHeight(draw, ROW*100);

    HPDF_Page_BeginText(draw);

    HPDF_Page_SetFontAndSize(draw, font, ROW*16);
    HPDF_Page_TextRect(draw, 0, ROW*75, ROW*100, 75, "It's a draw. Wow.", HPDF_TALIGN_CENTER, NULL);

    HPDF_Page_SetFontAndSize(draw, font, ROW*8);
    HPDF_Page_TextRect(draw, 0, 75, ROW*100, 0, "Click to Restart", HPDF_TALIGN_CENTER, NULL);
    HPDF_Page_EndText(draw);

    HPDF_Destination draw_dst = HPDF_Page_CreateDestination(draw);

    annot = HPDF_Page_CreateLinkAnnot(draw, rect, cover_dst);
    HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_NO_HIGHTLIGHT);
    HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);

    HPDF_Destination dst;

    board_inf_t *s, *tmp;
    HASH_ITER(hh, boards, s, tmp) {
        // Create new page
        s->page = HPDF_AddPage(pdf);
        HPDF_Page_SetWidth(s->page, ROW*100);
        HPDF_Page_SetHeight(s->page, ROW*100);


        // Draw background
        HPDF_Page_SetGrayFill(s->page, 1);
        HPDF_Page_Rectangle(s->page, 0, 0, ROW*100, ROW*100);
        HPDF_Page_Fill(s->page);

        // Draw graw boxes
        HPDF_Page_SetGrayFill(s->page, 0.95);
        for(size_t i = 0; i < BOARD_SIZE; i += 2) {
            HPDF_Page_Rectangle(s->page, (i % ROW) * 100, (i / ROW) * 100, 100, 100);
            HPDF_Page_Fill(s->page);
        }

        // Now to draw the marks
        HPDF_Page_SetGrayStroke(s->page, 0.25);
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
        if(s->winner != 0) {
            rect.left = 0;
            rect.right = ROW * 100;
            rect.top = ROW * 100;
            rect.bottom = 0;

            if(s->winner == 'X') {
                annot = HPDF_Page_CreateLinkAnnot(s->page, rect, x_win_dst);
            } else if(s->winner == 'O') {
                annot = HPDF_Page_CreateLinkAnnot(s->page, rect, o_win_dst);
            } else {
                annot = HPDF_Page_CreateLinkAnnot(s->page, rect, draw_dst);
            }

            HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_NO_HIGHTLIGHT);
            HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);

            continue;
        }

        move_list_t *tmp_head = s->moves_head;

        for(size_t i = 0; i < ROW; ++i) {
            for(size_t j = 0; j < ROW; ++j) {
                char cur = s->state[i * ROW + j];

                if(cur != ' ') continue;

                // This Shouldn't Happen II: Electric Boogaloo
                if(tmp_head == NULL) {
                    printf("ERROR: Too few list elements\n");
                    return EXIT_FAILURE;
                }

                // Generate a rectangular area
                rect.left = i * 100;
                rect.right = rect.left + 100;
                rect.top = j * 100;
                rect.bottom = rect.top + 100;

                board_inf_t *tmp = tmp_head->data;

                tmp_head = tmp_head->next;

                // Add a link to the page
                dst = HPDF_Page_CreateDestination(tmp->page);
                annot = HPDF_Page_CreateLinkAnnot(s->page, rect, dst);
                HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_NO_HIGHTLIGHT);
                HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
            }
        }
    }

    // Link from the cover to the first page
    board_inf_t *empty_board = NULL;
    HASH_FIND(hh, boards, state, BOARD_SIZE, empty_board);
    if(empty_board == NULL) return EXIT_FAILURE;

    rect.left = 0;
    rect.right = ROW * 100;
    rect.top = ROW * 100;
    rect.bottom = 0;

    dst = HPDF_Page_CreateDestination(empty_board->page);
    annot = HPDF_Page_CreateLinkAnnot(cover, rect, dst);
    HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_NO_HIGHTLIGHT);
    HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);

    HPDF_SaveToFile(pdf, "out.pdf");
    HPDF_Free(pdf);

    HASH_ITER(hh, boards, s, tmp) {
        HASH_DEL(boards, s);
        free_move_list(&(s->moves_head));
        free(s);
    }

    return EXIT_SUCCESS;
}
