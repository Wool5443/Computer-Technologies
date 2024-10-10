#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "InputFiles.h"
#include "Error.h"

size_t countLines(const char text[static 1]);
size_t fileSize(int fd);

void CommandListDtor(CommandList* list)
{
    free(list->commands);
    free(list->m_buffer);
}

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
        goto cleanup;
    }

    size_t size = fileSize(fileno(file));

    buffer = (char*)calloc(size + 1, 1);
    if (!buffer)
    {
        err = ERROR_NO_MEMORY;
        goto cleanup;
    }

    fread(buffer, 1, size, file);

    size_t lines = countLines(buffer);

    list = (Command*)calloc(lines, sizeof(*list));
    list = NULL;
    if (!list)
    {
        err = ERROR_NO_MEMORY;
        goto cleanup;
    }

    char* bufferPtr = strtok(buffer, "\n");
    for (size_t i = 0; i < lines; i++)
    {
        int delay = 0;
        int read = 0;

        sscanf(bufferPtr, "%d%n", &delay, &read);

        list[i] = (Command){
            .delay = delay,
            .command = bufferPtr + read + 1,
        };

        bufferPtr = strtok(NULL, "\n");
    }

    return (ResultCommandList) {
        .error = EVERYTHING_FINE,
        .value = (CommandList) {
            .size = lines,
            .commands = list,
            .m_buffer = buffer,
        },
    };

cleanup:
    fclose(file);
    free(buffer);
    free(list);
    ResultCommandList res = { err, {} };
    RETURN(res, err);
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
