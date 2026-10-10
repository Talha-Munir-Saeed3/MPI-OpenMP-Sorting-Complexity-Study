# Optimizing Classical Sorting with OpenMP & MPI

A performance study of five **O(n log n)** sorting algorithms (Heap, Tim, Intro, Merge, Quick), each implemented in **OpenMP** (shared-memory threads) and **MPI** (distributed processes), and benchmarked on arrays of up to **100 million** elements.

Built for the **Parallel & Distributed Computing (PDC)** course.

![C](https://img.shields.io/badge/C-00599C?logo=c&logoColor=white)
![OpenMP](https://img.shields.io/badge/OpenMP-parallel-blue)
![MPI](https://img.shields.io/badge/MPI-distributed-orange)
![License: MIT](https://img.shields.io/badge/License-MIT-green)

---

## Table of Contents

- [Overview](#overview)
- [Algorithms](#algorithms)
- [Experiment Setup](#experiment-setup)
- [Repository Structure](#repository-structure)
- [Getting Started](#getting-started)
- [Metrics](#metrics)
- [Results](#results)
- [Observations](#observations)
- [Limitations](#limitations)
- [Documentation](#documentation)
- [License](#license)

---

## Overview

Sorting large datasets is expensive, and classical O(n log n) algorithms do not automatically get faster just because you add cores. This project answers a practical question:

> **How do different sorting algorithms scale when we increase the number of threads (OpenMP) or processes (MPI)?**

For every algorithm and every dataset size we measure the serial time, parallel time, overhead, speedup and efficiency, then compare how each one scales.

## Algorithms

| Algorithm | Best | Average | Worst |
| --- | --- | --- | --- |
| Merge Sort | O(n log n) | O(n log n) | O(n log n) |
| Quick Sort | O(n log n) | O(n log n) | O(n²) |
| Heap Sort | O(n log n) | O(n log n) | O(n log n) |
| Intro Sort | O(n log n) | O(n log n) | O(n log n) |
| Tim Sort | O(n) | O(n log n) | O(n log n) |

O(n²) algorithms were deliberately excluded because they are impractical at 100M elements. The average case is assumed for all algorithms so the comparison stays fair.

## Experiment Setup

| Parameter | Value |
| --- | --- |
| Data type | `double` (random values from `rand() % 100000`) |
| Dataset sizes | 5M, 10M, 20M, 50M, 100M elements |
| OpenMP threads | 1, 2, 4, 8 (16 also tested, see `OUTPUTS/`) |
| MPI processes | 1, 2, 4 |
| Environment | Ubuntu (VirtualBox VM) |
| Hardware limits | up to 8 threads, 4 processes |

Each run reports three timings per test case: **random number generation time**, **sorting time** and **total execution time**.

## Repository Structure

```
.
├── MPI CODES/                     # MPI implementation of each algorithm
├── OPENMP CODES/                  # OpenMP implementation of each algorithm
├── OUTPUTS/                       # Terminal screenshots of every run (threads / processes)
├── PROJECT PROPOSAL/              # Initial proposal
├── PROJECT REPORT AND DESCRIPTION/# Full report
├── RESULTS AND GRAPHS/            # Result tables and performance graphs
├── LICENSE                        # MIT
└── README.md
```

## Getting Started

### Prerequisites

- GCC with OpenMP support
- An MPI implementation (OpenMPI or MPICH)

```bash
sudo apt update
sudo apt install build-essential openmpi-bin libopenmpi-dev
```

### Run an OpenMP version

The number of threads is passed as a command-line argument.

```bash
gcc -fopenmp -O2 IntroSortOmp.c -o intro_omp -lm
./intro_omp 4        # run with 4 threads
```

### Run an MPI version

The number of processes is set with `-np`.

```bash
mpicc -O2 MergeSort.c -o merge_mpi
mpirun -np 4 ./merge_mpi
```

> Replace the file names with the matching source file from `OPENMP CODES/` or `MPI CODES/`. Using `-O2` gives realistic timings; the large sizes (50M-100M) need a few GB of free RAM.

### Sample output

```
Test case 1: Random number generation time: 0.286605 seconds
Test case 1: Sorting time: 11.249498 seconds
Test case 1: Total execution time: 11.536252 seconds
...
```

## Metrics

| Metric | Formula |
| --- | --- |
| Speedup | `S = T_serial / T_parallel` |
| Efficiency | `E = S / p` (p = threads or processes) |
| Overhead | `T_overhead = p * T_parallel - T_serial` |

## Results

Results at the largest size (**100M elements**). Speedup is measured against each variant's own 1-thread / 1-process run. Full tables for every size are in the [results spreadsheet](<RESULTS AND GRAPHS/Project.xlsx>).

| Algorithm | OpenMP speedup (threads) | MPI speedup (4 processes) |
| --- | --- | --- |
| **Heap Sort** | **12.74x** (8 threads) | 3.99x |
| **Quick Sort** | 3.28x (4 threads) | **11.86x** |
| **Tim Sort** | 2.02x (8 threads) | 0.91x |
| **Intro Sort** | 0.93x (no gain) | 2.03x |
| **Merge Sort** | 0.51x (no gain) | 3.38x |

### Example: Heap Sort (OpenMP), 100M elements

| Threads | Time (s) | Speedup | Efficiency |
| --- | --- | --- | --- |
| 1 | 627.04 | 1.00 | 1.00 |
| 2 | 123.84 | 5.06 | 2.53 |
| 4 | 51.73 | 12.12 | 3.03 |
| 8 | 49.21 | 12.74 | 1.59 |

📊 **Full results for every algorithm, size, thread and process count:** [`Project.xlsx`](<RESULTS AND GRAPHS/Project.xlsx>)

## Observations

- **Scaling depends heavily on the algorithm.** Heap, Quick and Merge sort (in MPI) benefit the most from splitting the data, while Tim and Intro sort gain little.
- **Diminishing returns past the physical core count.** Going from 4 to 8 threads barely helped (e.g. Heap Sort 12.12x to 12.74x, Quick Sort 3.28x to 3.25x), and efficiency dropped sharply, as expected on a 4-core machine.
- **Synchronisation overhead can outweigh the gain.** The OpenMP Intro Sort and Merge Sort versions ran slower than their single-thread runs because of thread creation and coordination cost in the recursive calls.
- **MPI scales cleanly for chunk-based algorithms.** Merge Sort and Heap Sort over MPI show steady, predictable speedups as processes increase.
- **Larger inputs scale better.** Speedup and efficiency improve with array size because the fixed parallel overhead is amortised over more work.

## Limitations

- Speedups above `p` (efficiency > 1) for some Heap Sort and Quick Sort runs are **super-linear**. They come from the parallel versions solving smaller sub-problems (n/p each) rather than from the hardware, so they should be read as "faster than this serial baseline", not as ideal scaling.
- The MPI versions scatter the array, sort each chunk and gather the result; a final merge of the sorted chunks is not part of the timed pipeline.
- Experiments ran inside a VirtualBox VM, so absolute times vary with host load. Compare ratios, not raw seconds.
- Each configuration was run once per input size.

## Documentation

- Full write-up: `PROJECT REPORT AND DESCRIPTION/`
- Proposal: `PROJECT PROPOSAL/`
- Raw run screenshots: `OUTPUTS/`
- Tables and graphs: [`RESULTS AND GRAPHS/Project.xlsx`](<RESULTS AND GRAPHS/Project.xlsx>)

## Author

**Talha Munir Saeed** ([@Talha-Munir-Saeed3](https://github.com/Talha-Munir-Saeed3))

Parallelisation of the algorithms was done with some assistance from ChatGPT.

## License

Released under the [MIT License](LICENSE).
