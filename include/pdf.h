#pragma once

#include <hpdf.h>
#include <board.h>

int gen_pdf(board_inf_t *boards, HPDF_Doc *pdf);
