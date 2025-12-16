/**
 * Chess Game - Shared Constants
 * Definitions for colors and movement patterns.
 */

#include "types.h"

//==============================================================================
// COLOR PALETTE DEFINITIONS
//==============================================================================

const Color COLOR_LIGHT_SQUARE = {240, 217, 181, 255};
const Color COLOR_DARK_SQUARE = {181, 136, 99, 255};
const Color COLOR_SELECTED = {255, 255, 0, 100};
const Color COLOR_CHECK_HIGHLIGHT = {255, 0, 0, 150};
const Color COLOR_VALID_MOVE = {0, 255, 0, 80};
const Color COLOR_CAPTURE = {255, 0, 0, 80};
const Color COLOR_BACKGROUND = {40, 40, 40, 255};
const Color COLOR_OVERLAY_DARK = {0, 0, 0, 150};
const Color COLOR_OVERLAY_DARKER = {0, 0, 0, 180};
const Color COLOR_PANEL_BG = {60, 60, 60, 255};
const Color COLOR_BUTTON_BG = {100, 100, 100, 255};
const Color COLOR_BUTTON_HOVER = {130, 130, 130, 255};
const Color COLOR_TITLE_GOLD = {255, 215, 0, 255};
const Color COLOR_TITLE_SHADOW = {80, 60, 0, 255};

//==============================================================================
// MOVEMENT PATTERN DEFINITIONS
//==============================================================================

const int ROOK_DIRECTIONS[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
const int BISHOP_DIRECTIONS[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
const int KNIGHT_MOVES[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                                {1, -2},  {1, 2},  {2, -1},  {2, 1}};
