#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 10

void swap(int *p, int *q)
{
    int temp;
    temp = *p;
    *p = *q;
    *q = temp;
}

void generateMatrix(int matrix[MAX][MAX], int n)
{
    int row, col;
    for (row = 0; row < n; row++)
    {
        for (col = 0; col < n; col++)
        {
            *(*(matrix + row) + col) = rand() % 256;
        }
    }
}

void displayMatrix(int matrix[MAX][MAX], int n)
{
    int row, col;
    for (row = 0; row < n; row++)
    {
        for (col = 0; col < n; col++)
        {
            printf("%4d", *(*(matrix + row) + col));
        }

        printf("\n");
    }
}

void transpose(int matrix[MAX][MAX], int n)
{
    int row, col;
    for (row = 0; row < n; row++)
    {
        for (col = row + 1; col < n; col++)
        {
            swap(&(*(*(matrix + row) + col)), &(*(*(matrix + col) + row)));
        }
    }
}

void reverseRows(int matrix[MAX][MAX], int n)
{
    int row, left, right;
    for (row = 0; row < n; row++)
    {
        left = 0;
        right = n - 1;
        while (left < right)
        {
            swap(&(*(*(matrix + row) + left)), &(*(*(matrix + row) + right)));
            left++;
            right--;
        }
    }
}

void rotateMatrix(int matrix[MAX][MAX], int n)
{
    transpose(matrix, n);
    reverseRows(matrix, n);
}

void smoothMatrix(int matrix[MAX][MAX], int n)
{
    int row, col;
    int rowOffset, colOffset;
    int neighborRow, neighborCol;
    int sum, count, average;
    int rowA[MAX], rowB[MAX], rowC[MAX];
    int *prev, *curr, *next, *tempRow;
    int *sourceRow;

    prev = rowA;
    curr = rowB;
    next = rowC;

    for (col = 0; col < n; col++)
    {
        *(curr + col) = *(*(matrix + 0) + col);
    }

    for (col = 0; col < n; col++)
    {
        *(next + col) = *(*(matrix + 1) + col);
    }

    for (row = 0; row < n; row++)
    {
        for (col = 0; col < n; col++)
        {
            sum = 0;
            count = 0;
            for (rowOffset = -1; rowOffset <= 1; rowOffset++)
            {
                for (colOffset = -1; colOffset <= 1; colOffset++)
                {
                    neighborRow = row + rowOffset;
                    neighborCol = col + colOffset;
                    if (neighborRow >= 0 && neighborRow < n && neighborCol >= 0 && neighborCol < n)
                    {
                        if (rowOffset == -1)
                        {
                            sourceRow = prev;
                        }
                        else if (rowOffset == 0)
                        {
                            sourceRow = curr;
                        }
                        else
                        {
                            sourceRow = next;
                        }
                        sum += *(sourceRow + neighborCol);
                        count++;
                    }
                }
            }
            average = sum / count;
            *(*(matrix + row) + col) = average;
        }
        if (row < n - 1)
        {
            tempRow = prev;
            prev = curr;
            curr = next;
            next = tempRow;
            if (row + 2 < n)
            {
                for (col = 0; col < n; col++)
                {
                    *(next + col) =
                        *(*(matrix + row + 2) + col);
                }
            }
        }
    }
}

int main()
{
    int matrix[MAX][MAX];
    int n;

    srand((unsigned int)time(NULL));

    printf("Enter matrix size (2 to 10): ");

    if (scanf("%d", &n) != 1 || n < 2 || n > MAX)
    {
        printf("Invalid matrix size.\n");
        return 1;
    }

    generateMatrix(matrix, n);

    printf("\nOriginal Matrix:\n");
    displayMatrix(matrix, n);

    rotateMatrix(matrix, n);

    printf("\nMatrix after 90-degree clockwise rotation:\n");
    displayMatrix(matrix, n);

    smoothMatrix(matrix, n);

    printf("\nMatrix after 3x3 smoothing:\n");
    displayMatrix(matrix, n);

    return 0;
}
