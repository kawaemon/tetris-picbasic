#include <stdint.h>
#include <stdbool.h>
#include <stdio.h> // for debug
#include "../bindings.h"

// rules (applied to tetris logic):
//   - no pointers
//   - no signed values
//   - no string literals
//   - no local variables
//   - use only these types: byte and word

typedef uint8_t  byte;
typedef uint16_t word;

// markers
#define INLINE_IN_PICBASIC 

// utilties
#define LCDBUF_XY(x, y) v->lcd_buffer[y*20+x]

#undef false
#undef true
#define false 0
#define true 1

#define FALLING_BLOCK_CHAR 'D'
#define STABLE_BLOCK_CHAR 'O'
#define AIR_BLOCK_CHAR '.'

#define MINO_TYPES_LEN 7

// S is shared with all minos except for I

// xxxx
#define MINO_TYPE_I 0
// sx
// ss
#define MINO_TYPE_O 1
//  s
// xss
#define MINO_TYPE_T 2
// width overflows, must be placed manually
// s
// ssxx
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
* 3 xss.
* 4 ..x.
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
*   ....
*   ....
* 20,3  20,0
*/

void next_rand(TickVariables *v) {
    v->rand_t = v->rand_x ^ v->rand_x << 1;
    v->rand_x = v->rand_y;
    v->rand_y = v->rand_z;
    v->rand_z ^= v->rand_z >> 3 ^ v->rand_t ^ v->rand_t >> 5;
}

void INLINE_IN_PICBASIC place_mino(TickVariables *v, byte mino_type) {
    switch (mino_type) {
        case MINO_TYPE_I:
            LCDBUF_XY(0, 2) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(1, 2) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(2, 2) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(3, 2) = FALLING_BLOCK_CHAR;
            break;

        case MINO_TYPE_J:
            LCDBUF_XY(2, 3) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(3, 3) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(3, 2) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(3, 1) = FALLING_BLOCK_CHAR;
            break;

        default:
            LCDBUF_XY(2, 2) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(3, 2) = FALLING_BLOCK_CHAR;
            LCDBUF_XY(3, 1) = FALLING_BLOCK_CHAR;

            switch(v->i) {
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

void tick(TickContext *ctx) {
    TickVariables *v = &ctx->variables;
    
    if (!v->gaming) {
        for (v->i = 0; v->i < 80; v->i++) {
            v->lcd_buffer[v->i] = AIR_BLOCK_CHAR;
        }

        v->gaming = true;
    }

    // v->i: 落下中のブロックがある?
    // v->j: iterator
    v->i = false;
    for (v->j = 0; v->j < 80; v->j++) {
        if (v->lcd_buffer[v->j] == FALLING_BLOCK_CHAR) {
            v->i = true;
            break;
        }
    }

    if (!v->i) {
        next_rand(v);
        v->i = v->rand_z % MINO_TYPES_LEN;
        place_mino(v, v->i);
        return;
    }

    // i: y axis in game, x axis on LCD
    // j: x axis in game, y axis on LCD
    // k: true if cant fall anymore
    v->k = false;
    for(v->i = 20; v->i-- > 0;) {
        for (v->j = 0; v->j < 4; v->j++) {
            if (
                LCDBUF_XY(v->i, v->j) == FALLING_BLOCK_CHAR && 
               (v->i+1 == 20 || LCDBUF_XY(v->i+1, v->j) == STABLE_BLOCK_CHAR)
            ) {
                v->k = true;
                goto outside_of_fall_check_loop;
            }
        }
    }

outside_of_fall_check_loop:
    if (v->k == true) {
        for(v->i = 0; v->i < 80; v->i++) {
            if (v->lcd_buffer[v->i] == FALLING_BLOCK_CHAR) {
                v->lcd_buffer[v->i] = STABLE_BLOCK_CHAR;
            }
        }
        return;
    }

    for(v->i = 20; v->i-- > 0;) {
        for (v->j = 0; v->j < 4; v->j++) {
            if (LCDBUF_XY(v->i, v->j) == FALLING_BLOCK_CHAR) {
                LCDBUF_XY(v->i, v->j) = AIR_BLOCK_CHAR;
                LCDBUF_XY(v->i+1, v->j) = FALLING_BLOCK_CHAR;
            }
        }
    }
}
