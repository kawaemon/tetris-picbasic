#include <stdint.h>
#include <stdbool.h>
#include <stdio.h> // for debug
#include <unistd.h>
#include "../bindings.h"

typedef uint8_t  byte;
typedef uint16_t word;

#define LCDBUF_XY(x, y) v->lcd_buffer[((y)*20)+(x)]

#undef false
#undef true
#define false 0
#define true 1

#define DROPDOWN_INTERVAL 60

#define WIDTH 4
#define HEIGHT 20

#define NEG_OFFSET 70

#define LEFT_BUTTON_MASK   (1 << 0)
#define RIGHT_BUTTON_MASK  (1 << 1)
#define DOWN_BUTTON_MASK   (1 << 2)
#define ROTATE_BUTTON_MASK (1 << 3)

#define DEFAULT_ROT_CENTER_X 3
#define DEFAULT_ROT_CENTER_Y 2

#define FALLING_BLOCK_CHAR 'D'
#define STABLE_BLOCK_CHAR  'O'
#define AIR_BLOCK_CHAR     '.'

#define MINO_TYPES_LEN 7

// S is shared blocks with all minos except for I
// xxxx
#define MINO_TYPE_I 0
// sx
// ss
#define MINO_TYPE_O 1
//  s
// xss
#define MINO_TYPE_T 2
// s
// ssx
#define MINO_TYPE_J 3
// x
// s
// ss
#define MINO_TYPE_L 4
//  s
//  ss
//   x
#define MINO_TYPE_S 5
// xs
//  ss
#define MINO_TYPE_Z 6

/*
*
* 0,0 .................... 20,0
*     ....................
*     ....................
* 0,3 .................... 20,3
*
* 0,3   0,0
*  Y3210
* X
* 0 ....
* 1 .x..
* 2 xsx.
* 3 xssx
* 4 ..x.
* 5 ....
* 6 ....
*   ....
*   ....
*   ....
*   ....
*   ....
*   ....
*   ....
*   ....
*   ....
*   ....
*   ....
*   ....
*   ....
*   ....
* 20,3  20,0
*/

/*
* x... --> ....
* x...     ....
* x...     ....
* x...     xxxx
*
*
*/

void next_rand(TickVariables *v) {
    v->rand_t = v->rand_x ^ v->rand_x << 1;
    v->rand_x = v->rand_y;
    v->rand_y = v->rand_z;
    v->rand_z ^= v->rand_z >> 3 ^ v->rand_t ^ v->rand_t >> 5;
}

// l(in): 0 if move to right, 2 if move to left
// k(out): false if failed to move
void move_horizontally(TickVariables *v) {
    v->k = true;
    for(v->i = 0; v->i < 20; v->i++) {
        for (v->j = 0; v->j < 4; v->j++) {
            if (LCDBUF_XY(v->i, v->j) == FALLING_BLOCK_CHAR) {
                if (v->l == 2 && (v->j == 3 || LCDBUF_XY(v->i, v->j+v->l-1) == STABLE_BLOCK_CHAR)) {
                    v->k = false;
                    return;
                }

                if (v->l == 0 && (v->j == 0 || LCDBUF_XY(v->i, v->j+v->l-1) == STABLE_BLOCK_CHAR)) {
                    v->k = false;
                    return;
                }
            }
        }
    }

    if (v->l == 0) {
        // right
        v->rot_center_y -= 1;
    } else {
        v->rot_center_y += 1;
    }

    for(v->i = 20; v->i-- > 0;) {
        if (v->l == 2) {
            v->j = 3;
        } else {
            v->j = 0;
        }

        while (true) {
            if (v->l == 2){
                if (v->j == 0) break;
                v->j -= 1;
            } else {
                if (v->j == 3) break;
                v->j += 1;
            }

            if (LCDBUF_XY(v->i, v->j) == FALLING_BLOCK_CHAR) {
                LCDBUF_XY(v->i, v->j) = AIR_BLOCK_CHAR;
                LCDBUF_XY(v->i, v->j+v->l-1) = FALLING_BLOCK_CHAR;
            }
        }
    }
}

// k(out): false if failed to move
void move_down(TickVariables *v) {
    v->k = true;
    for(v->i = 0; v->i < 20; v->i++) {
        for (v->j = 0; v->j < 4; v->j++) {
            if (LCDBUF_XY(v->i, v->j) == FALLING_BLOCK_CHAR &&
               (v->i+1 == 20 || LCDBUF_XY(v->i+1, v->j) == STABLE_BLOCK_CHAR)) {
                v->k = false;
                return;
            }
        }
    }

    v->rot_center_x += 1;

    // this reversing is essential
    for(v->i = 20; v->i-- > 0;) {
        for (v->j = 0; v->j < 4; v->j++) {
            if (LCDBUF_XY(v->i, v->j) == FALLING_BLOCK_CHAR) {
                LCDBUF_XY(v->i, v->j) = AIR_BLOCK_CHAR;
                LCDBUF_XY(v->i+1, v->j) = FALLING_BLOCK_CHAR;
            }
        }
    }
}

// i: X
// j: Y
// k(out): rot_X
// l(out): rot_Y
void rotate_pos(TickVariables *v) {
    byte r, rotated_r, rotated_m;

    r = NEG_OFFSET - v->j - 1 + v->rot_center_y;
    if (WIDTH - v->j - 1 >= WIDTH - v->rot_center_y) {
        r += 1;
    }
    rotated_m = r + HEIGHT - v->rot_center_x;
    if (NEG_OFFSET <= r) {
        rotated_m -= 1;
    }
    v->k = NEG_OFFSET + HEIGHT - rotated_m - 1;


    r = NEG_OFFSET - v->i + v->rot_center_x - 1;
    if (HEIGHT - v->i - 1 >= HEIGHT - v->rot_center_x) {
        r += 1;
    }

    if (r < NEG_OFFSET) {
        // r_y is negative
        rotated_r = r + 2 * (NEG_OFFSET - r);
    } else {
        // r_y is positive
        rotated_r = r - 2 * (r - NEG_OFFSET);
    }

    rotated_m = rotated_r + WIDTH - v->rot_center_y;
    if (NEG_OFFSET <= rotated_r) {
        rotated_m -= 1;
    }

    v->l = NEG_OFFSET + WIDTH - rotated_m - 1;
}

void rotate(TickVariables *v) {
    for(v->i = 0; v->i < 20; v->i++) { // x
        for (v->j = 0; v->j < 4; v->j++) { // y
            if (LCDBUF_XY(v->i, v->j) == FALLING_BLOCK_CHAR) {
                rotate_pos(v);

                if (v->k >= 20 || v->l >= 4) {
                    return;
                }

                if (LCDBUF_XY(v->k, v->l) == STABLE_BLOCK_CHAR) {
                    return;
                }
            }
        }
    }

    for(v->i = 0; v->i < 20; v->i++) {
        for (v->j = 0; v->j < 4; v->j++) {
            if (LCDBUF_XY(v->i, v->j) == FALLING_BLOCK_CHAR) {
                rotate_pos(v);
                LCDBUF_XY(v->i, v->j) = AIR_BLOCK_CHAR;
                LCDBUF_XY(v->k, v->l) =  FALLING_BLOCK_CHAR;
            }
        }
    }
}

void freeze_blocks(TickVariables *v) {
    for(v->i = 0; v->i < 80; v->i++) {
        if (v->lcd_buffer[v->i] == FALLING_BLOCK_CHAR) {
            v->lcd_buffer[v->i] = STABLE_BLOCK_CHAR;
        }
    }
}


void place_mino(TickVariables *v, byte mino_type) {
    switch (mino_type) {
        case MINO_TYPE_I:
            LCDBUF_XY(0, 2) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(1, 2) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(2, 2) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(3, 2) = FALLING_BLOCK_CHAR;
            v->rot_center_x = DEFAULT_ROT_CENTER_X;
            v->rot_center_y = DEFAULT_ROT_CENTER_Y;
            break;

        default:
            LCDBUF_XY(2, 2) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(3, 2) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(3, 1) = FALLING_BLOCK_CHAR;
            v->rot_center_x = DEFAULT_ROT_CENTER_X;
            v->rot_center_y = DEFAULT_ROT_CENTER_Y;

            switch(mino_type) {
                case MINO_TYPE_J:
                    LCDBUF_XY(3, 0) = FALLING_BLOCK_CHAR;
                    break;
                case MINO_TYPE_O:
                    LCDBUF_XY(2, 1) = FALLING_BLOCK_CHAR;
                    break;
                case MINO_TYPE_T:
                    LCDBUF_XY(3, 3) = FALLING_BLOCK_CHAR;
                    break;
                case MINO_TYPE_L:
                    LCDBUF_XY(1, 2) = FALLING_BLOCK_CHAR;
                    break;
                case MINO_TYPE_S:
                    LCDBUF_XY(4, 1) = FALLING_BLOCK_CHAR;
                    break;
                case MINO_TYPE_Z:
                    LCDBUF_XY(2, 3) = FALLING_BLOCK_CHAR;
                    break;
            }
    }
}

void reset_button_state(TickContext *ctx) {
    TickVariables *v = &ctx->variables;

    if (!ctx->is_left_pressed && (v->button_state & LEFT_BUTTON_MASK) != 0) {
        v->button_state &= ~LEFT_BUTTON_MASK;
    }
    if (!ctx->is_right_pressed && (v->button_state & RIGHT_BUTTON_MASK) != 0) {
        v->button_state &= ~RIGHT_BUTTON_MASK;
    }
    if (!ctx->is_down_pressed && (v->button_state & DOWN_BUTTON_MASK) != 0) {
        v->button_state &= ~DOWN_BUTTON_MASK;
    }
    if (!ctx->is_rotate_pressed && (v->button_state & ROTATE_BUTTON_MASK) != 0) {
        v->button_state &= ~ROTATE_BUTTON_MASK;
    }
}

void handle_button_press(TickContext *ctx) {
    TickVariables *v = &ctx->variables;

    if (ctx->is_left_pressed && (v->button_state & LEFT_BUTTON_MASK) == 0) {
        v->button_state |= LEFT_BUTTON_MASK;
        v->l = 2;
        move_horizontally(v);
    }
    if (ctx->is_right_pressed && (v->button_state & RIGHT_BUTTON_MASK) == 0) {
        v->button_state |= RIGHT_BUTTON_MASK;
        v->l = 0;
        move_horizontally(v);
    }
    if (ctx->is_rotate_pressed && (v->button_state & ROTATE_BUTTON_MASK) == 0) {
        v->button_state |= ROTATE_BUTTON_MASK;
        rotate(v);
    }
    if (ctx->is_down_pressed && (v->button_state & DOWN_BUTTON_MASK) == 0) {
        v->button_state |= DOWN_BUTTON_MASK;
        v->k = true;
        while (v->k) {
            move_down(v);
        }
        freeze_blocks(v);
        v->tick_count = DROPDOWN_INTERVAL-10;
    }
}

void erase_filled_line(TickVariables *v) {
    for(v->i = 20; v->i-- > 0;) { // x
        v->k = true;
        for (v->j = 0; v->j < 4; v->j++) { // y
            if (LCDBUF_XY(v->i, v->j) == AIR_BLOCK_CHAR) {
                v->k = false;
                break;
            }
        }

        if (!v->k) {
            continue;
        }

        for (v->j = 0; v->j < 4; v->j++) { // y
            LCDBUF_XY(v->i, v->j) = AIR_BLOCK_CHAR;
        }

        for(v->k = v->i+1; v->k-- > 1;) { // x
            for (v->l = 0; v->l < 4; v->l++) { // y
                if (LCDBUF_XY(v->k-1, v->l) != AIR_BLOCK_CHAR) {
                    LCDBUF_XY(v->k, v->l) = LCDBUF_XY(v->k-1, v->l);
                    LCDBUF_XY(v->k-1, v->l) = AIR_BLOCK_CHAR;
                }
            }
        }
    }
}

void tick(TickContext *ctx) {
    TickVariables *v = &ctx->variables;

    if (!v->gaming) {
        for (v->i = 0; v->i < 80; v->i++) {
            v->lcd_buffer[v->i] = AIR_BLOCK_CHAR;
        }

        v->gaming = true;
    }

    reset_button_state(ctx);
    handle_button_press(ctx);

    v->tick_count += 1;
    if (v->tick_count % DROPDOWN_INTERVAL != 0) {
        return;
    }

    v->tick_count = 0;

    v->i = false;
    for (v->j = 0; v->j < 80; v->j++) {
        if (v->lcd_buffer[v->j] == FALLING_BLOCK_CHAR) {
            v->i = true;
            break;
        }
    }

    if (v->i == false) {
        erase_filled_line(v);
        next_rand(v);
        v->i = v->rand_z % MINO_TYPES_LEN;
        place_mino(v, v->i);
        return;
    }

    move_down(v);

    if (!v->k) {
        freeze_blocks(v);
    }
}
