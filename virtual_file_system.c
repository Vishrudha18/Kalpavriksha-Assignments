#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024
#define MAX_NAME_LEN 50
#define INPUT_SIZE 4096
#if NUM_BLOCKS < 1 || NUM_BLOCKS > 5000
#error "NUM_BLOCKS must be between 1 and 5000"
#endif

typedef struct FreeBlock
{
    int blockIndex;
    struct FreeBlock *prev;
    struct FreeBlock *next;
} FreeBlock;

typedef struct FileNode
{
    char name[MAX_NAME_LEN + 1];
    int isDirectory;
    struct FileNode *parent;
    struct FileNode *child;
    struct FileNode *next;
    int blockPointers[NUM_BLOCKS];
    int blockCount;
    size_t fileSize;
} FileNode;

unsigned char virtualDisk[NUM_BLOCKS][BLOCK_SIZE];

FreeBlock *freeHead = NULL;
FreeBlock *freeTail = NULL;

FileNode *root = NULL;
FileNode *cwd = NULL;

int addFreeBlock(int index)
{
    FreeBlock *node = malloc(sizeof(FreeBlock));
    if (node == NULL)
        return 0;
    node->blockIndex = index;
    node->prev = freeTail;
    node->next = NULL;

    if (freeHead == NULL)
        freeHead = node;
    else
        freeTail->next = node;

    freeTail = node;
    return 1;
}
int getFreeBlock(void)
{
    if (freeHead == NULL)
        return -1;
    FreeBlock *temp = freeHead;
    int index = temp->blockIndex;
    freeHead = temp->next;
    if (freeHead == NULL)
        freeTail = NULL;
    else
        freeHead->prev = NULL;
    free(temp);
    return index;
}
void initializeFreeList(void)
{
    for (int i = 0; i < NUM_BLOCKS; i++)
        addFreeBlock(i);
}
int countFreeBlocks(void)
{
    int count = 0;
    FreeBlock *temp = freeHead;
    while (temp != NULL)
    {
        count++;
        temp = temp->next;
    }
    return count;
}

FileNode *createNode(const char *name, int isDirectory, FileNode *parent)
{
    FileNode *node = calloc(1, sizeof(FileNode));
    if (node == NULL)
        return NULL;
    strcpy(node->name, name);
    node->isDirectory = isDirectory;
    node->parent = parent;
    return node;
}
FileNode *findChild(FileNode *directory, const char *name)
{
    if (directory == NULL || directory->child == NULL)
        return NULL;
    FileNode *temp = directory->child;
    do
    {
        if (strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }
    while (temp != directory->child);
    return NULL;
}

void insertChild(FileNode *parent, FileNode *node)
{
    if (parent->child == NULL)
    {
        parent->child = node;
        node->next = node;
        return;
    }
    FileNode *last = parent->child;
    while (last->next != parent->child)
        last = last->next;
    last->next = node;
    node->next = parent->child;
}

void removeChild(FileNode *parent, FileNode *node)
{
    FileNode *current = parent->child;
    FileNode *previous = NULL;
    do
    {
        if (current == node)
            break;
        previous = current;
        current = current->next;
    }
    while (current != parent->child);
    if (current != node)
        return;
    if (current->next == current)
    {
        parent->child = NULL;
    }
    else if (previous == NULL)
    {
        FileNode *last = parent->child;
        while (last->next != parent->child)
            last = last->next;
        parent->child = current->next;
        last->next = parent->child;
    }
    else
    {
        previous->next = current->next;
    }
    node->next = NULL;
    node->parent = NULL;
}

void initializeVFS(void)
{
    initializeFreeList();
    root = createNode("/", 1, NULL);
    cwd = root;
    printf("Compact VFS - ready. Type 'exit' to quit.\n");
}

int validName(const char *name)
{
    if (name == NULL || name[0] == '\0' ||
        strlen(name) > MAX_NAME_LEN ||
        strcmp(name, ".") == 0 || strcmp(name, "..") == 0 ||
        strchr(name, '/') != NULL)
        return 0;
    return 1;
}
void commandMkdir(const char *name)
{
    if (!validName(name))
    {
        printf("Error: Directory name must contain 1 to 50 characters.\n");
        return;
    }
    if (findChild(cwd, name) != NULL)
    {
        printf("Error: Name already exists.\n");
        return;
    }
    FileNode *directory = createNode(name, 1, cwd);
    if (directory == NULL)
    {
        printf("Error: Memory allocation failed.\n");
        return;
    }
    insertChild(cwd, directory);
    printf("Directory '%s' created successfully.\n", name);
}
void commandRmdir(const char *name)
{
    FileNode *directory = findChild(cwd, name);
    if (directory == NULL || !directory->isDirectory)
    {
        printf("Error: Directory not found.\n");
        return;
    }
    if (directory->child != NULL)
    {
        printf("Error: Directory is not empty.\n");
        return;
    }
    removeChild(cwd, directory);
    free(directory);
    printf("Directory '%s' removed successfully.\n", name);
}
void commandLs(void)
{
    if (cwd->child == NULL)
    {
        printf("(empty)\n");
        return;
    }
    FileNode *temp = cwd->child;
    do
    {
        printf("%s%s\n", temp->name,
               temp->isDirectory ? "/" : "");
        temp = temp->next;
    }
    while (temp != cwd->child);
}

void commandCreate(const char *name)
{
    if (!validName(name))
    {
        printf("Error: File name must contain 1 to 50 characters.\n");
        return;
    }
    if (findChild(cwd, name) != NULL)
    {
        printf("Error: Name already exists.\n");
        return;
    }
    FileNode *file = createNode(name, 0, cwd);
    if (file == NULL)
    {
        printf("Error: Memory allocation failed.\n");
        return;
    }
    insertChild(cwd, file);
    printf("File '%s' created successfully. No blocks allocated yet.\n", name);
}
void commandWrite(const char *name, const char *content)
{
    FileNode *file = findChild(cwd, name);
    if (file == NULL || file->isDirectory)
    {
        printf("Error: File not found.\n");
        return;
    }
    size_t length = strlen(content);
    int requiredBlocks = (int)((length + BLOCK_SIZE - 1) / BLOCK_SIZE);

    if (requiredBlocks > countFreeBlocks() + file->blockCount)
    {
        printf("Error: Disk is full.\n");
        return;
    }
   
    for (int i = 0; i < file->blockCount; i++)
    {
        int index = file->blockPointers[i];
        memset(virtualDisk[index], 0, BLOCK_SIZE);
        addFreeBlock(index);
    }
    file->blockCount = 0;
    file->fileSize = 0;
    
    size_t copied = 0;
    for (int i = 0; i < requiredBlocks; i++)
    {
        int index = getFreeBlock();
        if (index == -1)
        {
            printf("Error: Disk is full.\n");
            return;
        }
        file->blockPointers[file->blockCount++] = index;
        size_t remaining = length - copied;
        size_t amount = remaining < BLOCK_SIZE ? remaining : BLOCK_SIZE;
        memcpy(virtualDisk[index], content + copied, amount);
        copied += amount;
    }
    file->fileSize = length;
    printf("Data written successfully.\n");
}
void commandRead(const char *name)
{
    FileNode *file = findChild(cwd, name);
    if (file == NULL || file->isDirectory)
    {
        printf("Error: File not found.\n");
        return;
    }
    if (file->fileSize == 0)
    {
        printf("(empty)\n");
        return;
    }
    size_t remaining = file->fileSize;
    for (int i = 0; i < file->blockCount; i++)
    {
        size_t amount = remaining < BLOCK_SIZE ? remaining : BLOCK_SIZE;
        fwrite(virtualDisk[file->blockPointers[i]], 1, amount, stdout);
        remaining -= amount;
        if (remaining == 0)
            break;
    }
    printf("\n");
}
void commandDelete(const char *name)
{
    FileNode *file = findChild(cwd, name);
    if (file == NULL || file->isDirectory)
    {
        printf("Error: File not found.\n");
        return;
    }
  
    for (int i = 0; i < file->blockCount; i++)
    {
        int index = file->blockPointers[i];
        memset(virtualDisk[index], 0, BLOCK_SIZE);
        addFreeBlock(index);
    }
    removeChild(cwd, file);
    free(file);
    printf("File '%s' deleted successfully.\n", name);
}

void commandCd(const char *name)
{
    if (strcmp(name, "/") == 0)
    {
        cwd = root;
        return;
    }
    if (strcmp(name, "..") == 0)
    {
        if (cwd->parent != NULL)
            cwd = cwd->parent;
        return;
    }
    FileNode *directory = findChild(cwd, name);
    if (directory == NULL || !directory->isDirectory)
    {
        printf("Error: Directory not found.\n");
        return;
    }
    cwd = directory;
}
void printPath(void)
{
    FileNode *path[NUM_BLOCKS];
    int count = 0;
    FileNode *temp = cwd;
    while (temp != NULL && temp != root && count < NUM_BLOCKS)
    {
        path[count++] = temp;
        temp = temp->parent;
    }
    if (count == 0)
        printf("/");
    else
    {
        for (int i = count - 1; i >= 0; i--)
            printf("/%s", path[i]->name);
    }
}
void commandPwd(void)
{
    printPath();
    printf("\n");
}

void commandDf(void)
{
    int freeBlocks = countFreeBlocks();
    int usedBlocks = NUM_BLOCKS - freeBlocks;
    double usage = (double)usedBlocks / NUM_BLOCKS * 100.0;
    printf("Total Blocks: %d\n", NUM_BLOCKS);
    printf("Used Blocks: %d\n", usedBlocks);
    printf("Free Blocks: %d\n", freeBlocks);
    printf("Disk Usage: %.2f%%\n", usage);
}

void cleanupDirectory(FileNode *directory)
{
    if (directory == NULL || directory->child == NULL)
        return;
    FileNode *start = directory->child;
    FileNode *current = start;
    do
    {
        FileNode *next = current->next;
        if (current->isDirectory)
            cleanupDirectory(current);
        free(current);
        current = next;
    }
    while (current != start);
    directory->child = NULL;
}
void cleanupVFS(void)
{
    if (root != NULL)
    {
        cleanupDirectory(root);
        free(root);
        root = NULL;
        cwd = NULL;
    }
    FreeBlock *temp = freeHead;
    while (temp != NULL)
    {
        FreeBlock *next = temp->next;
        free(temp);
        temp = next;
    }
    freeHead = NULL;
    freeTail = NULL;
}

void printPrompt(void)
{
    printPath();
    printf("> ");
}
int main(void)
{
    initializeVFS();
    char line[INPUT_SIZE];
    while (1)
    {
        printPrompt();
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
        line[strcspn(line, "\n")] = '\0';
        char *command = strtok(line, " \t");
        if (command == NULL)
            continue;
        char *name = strtok(NULL, " \t");
        if (strcmp(command, "mkdir") == 0)
        {
            if (name == NULL)
                printf("Usage: mkdir <directory_name>\n");
            else
                commandMkdir(name);
        }
        else if (strcmp(command, "create") == 0)
        {
            if (name == NULL)
                printf("Usage: create <file_name>\n");
            else
                commandCreate(name);
        }
        else if (strcmp(command, "write") == 0)
        {
            if (name == NULL)
            {
                printf("Usage: write <file_name> <content>\n");
                continue;
            }
            char *content = strtok(NULL, "\n");
            if (content == NULL)
            {
                printf("Usage: write <file_name> <content>\n");
                continue;
            }
            while (*content == ' ' || *content == '\t')
                content++;
            commandWrite(name, content);
        }
        else if (strcmp(command, "read") == 0)
        {
            if (name == NULL)
                printf("Usage: read <file_name>\n");
            else
                commandRead(name);
        }
        else if (strcmp(command, "delete") == 0)
        {
            if (name == NULL)
                printf("Usage: delete <file_name>\n");
            else
                commandDelete(name);
        }
        else if (strcmp(command, "rmdir") == 0)
        {
            if (name == NULL)
                printf("Usage: rmdir <directory_name>\n");
            else
                commandRmdir(name);
        }
        else if (strcmp(command, "ls") == 0)
        {
            commandLs();
        }
        else if (strcmp(command, "cd") == 0)
        {
            if (name == NULL)
                printf("Usage: cd <directory_name|..|/>\n");
            else
                commandCd(name);
        }
        else if (strcmp(command, "pwd") == 0)
        {
            commandPwd();
        }
        else if (strcmp(command, "df") == 0)
        {
            commandDf();
        }
        else if (strcmp(command, "exit") == 0)
        {
            cleanupVFS();
            printf("Memory released. Exiting program...\n");
            return 0;
        }
        else
        {
            printf("Invalid command.\n");
        }
    }
    cleanupVFS();
    return 0;
}