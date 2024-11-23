#ifndef INPUT_FILES_H
#define INPUT_FILES_H

#include "Commands.h"
#include "Logger.h" // IWYU pragma: keep

DECLARE_RESULT(CommandList);

ResultCommandList CommandListCtor(const char filePath[static 1]);
void              CommandListDtor(CommandList list[static 1]);

#endif // INPUT_FILES_H
