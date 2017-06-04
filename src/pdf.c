#include <stdio.h>
#include <pdf.h>
#include <board.h>

void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data) {
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
}

// Generate the cover page
HPDF_Page gen_cover(bool page_nums, HPDF_Font font, const char *title, HPDF_Doc *pdf) {
    HPDF_Page cover = HPDF_AddPage(*pdf);
    HPDF_Page_SetWidth(cover, ROW * 100);
    HPDF_Page_SetHeight(cover, page_nums ? (ROW + 1) * 100 : ROW * 100);

    // If there's space for page numbers, move the title up a bit
    uint32_t height_offset = page_nums ? 50 : 0;

    HPDF_Page_BeginText(cover);

    HPDF_Page_SetFontAndSize(cover, font, ROW * 16);
    HPDF_Page_TextRect(cover, 0, ROW * 75 + height_offset, ROW * 100, 75 + height_offset, title,
                       HPDF_TALIGN_CENTER, NULL);

    // If in print mode, don't reference clicking
    if(!page_nums) {
        HPDF_Page_SetFontAndSize(cover, font, ROW * 8);
        HPDF_Page_TextRect(cover, 0, 75, ROW * 100, 0, "Click to Start", HPDF_TALIGN_CENTER, NULL);
        HPDF_Page_EndText(cover);
    }

    return cover;
}

// Draw the game over overlay
int draw_game_over(HPDF_Doc *pdf, HPDF_Page page, HPDF_Font font, const char *msg,
                   bool page_nums) {
    // Use a GState to draw with transparency
    HPDF_Page_GSave(page);
    HPDF_ExtGState gstate = HPDF_CreateExtGState(*pdf);
    HPDF_ExtGState_SetAlphaFill(gstate, 0.90);
    HPDF_Page_SetExtGState(page, gstate);

    // Draw a translucent white overlay
    HPDF_Page_SetGrayFill(page, 1);
    HPDF_Page_Rectangle(page, 0, 0, ROW * 100, ROW * 100);
    HPDF_Page_Fill(page);

    // Exit GState
    HPDF_Page_GRestore(page);

    // Draw text
    HPDF_Page_SetGrayFill(page, 0.25);

    HPDF_Page_BeginText(page);

    HPDF_Page_SetFontAndSize(page, font, ROW * 16);
    HPDF_Page_TextRect(page, 0, ROW * 75, ROW * 100, 75, msg, HPDF_TALIGN_CENTER, NULL);

    // If in print mode, don't print stuff about clicking
    if(!page_nums) {
        HPDF_Page_SetFontAndSize(page, font, ROW * 8);
        HPDF_Page_TextRect(page, 0, 75, ROW * 100, 0,
                           "Click to Continue", HPDF_TALIGN_CENTER, NULL);
        HPDF_Page_EndText(page);
    }

    return EXIT_SUCCESS;
}

// Generate a game state page
int gen_page(board_inf_t *board, bool page_nums, HPDF_Font font, HPDF_Doc *pdf) {
    board->page = HPDF_AddPage(*pdf);
    HPDF_Page_SetWidth(board->page, ROW * 100);

    if(page_nums) {
        HPDF_Page_SetHeight(board->page, (ROW + 1) * 100);

        HPDF_Page_SetGrayFill(board->page, 0.25);

        HPDF_Page_BeginText(board->page);

        HPDF_Page_SetFontAndSize(board->page, font, 100);

        char page_num[17] = {0};
        snprintf(page_num, 17, "%u", board->page_num);

        HPDF_Page_TextRect(board->page, 0, (ROW + 1) * 100, ROW * 100, ROW * 100, page_num,
                           HPDF_TALIGN_CENTER, NULL);

        HPDF_Page_EndText(board->page);
    } else {
        HPDF_Page_SetHeight(board->page, ROW * 100);
    }

    // Draw background
    HPDF_Page_SetGrayFill(board->page, 1);
    HPDF_Page_Rectangle(board->page, 0, 0, ROW * 100, ROW * 100);
    HPDF_Page_Fill(board->page);

    // Draw graw boxes
    HPDF_Page_SetGrayFill(board->page, 0.95);
    for(size_t i = 0; i < BOARD_SIZE; i += 2) {
        HPDF_Page_Rectangle(board->page, (i % ROW) * 100, (i / ROW) * 100, 100, 100);
        HPDF_Page_Fill(board->page);
    }

    // Now to draw the marks
    HPDF_Page_SetGrayStroke(board->page, 0.25);
    HPDF_Page_SetLineWidth(board->page, 5);

    for(size_t i = 0; i < ROW; ++i) {
        for(size_t j = 0; j < ROW; ++j) {
            char cur = board->state[i * ROW + j];

            if(cur == 'X') {
                HPDF_Page_MoveTo(board->page, i * 100 + 25, j * 100 + 25);
                HPDF_Page_LineTo(board->page, i * 100 + 75, j * 100 + 75);

                HPDF_Page_MoveTo(board->page, i * 100 + 75, j * 100 + 25);
                HPDF_Page_LineTo(board->page, i * 100 + 25, j * 100 + 75);
            } else if(cur == 'O') {
                HPDF_Page_Circle(board->page, i * 100 + 50, j * 100 + 50, 25);
            } else {
                continue;
            }

            HPDF_Page_Stroke(board->page);
        }
    }

    return EXIT_SUCCESS;
}

int gen_pdf(board_inf_t *boards, bool page_nums, HPDF_Doc *pdf) {
    *pdf = HPDF_New(error_handler, NULL);
    if(!pdf) {
        perror("Cold not create PdfDoc object");
        return EXIT_FAILURE;
    }

    // Use tree structure for pages
    HPDF_SetPagesConfiguration(*pdf, 64);

    // Compress PDF
    HPDF_SetCompressionMode(*pdf, HPDF_COMP_ALL);

    // Display outline by default
    HPDF_SetPageMode(*pdf, HPDF_PAGE_MODE_USE_OUTLINE);

    // If electronic mode, open in full screen mode
    if(!page_nums) HPDF_SetPageMode(*pdf, HPDF_PAGE_MODE_FULL_SCREEN);

    HPDF_Font font = HPDF_GetFont(*pdf, "Times-Roman", NULL);

    // Generate cover
    HPDF_Page cover = gen_cover(page_nums, font, "The Complete Tic-Tac-Toe", pdf);
    HPDF_Destination cover_dst = HPDF_Page_CreateDestination(cover);
    HPDF_SetOpenAction(*pdf, cover_dst);

    // Create outline destination for the cover
    HPDF_Outline cover_outline = HPDF_CreateOutline(*pdf, NULL, "Cover", NULL);
    HPDF_Outline_SetDestination(cover_outline, cover_dst);

    // Create outline destination for the game itself
    HPDF_Outline game_outline = HPDF_CreateOutline(*pdf, NULL, "Game", NULL);

    // Link annotation variables
    HPDF_Rect rect;
    rect.left = 0;
    rect.right = ROW * 100;
    rect.top = ROW * 100;
    rect.bottom = 0;

    HPDF_Annotation annot;
    HPDF_Destination dst;

    uint32_t page_num = 0;

    board_inf_t *s, *tmp;
    HASH_ITER(hh, boards, s, tmp) {
        s->page_num = ++page_num;

        // Create new page
        gen_page(s, page_nums, font, pdf);

        char page_num[17] = {0};
        snprintf(page_num, 17, "%u", s->page_num);

        // Add page to the outline
        dst = HPDF_Page_CreateDestination(s->page);
        HPDF_Outline outline = HPDF_CreateOutline(*pdf, game_outline, page_num, NULL);
        HPDF_Outline_SetDestination(outline, dst);
    }

    HASH_ITER(hh, boards, s, tmp) {
        // Don't add any links for won games
        if(s->winner != 0) {
            rect.left = 0;
            rect.right = ROW * 100;
            rect.top = ROW * 100;
            rect.bottom = 0;

            if(s->winner == 'X') {
                draw_game_over(pdf, s->page, font, "X Won!", page_nums);
            } else if(s->winner == 'O') {
                draw_game_over(pdf, s->page, font, "O Won!", page_nums);
            } else {
                draw_game_over(pdf, s->page, font, "A draw. Wow.", page_nums);
            }

            annot = HPDF_Page_CreateLinkAnnot(s->page, rect, cover_dst);

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
                rect.bottom = j * 100;
                rect.top = rect.bottom + 100;

                board_inf_t *tmp = tmp_head->data;

                tmp_head = tmp_head->next;

                // If it's print mode, add page numbers to the page
                if(page_nums) {
                    HPDF_Page_SetGrayFill(s->page, 0.25);
                    HPDF_Page_BeginText(s->page);
                    HPDF_Page_SetFontAndSize(s->page, font, 36);

                    char page_num[17] = {0};
                    snprintf(page_num, 17, "%u", tmp->page_num);

                    HPDF_Page_TextRect(s->page, rect.left, rect.top - (100 - 42) / 2, rect.right,
                                       rect.bottom + (100 - 42) / 2, page_num, HPDF_TALIGN_CENTER,
                                       NULL);

                    HPDF_Page_EndText(s->page);

                    // Don't bother adding links
                    continue;
                }

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
    char empty_state[BOARD_SIZE];
    memset(empty_state, ' ', BOARD_SIZE);
    HASH_FIND(hh, boards, empty_state, BOARD_SIZE, empty_board);
    if(empty_board == NULL) return EXIT_FAILURE;

    rect.left = 0;
    rect.right = ROW * 100;
    rect.top = ROW * 100;
    rect.bottom = 0;

    dst = HPDF_Page_CreateDestination(empty_board->page);
    annot = HPDF_Page_CreateLinkAnnot(cover, rect, dst);
    HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_NO_HIGHTLIGHT);
    HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);

    // Add custom page numbering
    HPDF_AddPageLabel(*pdf, 1, HPDF_PAGE_NUM_STYLE_DECIMAL, 0, "Cover ");
    HPDF_AddPageLabel(*pdf, 2, HPDF_PAGE_NUM_STYLE_DECIMAL, 1, "Game State ");

    // Make game outline open to empty state
    HPDF_Outline_SetDestination(game_outline, dst);

    return EXIT_SUCCESS;
}
