#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "array_io.h"
#include "array_op.h"
#include "timer.h"
#include "matrix_utils.h"


int main(int argc, char *argv[])
{
    int matrix_size, block_size;
    int i;
    int return_code = 0;

    double *matrix = NULL;
    double *diagonal = NULL;
    double *vector_answer = NULL;
    double *vector = NULL;
    double *exact_rhs = NULL;
    double *rhs = NULL;
    double *workspace = NULL;

    double residual, rhs_norm, answer_error;

    timer_start();

    /* input: ./cholesky_solver n m [file] */
    if (argc == 3 || argc == 4)
    {
        matrix_size = atoi(argv[1]);
        block_size = atoi(argv[2]);

        if (matrix_size <= 0 || block_size <= 0 || block_size > matrix_size)
        {
            printf("Wrong input\n");
            return -1;
        }

        /* Individual allocations for better safety and readability */
        matrix = (double*)malloc(get_symmetric_matrix_size(matrix_size) * sizeof(double));
        diagonal = (double*)malloc(matrix_size * sizeof(double));
        vector_answer = (double*)malloc(matrix_size * sizeof(double));
        vector = (double*)malloc(matrix_size * sizeof(double));
        exact_rhs = (double*)malloc(matrix_size * sizeof(double));
        rhs = (double*)malloc(matrix_size * sizeof(double));
        workspace = (double*)malloc(3 * (size_t)block_size * block_size * sizeof(double));

        if (!matrix || !diagonal || !vector_answer || !vector || !exact_rhs || !rhs || !workspace)
        {
            printf("Not enough memory\n");
            return_code = -2;
            goto cleanup;
        }

        memset(matrix, 0, get_symmetric_matrix_size(matrix_size) * sizeof(double));
        memset(diagonal, 0, matrix_size * sizeof(double));
        memset(vector_answer, 0, matrix_size * sizeof(double));
        memset(vector, 0, matrix_size * sizeof(double));
        memset(exact_rhs, 0, matrix_size * sizeof(double));
        memset(rhs, 0, matrix_size * sizeof(double));
        memset(workspace, 0, 3 * (size_t)block_size * block_size * sizeof(double));

        fill_vector_answer(matrix_size, vector_answer);

        if (argc == 3)
        {
            if (fill_matrix(matrix_size, matrix, vector_answer, rhs))
            {
                printf("Cannot fill matrix\n");
                return_code = -3;
                goto cleanup;
            }
        }

        if (argc == 4)
        {
            if (read_matrix(matrix_size, &matrix, vector_answer, rhs, argv[3])) 
            {
                printf("Cannot read matrix\n");
                return_code = -4;
                goto cleanup;
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
        return_code = -10;
        goto cleanup;
    }

    print_time("on cholesky decomposition");
    
    if (solve_lower_triangle_matrix_system(matrix_size, matrix, vector, workspace, block_size))
    {
        printf("Cannot solve R^T y = b part\n");
        return_code = -11;
        goto cleanup;
    }

    if (solve_upper_triangle_matrix_diagonal_system(matrix_size, matrix, diagonal, vector, workspace, block_size))
    {
        printf("Cannot solve D R x = y part\n");
        return_code = -12;
        goto cleanup;
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
            return_code = -3;
            goto cleanup;
        }
    }

    if (argc == 4)
    {
        if (read_matrix(matrix_size, &matrix, vector, rhs, argv[3])) 
        {
            printf("Cannot read matrix\n");
            return_code = -4;
            goto cleanup;
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

cleanup:
    if (matrix) free(matrix);
    if (diagonal) free(diagonal);
    if (vector_answer) free(vector_answer);
    if (vector) free(vector);
    if (exact_rhs) free(exact_rhs);
    if (rhs) free(rhs);
    if (workspace) free(workspace);

    return return_code;
}
