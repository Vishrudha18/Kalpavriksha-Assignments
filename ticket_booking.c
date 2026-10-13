#include <stdio.h>
#include <string.h>

#define STANDARD_PRICE 150
#define PREMIUM_PRICE 250
#define RECLINER_PRICE 400

int main()
{
    char name[50];
    int no_of_tickets;
    int choice;

    if(scanf("%49s",name)!=1){
        printf("Invalid customer name");
        return 1;
    }

    if(scanf("%d",&no_of_tickets)!=1){
        printf("Invalid ticket count");
        return 1;
    }
    
    if(no_of_tickets < 1 || no_of_tickets > 20){
        printf("Invalid ticket range");
        return 1;
    }

    float subtotal = 0;

    float discount;

    if(scanf("%d",&choice)!=1){
        printf("Invalid choice");
        return 1;
    }

    char category[30];
    switch(choice){
        case 1:
        strcpy(category, "Standard");
        subtotal = STANDARD_PRICE * no_of_tickets;
        break;

        case 2:
        strcpy(category, "Premium");
        subtotal = PREMIUM_PRICE * no_of_tickets;
        break;

        case 3:
        strcpy(category, "Recliner");
        subtotal = RECLINER_PRICE * no_of_tickets;
        break;

        default:
        printf("Invalid category");
        return 1;
    }

    if(no_of_tickets >= 5){
        discount = subtotal * 10 / 100;
    }
    else{
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