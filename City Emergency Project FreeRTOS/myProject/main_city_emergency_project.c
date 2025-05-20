/**
******************************************************************************
* @file           : main_city_emergency_project.c
* @author         : Nimrod Elstein 
* @brief          : User application main program
******************************************************************************
* 
* This FreeRTOS simulator project is the final project for 
* RTG collage RT Concepts course, class of 2024-2025.
* This project simulates a city emergency dispatcher program.
* 
******************************************************************************
*/

#include "city_emergency_project.h"

QueueHandle_t xPoliceQueue, xAmbulanceQueue, xFireQueue;                    // initialize queue handles
SemaphoreHandle_t xPoliceSemaphore, xAmbulanceSemaphore, xFireSemaphore;   // intialize semaphore handles
SemaphoreHandle_t xLogMutex, xResourceMutex, xEventBufferMutex;           // initialize mutex handles
DepartmentParams policeParams, ambulanceParams, fireParams;              // intialize department parameters (metadata) structs

Event eventBuffer[MAX_EVENTS];   // initialize the event buffer
int eventCount = 0;             // initialize the event counter (number of pending events before dispatchment)

void main_city_emergency_project(void) {

    srand((unsigned int) time(NULL));  // seed the random number generator

    xPoliceQueue = xQueueCreate(DEPARTMENT_QUEUE_LEN, sizeof(Event));       // create queues   
    xAmbulanceQueue = xQueueCreate(DEPARTMENT_QUEUE_LEN, sizeof(Event));
    xFireQueue = xQueueCreate(DEPARTMENT_QUEUE_LEN, sizeof(Event));

    xPoliceSemaphore = xSemaphoreCreateCounting(MAX_POLICE, MAX_POLICE);    // create semaphores
    xAmbulanceSemaphore = xSemaphoreCreateCounting(MAX_AMBULANCE, MAX_AMBULANCE);
    xFireSemaphore = xSemaphoreCreateCounting(MAX_FIRE, MAX_FIRE);

    xEventBufferMutex = xSemaphoreCreateMutex();   // create mutexes
    xLogMutex = xSemaphoreCreateMutex();
    xResourceMutex = xSemaphoreCreateMutex();

    policeParams.queue = xPoliceQueue;     // create the police parameter struct
    policeParams.semaphore = xPoliceSemaphore;
    policeParams.departmentName = "Police";

    ambulanceParams.queue = xAmbulanceQueue;    // create the ambulance parameter struct
    ambulanceParams.semaphore = xAmbulanceSemaphore;
    ambulanceParams.departmentName = "Ambulance";

    fireParams.queue = xFireQueue;    // create the fire parameter struct
    fireParams.semaphore = xFireSemaphore;
    fireParams.departmentName = "Fire Department";

    /* create all tasks */
    xTaskCreate(EventGeneratorTask, "EventGen", configMINIMAL_STACK_SIZE * 4, NULL, 2, NULL);
    xTaskCreate(DispatcherTask, "Dispatcher", configMINIMAL_STACK_SIZE * 4, NULL, 3, NULL);
    xTaskCreate(DepartmentTask, "Police", configMINIMAL_STACK_SIZE * 4, &policeParams, 2, NULL);
    xTaskCreate(DepartmentTask, "Ambulance", configMINIMAL_STACK_SIZE * 4, &ambulanceParams, 2, NULL);
    xTaskCreate(DepartmentTask, "Fire", configMINIMAL_STACK_SIZE * 4, &fireParams, 2, NULL);
    xTaskCreate(UpdateDisplayTask, "StatusDisplay", configMINIMAL_STACK_SIZE * 4, NULL, 1, NULL);

    vTaskStartScheduler();
}
