#pragma once
#include <uthash.h>
#include <hpdf.h>

#define ROW 3
#define BOARD_SIZE ROW * ROW

typedef struct move_list {
    struct move_list *next;

    void *data;
} move_list_t;

typedef struct board_inf {
    move_list_t *moves_head;
    move_list_t *moves_tail;

    char state[BOARD_SIZE];
    char next_move;
    char winner;

    HPDF_Page page;

    UT_hash_handle hh;
} board_inf_t;
