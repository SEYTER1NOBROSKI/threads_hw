# Multithreaded Array Generator & Sorter

This project demonstrates a multi-threaded C application using the **POSIX Threads (pthreads)** library. It simulates a data processing pipeline where one thread generates random data and another thread sorts it, utilizing shared memory for efficiency.

## Project Overview

The program performs a fixed number of iterations (rounds). In each round:
1.  **Generator Thread**: Fills an array with random numbers (0-100) and writes them to a file.
2.  **Sorter Thread**: Sorts the *same* array in memory (using Bubble Sort) and writes the result to the file.

## Key Technical Concepts

### 1. Shared Memory Architecture
Instead of writing data to a file and reading it back for sorting (which is slow), this program uses **Shared Memory**:
* The integer array is allocated in `main`.
* A pointer to this array is passed to both threads via the `ThreadArgs` structure.
* The Sorter thread accesses the data directly from RAM, ensuring high performance.

### 2. Thread Synchronization
The program uses `pthread_join()` to enforce a strict sequence:
* Main starts `fill_arr` -> waits for it to finish.
* Main starts `sort_arr` -> waits for it to finish.
This ensures the sorting algorithm never runs on empty or incomplete data.

### 3. Thread-Safe Random Generation
The program uses `rand_r()` instead of the standard `rand()`. This is a thread-safe approach that maintains the state of the random number generator via a `seed` variable passed to the thread.

### 4. Output Formatting
The file output logic includes checks to ensure clean formatting:
* **Iteration IDs:** Each line is tagged with the round number (e.g., `[0]`, `[1]`).
* **Comma Handling:** The program detects the last element of the array to avoid printing a trailing comma (e.g., `1, 2, 3` instead of `1, 2, 3,`).

## Building and Running

### Prerequisites
* GCC Compiler
* Linux/Unix environment (or WSL)

### Compilation
You must explicitly link the pthread library using the `-pthread` flag.

```bash
gcc main.c -o array_app -pthread
```

### Execution
Run the compiled executable:
```bash
./array_app
```