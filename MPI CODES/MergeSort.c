
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

void Merge(double arr[], int low, int mid, int high) {
    int i = low, j = mid + 1, k = 0;
    double *temp = (double *)malloc((high - low + 1) * sizeof(double));

    while (i <= mid && j <= high) {
        if (arr[i] <= arr[j])
            temp[k++] = arr[i++];
        else
            temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= high) temp[k++] = arr[j++];

    for (i = low, k = 0; i <= high; i++, k++)
        arr[i] = temp[k];

    free(temp);
}

void MergeSort(double arr[], int low, int high) {
    if (low >= high) return;
    int mid = low + (high - low) / 2;

    MergeSort(arr, low, mid);
    MergeSort(arr, mid + 1, high);
    Merge(arr, low, mid, high);
}

int main(int argc, char* argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Test cases for array sizes
    long int test_cases[] = {5000000, 10000000, 20000000, 50000000, 100000000};
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int t = 0; t < num_cases; t++) {
        int array_size = test_cases[t];
        double *array = NULL;
        double *local_array = NULL;
        int local_size = array_size / size;

        // Timer for total execution time of the test case
        double total_start = MPI_Wtime();

        // Timer for random number generation
        double rand_start, rand_end;

        if (rank == 0) {
            array = (double *)malloc(array_size * sizeof(double));
            rand_start = MPI_Wtime();

            // Generate random numbers for the test case
            for (int i = 0; i < array_size; i++)
                array[i] = rand() % 100000;

            rand_end = MPI_Wtime();
            printf("Test case %d: Random number generation time: %f seconds\n", t + 1, rand_end - rand_start);
        }

        // Scatter the array to all processes
        local_array = (double *)malloc(local_size * sizeof(double));
        MPI_Scatter(array, local_size, MPI_DOUBLE, local_array, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // Timer for sorting
        double sort_start = MPI_Wtime();
        MergeSort(local_array, 0, local_size - 1);
        double sort_end = MPI_Wtime();

        // Gather the sorted subarrays
        MPI_Gather(local_array, local_size, MPI_DOUBLE, array, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // Output sorting time for each test case
        if (rank == 0) {
            printf("Test case %d: Sorting time: %f seconds\n", t + 1, sort_end - sort_start);
        }

        // Timer for total execution time of the test case
        double total_end = MPI_Wtime();
        if (rank == 0) {
            printf("Test case %d: Total execution time: %f seconds\n", t + 1, total_end - total_start);
        }

        free(local_array);
        if (rank == 0) free(array);
    }

    MPI_Finalize();
    return 0;
}

