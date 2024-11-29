#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define QUEUE_SIZE 10

int bottle_queue = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t not_full, not_empty;

void* producer(void* arg) {
    while(1) {
        sem_wait(&not_full);
        pthread_mutex_lock(&mutex);

        if (bottle_queue < QUEUE_SIZE) {
            bottle_queue++;
            printf("Produced a bottle. Queue size: %d\n", bottle_queue);
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&not_empty);
        sleep(2);  // 2-second delay
    }
    return NULL;
}

void* consumer(void* arg) {
    while(1) {
        sem_wait(&not_empty);
        pthread_mutex_lock(&mutex);

        if (bottle_queue > 0) {
            bottle_queue--;
            printf("Consumed a bottle. Queue size: %d\n", bottle_queue);
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&not_full);
        sleep(3);  // 3-second delay
    }
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_thread;

    // Initialize semaphores
    sem_init(&not_full, 0, QUEUE_SIZE);
    sem_init(&not_empty, 0, 0);

    // Create threads
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    // Wait for threads
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    // Clean up
    sem_destroy(&not_full);
    sem_destroy(&not_empty);

    return 0;
}