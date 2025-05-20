/**
******************************************************************************
* @file           : city_emergency_project.h
* @author         : Nimrod Elstein 
* @brief          : Header file related to the all (user application) source files in the city emergency dispatcher program
******************************************************************************
* 
* This FreeRTOS simulator project is the final project for 
* RTG collage RT Concepts course, class of 2024-2025.
* This project simulates a city emergency dispatcher program.
* 
******************************************************************************
*/

#ifndef CITY_EMERGENCY_PROJECT_H
#define CITY_EMERGENCY_PROJECT_H

/* Includes */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/////////////////////////

/* Defines */

#define CODE_POLICE     1   // department codes
#define CODE_AMBULANCE  2
#define CODE_FIRE       3

#define MAX_CODE        3      // maximum code value (for randome generation)
#define MAX_PRIORITY    3     // maximum priority value (for randome generation)

#define MAX_POLICE      4   // department maximum available resources
#define MAX_AMBULANCE   3
#define MAX_FIRE        2

#define DEPARTMENT_QUEUE_LEN 5    // queue length for each department, if no resources are available
#define MAX_EVENTS      10       // maximum amount of generated events before dispatched to departments

#define DISPATCH_TIME_CONST_MS          1500       // constant time (ms) for dispatcher to dispatcha call (event)
#define EVENT_GEN_TIME_MAX_MS           2000      // maximum time (ms) for random event generator
#define EVENT_GEN_TIME_MIN_MS           1000     // minimum time (ms) for random event generator
#define DEPARTMENT_HANDLE_TIME_MAX_MS   8000    // maximum time (ms) for random handle time of a department
#define DEPARTMENT_HANDLE_TIME_MIN_MS   3000   // minimum time (ms) for random handle time of a department

#define MAX_LOG_LINES 10   // maximum number of logger message lines shown in terminal display

///////////////////////////////// end Defines

/* Variables */

typedef struct {   // emergency event object
    int code;
    int priority;
} Event;

typedef struct {   // department parameters (metadata) object
    QueueHandle_t queue;
    SemaphoreHandle_t semaphore;
    const char *departmentName;
} DepartmentParams;

typedef struct {   //  arguments object for the event handler task
    Event evt;
    DepartmentParams *params;
    BaseType_t borrowed;
    SemaphoreHandle_t borrowedFrom;
} EventHandlerArgs;

extern QueueHandle_t xPoliceQueue, xAmbulanceQueue, xFireQueue;   // queue handles
extern SemaphoreHandle_t xPoliceSemaphore, xAmbulanceSemaphore, xFireSemaphore;  // semasphore handles
extern SemaphoreHandle_t xLogMutex, xResourceMutex, xEventBufferMutex;   

extern Event eventBuffer[MAX_EVENTS];  // event buffer for generated calls (events) before dispatched
extern int eventCount;   // pending events counter

///////////////////////////////// end Variables

/* Function Signatures */

/**
 * @brief task function for generating random emergency calls (events).
 *
 * This task function simulates incoming emergency calls by creating random events
 * at random time intervals and inserting them into a shared priority event buffer (eventBuffer).
 * Events are categorized by department code (Police, Ambulance, Fire) and assigned a random priority.
 * 
 * @param pvParameters Not used. Pass NULL.
 *
 * @return void
 * 
 * @note This task should be started during system initialization.
 * 
 * @warning If the eventBuffer is full (reaches MAX_EVENTS), new events are dropped.
 */
void EventGeneratorTask(void *pvParameters);

/**
 * @brief Function to insert a generated event into the eventBuffer in descending priority order.
 *
 * This function adds a new emergency event to the global event buffer,
 * with a descending priority order (highest priority first).
 * If the buffer is full (`MAX_EVENTS` reached), the event is dropped.
 *
 *
 * @param evt The event to insert into the buffer.
 *
 * @warning If the event buffer is full, the event will be dropped.
 */
void insert_event(Event evt);

/**
 * @brief Task function that dispatches the highest-priority event to the appropriate department queue.
 *
 * This task  retrieves events from the eventBuffer and sends them to the correct department queue
 * (Police, Ambulance, or Fire Department) based on event code.
 *
 * @param pvParameters Not used. Pass NULL.
 *
 * @note his task should be started during system initialization.
 * 
 * @warning Dispatcher will delay an event when queues are full, dispatcher is blocked during the delay but does not indefinitely.
 */
void DispatcherTask(void *pvParameters);

/**
 * @brief Function that etrieves and removes the highest-priority event from the eventBuffer.
 *
 * This function takes the event with the highest priority from the eventBuffer.
 * Events are stored in descending priority order, so the first event is always the most urgent.
 *
 * @param[out] evtOut Pointer to an Event structure that will receive the result.
 *
 * @return integer that is 1 if an event was successfully retrieved, 0 if the buffer was empty.
 */
int get_highest_priority_event(Event *evtOut);

/**
 * @brief Task function, per emergency department, that recives events and handles them.
 *
 * This task funcrion waits for events from the department's event queue (police, ambulance, or fire). 
 * When receiving an event, it attempts to get a local resource from its own semaphore.
 * If no local resource is available, it will attempt to borrow from other departments.
 * If a resource (local or borrowed) is available, a new event handler task starts to process the event.
 * If non are available, the event is requeued, and this task function waits briefly before retrying.
 *
 * @param pvParameters A pointer to a DepartmentParams structure specifying the queue, semaphore, and name for the department.
 *
 * @return void
 * 
 * @note This task should be started during system initialization, once per department.
 * 
 * @warning If there are no available resources, events are delayed until resources become available.
 */
void DepartmentTask(void *pvParameters);

/**
 * @brief Task function that handles a single emergency event.
 *
 * This task simulates processing an event by delaying for a random time
 * It then releases the resource it used, either the department's own local semaphore
 * or a borrowed resource from another department.
 *
 *
 * @param pvParameters A pointer to a EventHandlerArgs struct containing the event, department parameters and borrow status.
 *
 * @return void
 */
void EventHandlerTask(void *pvParameters);

/**
 * @brief Task function that updates the terminal diapay with current system status and recent log messages.
 *
 * This task function clears and reprints the terminal display with real-time information about the system.
 * Displaying to the user:
 * - The last MAX_LOG_LINES logged messages (most recent message at the bottom)
 * - Pending calls waiting in the eventBuffer, prior to being dispatched
 * - Active department tasks currently handling events
 * - Current resource availability
 * - Department queue lengths (events waiting for resource avilability)
 *
 *
 * @param pvParameters Not used. Pass NULL.
 * 
 * @return void
 */
void UpdateDisplayTask(void *pvParameters);

/**
 * @brief Logger function to display a message for each performed action in the system.
 *
 * Stores the provided log message in a fixed-size circular bufferused by the status display task.
 * Only the most recent messages (up to MAX_LOG_LINES) are displayed.
 *
 * @param msg A message string to be displayed.
 * 
 * @return void
 */
void log_message(const char *msg);

///////////////////////////////// end Function Signatures

#endif
