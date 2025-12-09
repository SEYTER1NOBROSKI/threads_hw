#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>

#define NUM_ROUNDS 1000
#define LINE_SIZE 128

sem_t sem_empty;
sem_t sem_full;

typedef struct {
	char buffer[LINE_SIZE + 1];
	char sorted_buffer[LINE_SIZE + 1];
	FILE* output_fp;
	int id;
} ThreadArgs;

void* fill_arr(void* arg) {
	ThreadArgs* args = (ThreadArgs*)arg;
	int current_line = 0;

	FILE* input_fp = fopen("input_text.txt", "r");
	if (!input_fp) {
		printf("Error opening input_text.txt");
		pthread_exit(NULL);
	}

	char local_temp[LINE_SIZE + 1];

	while (fgets(local_temp, LINE_SIZE + 1, input_fp) != NULL) {

		sem_wait(&sem_empty);

		size_t len = strlen(local_temp);
		if (len > 0 && local_temp[len - 1] == '\n') {
			local_temp[len - 1] = '\0';
		}

		strcpy(args->buffer, local_temp);
		strcpy(args->sorted_buffer, local_temp);

		args->id = current_line++;

		if (args->output_fp) {
			fprintf(args->output_fp, "T[%d] - %s\n", args->id, args->buffer);
			fflush(args->output_fp);
		}

		sem_post(&sem_full);
	}

	sem_wait(&sem_empty);
	args->id = -1;
	sem_post(&sem_full);

	fclose(input_fp);
	pthread_exit(NULL);
}

void* sort_arr(void* arg) {
	ThreadArgs* args = (ThreadArgs*)arg;

	while (1) {
		sem_wait(&sem_full);

		if (args->id == -1) {
			sem_post(&sem_empty);
			break; 
		}

		size_t len = strlen(args->sorted_buffer);

		for (size_t i = 0; i + 1 < len; i++) {
			for (size_t j = 0; j + 1 < len - i; j++) {
				if (args->sorted_buffer[j] > args->sorted_buffer[j + 1]) {
					char temp = args->sorted_buffer[j];
					args->sorted_buffer[j] = args->sorted_buffer[j + 1];
					args->sorted_buffer[j + 1] = temp;
				}
			}
		}

		if (args->output_fp) {
			fprintf(args->output_fp, "Sorted T[%d] - %s\n", args->id, args->sorted_buffer);
			fprintf(args->output_fp, "----------------\n");
			fflush(args->output_fp);
		}

		sem_post(&sem_empty);
	}

	pthread_exit(NULL);
}

int main() {
	// semaphore init
	// sem_empty = 1 (one free space on the start)
	sem_init(&sem_empty, 0, 1);
	// sem_full = 0 (no data on start)
	sem_init(&sem_full, 0, 0);

	pthread_t thread_fill, thread_sort;

	ThreadArgs args;

	args.output_fp = fopen("result.txt", "w");
	if (args.output_fp == NULL) {
		printf("Erorr opening file result.txt");
		return 1;
	}

	pthread_create(&thread_sort, NULL, sort_arr, (void*)&args);

	pthread_create(&thread_fill, NULL, fill_arr, (void*)&args);

	pthread_join(thread_sort, NULL);
	pthread_join(thread_fill, NULL);

	fclose(args.output_fp);

	sem_destroy(&sem_empty);
	sem_destroy(&sem_full);

	printf("Done! Check result.txt\n");
	return 0;
}