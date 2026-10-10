#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// Function to partition the array for QuickSort
int Partition(double arr[], int low, int high) {
    double pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (arr[j] <= pivot) {
            i++;
            // Swap arr[i] and arr[j]
            double temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    // Swap arr[i + 1] and arr[high] (or pivot)
    double temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;

    return (i + 1);
}

// QuickSort function
void QuickSort(double arr[], int low, int high) {
    if (low < high) {
        int pi = Partition(arr, low, high);

        // Recursively sort elements before and after partition
        QuickSort(arr, low, pi - 1);
        QuickSort(arr, pi + 1, high);
    }
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
        QuickSort(local_array, 0, local_size - 1);
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

