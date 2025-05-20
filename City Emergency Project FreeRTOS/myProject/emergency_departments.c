/**
******************************************************************************
* @file           : emergency_departments.c
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

void EventHandlerTask(void *pvParameters) {

    EventHandlerArgs *args = (EventHandlerArgs *)pvParameters;  // get the the input event parameters
    Event evt = args->evt;
    DepartmentParams *params = args->params;
    const char *deptName = params->departmentName;

    TickType_t startTick = xTaskGetTickCount();  // handle the event with a random duration time, calc the duration for logger message
    vTaskDelay(pdMS_TO_TICKS((rand() % (DEPARTMENT_HANDLE_TIME_MAX_MS - DEPARTMENT_HANDLE_TIME_MIN_MS) ) + DEPARTMENT_HANDLE_TIME_MIN_MS));
    TickType_t endTick = xTaskGetTickCount();
    TickType_t duration = endTick - startTick;

    if (args->borrowed && args->borrowedFrom != NULL) {  // give back the resourcse (semaphore), local or borrowed
        xSemaphoreGive(args->borrowedFrom);
    } else {
        xSemaphoreGive(params->semaphore);
    }

    char msg[200];   // send message to logger
    snprintf(msg, sizeof(msg), "%s completed event in %lu ticks", deptName, (unsigned long)duration);
    log_message(msg);

    free(args);  // free dynamic memory

    vTaskDelete(NULL);  // delete this task when completed
}

void DepartmentTask(void *pvParameters) { 

    DepartmentParams *params = (DepartmentParams *) pvParameters;   // get the the input department parameters
    QueueHandle_t queue = params->queue;
    SemaphoreHandle_t semaphore = params->semaphore;
    const char *deptName = params->departmentName;

    while (1) {

        Event evt;   // intialize an empty event object

        if (xQueueReceive(queue, &evt, portMAX_DELAY) == pdPASS) {   // get event from the department's queue (if here is one)

            BaseType_t local = xSemaphoreTake(semaphore, 0);  // get a local resource, local is true if local resource is available and false if not
            BaseType_t borrowed = pdFALSE;                   // initialize a borrowed flag to false, if a resource will be borrowed we switch to true
            SemaphoreHandle_t borrowedFrom = NULL;          // initialize a semaphore handle from who a resource will be borrowed from

            if (!local) {  // if no local resources available (department's own)

                xSemaphoreTake(xResourceMutex, portMAX_DELAY);  // take a mutex, blocking the task so that only one department can borrow at a time

                // resource borrow logic, simple stupid -  check who has one and take it
                if (strcmp(deptName, "Ambulance") == 0) {      
                    if (uxSemaphoreGetCount(xPoliceSemaphore) > 0 && xSemaphoreTake(xPoliceSemaphore, 0)) {
                        borrowed = pdTRUE; 
                        borrowedFrom = xPoliceSemaphore;
                        log_message("Ambulance borrowed resource from Police");
                    } else if (uxSemaphoreGetCount(xFireSemaphore) > 0 && xSemaphoreTake(xFireSemaphore, 0)) {
                        borrowed = pdTRUE; 
                        borrowedFrom = xFireSemaphore;
                        log_message("Ambulance borrowed resource from Fire Department");
                    }
                } else if (strcmp(deptName, "Fire Department") == 0) {
                        if (uxSemaphoreGetCount(xPoliceSemaphore) > 0 && xSemaphoreTake(xPoliceSemaphore, 0)) {
                            borrowed = pdTRUE; 
                            borrowedFrom = xPoliceSemaphore;
                            log_message("Fire Department borrowed resource from Police");
                        } else if (uxSemaphoreGetCount(xAmbulanceSemaphore) > 0 && xSemaphoreTake(xAmbulanceSemaphore, 0)) {
                            borrowed = pdTRUE; 
                            borrowedFrom = xAmbulanceSemaphore;
                            log_message("Fire Department borrowed resource from Ambulance");
                        }
                } else if (strcmp(deptName, "Police") == 0) {
                        if (uxSemaphoreGetCount(xFireSemaphore) > 0 && xSemaphoreTake(xFireSemaphore, 0)) {
                            borrowed = pdTRUE; 
                            borrowedFrom = xFireSemaphore;
                            log_message("Police borrowed resource from Fire Department");
                        } else if (uxSemaphoreGetCount(xAmbulanceSemaphore) > 0 && xSemaphoreTake(xAmbulanceSemaphore, 0)) {
                            borrowed = pdTRUE; 
                            borrowedFrom = xAmbulanceSemaphore;
                            log_message("Police borrowed resource from Ambulance");
                        }
                }

                xSemaphoreGive(xResourceMutex);   // release the mutex and allow other departments to borrow

            }

            if (local || borrowed) {  // if there is an available resource, local or borrowed

                char msg[200];      // initialize a message string
                snprintf(msg, sizeof(msg), "%s handling event (priority %d)%s", deptName, evt.priority, borrowed ? " [borrowed]" : "");
                log_message(msg); // send a "handling event" message to logger

                EventHandlerArgs *args = malloc(sizeof(EventHandlerArgs));  // allocate dynamic memory for even handler arguments
                
                args->evt = evt;      // assign the event handler task arguments
                args->params = params;
                args->borrowed = borrowed;
                args->borrowedFrom = borrowedFrom;

                xTaskCreate(EventHandlerTask, "EventWorker", configMINIMAL_STACK_SIZE * 4, args, 2, NULL);  // handle an event task

            } else {  // if no resources available

              char msg[200];     // initialize a message string
              snprintf(msg, sizeof(msg), "%s No available or borrowed resources, event delayed", deptName);
              log_message(msg);   // send message to logger
              xQueueSendToBack(queue, &evt, portMAX_DELAY);   // send the event back to the department's queue, if queue is full then task is blocked until queue space is available
              vTaskDelay(pdMS_TO_TICKS(500));   // 0.5 sec hardcoded delay

            }
        }
    }
}