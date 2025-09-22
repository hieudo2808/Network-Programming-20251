#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linklist.h"

void saveToFile(Data dataToSave);
void saveAllData(List *list);
void loadData(List **list);
void saveHistory(char username[]);
void loadHistory(char username[]);