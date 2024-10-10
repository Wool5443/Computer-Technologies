#ifndef INPUT_FILES_H
#define INPUT_FILES_H

#include "Commands.h"
#include "Error.h"

DECLARE_RESULT(CommandList);

void              CommandListDtor(CommandList* list);
ResultCommandList CommandListCtor(const char filePath[static 1]);

#endif // INPUT_FILES_H
