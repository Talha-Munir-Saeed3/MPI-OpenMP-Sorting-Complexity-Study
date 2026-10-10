#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// Function to heapify a subtree rooted at index i
void Heapify(double arr[], int n, int i) {
    int largest = i; // Initialize largest as root
    int left = 2 * i + 1; // left = 2*i + 1
    int right = 2 * i + 2; // right = 2*i + 2

    // If left child is larger than root
    if (left < n && arr[left] > arr[largest])
        largest = left;

    // If right child is larger than largest so far
    if (right < n && arr[right] > arr[largest])
        largest = right;

    // If largest is not root
    if (largest != i) {
        double temp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = temp;

        // Recursively heapify the affected sub-tree
        Heapify(arr, n, largest);
    }
}

// Function to perform heap sort
void HeapSort(double arr[], int n) {
    // Build a maxheap.
    for (int i = n / 2 - 1; i >= 0; i--)
        Heapify(arr, n, i);

    // One by one extract elements
    for (int i = n - 1; i > 0; i--) {
        // Move current root to end
        double temp = arr[0];
        arr[0] = arr[i];
        arr[i] = temp;

        // Call heapify on the reduced heap
        Heapify(arr, i, 0);
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
        HeapSort(local_array, local_size);
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

