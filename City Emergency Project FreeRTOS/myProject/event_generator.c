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

void EventGeneratorTask(void *pvParameters) {
    
    while (1) {
        Event evt;   // initialize an event object
        evt.code = (rand() % MAX_CODE) + 1;   // random choice of department code
        evt.priority = (rand() % MAX_PRIORITY) + 1;   // random choice of priority
        insert_event(evt);   // insert the event to the eventBuffer
        vTaskDelay(pdMS_TO_TICKS((rand() % (EVENT_GEN_TIME_MAX_MS - EVENT_GEN_TIME_MIN_MS) ) + EVENT_GEN_TIME_MIN_MS));   // random event generation time
    }
}

void insert_event(Event evt) {

    xSemaphoreTake(xEventBufferMutex, portMAX_DELAY);   // take a mutex, blocking the task so that only one event can be inserted at a time

    if (eventCount < MAX_EVENTS) {  // if event buffer is not full

        int i = eventCount - 1;   // place the new event in buffer according to the priority value (highest priority first in buffer)
        while (i >= 0 && eventBuffer[i].priority < evt.priority) {
            eventBuffer[i + 1] = eventBuffer[i];
            i--;
        }
        eventBuffer[i + 1] = evt;

        eventCount++;   // update the total pending events amount
        
    } else {   // if eventBuffer is full, event is dropped

        log_message("Warning: Event generation buffer full. Event dropped.");   // send message to logger
        
    }

    xSemaphoreGive(xEventBufferMutex);   // release the mutex
}
