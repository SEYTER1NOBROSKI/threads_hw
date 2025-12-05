#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ROUNDS 1000
#define ARRAY_SIZE 10

typedef struct {
	int* arr;
	unsigned int seed;
	int id;
} ThreadArgs;

void* fill_arr(void* arg) {
	ThreadArgs* args = (ThreadArgs*)arg;

	FILE* fptr = fopen("arr_file.txt", "a");
	if (fptr == NULL) {
		printf("Error opening file");
		pthread_exit(NULL);
	}

	fprintf(fptr, "Generated [%d] - ", args->id);

	for (int i = 0; i < ARRAY_SIZE; i++) {
		args->arr[i] = rand_r(&args->seed) % 101;
		if (i == ARRAY_SIZE - 1) {
			fprintf(fptr, "%d", args->arr[i]);
		} else {
			fprintf(fptr, "%d,", args->arr[i]);
		}
	}
	fprintf(fptr, "\n");

	fclose(fptr);
	pthread_exit(NULL);
}

void* sort_arr(void* arg) {
	ThreadArgs* args = (ThreadArgs*)arg;

	for (int i = 0; i < ARRAY_SIZE - 1; i++) {
		for (int j = 0; j < ARRAY_SIZE - i - 1; j++) {
			if (args->arr[j] > args->arr[j + 1]) {
				int temp = args->arr[j];
				args->arr[j] = args->arr[j + 1];
				args->arr[j + 1] = temp;
			}
		}
	}

	FILE* file = fopen("arr_file.txt", "a");
	if (file == NULL) {
		printf("Error opening file");
		pthread_exit(NULL);
	}

	fprintf(file, "Sorted [%d] - ", args->id);
	for (int i = 0; i < ARRAY_SIZE; i++) {
		if (i == ARRAY_SIZE - 1) {
			fprintf(file, "%d", args->arr[i]);
		} else {
			fprintf(file, "%d,", args->arr[i]);
		}
	}
	fprintf(file, "\n----------------\n");

	fclose(file);
	pthread_exit(NULL);
}

int main() {
	int shared_arr[ARRAY_SIZE];
	pthread_t thread_fill, thread_sort;

	FILE* clean = fopen("arr_file.txt", "w");
	if (clean) fclose(clean);

	ThreadArgs args;
	args.arr = shared_arr;
	args.seed = time(NULL);

	for (int i = 0; i < NUM_ROUNDS; i++) {
		args.id = i;

		if(pthread_create(&thread_fill, NULL, fill_arr, (void*)&args) != 0) {
			printf("Failed to create fill thread");
			return 1;
		}

		pthread_join(thread_fill, NULL);

		if (pthread_create(&thread_sort, NULL, sort_arr, (void*)&args) != 0) {
			printf("Failed to create sort thread");
			return 1;
		}

		pthread_join(thread_sort, NULL);
	}

	printf("Done! Check arr_file.txt\n");
	return 0;
}