#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

#define RUN 32 // Minimum size of a run for Timsort

// Parallelized Insertion Sort for small runs
void InsertionSort(double arr[], int left, int right) {
    for (int i = left + 1; i <= right; i++) {
        double temp = arr[i];
        int j = i - 1;

        // Inner loop for insertion sort
        while (j >= left && arr[j] > temp) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = temp;
    }
}

// Merge function to combine two sorted subarrays
void Merge(double arr[], int left, int mid, int right) {
    int len1 = mid - left + 1, len2 = right - mid;
    double *leftArr = (double *)malloc(len1 * sizeof(double));
    double *rightArr = (double *)malloc(len2 * sizeof(double));

    for (int i = 0; i < len1; i++) leftArr[i] = arr[left + i];
    for (int i = 0; i < len2; i++) rightArr[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = left;
    while (i < len1 && j < len2) {
        if (leftArr[i] <= rightArr[j])
            arr[k++] = leftArr[i++];
        else
            arr[k++] = rightArr[j++];
    }

    while (i < len1) arr[k++] = leftArr[i++];
    while (j < len2) arr[k++] = rightArr[j++];

    free(leftArr);
    free(rightArr);
}

// Timsort algorithm with MPI parallelization
void Timsort(double arr[], int n, int rank, int size) {
    int local_size = n / size;
    double *local_array = (double *)malloc(local_size * sizeof(double));

    // Scatter the data to all processes
    MPI_Scatter(arr, local_size, MPI_DOUBLE, local_array, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Sort individual subarrays of size RUN
    for (int i = 0; i < n; i += RUN) {
        InsertionSort(local_array, i, (i + RUN - 1 < local_size - 1) ? (i + RUN - 1) : (local_size - 1));
    }

    // Gather the sorted subarrays back to the root process
    MPI_Gather(local_array, local_size, MPI_DOUBLE, arr, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Merge the subarrays in parallel at the root process
    if (rank == 0) {
        for (int size = RUN; size < n; size = 2 * size) {
            for (int left = 0; left < n; left += 2 * size) {
                int mid = left + size - 1;
                int right = ((left + 2 * size - 1) < (n - 1)) ? (left + 2 * size - 1) : (n - 1);

                if (mid < right) {
                    Merge(arr, left, mid, right);
                }
            }
        }
    }

    free(local_array);
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

        // Timer for sorting
        double sort_start = MPI_Wtime();

        // Call Timsort to sort the array in parallel
        Timsort(array, array_size, rank, size);

        // Sorting end time
        double sort_end = MPI_Wtime();
        if (rank == 0) {
            printf("Test case %d: Sorting time: %f seconds\n", t + 1, sort_end - sort_start);
        }

        // Timer for total execution time of the test case
        double total_end = MPI_Wtime();
        if (rank == 0) {
            printf("Test case %d: Total execution time: %f seconds\n", t + 1, total_end - total_start);
            free(array);
        }
    }

    MPI_Finalize();
    return 0;
}

