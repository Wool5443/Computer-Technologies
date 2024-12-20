#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Restaurant.h"
#include "Logger.h"
#include "Vector.h"
#include "String.h"

#define SHM_NAME "/tableSharedMem"
#define SEM_FREE_SPACE "/semFreeSpace"
#define SEM_WET_DISHES "/semWetDishes"
#define SEM_TABLE_MUTEX "/semTableMutex"

static const int SEM_MODE = 0666;
static const int DRY_TIME = 0;

static const size_t PIPE_SIZE = sizeof(bool);

typedef struct
{
    const char* name;
    size_t count;
} Order;
typedef Order* OrderList;

typedef struct
{
    const char* name;
    size_t time;
} Dish;
typedef Dish* DishList;

typedef struct
{
    const char* name;
    size_t number;
} Entry;
typedef Entry* EntryList;

typedef struct
{
    EntryList value;
    ErrorCode errorCode;
    String buffer;
} ResultEntryList;

typedef const char** Table;

static void sigintHandler(UNUSED int signum);

static ResultEntryList parseFile(const char filePath[static 1]);

static void washer(OrderList orders, DishList dishes, unsigned tableLimit, int pipefd[static 1]);
static void dryer(DishList dishes, unsigned tableLimit, int pipefd[static 1]);

ErrorCode RunRestaurant(const char ordersFilePath[static 1], const char timeTableFilePath[static 1])
{
    ERROR_CHECKING();

    assert(ordersFilePath);

    signal(SIGINT, sigintHandler);

    unsigned tableLimit = 0;
    Table table = {};
    OrderList orders = {};
    String ordersBuffer = {};
    DishList dishes = {};
    String dishesBuffer = {};

    int sharedfd = -1;

    int washerPipe[2] = {};
    int dryerPipe[2] = {};

    sem_t* semTableMutex = NULL;
    sem_t* semFreeSpace = NULL;
    sem_t* semWetDishes = NULL;

    const char* tableLimitStr = getenv("TABLE_LIMIT");
    if (!tableLimitStr)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to fetch TABLE_LIMIT: %s");
    }
    tableLimit = atoi(tableLimitStr);

    if (pipe(washerPipe) == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to pipe: %s");
    }
    if (pipe(dryerPipe) == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to pipe: %s");
    }
    if (fcntl(dryerPipe[0], F_SETFL, fcntl(dryerPipe[0], F_GETFL) | O_NONBLOCK))
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to fcntl read pipe: %s");
    }

    ResultEntryList ordersResult = parseFile(ordersFilePath);
    if ((err = ordersResult.errorCode))
    {
        ERROR_LEAVE();
    }
    orders = (OrderList)ordersResult.value;
    ordersBuffer = ordersResult.buffer;

    ResultEntryList dishesResult = parseFile(timeTableFilePath);
    if ((err = dishesResult .errorCode))
    {
        ERROR_LEAVE();
    }
    dishes = (DishList)dishesResult .value;
    dishesBuffer = dishesResult.buffer;

    sharedfd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (sharedfd == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open shared memory %s: %s", SHM_NAME);
    }
    if (ftruncate(sharedfd, tableLimit * sizeof(*table) + sizeof(VHeader_)) == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to truncate: %s");
    }

    table = mmap(NULL, tableLimit * sizeof(*table) + sizeof(VHeader_), PROT_READ | PROT_WRITE, MAP_SHARED, sharedfd, 0);
    if (!table)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to mmap: %s");
    }
    else
    {
        VHeader_* header = (VHeader_*)table;
        header->capacity = tableLimit;
        header->size = 0;

        table = (Table)(header + 1);
    }

    if (close(sharedfd) == -1)
    {
        sharedfd = -1;
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to close sharedfd: %s");
    }
    sharedfd = -1;

    semFreeSpace = sem_open(SEM_FREE_SPACE, O_CREAT | O_EXCL, SEM_MODE, tableLimit);
    if (!semFreeSpace)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open %s: %s", SEM_FREE_SPACE);
    }

    int semval = 0;
    if (sem_getvalue(semFreeSpace, &semval) == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed sem_getvalue: %s");
    }

    semTableMutex = sem_open(SEM_TABLE_MUTEX, O_CREAT | O_EXCL, SEM_MODE, 1);
    if (!semTableMutex)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open %s: %s", SEM_TABLE_MUTEX);
    }

    semWetDishes = sem_open(SEM_WET_DISHES, O_CREAT | O_EXCL, SEM_MODE, 0);
    if (!semWetDishes)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open %s: %s", SEM_WET_DISHES);
    }

    pid_t washerPid = fork();

    if (washerPid == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to fork washer: %s");
    }
    else if (washerPid == 0)
    {
        washer(orders, dishes, tableLimit, washerPipe);
        goto LOCAL_CLEANUP;
    }

    pid_t dryerPid = fork();

    if (dryerPid == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to fork washer: %s");
    }
    else if (dryerPid == 0)
    {
        dryer(dishes, tableLimit, dryerPipe);
        goto LOCAL_CLEANUP;
    }

    bool done = false;
    if (read(washerPipe[0], &done, PIPE_SIZE) == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to read from washer: %s");
    }

    if (write(dryerPipe[1], &done, PIPE_SIZE) == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to write to dryer: %s");
    }

    wait(NULL);

ERROR_CASE
    if (sharedfd != -1) shm_unlink(SHM_NAME);
    if (semTableMutex) sem_unlink(SEM_TABLE_MUTEX);
    if (semFreeSpace) sem_unlink(SEM_FREE_SPACE);
    if (semWetDishes) sem_unlink(SEM_WET_DISHES);

LOCAL_CLEANUP:
    if (table) munmap(GET_HEADER(table), VecCapacity(table) * sizeof(*table) + sizeof(VHeader_));

    VecDtor(orders);
    VecDtor(dishes);
    StringDtor(&ordersBuffer);
    StringDtor(&dishesBuffer);

    if (washerPipe[0])
    {
        close(washerPipe[0]);
        close(washerPipe[0]);
    }
    if (dryerPipe[0])
    {
        close(dryerPipe[0]);
        close(dryerPipe[0]);
    }

    return err;
}

static ResultEntryList parseFile(const char filePath[static 1])
{
    ERROR_CHECKING();

    assert(filePath);

    FILE* file = NULL;
    String content = {};
    EntryList entries = {};

    ResultString contentRes = StringReadFile(filePath);
    if ((err = contentRes.errorCode))
    {
        ERROR_LEAVE();
    }
    content = contentRes.value;

    char* contentPtr = content.data;

    while (contentPtr < content.data + content.size)
    {
        char* colon = strchr(contentPtr, ':');

        if (!colon) break;

        *colon = '\0';

        size_t number = 0, numberLength = 0;
        sscanf(colon + 1, "%zu%zn", &number, &numberLength);

        Entry entry = { contentPtr, number };
        contentPtr = colon + numberLength + 2;

        CHECK_ERROR(VecAdd(entries, entry), "Error adding entry %s:%zu", entry.name, entry.number);
    }

    return (ResultEntryList)
    {
        entries,
        EVERYTHING_FINE,
        content,
    };

ERROR_CASE
    if (file) fclose(file);
    StringDtor(&content);
    VecDtor(entries);

    return (ResultEntryList){ {}, err, {} };
}

size_t findDishTime(DishList dishes, const char* name)
{
    assert(dishes);

    for (size_t i = 0, end = VecSize(dishes); i < end; i++)
    {
        if (strcmp(dishes[i].name, name) == 0)
        {
            return dishes[i].time;
        }
    }

    return 0;
}

static void washer(OrderList orders, DishList dishes, unsigned tableLimit, int pipefd[static 1])
{
    ERROR_CHECKING();

    assert(orders);
    assert(dishes);
    assert(pipefd);

    int sharedfd = -1;
    sem_t* semTableMutex = NULL;
    sem_t* semFreeSpace = NULL;
    sem_t* semWetDishes = NULL;
    Table table = {};

    if (close(pipefd[0]) == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to close read pipe: %s");
    }

    sharedfd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (sharedfd == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open shared memory: %s");
    }
    else
    {
        VHeader_* header = mmap(NULL, tableLimit * sizeof(*table) + sizeof(VHeader_),
                                PROT_READ | PROT_WRITE, MAP_SHARED, sharedfd, 0);
        table = (Table)(header + 1);
    }

    if (close(sharedfd) == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open shared memory: %s");
    }

    semTableMutex = sem_open(SEM_TABLE_MUTEX, 0);
    if (!semTableMutex)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open %s: %s", SEM_TABLE_MUTEX);
    }

    semFreeSpace = sem_open(SEM_FREE_SPACE, 0);
    if (!semFreeSpace )
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open %s: %s", SEM_FREE_SPACE);
    }

    semWetDishes = sem_open(SEM_WET_DISHES, 0);
    if (!semWetDishes )
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open %s: %s", SEM_WET_DISHES);
    }

    for (size_t i = 0, end = VecSize(orders); i < end; i++)
    {
        Order order = orders[i];
        size_t time = findDishTime(dishes, order.name);

        for (size_t j = 0; j < order.count; j++)
        {
            sem_wait(semFreeSpace);

            fprintf(stdout, "Washing %s\n", order.name);
            sleep(time);
            fprintf(stdout, "Washed %s\n", order.name);

            sem_wait(semTableMutex);
            VecAdd(table, order.name);
            sem_post(semTableMutex);

            sem_post(semWetDishes);
        }
    }

    bool done = true;
    if (write(pipefd[1], &done, sizeof(done)) == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to write to pipe: %s");
    }
    if (close(pipefd[1]) == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to close write pipe: %s");
    }

ERROR_CASE
    if (sharedfd != -1) close(sharedfd);
    if (table) munmap(GET_HEADER(table), VecCapacity(table) * sizeof(*table) + sizeof(VHeader_));
    if (semTableMutex) sem_close(semTableMutex);
    if (semFreeSpace) sem_close(semFreeSpace);
    if (semWetDishes) sem_close(semWetDishes);
}

static void dryer(DishList dishes, unsigned tableLimit, int pipefd[static 1])
{
    ERROR_CHECKING();

    assert(dishes);
    assert(pipefd);

    int sharedfd = -1;
    sem_t* semTableMutex = NULL;
    sem_t* semFreeSpace = NULL;
    sem_t* semWetDishes = NULL;
    Table table = {};

    if (close(pipefd[1]) == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to close write pipe[%d]: %s", pipefd[1]);
    }

    sharedfd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (sharedfd == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open shared memory: %s");
    }
    else
    {
        VHeader_* header = mmap(NULL, tableLimit * sizeof(*table) + sizeof(VHeader_),
                                PROT_READ | PROT_WRITE, MAP_SHARED, sharedfd, 0);
        table = (Table)(header + 1);
    }

    if (close(sharedfd) == -1)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open shared memory: %s");
    }

    semTableMutex = sem_open(SEM_TABLE_MUTEX, 0);
    if (!semTableMutex)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open %s: %s", SEM_TABLE_MUTEX);
    }

    semFreeSpace = sem_open(SEM_FREE_SPACE, 0);
    if (!semFreeSpace)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open %s: %s", SEM_FREE_SPACE);
    }

    semWetDishes = sem_open(SEM_WET_DISHES, 0);
    if (!semWetDishes)
    {
        HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to open %s: %s", SEM_WET_DISHES);
    }

    bool running = true;

    while (running)
    {
        bool done = false;

        if (read(pipefd[0], &done, sizeof(done)) == -1)
        {
            if (errno != EAGAIN)
            {
                HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to read from pipe: %s");
            }
        }
        else
        {
            running = false;
            if (close(pipefd[0]) == -1)
            {
                HANDLE_ERRNO_ERROR(ERROR_LINUX, "Failed to close read pipe: %s");
            }
        }

        sem_wait(semWetDishes);

        sem_wait(semTableMutex);
        const char* name = VecPop(table);
        sem_post(semTableMutex);

        fprintf(stdout, "%20sDrying %s\n", "", name);
        sleep(DRY_TIME);
        fprintf(stdout, "%20sDried %s\n", "", name);

        sem_post(semFreeSpace);
    }

ERROR_CASE
    if (sharedfd != -1) close(sharedfd);
    if (table) munmap(GET_HEADER(table), VecCapacity(table) * sizeof(*table) + sizeof(VHeader_));
    if (semTableMutex) sem_close(semTableMutex);
    if (semFreeSpace) sem_close(semFreeSpace);
    if (semWetDishes) sem_close(semWetDishes);
}

static void sigintHandler(UNUSED int signum)
{
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_TABLE_MUTEX);
    sem_unlink(SEM_FREE_SPACE);
    sem_unlink(SEM_WET_DISHES);

    exit(0);
}
