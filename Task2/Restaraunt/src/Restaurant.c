#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Restaurant.h"
#include "Logger.h"
#include "Vector.h"
#include "String.h"

#define HANDLE_LINUX_ERROR(...)                                             \
    int ern = errno;                                                        \
    err = ERROR_LINUX;                                                      \
    LogError(__VA_ARGS__, strerror(ern));                                   \
    ERROR_LEAVE()

#define SHM_NAME "/tableSharedMem"
#define SEM_FREE_SPACE "/semFreeSpace"
#define SEM_WET_DISHES "/semWetDishes"
#define SEM_TABLE_MUTEX "/semTableMutex"

static const int SEM_MODE = 0666;
static const int DRY_TIME = 5;

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

static ResultEntryList parseFile(const char filePath[static 1]);

static void washer(OrderList orders, DishList dishes, unsigned tableLimit, int* doneWashingfd);
static void dryer(DishList dishes, unsigned tableLimit, int* doneWashingfd);

ErrorCode RunRestaurant(const char ordersFilePath[static 1], const char timeTableFilePath[static 1])
{
    ERROR_CHECKING();

    assert(ordersFilePath);

    unsigned tableLimit = 0;
    Table table = {};
    OrderList orders = {};
    String ordersBuffer = {};
    DishList dishes = {};
    String dishesBuffer = {};

    int sharedfd = -1;

    sem_t* semTableMutex = NULL;
    sem_t* semFreeSpace = NULL;
    sem_t* semWetDishes = NULL;

    const char* tableLimitStr = getenv("TABLE_LIMIT");
    if (!tableLimitStr)
    {
        HANDLE_LINUX_ERROR("Failed to fetch TABLE_LIMIT: %s");
    }
    tableLimit = atoi(tableLimitStr);

    int doneWashingfd[2] = {};
    if (pipe(doneWashingfd) == -1)
    {
        HANDLE_LINUX_ERROR("Failed to pipe: %s");
    }
    if (fcntl(doneWashingfd[0], F_SETFL, fcntl(doneWashingfd[0], F_GETFL) | O_NONBLOCK))
    {
        HANDLE_LINUX_ERROR("Failed to fcntl read pipe: %s");
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
        HANDLE_LINUX_ERROR("Failed to open shared memory %s: %s", SHM_NAME);
    }
    if (ftruncate(sharedfd, tableLimit * sizeof(*table) + sizeof(VHeader_)) == -1)
    {
        HANDLE_LINUX_ERROR("Failed to truncate: %s");
    }

    table = mmap(NULL, tableLimit * sizeof(*table) + sizeof(VHeader_), PROT_READ | PROT_WRITE, MAP_SHARED, sharedfd, 0);
    if (!table)
    {
        HANDLE_LINUX_ERROR("Failed to mmap: %s");
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
        HANDLE_LINUX_ERROR("Failed to close sharedfd: %s");
    }
    sharedfd = -1;

    semFreeSpace = sem_open(SEM_FREE_SPACE, O_CREAT | O_EXCL, SEM_MODE, tableLimit);
    if (!semFreeSpace)
    {
        HANDLE_LINUX_ERROR("Failed to open %s: %s", SEM_FREE_SPACE);
    }

    int semval = 0;
    if (sem_getvalue(semFreeSpace, &semval) == -1)
    {
        HANDLE_LINUX_ERROR("Failed sem_getvalue: %s");
    }

    semTableMutex = sem_open(SEM_TABLE_MUTEX, O_CREAT | O_EXCL, SEM_MODE, 1);
    if (!semTableMutex )
    {
        HANDLE_LINUX_ERROR("Failed to open %s: %s", SEM_TABLE_MUTEX);
    }

    semWetDishes = sem_open(SEM_WET_DISHES, O_CREAT | O_EXCL, SEM_MODE, 0);
    if (!semWetDishes )
    {
        HANDLE_LINUX_ERROR("Failed to open %s: %s", SEM_WET_DISHES);
    }

    pid_t washerPid = fork();

    if (washerPid == -1)
    {
        HANDLE_LINUX_ERROR("Failed to fork washer: %s");
    }
    else if (washerPid == 0)
    {
        washer(orders, dishes, tableLimit, doneWashingfd);
    }

    pid_t dryerPid = fork();

    if (dryerPid == -1)
    {
        HANDLE_LINUX_ERROR("Failed to fork washer: %s");
    }
    else if (dryerPid  == 0)
    {
        dryer(dishes, tableLimit, doneWashingfd);
    }

    wait(NULL);
    wait(NULL);

ERROR_CASE
    VecDtor(orders);
    VecDtor(dishes);
    StringDtor(&ordersBuffer);
    StringDtor(&dishesBuffer);

    if (sharedfd != -1) shm_unlink(SHM_NAME);
    if (table) munmap(GET_HEADER(table), VecCapacity(table) * sizeof(*table) + sizeof(VHeader_));
    if (semTableMutex) sem_unlink(SEM_TABLE_MUTEX);
    if (semFreeSpace) sem_unlink(SEM_FREE_SPACE);
    if (semWetDishes) sem_unlink(SEM_WET_DISHES);
    if (doneWashingfd[0])
    {
        close(doneWashingfd[0]);
        close(doneWashingfd[0]);
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

        if ((err = VecAdd(entries, entry)))
        {
            LogError("Error adding entry");
            ERROR_LEAVE();
        }
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

static void washer(OrderList orders, DishList dishes, unsigned tableLimit, int* doneWashingfd)
{
    ERROR_CHECKING();

    assert(orders);
    assert(dishes);
    assert(doneWashingfd);

    int sharedfd = -1;
    sem_t* semTableMutex = NULL;
    sem_t* semFreeSpace = NULL;
    sem_t* semWetDishes = NULL;
    Table table = {};

    if (close(doneWashingfd[0]) == -1)
    {
        HANDLE_LINUX_ERROR("Failed to close read pipe: %s");
    }

    sharedfd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (sharedfd == -1)
    {
        HANDLE_LINUX_ERROR("Failed to open shared memory: %s");
    }
    else
    {
        VHeader_* header = mmap(NULL, tableLimit * sizeof(*table) + sizeof(VHeader_),
                                PROT_READ | PROT_WRITE, MAP_SHARED, sharedfd, 0);
        table = (Table)(header + 1);
    }

    if (close(sharedfd) == -1)
    {
        HANDLE_LINUX_ERROR("Failed to open shared memory: %s");
    }

    semTableMutex = sem_open(SEM_TABLE_MUTEX, 0);
    if (!semTableMutex)
    {
        HANDLE_LINUX_ERROR("Failed to open %s: %s", SEM_TABLE_MUTEX);
    }

    semFreeSpace = sem_open(SEM_FREE_SPACE, 0);
    if (!semFreeSpace )
    {
        HANDLE_LINUX_ERROR("Failed to open %s: %s", SEM_FREE_SPACE);
    }

    semWetDishes = sem_open(SEM_WET_DISHES, 0);
    if (!semWetDishes )
    {
        HANDLE_LINUX_ERROR("Failed to open %s: %s", SEM_WET_DISHES);
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
    if (write(doneWashingfd[1], &done, sizeof(done)) == -1)
    {
        HANDLE_LINUX_ERROR("Failed to write to pipe: %s");
    }
    if (close(doneWashingfd[1]) == -1)
    {
        HANDLE_LINUX_ERROR("Failed to close write pipe: %s");
    }

ERROR_CASE
    if (sharedfd != -1) close(sharedfd);
    if (table) munmap(GET_HEADER(table), VecCapacity(table) * sizeof(*table) + sizeof(VHeader_));
    if (semTableMutex) sem_close(semTableMutex);
    if (semFreeSpace) sem_close(semFreeSpace);
    if (semWetDishes) sem_close(semWetDishes);
}

static void dryer(DishList dishes, unsigned tableLimit, int* doneWashingfd)
{
    ERROR_CHECKING();

    assert(dishes);
    assert(doneWashingfd);

    int sharedfd = -1;
    sem_t* semTableMutex = NULL;
    sem_t* semFreeSpace = NULL;
    sem_t* semWetDishes = NULL;
    Table table = {};

    if (close(doneWashingfd[1]) == -1)
    {
        HANDLE_LINUX_ERROR("Failed to close write pipe[%d]: %s", doneWashingfd[1]);
    }

    sharedfd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (sharedfd == -1)
    {
        HANDLE_LINUX_ERROR("Failed to open shared memory: %s");
    }
    else
    {
        VHeader_* header = mmap(NULL, tableLimit * sizeof(*table) + sizeof(VHeader_),
                                PROT_READ | PROT_WRITE, MAP_SHARED, sharedfd, 0);
        table = (Table)(header + 1);
    }

    if (close(sharedfd) == -1)
    {
        HANDLE_LINUX_ERROR("Failed to open shared memory: %s");
    }

    semTableMutex = sem_open(SEM_TABLE_MUTEX, 0);
    if (!semTableMutex)
    {
        HANDLE_LINUX_ERROR("Failed to open %s: %s", SEM_TABLE_MUTEX);
    }

    semFreeSpace = sem_open(SEM_FREE_SPACE, 0);
    if (!semFreeSpace)
    {
        HANDLE_LINUX_ERROR("Failed to open %s: %s", SEM_FREE_SPACE);
    }

    semWetDishes = sem_open(SEM_WET_DISHES, 0);
    if (!semWetDishes)
    {
        HANDLE_LINUX_ERROR("Failed to open %s: %s", SEM_WET_DISHES);
    }

    bool running = true;

    while (running)
    {
        bool done = false;

        if (read(doneWashingfd[0], &done, sizeof(done)) == -1)
        {
            if (errno != EAGAIN)
            {
                HANDLE_LINUX_ERROR("Failed to read from pipe: %s");
            }
        }
        else
        {
            running = false;
            if (close(doneWashingfd[0]) == -1)
            {
                HANDLE_LINUX_ERROR("Faile to close read pipe: %s");
            }
        }

        sem_wait(semWetDishes);

        sem_wait(semTableMutex);
        while (VecSize(table) > 0)
        {
            const char* name = VecPop(table);
            sem_post(semTableMutex);

            fprintf(stdout, "Drying %s\n", name);
            sleep(DRY_TIME);
            fprintf(stdout, "Dried %s\n", name);

            sem_post(semFreeSpace);
        }
    }

ERROR_CASE
    if (sharedfd != -1) close(sharedfd);
    if (table) munmap(GET_HEADER(table), VecCapacity(table) * sizeof(*table) + sizeof(VHeader_));
    if (semTableMutex) sem_close(semTableMutex);
    if (semFreeSpace) sem_close(semFreeSpace);
    if (semWetDishes) sem_close(semWetDishes);
}
