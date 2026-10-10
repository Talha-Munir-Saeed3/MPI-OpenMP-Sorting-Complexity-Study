#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define MAX_DEPTH 32 // Maximum recursion depth for introsort
#define INSERTION_SORT_THRESHOLD 16 // Threshold for switching to insertion sort
int MedianOfThree(double arr[], int low, int high) {
    int mid = low + (high - low) / 2;

    // Order the elements: arr[low], arr[mid], arr[high]
    if (arr[low] > arr[mid])
        swap(&arr[low], &arr[mid]);
    if (arr[low] > arr[high])
        swap(&arr[low], &arr[high]);
    if (arr[mid] > arr[high])
        swap(&arr[mid], &arr[high]);

    // Use the median as the pivot (arr[mid])
    swap(&arr[mid], &arr[high]); // Place pivot at the end
    return arr[high];
}
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

// Partition function for Quicksort
int Partition(double arr[], int low, int high) {
    double pivot =  MedianOfThree(arr, low, high);
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            double temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    double temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;

    return i + 1;
}

// Heapify function for Heap Sort
void Heapify(double arr[], int n, int i) {
    int largest = i; // Initialize largest as root
    int left = 2 * i + 1; // Left child
    int right = 2 * i + 2; // Right child

    if (left < n && arr[left] > arr[largest])
        largest = left;

    if (right < n && arr[right] > arr[largest])
        largest = right;

    if (largest != i) {
        double temp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = temp;

        Heapify(arr, n, largest);
    }
}

// Heap Sort function
void HeapSort(double arr[], int n) {
    // Build a max heap
    for (int i = n / 2 - 1; i >= 0; i--)
        Heapify(arr, n, i);

    // Extract elements from the heap
    for (int i = n - 1; i > 0; i--) {
        double temp = arr[0];
        arr[0] = arr[i];
        arr[i] = temp;

        // Heapify the reduced heap
        Heapify(arr, i, 0);
    }
}

// Parallelized Quicksort function with Introsort depth control
void IntrosortHelper(double arr[], int low, int high, int depth_limit) {
    if (high - low < INSERTION_SORT_THRESHOLD) {
        InsertionSort(arr, low, high);
    } else if (depth_limit == 0) {
        // Switch to Heap Sort when recursion depth is exhausted
        HeapSort(arr + low, high - low + 1);
    } else {
        int pivot_index = Partition(arr, low, high);

        // Parallelize the recursive calls
        #pragma omp parallel sections
        {
            #pragma omp section
            IntrosortHelper(arr, low, pivot_index - 1, depth_limit - 1);

            #pragma omp section
            IntrosortHelper(arr, pivot_index + 1, high, depth_limit - 1);
        }
    }
}

// Main Introsort function with depth control
void Introsort(double arr[], int n) {
    int depth_limit = (int)(2 * log(n)); // Maximum depth for introsort
    IntrosortHelper(arr, 0, n - 1, depth_limit);
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
        Introsort(array, array_size);
        double sort_end = omp_get_wtime();
        printf("Test case %d: Sorting time: %f seconds\n", t + 1, sort_end - sort_start);

        // Timer for total execution time of the test case
        double total_end = omp_get_wtime();
        printf("Test case %d: Total execution time: %f seconds\n", t + 1, total_end - total_start);

        free(array);
    }

    return 0;
}
