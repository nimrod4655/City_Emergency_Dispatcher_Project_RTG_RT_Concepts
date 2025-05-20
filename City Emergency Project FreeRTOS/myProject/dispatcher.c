/**
******************************************************************************
* @file           : dispatcher.c
* @author         : Nimrod Elstein 
* @brief          : Source code related to the dispatcher
******************************************************************************
* 
* This FreeRTOS simulator project is the final project for 
* RTG collage RT Concepts course, class of 2024-2025.
* This project simulates a city emergency dispatcher program.
* 
******************************************************************************
*/

#include "city_emergency_project.h"

void DispatcherTask(void *pvParameters) {
    while (1) {

        Event evt;  // intialize event object

        if (get_highest_priority_event(&evt)) {   // get the highest priority event from the eventBuffer

            char msg[200];   // initialize message string

            const char *target = (evt.code == CODE_POLICE) ? "Police" : (evt.code == CODE_AMBULANCE) ? "Ambulance" : "Fire Department";  // get the event's target department
            
            snprintf(msg, sizeof(msg), "Dispatcher sent event to %s (priority %d)", target, evt.priority);  // make the logger message
            log_message(msg);  // logger message

            switch (evt.code) {  // send event to the correct department's queue. if queue is full, send message and delay the dispatching.
                case CODE_POLICE:
                    if (xQueueSendToBack(xPoliceQueue, &evt, pdMS_TO_TICKS(100)) != pdPASS) {
                        log_message("Warning: Police queue full. Dispatcher dropped or delayed event.");
                    }
                    break;
                case CODE_AMBULANCE:
                    if (xQueueSendToBack(xAmbulanceQueue, &evt, pdMS_TO_TICKS(100)) != pdPASS) {
                        log_message("Warning: Ambulance queue full. Dispatcher dropped or delayed event.");
                    }
                    break;
                case CODE_FIRE:
                    if (xQueueSendToBack(xFireQueue, &evt, pdMS_TO_TICKS(100)) != pdPASS) {
                        log_message("Warning: Fire queue full. Dispatcher dropped or delayed event.");
                    }
                    break;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(DISPATCH_TIME_CONST_MS));  // const dispatcher work time
    }
}

int get_highest_priority_event(Event *evtOut) {

    int event_retrieved = 0;  // intialize the returend event flag, 0 means buffer was empty and no event retrieved

    xSemaphoreTake(xEventBufferMutex, portMAX_DELAY);  // lock the eventBuffer with mutex tso other tasks cannot access the eventBuffer

    if (eventCount > 0) {  // if the eventBuffer is not empty

        *evtOut = eventBuffer[0];  // take the highest priority event (first) to the output parameter

        for (int i = 1; i < eventCount; i++) {  // shift all other events one back
            eventBuffer[i - 1] = eventBuffer[i];
        }

        eventCount--;  // correct the event counter

        event_retrieved = 1;
    }

    xSemaphoreGive(xEventBufferMutex);  //  release the mutex nd let other tasks access the eventBuffer

    return event_retrieved;  // event retreived flag
}

