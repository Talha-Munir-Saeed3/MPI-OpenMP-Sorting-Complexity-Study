#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define RUN 32 // Minimum size of a run for Timsort

// Parallelized Insertion Sort for small runs
void InsertionSort(double arr[], int left, int right) {
    #pragma omp parallel for schedule(static)
    for (int i = left + 1; i <= right; i++) {
        double temp = arr[i];
        int j = i - 1;
        
        // Inner loop for insertion sort, executed serially inside the parallelized outer loop
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

// Timsort algorithm with OpenMP parallelization
void Timsort(double arr[], int n) {
    // Sort individual subarrays of size RUN in parallel
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i += RUN) {
        InsertionSort(arr, i, (i + RUN - 1 < n - 1) ? (i + RUN - 1) : (n - 1));
    }

    // Merge runs to form sorted subarrays
    for (int size = RUN; size < n; size = 2 * size) {
        #pragma omp parallel for schedule(static)
        for (int left = 0; left < n; left += 2 * size) {
            int mid = left + size - 1;
            int right = ((left + 2 * size - 1) < (n - 1)) ? (left + 2 * size - 1) : (n - 1);

            if (mid < right) {
                Merge(arr, left, mid, right);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    int num_threads = omp_get_max_threads();

    if (argc > 1) {
        num_threads = atoi(argv[1]);
        if (num_threads <= 0) {
            printf("Error: Number of threads must be greater than zero.\n");
            return 1;
        }
    }

    omp_set_num_threads(num_threads);

    // Test cases for array sizes
    long int test_cases[] = {5000000, 10000000, 20000000, 50000000, 100000000};
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int t = 0; t < num_cases; t++) {
        int array_size = test_cases[t];
        double *array = (double *)malloc(array_size * sizeof(double));

        // Timer for total execution time of the test case
        double total_start = omp_get_wtime();

        // Generate random numbers
        double rand_start = omp_get_wtime();
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < array_size; i++) {
            array[i] = rand() % 100000;
        }
        double rand_end = omp_get_wtime();
        printf("Test case %d: Random number generation time: %f seconds\n", t + 1, rand_end - rand_start);

        // Sort the array
        double sort_start = omp_get_wtime();
        Timsort(array, array_size);
        double sort_end = omp_get_wtime();
        printf("Test case %d: Sorting time: %f seconds\n", t + 1, sort_end - sort_start);

        // Timer for total execution time of the test case
        double total_end = omp_get_wtime();
        printf("Test case %d: Total execution time: %f seconds\n", t + 1, total_end - total_start);

        free(array);
    }

    return 0;
}

