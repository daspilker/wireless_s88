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

void rf12_init(void);

void rf12_txdata(unsigned char *data, unsigned char number);

void rf12_rxdata(unsigned char *data, unsigned char number);
