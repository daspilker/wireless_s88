//
// Copyright (c) Daniel A. Spilker.
// All rights reserved.
//
// This work is licensed under the Creative Commons Attribution 3.0 Germany License.
// To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/de/.
//
// http://daniel-spilker.com/
//

#include <stdint.h>
#include "stdbool.h"

void rf12_init(uint8_t node_id);

void rf12_txdata(uint8_t node_id, uint8_t *data, uint8_t number);

void rf12_rxdata(uint8_t *data, uint8_t number);

bool rf12_rxdata_timeout(uint8_t *data, uint8_t number);

bool rf12_can_send(void);
