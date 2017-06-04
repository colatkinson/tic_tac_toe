#pragma once

#include <hpdf.h>
#include <board.h>

#include <stdbool.h>

int gen_pdf(board_inf_t *boards, bool page_nums, HPDF_Doc *pdf);
