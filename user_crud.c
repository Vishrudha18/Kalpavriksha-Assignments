#include <stdio.h>

struct User {
    int id;
    char name[50];
    int age;
};

void createUser() {
    struct User user;

    FILE *file = fopen("users.txt", "a");

    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    printf("Enter User ID: ");
    scanf("%d", &user.id);

    printf("Enter Name: ");
    scanf(" %[^\n]", user.name);

    printf("Enter Age: ");
    scanf("%d", &user.age);

    fprintf(file, "%d|%s|%d\n", user.id, user.name, user.age);

    fclose(file);

    printf("User added successfully.\n");
}

void readUsers() {
    struct User user;

    FILE *file = fopen("users.txt", "r");

    if (file == NULL) {
        printf("No users found.\n");
        return;
    }

    printf("\n===== USER LIST =====\n");
    printf("ID\tName\t\tAge\n");
    printf("----------------------------\n");

    while (fscanf(file, "%d|%[^|]|%d", &user.id, user.name, &user.age) == 3) {
        printf("%d\t%s\t\t%d\n", user.id, user.name, user.age);
    }

    fclose(file);
}

void updateUser() {
    int id;
    int found = 0;
    struct User user;

    FILE *file = fopen("users.txt", "r");

    if (file == NULL) {
        printf("No users found.\n");
        return;
    }

    FILE *temp = fopen("temp.txt", "w");

    if (temp == NULL) {
        printf("Error creating temporary file.\n");
        fclose(file);
        return;
    }

    printf("Enter User ID to update: ");
    scanf("%d", &id);

    while (fscanf(file, "%d|%[^|]|%d", &user.id, user.name, &user.age) == 3) {
        if (user.id == id) {
            found = 1;
            printf("Enter new name: ");
            scanf(" %[^\n]", user.name);
            
            printf("Enter new age: ");
            scanf("%d", &user.age);
        }

        fprintf(temp, "%d|%s|%d\n", user.id, user.name, user.age);
    }
    fclose(file);
    fclose(temp);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (found) {
        printf("User updated successfully.\n");
    } else {
        printf("User not found.\n");
    }
}

void deleteUser() {
    int id;
    int found = 0;
    struct User user;

    FILE *file = fopen("users.txt", "r");

    if (file == NULL) {
        printf("No users found.\n");
        return;
    }

    FILE *temp = fopen("temp.txt", "w");

    if (temp == NULL) {
        printf("Error creating temporary file.\n");
        fclose(file);
        return;
    }

    printf("Enter User ID to delete: ");
    scanf("%d", &id);

    while (fscanf(file, "%d|%[^|]|%d", &user.id, user.name, &user.age) == 3) {
        if (user.id == id) {
            found = 1;
        } else {
            fprintf(temp, "%d|%s|%d\n", user.id, user.name, user.age);
        }
    }

    fclose(file);
    fclose(temp);
    
    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (found) {
        printf("User deleted successfully.\n");
    } else {
        printf("User not found.\n");
    }
}

int main() {
    int choice;

    do {
        printf("\n===== USER MANAGEMENT SYSTEM =====\n");
        printf("1. Create User\n");
        printf("2. Read Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        scanf("%d", &choice);

        switch (choice) {
            case 1:
                createUser();
                break;

            case 2:
                readUsers();
                break;

            case 3:
                updateUser();
                break;

            case 4:
                deleteUser();
                break;

            case 5:
                printf("Exiting program...\n");
                break;

            default:
                printf("Invalid choice. Please try again.\n");
        }

    } while (choice != 5);

    return 0;
}