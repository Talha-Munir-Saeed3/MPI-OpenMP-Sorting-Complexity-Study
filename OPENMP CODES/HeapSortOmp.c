#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

// Define default number of threads (can be overwritten at runtime)
#define DEFAULT_THREADS 4

// Check if the number of threads is provided and valid
#ifndef NUM_THREADS
    #define NUM_THREADS DEFAULT_THREADS
#endif

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
    // Parallelize the heapify process
    #pragma omp parallel for schedule(static)
    for (int i = n / 2 - 1; i >= 0; i--) {
        Heapify(arr, n, i);
    }

    // Parallelize the sorting process
    #pragma omp parallel for schedule(dynamic)
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
    int num_threads;

    // Check if the user provided the number of threads as a command-line argument
    if (argc < 2) {
        printf("Using default number of threads: %d\n", NUM_THREADS);
        num_threads = NUM_THREADS; // Use the default value if not provided
    } else {
        num_threads = atoi(argv[1]);
    }

    // Check if the number of threads is positive (non-zero and non-negative)
    if (num_threads <= 0) {
        printf("Error: The number of threads must be a positive integer greater than zero.\n");
        return 1;
    }

    // Set the number of threads for OpenMP
    omp_set_num_threads(num_threads);

    // Test cases for array sizes
    long int test_cases[] = {5000000, 10000000, 20000000, 50000000, 100000000};
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    // Timer for execution time
    double total_start, total_end;
    total_start = omp_get_wtime();

    // Iterate over each test case
    for (int t = 0; t < num_cases; t++) {
        int array_size = test_cases[t];
        double *array = NULL;

        // Timer for total execution time of the test case
        double rand_start, rand_end;

        array = (double *)malloc(array_size * sizeof(double));
        rand_start = omp_get_wtime();

        // Generate random numbers for the test case
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < array_size; i++) {
            array[i] = rand() % 100000;
        }

        rand_end = omp_get_wtime();
        printf("Test case %d: Random number generation time: %f seconds\n", t + 1, rand_end - rand_start);

        // Timer for sorting
        double sort_start = omp_get_wtime();
        HeapSort(array, array_size);
        double sort_end = omp_get_wtime();

        // Output sorting time for each test case
        printf("Test case %d: Sorting time: %f seconds\n", t + 1, sort_end - sort_start);

        // Output the total execution time
        double total_end = omp_get_wtime();
        printf("Test case %d: Total execution time: %f seconds\n", t + 1, total_end - total_start);

        free(array);
    }

    // Get thread-specific information if OpenMP is enabled
    #ifdef _OPENMP
        #pragma omp parallel
        {
            int my_rank = omp_get_thread_num(); // Get the thread rank
            int num_threads_in_parallel = omp_get_num_threads(); // Get total number of threads
            printf("Thread %d of %d\n", my_rank, num_threads_in_parallel);
        }
    #else
        printf("OpenMP is not enabled.\n");
    #endif

    return 0;
}

