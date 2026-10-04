#include <stdio.h>
#include <ctype.h>

char expression[100];
int position = 0, error = 0;

int expressionValue();
int termValue();
int factorValue();
int validateExpression(char expression[]);

int factorValue(){
    int number = 0;
    
    while(expression[position] == ' '){
        position++;
    }

    if(isdigit(expression[position])){
        while(isdigit(expression[position])){
            number = number * 10 + (expression[position] - '0');
            position++;
        }
        return number;
    }

    printf("Error: Invalid character '%c'.\n", expression[position]);
    return 0;
}

int termValue(){
    int result = factorValue();

    while(1){
        while(expression[position] == ' '){
            position++;
        }

        if(expression[position] == '*'){
            position++;
            result = result * factorValue();
        }
        else if(expression[position] == '/'){
            position++;
            int number = factorValue();

            if(number == 0){
                printf("Error: Division by zero.\n");
                error = 1;
                return 0;
            }

            result = result / number;
        }
        else{
            break;
        }
    }

    return result;
}

int expressionValue(){
    int result = termValue();

    while(1){
        while(expression[position] == ' '){
            position++;
        }

        if(expression[position] == '+'){
            position++;
            result = result + termValue();
        }
        else if(expression[position] == '-'){
            position++;
            result = result - termValue();
        }
        else{
            break;
        }
    }

    return result;
}

int validateExpression(char expression[]){
    int expectingNumber = 1;
    int lastWasDigit = 0;
    int spaceAfterNumber = 0;

    for(int i = 0; expression[i] != '\0'; i++){
        if(expression[i] == ' '){
            if(lastWasDigit){
                spaceAfterNumber = 1;
            }
            continue;
        }

        if(expression[i] == '\n'){
            continue;
        }

        if(expression[i] >= '0' && expression[i] <= '9'){
            if(spaceAfterNumber){
                return 0;
            }

            if(expectingNumber == 0 && lastWasDigit == 0){
                return 0;
            }

            expectingNumber = 0;
            lastWasDigit = 1;
            spaceAfterNumber = 0;
        }
        else if(expression[i] == '+' ||
                expression[i] == '-' ||
                expression[i] == '*' ||
                expression[i] == '/'){
            if(expectingNumber == 1){
                return 0;
            }

            expectingNumber = 1;
            lastWasDigit = 0;
            spaceAfterNumber = 0;
        }
        else{
            return 0;
        }
    }

    if(expectingNumber == 1){
        return 0;
    }

    return 1;
}

int main(){
    printf("Enter expression: ");
    fgets(expression, sizeof(expression), stdin);
    if(!validateExpression(expression)){
        printf("Error: Invalid expression.\n");
        return 1;
    }

    position = 0;
    int result = expressionValue();

    if(error == 1){
        return 1;
    }
    while(expression[position] == ' '){
        position++;
    }
    if(expression[position] != '\0' && expression[position] != '\n'){
        printf("Error: Invalid expression.\n");
        return 1;
    }
    printf("%d\n", result);

    return 0;
}