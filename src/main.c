#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "array_io.h"
#include "array_op.h"
#include "timer.h"


int main(int argc, char *argv[])
{
    int matrix_size, block_size;
    int i;
    int len;

    double *matrix;
    double *diagonal;
    double *vector_answer;
    double *vector;
    double *exact_rhs;
    double *rhs;
    double *workspace;

    double residual, rhs_norm, answer_error;

    timer_start();

    /* input: ./a.out n m [file] */
    if (argc == 3 || argc == 4)
    {
        matrix_size = atoi(argv[1]);
        block_size = atoi(argv[2]);

        if (matrix_size == 0 || block_size == 0 || block_size > matrix_size)
        {
            printf("Wrong input\n");
            return -1;
        }

        len = 0;
        len = 3*block_size*block_size + ((matrix_size*(matrix_size+1))/2) + 5*matrix_size;
        len *= sizeof(double);

        if (!(matrix = (double*)malloc(len))) // note to delete "2 * " !!!
        {
            printf("Not enough memory\n");
            return -2;
        }

        memset(matrix, 0, len);

        diagonal = matrix + ((matrix_size*(matrix_size+1))/2); // note to delete 2x !!!
        vector_answer = diagonal + matrix_size;
        vector = vector_answer + matrix_size;
        exact_rhs = vector + matrix_size;
        rhs = exact_rhs + matrix_size;
        workspace = rhs + matrix_size;

        fill_vector_answer(matrix_size, vector_answer);

        if (argc == 3)
        {
            if (fill_matrix(matrix_size, matrix, vector_answer, rhs))
            {
                printf("Cannot fill matrix\n");
                return -3;
            }
        }

        if (argc == 4)
        {
            if (read_matrix(matrix_size, &matrix, vector_answer, rhs, argv[3])) 
            {
                printf("Cannot read matrix\n");
                return -4;
            }
        }

        for (i = 0; i < matrix_size; i++)
        {
            exact_rhs[i] = rhs[i];
            vector[i] = rhs[i];
        }
    }
    else
    {
        printf("Wrong input, try again\n");
        return 0;
    }

    print_time("on initialization");

    /*
    if (matrix_size < 10)
    {
        printf("matrix:\n");
        printf_matrix(matrix_size, matrix);

        printf("\n");
        int i;
        for (i = 0; i < matrix_size; ++i)
            printf("%.3f ", c[i]);
        printf("\n\n");
    }
    */
    if (matrix_size < 15)
    {
        printf("matrix A:\n");
        printf_matrix(matrix_size, matrix);

        printf("\nrhs:\n");
        for (i = 0; i < matrix_size; ++i)
            printf("%.10f ", rhs[i]);
        printf("\n\n");
    }

    if (cholesky(matrix_size, matrix, diagonal, workspace, block_size))
    {
        printf("Cannot apply Cholesky method for this matrix\n");
        free(matrix);

        return -10;
    }

    print_time("on cholesky decomposition");
    
    if (solve_lower_triangle_matrix_system(matrix_size, matrix, vector, workspace, block_size))
    {
        printf("Cannot solve R^T y = b part\n");
        free(matrix);

        return -11;
    }

    if (solve_upper_triangle_matrix_diagonal_system(matrix_size, matrix, diagonal, vector, workspace, block_size))
    {
        printf("Cannot solve D R x = y part\n");
        free(matrix);

        return -12;
    }
        
    if (matrix_size < 15)
    {
        printf("cholesky decomposition:\n");
        printf_matrix(matrix_size, matrix);

        printf("\ndiagonal:\n");
        for (i = 0; i < matrix_size; i++)
            printf("%.1f ", diagonal[i]);    
        printf("\n\n");
    }

    print_time("on algorithm");

    residual = 0;
    rhs_norm = 0;
    answer_error = 0;

    if (argc == 3)
    {
        if (fill_matrix(matrix_size, matrix, vector, rhs))
        {
            printf("Cannot fill matrix\n");
            return -3;
        }
    }

    if (argc == 4)
    {
        if (read_matrix(matrix_size, &matrix, vector, rhs, argv[3])) 
        {
            printf("Cannot read matrix\n");
            return -4;
        }
    }


    for (i = 0; i < matrix_size; ++i)
    {
        residual += (exact_rhs[i] - rhs[i]) * (exact_rhs[i] - rhs[i]);
        rhs_norm += exact_rhs[i] * exact_rhs[i];
        answer_error += (vector_answer[i] - vector[i]) * (vector_answer[i] - vector[i]);
    }

    residual = sqrt(residual);
    rhs_norm = sqrt(rhs_norm);
    answer_error = sqrt(answer_error);


    printf("Answer:\n");
    for (i = 0; i < matrix_size && i < 5; ++i)
        printf("%.10lf ", vector[i]);
    printf("\n\n");

    printf("Error: %11.5le ; Residual: %11.5le (%11.5le)\n", answer_error, residual, residual / rhs_norm);

    free(matrix);

    return 0;
}


