#include <stdint.h>
#include <stdbool.h>
#include "../bindings.h"

void tick(TickContext *ctx) {
    ctx->variables.lcd_buffer[2] = 'C';
}
