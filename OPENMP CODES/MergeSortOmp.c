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

// Merge two subarrays
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

// Merge Sort function
void MergeSort(double arr[], int low, int high) {
    if (low >= high) return;

    int mid = low + (high - low) / 2;

    // Parallelize the recursive calls
    #pragma omp task shared(arr)
    MergeSort(arr, low, mid);

    #pragma omp task shared(arr)
    MergeSort(arr, mid + 1, high);

    #pragma omp taskwait
    Merge(arr, low, mid, high);
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

    for (int t = 0; t < num_cases; t++) {
        int array_size = test_cases[t];
        double *array = NULL;

        // Timer for total execution time of the test case
        double total_start = omp_get_wtime();

        // Timer for random number generation
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

        // Start MergeSort with OpenMP
        #pragma omp parallel
        {
            #pragma omp single
            MergeSort(array, 0, array_size - 1);
        }

        double sort_end = omp_get_wtime();

        // Output sorting time for each test case
        printf("Test case %d: Sorting time: %f seconds\n", t + 1, sort_end - sort_start);

        // Timer for total execution time of the test case
        double total_end = omp_get_wtime();
        printf("Test case %d: Total execution time: %f seconds\n", t + 1, total_end - total_start);
      /*  for(int i=0;i<100;i++)
        {
            printf("%f",array[i+624950]);
        }*/
        
        //Testing case at break point for 8 threads 8 threads 625000 per thread
        //previous thread and next thread elements to see if our code sorts them

        free(array);
    }

    return 0;
}

