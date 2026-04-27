#ifndef AXK_KEY_H
#define AXK_KEY_H

#include <stdbool.h>
#include <stdint.h>

void axk_key_scan(void);
void axk_key_output(uint8_t num);
void axk_key_state(void);

#endif /* AXK_KEY_H */
