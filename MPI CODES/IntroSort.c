#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

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
// Insertion Sort for small runs
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

    // Extract elements from heap
    for (int i = n - 1; i > 0; i--) {
        double temp = arr[0];
        arr[0] = arr[i];
        arr[i] = temp;

        // Heapify the reduced heap
        Heapify(arr, i, 0);
    }
}

// Partition function for Quicksort
int Partition(double arr[], int low, int high) {
    double pivot =  MedianOfThree(arr, low, high);
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

// Introsort Helper Function with Depth Control
void IntrosortHelper(double arr[], int low, int high, int depth_limit) {
    if (high - low < INSERTION_SORT_THRESHOLD) {
        InsertionSort(arr, low, high);
    } else if (depth_limit == 0) {
        // Switch to Heap Sort when recursion depth is exhausted
        HeapSort(arr + low, high - low + 1);
    } else {
        int pi = Partition(arr, low, high);

        // Recursively sort elements before and after partition
        IntrosortHelper(arr, low, pi - 1, depth_limit - 1);
        IntrosortHelper(arr, pi + 1, high, depth_limit - 1);
    }
}

// Main Introsort function with depth control
void Introsort(double arr[], int n) {
    int depth_limit = (int)(2 * log(n)); // Maximum depth for introsort
    IntrosortHelper(arr, 0, n - 1, depth_limit);
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
        Introsort(local_array, local_size);
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
