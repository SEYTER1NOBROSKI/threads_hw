#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_ROUNDS 1000
#define LINE_SIZE 128

pthread_mutex_t lock;

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


	while (fgets(args->buffer, LINE_SIZE + 1, input_fp) != NULL) {

		size_t len = strlen(args->buffer);
		if (len > 0 && args->buffer[len - 1] == '\n') {
			args->buffer[len - 1] = '\0';
		}

		pthread_mutex_lock(&lock);

		strcpy(args->sorted_buffer, args->buffer);

		args->id = current_line++;

		if (args->output_fp) {
			fprintf(args->output_fp, "T[%d] - %s\n", args->id, args->buffer);
			fflush(args->output_fp);
		}

		pthread_mutex_unlock(&lock);
	}

	fclose(input_fp);
	pthread_exit(NULL);
}

void* sort_arr(void* arg) {
	ThreadArgs* args = (ThreadArgs*)arg;

	while (1) {
		pthread_mutex_lock(&lock);

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

		pthread_mutex_unlock(&lock);
	}

	pthread_exit(NULL);
}

int main() {
	pthread_mutex_init(&lock, NULL);

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

	pthread_mutex_destroy(&lock);

	printf("Done! Check result.txt\n");
	return 0;
}