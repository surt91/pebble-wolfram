#ifndef CELLULAR_AUTOMATA_H
#define CELLULAR_AUTOMATA_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void ca_main(uint8_t rule, int x, int y, uint8_t *out, int random_seed);

#endif //CELLULAR_AUTOMATA_H
