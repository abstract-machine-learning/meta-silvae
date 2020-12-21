#ifndef FOREST_MAPPER_SILVA_H
#define FOREST_MAPPER_SILVA_H

#include <stdio.h>

#include "../forest.h"
#include "../../decision_tree/mapper/silva.h"


void forest_mapper_silva_load(FILE *fh, Forest *forest, const Dataset dataset);

void forest_mapper_silva_save(FILE *fh, const Forest forest);

#endif
