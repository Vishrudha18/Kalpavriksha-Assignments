#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_EMPLOYEES 100
#define NAME_LENGTH 50
#define DESIGNATION_LENGTH 50

struct Employee
{
    int id;
    char name[NAME_LENGTH];
    char designation[DESIGNATION_LENGTH];
    struct Employee *manager;
    struct Employee *reports[MAX_EMPLOYEES];
    int report_count;
    int active;
};

struct Employee employees[MAX_EMPLOYEES];
int employee_count = 0;
struct Employee *ceo = NULL;

int readInt(const char prompt[], int *value)
{
    char input[100];
    char *end;
    long number;

    printf("%s", prompt);

    if (fgets(input, sizeof(input), stdin) == NULL)
        return 0;

    number = strtol(input, &end, 10);

    if (end == input)
        return 0;

    while (*end == ' ' || *end == '\t')
        end++;

    if (*end != '\n' && *end != '\0')
        return 0;

    if (number < INT_MIN || number > INT_MAX)
        return 0;

    *value = (int)number;
    return 1;
}

int readText(const char prompt[], char destination[], int size)
{
    size_t length;
    int ch;

    printf("%s", prompt);

    if (fgets(destination, size, stdin) == NULL)
        return 0;

    length = strlen(destination);

    if (length > 0 && destination[length - 1] == '\n'){
        destination[length - 1] = '\0';
    }
    else{
        ch = getchar();

        if (ch != '\n' && ch != EOF)
        {
            while (ch != '\n' && ch != EOF)
                ch = getchar();

            return 0;
        }
    }

    if (destination[0] == '\0')
        return 0;

    return 1;
}

int isValidText(const char text[])
{
    int i;
    for (i = 0; text[i] != '\0'; i++){
        if (!((text[i] >= 'A' && text[i] <= 'Z') || (text[i] >= 'a' && text[i] <= 'z') || text[i] == ' '))
        {
            return 0;
        }
    }

    return 1;
}

struct Employee *findEmployee(int id)
{
    int i;

    for (i = 0; i < MAX_EMPLOYEES; i++)
    {
        if (employees[i].active && employees[i].id == id)
            return &employees[i];
    }

    return NULL;
}

struct Employee *findFreeSlot(void)
{
    int i;

    for (i = 0; i < MAX_EMPLOYEES; i++)
    {
        if (!employees[i].active)
            return &employees[i];
    }

    return NULL;
}

void initializeEmployee(struct Employee *employee, int id, const char name[], const char designation[])
{
    int i;

    employee->id = id;
    strcpy(employee->name, name);
    strcpy(employee->designation, designation);

    employee->manager = NULL;
    employee->report_count = 0;
    employee->active = 1;

    for (i = 0; i < MAX_EMPLOYEES; i++)
    {
        employee->reports[i] = NULL;
    }
}

int addEmployee(int id, const char name[], const char designation[], int manager_id)
{
    struct Employee *manager;
    struct Employee *new_employee;
    
    if (findEmployee(id) != NULL)
    {
        printf("Error: Employee ID already exists.\n");
        return 0;
    }

    if (employee_count == 0)
    {
        new_employee = findFreeSlot();
        
        if (new_employee == NULL)
        {
            printf("Employee capacity reached.\n");
            return 0;
        }
        
        initializeEmployee(new_employee, id, name, "CEO");
        
        new_employee->manager = NULL;
        ceo = new_employee;
        employee_count++;
        
        printf("%s has been set as the CEO.\n", name);
        return 1;
    }

    manager = findEmployee(manager_id);

    if (manager == NULL)
    {
        printf("Error: Manager not found.\n");
        return 0;
    }

    if (manager->report_count >= MAX_EMPLOYEES)
    {
        printf("Error: Manager's reports list is full.\n");
        return 0;
    }

    new_employee = findFreeSlot();

    if (new_employee == NULL)
    {
        printf("Error: Employee capacity reached.\n");
        return 0;
    }

    initializeEmployee(new_employee, id, name, designation);

    new_employee->manager = manager;

    manager->reports[manager->report_count] = new_employee;
    manager->report_count++;

    employee_count++;

    printf("Employee added successfully.\n");
    return 1;
}

void removeReportReference(struct Employee *manager, struct Employee *employee)
{
    int i, j;

    for (i = 0; i < manager->report_count; i++)
    {
        if (manager->reports[i] == employee)
        {
            for (j = i; j < manager->report_count - 1; j++)
            {
                manager->reports[j] = manager->reports[j + 1];
            }

            manager->report_count--;
            manager->reports[manager->report_count] = NULL;
            return;
        }
    }
}

struct Employee *findSiblingWithFewestReports(struct Employee *employee)
{
    struct Employee *manager = employee->manager;
    struct Employee *candidate = NULL;
    int i;

    for (i = 0; i < manager->report_count; i++)
    {
        struct Employee *sibling = manager->reports[i];

        if (sibling == employee)
            continue;

        if (candidate == NULL || sibling->report_count < candidate->report_count)
        {
            candidate = sibling;
        }
    }

    return candidate;
}

int removeEmployee(int id, struct Employee *ceo)
{
    struct Employee *employee = findEmployee(id);
    struct Employee *superior;
    struct Employee *recipient;
    int i;

    if (employee == NULL){
        printf("Error: Employee not found.\n");
        return 0;
    }

    if (employee == ceo){
        printf("Error: The CEO cannot be removed.\n");
        return 0;
    }

    superior = employee->manager;

    if (superior == NULL){
        printf("Error: Employee has no reporting manager.\n");
        return 0;
    }

    recipient = findSiblingWithFewestReports(employee);

    if (recipient == NULL)
        recipient = superior;

    if (recipient->report_count + employee->report_count > MAX_EMPLOYEES){
        printf("Error: Cannot transfer reports; recipient's "
               "reports list is full.\n");
        return 0;
    }

    for (i = 0; i < employee->report_count; i++){
        struct Employee *child = employee->reports[i];

        recipient->reports[recipient->report_count] = child;
        recipient->report_count++;

        child->manager = recipient;
    }

    removeReportReference(superior, employee);

    for (i = 0; i < MAX_EMPLOYEES; i++){
        employee->reports[i] = NULL;
    }
    employee->manager = NULL;
    employee->report_count = 0;
    employee->active = 0;

    employee_count--;

    printf("Employee removed successfully.\n");

    if (recipient == superior){
        printf("Direct reports transferred to the superior: %s\n",
               superior->name);
    }
    else{
        printf("Direct reports transferred to sibling: %s\n",
               recipient->name);
    }

    return 1;
}

int countDescendants(struct Employee *employee)
{
    int total = 0;
    int i;
    for (i = 0; i < employee->report_count; i++){
        total += 1;
        total += countDescendants(employee->reports[i]);
    }
    return total;
}

void displayHierarchy(struct Employee *employee, int level)
{
    int i, j;

    for (j = 0; j < level; j++)
        printf("    ");

    printf("%d - %s - %s\n", employee->id, employee->name, employee->designation);

    for (i = 0; i < employee->report_count; i++){
        displayHierarchy(employee->reports[i], level + 1);
    }
}

void displayEmployeeHierarchy(int id)
{
    struct Employee *employee = findEmployee(id);

    if (employee == NULL){
        printf("Error: Employee not found.\n");
        return;
    }
    printf("\n");
    displayHierarchy(employee, 0);
    printf("\nTotal Employees under %d: %d\n", id, countDescendants(employee));
}

void displayCompleteHierarchy(struct Employee *ceo)
{
    if (ceo == NULL || !ceo->active){
        printf("CEO not found.\n");
        return;
    }
    printf("\n===== ORGANIZATION HIERARCHY =====\n\n");
    displayHierarchy(ceo, 0);
    printf("\nTotal Employees: %d\n", employee_count);
}

void addEmployeeMenu(void)
{
    int id, manager_id;
    char name[NAME_LENGTH];
    char designation[DESIGNATION_LENGTH];

    if (!readInt("Enter employee ID: ", &id)){
        printf("Invalid ID.\n");
        return;
    }

    if (!readText("Enter employee name: ", name, sizeof(name))){
        printf("Invalid name or name too long.\n");
        return;
    }

    if (!isValidText(name)){
        printf("Invalid name. Use letters and spaces only.\n");
        return;
    }

    if (employee_count == 0){
        addEmployee(id, name, "CEO", 0);
        return;
    }

    if (!readText("Enter designation: ", designation, sizeof(designation))){
        printf("Invalid designation or designation too long.\n");
        return;
    }

    if (!isValidText(designation)){
        printf("Invalid designation. Use letters and spaces only.\n");
        return;
    }

    if (!readInt("Enter reporting manager's ID: ", &manager_id)){
        printf("Invalid manager ID.\n");
        return;
    }
    addEmployee(id, name, designation, manager_id);
}

void removeEmployeeMenu(struct Employee *ceo)
{
    int id;
    if (!readInt("Enter employee ID to remove: ", &id)){
        printf("Invalid ID.\n");
        return;
    }
    removeEmployee(id, ceo);
}

int main(void)
{
    int choice;

    printf("=== Employee Hierarchy Management System ===\n");

    while (1)
    {
        printf("\n========== MENU ==========\n");
        printf("1. Add Employee\n");
        printf("2. Remove Employee\n");
        printf("3. Display Employee Hierarchy\n");
        printf("4. Display Complete Hierarchy\n");
        printf("5. Exit\n");

        if (!readInt("Enter your choice: ", &choice)){
            printf("Invalid choice. Enter a number from 1 to 5.\n");
            continue;
        }

        switch (choice){
            case 1:
                addEmployeeMenu();
                break;

            case 2:
                removeEmployeeMenu(ceo);
                break;

            case 3:
            {
                int id;
                if (!readInt("Enter employee ID: ", &id))
                {
                    printf("Invalid ID.\n");
                    break;
                }
                displayEmployeeHierarchy(id);
                break;
            }

            case 4:
                displayCompleteHierarchy(ceo);
                break;

            case 5:
                printf("Exiting program.\n");
                return 0;

            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}