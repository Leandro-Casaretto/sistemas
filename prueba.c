#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"

#define PRINT_DELAY_SECONDS 1 // Retraso de 1 segundo por carácter

lo
    const char* message = (const char*)arg;

    Message* newMessage = (Message*)malloc(sizeof(Message));
    strcpy(newMessage->message, message);

    // Bloquea el acceso a la cola con el mutex
    pthread_mutex_lock(&queue_mutex);

    // Agrega el mensaje a la cola
    enqueue(&messageQueue, newMessage);

    // Libera el semáforo de orden de impresión
    sem_post(&print_order_semaphore);

    // Desbloquea el acceso a la cola
    pthread_mutex_unlock(&queue_mutex);

    pthread_exit(NULL);
}

void* computer_thread3(void* arg) {
    // Genera un mensaje de ejemplo
    const char* message = (const char*)arg;

    Message* newMessage = (Message*)malloc(sizeof(Message));
    strcpy(newMessage->message, message);

    // Bloquea el acceso a la cola con el mutex
    pthread_mutex_lock(&queue_mutex);

    // Agrega el mensaje a la cola
    enqueue(&messageQueue, newMessage);

    // Libera el semáforo de orden de impresión
    sem_post(&print_order_semaphore);

    // Desbloquea el acceso a la cola
    pthread_mutex_unlock(&queue_mutex);

    pthread_exit(NULL);
}

void* printer_thread(void* arg) {
    while (1) {
        // Espera a que haya mensajes en la cola
        sem_wait(&print_order_semaphore);

        // Espera a que haya mensajes en la cola y obtiene el semáforo de la impresora 1
        sem_wait(&printer_semaphore1);

        // Espera a que haya mensajes en la cola
        pthread_mutex_lock(&queue_mutex);

        // Verifica si todos los mensajes se han completado
        if (completedMessages == totalMessages && isQueueEmpty(&messageQueue)) {
            // Desbloquea el acceso a la cola
            pthread_mutex_unlock(&queue_mutex);
            break; // Termina el hilo de impresión
        }

        // Verifica si la cola está vacía
        if (isQueueEmpty(&messageQueue)) {
            // Desbloquea el acceso a la cola
            pthread_mutex_unlock(&queue_mutex);
            // Libera el semáforo de la impresora 1
            sem_post(&printer_semaphore1);
            continue; // Vuelve a esperar por nuevos mensajes en la cola
        }

        // Obtiene el mensaje de la cola
        Message* printedMessage = (Message*)dequeue(&messageQueue);

        // Desbloquea el acceso a la cola
        pthread_mutex_unlock(&queue_mutex);

        // Imprime el mensaje con retraso entre los caracteres
        FILE* fp = fopen("/dev/pts/3", "w");
        if (fp == NULL) {
            printf("Error al abrir el dispositivo de terminal.\n");
            exit(-1);
        }

        fprintf(fp, "Printing document\n");
        fflush(fp);
		
        int i = 0;
        while (i < strlen(printedMessage->message)) {
            fprintf(fp, "%c", printedMessage->message[i]);
            fflush(fp);
            sleep(PRINT_DELAY_SECONDS);
            i++;
        }

        fprintf(fp, "\n");
        fclose(fp);

        free(printedMessage);

        // Incrementa el contador de mensajes completados
        completedMessages++;

        // Libera el semáforo de la impresora 1
        sem_post(&printer_semaphore1);
    }

    pthread_exit(NULL);
}

void* printer_thread2(void* arg) {
    while (1) {
        // Espera a que haya mensajes en la cola
        sem_wait(&print_order_semaphore);

        // Espera a que haya mensajes en la cola y obtiene el semáforo de la impresora 2
        sem_wait(&printer_semaphore2);

        // Espera a que haya mensajes en la cola
        pthread_mutex_lock(&queue_mutex);

        // Verifica si todos los mensajes se han completado
        if (completedMessages == totalMessages && isQueueEmpty(&messageQueue)) {
            // Desbloquea el acceso a la cola
            pthread_mutex_unlock(&queue_mutex);
            break; // Termina el hilo de impresión
        }

        // Verifica si la cola está vacía
        if (isQueueEmpty(&messageQueue)) {
            // Desbloquea el acceso a la cola
            pthread_mutex_unlock(&queue_mutex);
            // Libera el semáforo de la impresora 2
            sem_post(&printer_semaphore2);
            continue; // Vuelve a esperar por nuevos mensajes en la cola
        }

        // Obtiene el mensaje de la cola
        Message* printedMessage = (Message*)dequeue(&messageQueue);

        // Desbloquea el acceso a la cola
        pthread_mutex_unlock(&queue_mutex);

        // Imprime el mensaje con retraso entre los caracteres
        FILE* fp = fopen("/dev/pts/2", "w");
        if (fp == NULL) {
            printf("Error al abrir el dispositivo de terminal.\n");
            exit(-1);
        }

        fprintf(fp, "Printing document\n");
        fflush(fp);

        int i = 0;
        while (i < strlen(printedMessage->message)) {
            fprintf(fp, "%c", printedMessage->message[i]);
            fflush(fp);
            sleep(PRINT_DELAY_SECONDS);
            i++;
        }

        fprintf(fp, "\n");
        fclose(fp);

        free(printedMessage);

        // Incrementa el contador de mensajes completados
        completedMessages++;

        // Libera el semáforo de la impresora 2
        sem_post(&printer_semaphore2);
    }

    pthread_exit(NULL);
}

int main() {

    
    // Inicializa la cola
    initializeQueue(&messageQueue);

    // Inicializa el mutex y los semáforos
    pthread_mutex_init(&queue_mutex, NULL);
    sem_init(&printer_semaphore1, 0, 0);
    sem_init(&printer_semaphore2, 0, 0);
    sem_init(&print_order_semaphore, 0, 0);

    // Crea los hilos de las computadoras y la impresora
    pthread_t computer1, computer2, computer3, printer1, printer2;
    const char* message1 = "Why did the C programmer always carry a ladder? Because they heard the language had a lot of pointers!";
    const char* message2 = "How did the C programmer escape from prison? They used a NULL pointer to break free!";
    const char* message3 = "Did you hear about the C programmer who got stuck in an infinite loop? They forgot to increment their way out!";
    pthread_create(&computer1, NULL, computer_thread, (void*)message1);
    pthread_create(&computer2, NULL, computer_thread2, (void*)message2);
    pthread_create(&computer3, NULL, computer_thread3, (void*)message3);
    pthread_create(&printer1, NULL, printer_thread, NULL);
    pthread_create(&printer2, NULL, printer_thread2, NULL);

    // Espera a que los hilos terminen
    pthread_join(computer1, NULL);
    pthread_join(computer2, NULL);
    pthread_join(computer3, NULL);

    // Establece el número total de mensajes
    totalMessages = completedMessages;

    // Libera los semáforos para que los hilos de impresión puedan salir del bucle
    sem_post(&print_order_semaphore);
    sem_post(&printer_semaphore1);
    sem_post(&printer_semaphore2);

    // Espera a que los hilos de impresión terminen
    pthread_join(printer1, NULL);
    pthread_join(printer2, NULL);

    // Destruye el mutex y los semáforos
    pthread_mutex_destroy(&queue_mutex);
    sem_destroy(&printer_semaphore1);
    sem_destroy(&printer_semaphore2);
    sem_destroy(&print_order_semaphore);

    return 0;
}