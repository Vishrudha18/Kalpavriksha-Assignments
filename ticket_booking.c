#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define STANDARD_PRICE 150
#define PREMIUM_PRICE 250
#define RECLINER_PRICE 400

int main()
{
    char name[50];
    char input[100];
    char *endptr;
    int no_of_tickets;
    int choice;
    int price;
    char category[30];

    float subtotal;
    float discount;

    if (fgets(name, sizeof(name), stdin) == NULL) {
        printf("Invalid customer name");
        return 1;
    }

    name[strcspn(name, "\n")] = '\0';

    if (name[0] == '\0' || strpbrk(name, " \t\r\n") != NULL) {
        printf("Invalid customer name");
        return 1;
    }

    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Invalid ticket count");
        return 1;
    }

    no_of_tickets = (int)strtol(input, &endptr, 10);

    while (isspace((unsigned char)*endptr)) {
        endptr++;
    }

    if (endptr == input || *endptr != '\0') {
        printf("Invalid ticket count");
        return 1;
    }

    if (no_of_tickets < 1 || no_of_tickets > 20) {
        printf("Invalid ticket range");
        return 1;
    }

    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Invalid choice");
        return 1;
    }

    choice = (int)strtol(input, &endptr, 10);

    while (isspace((unsigned char)*endptr)) {
        endptr++;
    }

    if (endptr == input || *endptr != '\0') {
        printf("Invalid choice");
        return 1;
    }

    switch (choice) {
        case 1:
            strcpy(category, "Standard");
            price = STANDARD_PRICE;
            break;

        case 2:
            strcpy(category, "Premium");
            price = PREMIUM_PRICE;
            break;

        case 3:
            strcpy(category, "Recliner");
            price = RECLINER_PRICE;
            break;

        default:
            printf("Invalid category");
            return 1;
    }

    subtotal = (float)price * no_of_tickets;

    if (no_of_tickets >= 5) {
        discount = subtotal * 10 / 100;
    } else {
        discount = 0;
    }

    float final_amount = subtotal - discount;

    printf("Customer: %s\n", name);
    printf("Ticket Category: %s\n", category);
    printf("Tickets: %d\n", no_of_tickets);
    printf("Subtotal: %.2f\n", subtotal);
    printf("Discount: %.2f\n", discount);
    printf("Final Bill: %.2f\n", final_amount);
}