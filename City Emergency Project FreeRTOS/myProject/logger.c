/**
******************************************************************************
* @file           : event_generator.c
* @author         : Nimrod Elstein 
* @brief          : Source code related to the emergency departments
******************************************************************************
* 
* This FreeRTOS simulator project is the final project for 
* RTG collage RT Concepts course, class of 2024-2025.
* This project simulates a city emergency dispatcher program.
* 
******************************************************************************
*/

#include "city_emergency_project.h"

static char logBuffer[MAX_LOG_LINES][200];   // initialize the log messages buffer (MAX_LOG_LINES to be displayed)
static int logIndex = 0;    // message index variable
static int logCount = 0;   // log messages counter

void log_message(const char *msg) {

    xSemaphoreTake(xLogMutex, portMAX_DELAY);   // take a mutex and block other tasks from logging messages

    strncpy(logBuffer[logIndex], msg, sizeof(logBuffer[logIndex]) - 1);
    logBuffer[logIndex][sizeof(logBuffer[logIndex]) - 1] = '\0';
    logIndex = (logIndex + 1) % MAX_LOG_LINES;
    if (logCount < MAX_LOG_LINES) logCount++;

    xSemaphoreGive(xLogMutex);   // release the mutex
}


void UpdateDisplayTask(void *pvParameters) {

    while (1) {

        printf("\033[2J\033[H"); // ANSI clear screen 

        /* print the log messages */

        xSemaphoreTake(xLogMutex, portMAX_DELAY);  // take a mutex and block other tasks from logging messages

        printf("--- LOG MESSAGES ---\n\n");

        int start = (logIndex - logCount + MAX_LOG_LINES) % MAX_LOG_LINES;
        for (int i = 0; i < logCount; i++) {
            int idx = (start + i) % MAX_LOG_LINES;
            printf("[LOG] %s\n", logBuffer[idx]);
        }
        printf("\n---------------------\n");

        xSemaphoreGive(xLogMutex);  // release the mutex

        ////////////////////////////////// end print log messages

        /* print current system status */

        xSemaphoreTake(xEventBufferMutex, portMAX_DELAY);  // take a mutex and block other tasks from adding events to buffer

        printf("\n--- SYSTEM STATUS ---\n");

        printf("\nPending Calls: %d\n", eventCount);
        for (int i = 0; i < eventCount; i++) {
            const char *type = eventBuffer[i].code == CODE_POLICE ? "Police" :
                               eventBuffer[i].code == CODE_AMBULANCE ? "Ambulance" : "Fire";
            printf("  [%d] %s (priority %d)\n", i + 1, type, eventBuffer[i].priority);
        }

        xSemaphoreGive(xEventBufferMutex);   // release the mutex

        printf("\nActive Department Tasks:\n");
        printf("  Police:    %lu\n", MAX_POLICE - uxSemaphoreGetCount(xPoliceSemaphore));
        printf("  Ambulance: %lu\n", MAX_AMBULANCE - uxSemaphoreGetCount(xAmbulanceSemaphore));
        printf("  Fire:      %lu\n", MAX_FIRE - uxSemaphoreGetCount(xFireSemaphore));

        printf("\nResources Available:\n");
        printf("  Police:    %lu\n", uxSemaphoreGetCount(xPoliceSemaphore));
        printf("  Ambulance: %lu\n", uxSemaphoreGetCount(xAmbulanceSemaphore));
        printf("  Fire:      %lu\n", uxSemaphoreGetCount(xFireSemaphore));

        printf("\nQueue Lengths:\n");
        printf("  Police:    %lu\n", uxQueueMessagesWaiting(xPoliceQueue));
        printf("  Ambulance: %lu\n", uxQueueMessagesWaiting(xAmbulanceQueue));
        printf("  Fire:      %lu\n", uxQueueMessagesWaiting(xFireQueue));

        printf("\n---------------------\n");
        
        ////////////////////////////////// end print system status

        
        vTaskDelay(pdMS_TO_TICKS(500));  // 0.5 sec delay
    }
}