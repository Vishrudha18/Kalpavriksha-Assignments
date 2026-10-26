#include <stdio.h>

struct Student
{
    int roll_no;
    char name[50];
    int mark1, mark2, mark3;
};

int calculateTotal(int mark1, int mark2, int mark3)
{
    return mark1 + mark2 + mark3;  
}

float calculateAverage(int total)
{
    float avg = total/(float)3;
    return avg;
}

char calculateGrade(float average)
{
    if(average>=85){
        return 'A';
    }
    else if(average>=70 && average<85){
        return 'B';
    }
    else if(average>=50 && average<70){
        return 'C';
    }
    else if(average>=35 && average<50){
        return 'D';
    }
    else{
        return 'F';
    }
}

void printRollNumbers(struct Student data[], int index, int no_of_stud)
{
    if(index >= no_of_stud){
        return;
    }
    printf("%d ",data[index].roll_no);
    printRollNumbers(data, index+1, no_of_stud);
}

int main()
{
    int no_of_stud;
    scanf("%d", &no_of_stud);
    struct Student data[no_of_stud];

    for(int i = 0; i < no_of_stud; i++){
        scanf("%d %49s %d %d %d",&data[i].roll_no, data[i].name,
            &data[i].mark1, &data[i].mark2, &data[i].mark3);
    }

    for(int i = 0; i < no_of_stud; i++){    
        printf("Roll: %d\n", data[i].roll_no);

        printf("Name: %s\n", data[i].name);

        int total_marks = calculateTotal(data[i].mark1, data[i].mark2, data[i].mark3);
        printf("Total: %d\n", total_marks);

        float avg = calculateAverage(total_marks);
        printf("Average: %.2f\n", avg);

        char grade = calculateGrade(avg);
        printf("Grade: %c\n", grade);

        int stars = 0;
        if (avg < 35){
            continue;
        }


        switch(grade){
            case 'A':
            stars = 5;
            break;

            case 'B':
            stars = 4;
            break;

            case 'C':
            stars = 3;
            break;

            case 'D':
            stars = 2;
            break;
        }

        printf("Performance: ");
        for(int j=1; j<=stars; j++){
            printf("*");
        }

        printf("\n\n");
    }

    printf("List of Roll Numbers (via recursion): ");
    printRollNumbers(data , 0, no_of_stud);
}