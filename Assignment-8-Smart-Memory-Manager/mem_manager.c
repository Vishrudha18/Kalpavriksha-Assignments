#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#define MAX_POOL_SIZE 1000000

typedef struct
{
    int start;
    int size;
    int allocationID;
    int isAllocated;
} MemoryBlock;

unsigned char *memoryPool = NULL;
MemoryBlock *blocks = NULL;

int poolSize = 0;
int blockCount = 0;
int capacity = 0;
int nextAllocationID = 1;
int allocationStrategy = 1;

int initializeMemory(void)
{
    printf("Enter memory pool size (1-%d): ", MAX_POOL_SIZE);
    if (scanf("%d", &poolSize) != 1 ||
        poolSize < 1 || poolSize > MAX_POOL_SIZE)
    {
        printf("Invalid pool size.\n");
        return 0;
    }
    memoryPool = malloc((size_t)poolSize);
    if (memoryPool == NULL)
    {
        printf("Memory pool allocation failed.\n");
        return 0;
    }
    memset(memoryPool, 0, (size_t)poolSize);
    blocks = malloc(sizeof(MemoryBlock));
    if (blocks == NULL)
    {
        printf("Metadata allocation failed.\n");
        free(memoryPool);
        memoryPool = NULL;
        return 0;
    }
    capacity = 1;
    blockCount = 1;

    blocks[0].start = 0;
    blocks[0].size = poolSize;
    blocks[0].allocationID = 0;
    blocks[0].isAllocated = 0;
    printf("\nMemory pool initialized successfully!\n");
    return 1;
}

int resizeMetadata(int newCapacity)
{
    if (newCapacity <= 0 || (size_t)newCapacity > SIZE_MAX / sizeof(MemoryBlock))
    {
        return 0;
    }
    MemoryBlock *temp;
    temp = realloc(blocks, (size_t)newCapacity * sizeof(MemoryBlock));
    if (temp == NULL)
    {
        return 0;
    }
    blocks = temp;
    capacity = newCapacity;
    return 1;
}

int ensureMetadataCapacity(void)
{
    if (blockCount < capacity)
    {
        return 1;
    }
    if (capacity > INT_MAX / 2)
    {
        return 0;
    }
    int newCapacity = capacity * 2;
    if (newCapacity < 2)
    {
        newCapacity = 2;
    }
    return resizeMetadata(newCapacity);
}

int findFreeBlock(int requestedSize, int strategy)
{
    int selectedIndex = -1;
    for (int i = 0; i < blockCount; i++)
    {
        if (blocks[i].isAllocated == 0 && blocks[i].size >= requestedSize)
        {
            if (strategy == 1)
            {
                return i;
            }
            if (selectedIndex == -1)
            {
                selectedIndex = i;
            }
            else if (strategy == 2 && blocks[i].size < blocks[selectedIndex].size)
            {
                selectedIndex = i;
            }
            else if (strategy == 3 && blocks[i].size > blocks[selectedIndex].size)
            {
                selectedIndex = i;
            }
        }
    }
    return selectedIndex;
}

int allocateMemory(int requestedSize, int strategy)
{
    if (requestedSize <= 0 || requestedSize > poolSize)
    {
        printf("Invalid allocation size.\n");
        return -1;
    }
    if (strategy < 1 || strategy > 3)
    {
        printf("Invalid allocation strategy.\n");
        return -1;
    }
    if (nextAllocationID <= 0 || nextAllocationID == INT_MAX)
    {
        printf("Allocation ID limit reached.\n");
        return -1;
    }
    int index = findFreeBlock(requestedSize, strategy);
    if (index == -1)
    {
        int totalFree = 0;
        int largestFree = 0;
        for (int i = 0; i < blockCount; i++)
        {
            if (!blocks[i].isAllocated)
            {
                totalFree += blocks[i].size;
                if (blocks[i].size > largestFree)
                {
                    largestFree = blocks[i].size;
                }
            }
        }
        printf("\nAllocation failed.\n");
        if (totalFree >= requestedSize)
        {
            printf("Reason: External fragmentation.\n");
            printf("Requested size: %d bytes\n", requestedSize);
            printf("Total free memory: %d bytes\n", totalFree);
            printf("Largest free block: %d bytes\n", largestFree);

            printf("Individual free blocks: ");
            for (int i = 0; i < blockCount; i++)
            {
                if (!blocks[i].isAllocated)
                {
                    printf("%d bytes ", blocks[i].size);
                }
            }
            printf("\n");
        }
        else
        {
            printf("Reason: Insufficient total free memory.\n");
            printf("Requested size: %d bytes\n", requestedSize);
            printf("Total free memory: %d bytes\n", totalFree);
        }
        return -1;
    }

    if (blocks[index].size > requestedSize)
    {
        if (!ensureMetadataCapacity())
        {
            printf("Unable to increase metadata capacity.\n");
            return -1;
        }

        for (int i = blockCount; i > index + 1; i--)
        {
            blocks[i] = blocks[i - 1];
        }

        blocks[index + 1].start = blocks[index].start + requestedSize;
        blocks[index + 1].size = blocks[index].size - requestedSize;
        blocks[index + 1].allocationID = 0;
        blocks[index + 1].isAllocated = 0;
        blocks[index].size = requestedSize;
        blockCount++;
    }
    int id = nextAllocationID++;
    blocks[index].allocationID = id;
    blocks[index].isAllocated = 1;
    printf("\nAllocation successful!\n");
    printf("Allocation ID : %d\n", id);
    printf("Start offset  : %d\n", blocks[index].start);
    printf("Allocated size: %d bytes\n", blocks[index].size);
    return id;
}

void mergeFreeBlocks(void)
{
    int i = 0;
    while (i < blockCount - 1)
    {
        if (blocks[i].isAllocated == 0 && blocks[i + 1].isAllocated == 0 && blocks[i].start + blocks[i].size == blocks[i + 1].start)
        {
            blocks[i].size += blocks[i + 1].size;
            for (int j = i + 1; j < blockCount - 1; j++)
            {
                blocks[j] = blocks[j + 1];
            }
            blockCount--;
        }
        else
        {
            i++;
        }
    }
}

void releaseMemory(int allocationID)
{
    if (allocationID <= 0)
    {
        printf("Invalid allocation ID.\n");
        return;
    }
    int index = -1;
    for (int i = 0; i < blockCount; i++)
    {
        if (blocks[i].isAllocated == 1 && blocks[i].allocationID == allocationID)
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        printf("Invalid ID or allocation already released.\n");
        return;
    }
    blocks[index].isAllocated = 0;
    blocks[index].allocationID = 0;
    printf("Allocation ID %d released successfully.\n", allocationID);
    mergeFreeBlocks();
}

int resizeAllocation(int allocationID, int newSize)
{
    if (allocationID <= 0 || newSize <= 0 || newSize > poolSize)
    {
        printf("Invalid allocation ID or size.\n");
        return 0;
    }
    int index = -1;
    for (int i = 0; i < blockCount; i++)
    {
        if (blocks[i].isAllocated && blocks[i].allocationID == allocationID)
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        printf("Allocation ID not found.\n");
        return 0;
    }
    int oldSize = blocks[index].size;
    int oldStart = blocks[index].start;
    if (newSize == oldSize)
    {
        printf("New size is the same as the current size.\n");
        return 1;
    }
    if (newSize < oldSize)
    {
        int releasedSize = oldSize - newSize;
        if (index + 1 < blockCount && blocks[index + 1].isAllocated == 0)
        {
            blocks[index + 1].start = oldStart + newSize;
            blocks[index + 1].size += releasedSize;
        }
        else
        {
            if (!ensureMetadataCapacity())
            {
                printf("Unable to increase metadata capacity.\n");
                return 0;
            }
            for (int i = blockCount; i > index + 1; i--)
            {
                blocks[i] = blocks[i - 1];
            }
            blocks[index + 1].start = oldStart + newSize;
            blocks[index + 1].size = releasedSize;
            blocks[index + 1].allocationID = 0;
            blocks[index + 1].isAllocated = 0;
            blockCount++;
        }

        blocks[index].size = newSize;
        mergeFreeBlocks();
        printf("Allocation shrunk successfully.\n");
        return 1;
    }
    int extraSize = newSize - oldSize;

    if (index + 1 < blockCount && blocks[index + 1].isAllocated == 0 && blocks[index + 1].size >= extraSize)
    {
        if (blocks[index + 1].size == extraSize)
        {
            blocks[index].size = newSize;
            for (int i = index + 1; i < blockCount - 1; i++)
            {
                blocks[i] = blocks[i + 1];
            }
            blockCount--;
        }
        else
        {
            blocks[index + 1].start += extraSize;
            blocks[index + 1].size -= extraSize;
            blocks[index].size = newSize;
        }
        printf("Allocation expanded in place successfully.\n");
        return 1;
    }

    int temporaryID = allocateMemory(newSize, allocationStrategy);

    if (temporaryID == -1)
    {
        printf("Resize failed. Original allocation is unchanged.\n");
        return 0;
    }
    int newIndex = -1;
    for (int i = 0; i < blockCount; i++)
    {
        if (blocks[i].isAllocated && blocks[i].allocationID == temporaryID)
        {
            newIndex = i;
            break;
        }
    }
    if (newIndex == -1)
    {
        printf("Internal error during resize.\n");
        releaseMemory(temporaryID);
        return 0;
    }

    int newStart = blocks[newIndex].start;
    memcpy(memoryPool + newStart, memoryPool + oldStart, (size_t)oldSize);
    releaseMemory(allocationID);
    for (int i = 0; i < blockCount; i++)
    {
        if (blocks[i].isAllocated && blocks[i].allocationID == temporaryID)
        {
            blocks[i].allocationID = allocationID;
            break;
        }
    }
    printf("Allocation moved and resized successfully.\n");
    printf("New start: %d\n", newStart);
    printf("New size : %d\n", newSize);
    return 1;
}

void displayMemoryMap(void)
{
    printf("\n========== MEMORY MAP ==========\n");
    printf("%-8s %-12s %-12s %-12s %-12s\n", "Index", "Start", "Size", "ID", "Status");
    for (int i = 0; i < blockCount; i++)
    {
        printf("%-8d %-12d %-12d %-12d %-12s\n", i, blocks[i].start, blocks[i].size, blocks[i].allocationID, blocks[i].isAllocated ? "Allocated" : "Free");
    }

    printf("================================\n");
}

void displayStatistics(void)
{
    int allocatedMemory = 0;
    int freeMemory = 0;
    int allocatedBlocks = 0;
    int freeBlocks = 0;
    int largestFreeBlock = 0;

    for (int i = 0; i < blockCount; i++)
    {
        if (blocks[i].isAllocated)
        {
            allocatedMemory += blocks[i].size;
            allocatedBlocks++;
        }
        else
        {
            freeMemory += blocks[i].size;
            freeBlocks++;

            if (blocks[i].size > largestFreeBlock)
            {
                largestFreeBlock = blocks[i].size;
            }
        }
    }

    printf("\n========== MEMORY STATISTICS ==========\n");

    printf("Total memory          : %d bytes\n", poolSize);
    printf("Allocated memory      : %d bytes\n", allocatedMemory);
    printf("Free memory           : %d bytes\n", freeMemory);
    printf("Allocated blocks      : %d\n", allocatedBlocks);
    printf("Free blocks           : %d\n", freeBlocks);
    printf("Largest free block    : %d bytes\n", largestFreeBlock);

    printf("=======================================\n");
}

void changeStrategy(void)
{
    int choice;
    printf("\nSelect allocation strategy:\n");
    printf("1. First Fit\n");
    printf("2. Best Fit\n");
    printf("3. Worst Fit\n");
    printf("Enter choice: ");

    if (scanf("%d", &choice) != 1)
    {
        printf("Invalid input.\n");
        return;
    }
    if (choice < 1 || choice > 3)
    {
        printf("Invalid strategy.\n");
        return;
    }
    allocationStrategy = choice;
    printf("Allocation strategy changed to: %s\n", choice == 1 ? "First Fit" : choice == 2 ? "Best Fit" : "Worst Fit");
}

void displayMenu(void)
{
    printf("\n========== SMART MEMORY MANAGER ==========\n");
    printf("1. Allocate Memory\n");
    printf("2. Release Memory\n");
    printf("3. Resize Allocation\n");
    printf("4. Display Memory Map\n");
    printf("5. Display Memory Statistics\n");
    printf("6. Change Allocation Strategy\n");
    printf("7. Exit\n");
    printf("==========================================\n");
}

void cleanupMemory(void)
{
    free(memoryPool);
    free(blocks);

    memoryPool = NULL;
    blocks = NULL;

    poolSize = 0;
    blockCount = 0;
    capacity = 0;
}

int main(void)
{
    if (!initializeMemory())
    {
        cleanupMemory();
        return 1;
    }
    changeStrategy();

    int choice;
    int size;
    int id;

    while (1)
    {
        displayMenu();
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input. Please enter a number.\n");
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
            {
            }
            continue;
        }

        switch (choice)
        {
            case 1:
                printf("Enter allocation size: ");
                if (scanf("%d", &size) != 1)
                {
                    printf("Invalid input.\n");
                    break;
                }
                allocateMemory(size, allocationStrategy);
                break;

            case 2:
                printf("Enter allocation ID to release: ");
                if (scanf("%d", &id) != 1)
                {
                    printf("Invalid input.\n");
                    break;
                }
                releaseMemory(id);
                break;

            case 3:
                printf("Enter allocation ID: ");
                if (scanf("%d", &id) != 1)
                {
                    printf("Invalid input.\n");
                    break;
                }
                printf("Enter new allocation size: ");
                if (scanf("%d", &size) != 1)
                {
                    printf("Invalid input.\n");
                    break;
                }
                resizeAllocation(id, size);
                break;

            case 4:
                displayMemoryMap();
                break;

            case 5:
                displayStatistics();
                break;

            case 6:
                changeStrategy();
                break;

            case 7:
                cleanupMemory();
                printf("Memory manager closed successfully.\n");
                return 0;

            default:
                printf("Invalid menu choice.\n");
        }
    }
    return 0;
}