#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../LinkedList/account.h"

void saveToFile(Data dataToSave);
void saveAllData(List *list);
void loadData(List **list);