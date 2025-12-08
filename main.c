#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_ROUNDS 1000
#define LINE_SIZE 128

pthread_mutex_t lock;
pthread_cond_t buffer_empty;
pthread_cond_t data_ready;

int buffer_filled = 0; //flag 0 - empty, 1 - ready
int EOF_reached = 0;

typedef struct {
	char buffer[LINE_SIZE + 1];
	char sorted_buffer[LINE_SIZE + 1];
	FILE* input_fp;
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


	while (1) {
		pthread_mutex_lock(&lock);

		while (buffer_filled == 1 && !EOF_reached) {
			pthread_cond_wait(&buffer_empty, &lock);
		}

		if (EOF_reached) {
			pthread_mutex_unlock(&lock);
			break;
		}

		if (fgets(args->buffer, LINE_SIZE + 1, input_fp) == NULL) {
			EOF_reached = 1;
			buffer_filled = 0;
			pthread_mutex_unlock(&lock);
			pthread_cond_signal(&data_ready);
			break;
		}

		size_t len = strlen(args->buffer);
		if (len > 0 && args->buffer[len - 1] == '\n') {
			args->buffer[len - 1] = '\0';
		}

		strcpy(args->sorted_buffer, args->buffer);

		args->id = current_line++;

		FILE* output = fopen("result.txt", "a");
		if (output) {
			fprintf(output, "T[%d] - %s\n", args->id, args->buffer);
			fclose(output);
		}

		buffer_filled = 1;

		pthread_mutex_unlock(&lock);
		pthread_cond_signal(&data_ready);
	}

	fclose(input_fp);
	pthread_exit(NULL);
}

void* sort_arr(void* arg) {
	ThreadArgs* args = (ThreadArgs*)arg;

	while (1) {
		pthread_mutex_lock(&lock);

		while (buffer_filled == 0 && !EOF_reached) {
			pthread_cond_wait(&data_ready, &lock);
		}

		if (EOF_reached && buffer_filled == 0) {
			pthread_mutex_unlock(&lock);
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

		FILE* output = fopen("result.txt", "a");
		if (output) {
			fprintf(output, "Sorted T[%d] - %s\n", args->id, args->sorted_buffer);
			fprintf(output, "----------------\n");
			fclose(output);
		}

		buffer_filled = 0;

		pthread_mutex_unlock(&lock);
		pthread_cond_signal(&buffer_empty);
	}

	pthread_exit(NULL);
}

int main() {
	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&buffer_empty, NULL);
	pthread_cond_init(&data_ready, NULL);
	char buffer[LINE_SIZE + 1];
	pthread_t thread_fill, thread_sort;

	FILE* clean = fopen("result.txt", "w");
	if (clean) fclose(clean);

	ThreadArgs args;

	pthread_create(&thread_sort, NULL, sort_arr, (void*)&args);

	pthread_create(&thread_fill, NULL, fill_arr, (void*)&args);

	pthread_join(thread_sort, NULL);
	pthread_join(thread_fill, NULL);

	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&data_ready);
	pthread_cond_destroy(&buffer_empty);

	printf("Done! Check result.txt\n");
	return 0;
}