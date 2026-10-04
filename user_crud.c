#include <stdio.h>
#include <ctype.h>
struct User {
    int id;
    char name[50];
    int age;
};

int isValidName(char name[]) {
    for (int i = 0; name[i] != '\0'; i++) {
        if (!isalpha((unsigned char)name[i]) && name[i] != ' ') {
            return 0;
        }
    }

    return 1;
}

int isValidAge(int age) {
    return age >= 1;
}

void createUser() {
    struct User user;

    FILE *file = fopen("users.txt", "a");

    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    printf("Enter User ID: ");

    if (scanf("%d", &user.id) != 1) {
        printf("Invalid User ID. Please enter a number.\n");
        while (getchar() != '\n');
        fclose(file);
        return;
    }

    printf("Enter Name: ");
    scanf(" %49[^\n]", user.name);

    if (!isValidName(user.name)) {
        printf("Invalid name. Only letters and spaces are allowed.\n");
        fclose(file);
        return;
    }

    printf("Enter Age: ");

    if (scanf("%d", &user.age) != 1) {
        printf("Invalid age. Please enter a number.\n");
        while (getchar() != '\n');
        fclose(file);
        return;
    }

    if (!isValidAge(user.age)) {
        printf("Invalid age. Age must be positive.\n");
        fclose(file);
        return;
    }

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

    while (fscanf(file, "%d|%49[^|]|%d", &user.id, user.name, &user.age) == 3) {
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
    if (scanf("%d", &id) != 1) {
        printf("Invalid User ID. Please enter a number.\n");
        while (getchar() != '\n');
        fclose(file);
        fclose(temp);
        remove("temp.txt");
        return;
    }

    while (fscanf(file, "%d|%49[^|]|%d", &user.id, user.name, &user.age) == 3) {
        if (user.id == id) {
            found = 1;
            printf("Enter new name: ");
            scanf(" %49[^\n]", user.name);

            if (!isValidName(user.name)) {
                printf("Invalid name. Only letters and spaces are allowed.\n");
                fclose(file);
                fclose(temp);
                remove("temp.txt");
                return;
            }
            
            printf("Enter new age: ");

            if (scanf("%d", &user.age) != 1) {
                printf("Invalid age. Please enter a number.\n");
                while (getchar() != '\n');
                fclose(file);
                fclose(temp);
                remove("temp.txt");
                return;
            }

            if (!isValidAge(user.age)) {
                printf("Invalid age. Age must be positive.\n");
                fclose(file);
                fclose(temp);
                remove("temp.txt");
                return;
            }
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
    
    if (scanf("%d", &id) != 1) {
        printf("Invalid User ID. Please enter a number.\n");
        while (getchar() != '\n');
        fclose(file);
        fclose(temp);
        remove("temp.txt");
        return;
    }

    while (fscanf(file, "%d|%49[^|]|%d", &user.id, user.name, &user.age) == 3) {
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

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }

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