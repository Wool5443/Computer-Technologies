#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "InputFiles.h"

DECLARE_RESULT_SOURCE(CommandList);

size_t countLines(const char text[static 1]);
size_t fileSize(int fd);

ResultCommandList CommandListCtor(const char filePath[static 1])
{
    assert(filePath);

    ERROR_CHECKING();

    FILE*    file   = NULL;
    char*    buffer = NULL;
    Command* list   = NULL;

    file = fopen(filePath, "r");
    if (!file)
    {
        err = ERROR_BAD_FILE;
        ERROR_LEAVE();
    }

    size_t size = fileSize(fileno(file));

    buffer = (char*)calloc(size + 1, 1);
    if (!buffer)
    {
        err = ERROR_NO_MEMORY;
        ERROR_LEAVE();
    }

    fread(buffer, 1, size, file);

    size_t lines = countLines(buffer);

    list = (Command*)calloc(lines, sizeof(*list));
    if (!list)
    {
        err = ERROR_NO_MEMORY;
        ERROR_LEAVE();
    }

    char* bufferPtr = strtok(buffer, "\n");
    for (size_t i = 0; i < lines; i++)
    {
        int delay = 0;
        int read = 0;

        sscanf(bufferPtr, "%d%n", &delay, &read);

        bufferPtr += read + 1;
        Command cmd = {
            .delay = delay,
            .command = bufferPtr,
            .args[0] = bufferPtr,
        };

        fprintf(stderr, "%s\n", bufferPtr);

        char* argPtr = strchr(bufferPtr, ' ');
        size_t argIndex = 1;

        while (argPtr)
        {
            if (argIndex == MAX_ARGS)
            {
                err = ERROR_INDEX_OUT_OF_BOUNDS;
                ERROR_LEAVE();
            }

            *(argPtr++) = '\0';
            cmd.args[argIndex++] = argPtr;
            argPtr = strchr(argPtr, ' ');
        }

        list[i] = cmd;

        bufferPtr = strtok(NULL, "\n");
    }

    return ResultCommandListCtor(
        (CommandList)
        {
            .size = lines,
            .commands = list,
            .m_buffer = buffer,
        },
        err
    );

ERROR_CASE
    fclose(file);
    free(buffer);
    free(list);

    if (err)
        LogError();

    return ResultCommandListCtor((CommandList){}, err);
}

void CommandListDtor(CommandList list[static 1])
{
    assert(list);

    free(list->commands);
    free(list->m_buffer);
}

size_t countLines(const char text[static 1])
{
    assert(text);

    size_t lines = 0;
    const char* endLine = strchr(text, '\n');

    while (endLine)
    {
        lines++;
        endLine = strchr(endLine + 1, '\n');
    }

    return lines;
}

size_t fileSize(int fd)
{
    struct stat st;
    fstat(fd, &st);

    return st.st_size;
}
