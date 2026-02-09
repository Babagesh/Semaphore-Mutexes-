/***************************************************************************//**
 * @file
 * @brief Blink examples functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"
#include "sl_simple_button_instances.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/




#ifndef BLINK_TASK_STACK_SIZE
#define BLINK_TASK_STACK_SIZE      configMINIMAL_STACK_SIZE
#endif

#ifndef BLINK_TASK_PRIO
#define BLINK_TASK_PRIO            20
#endif

#ifndef EXAMPLE_USE_STATIC_ALLOCATION
#define EXAMPLE_USE_STATIC_ALLOCATION      1
#endif

extern SemaphoreHandle_t mutex_handle;


/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void task2_task(void *arg);
void timer_callback(TimerHandle_t timer_handle);
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize blink example.
 ******************************************************************************/
void task2_init(void)
{
  TaskHandle_t xHandle = NULL;

#if (EXAMPLE_USE_STATIC_ALLOCATION == 1)

  static StaticTask_t xTaskBuffer;
  static StackType_t  xStack[BLINK_TASK_STACK_SIZE];

  // Create Blink Task without using any dynamic memory allocation
  xHandle = xTaskCreateStatic(task2_task,
                              "blink task",
                              BLINK_TASK_STACK_SIZE,
                              ( void * ) NULL,
                              tskIDLE_PRIORITY + 1,
                              xStack,
                              &xTaskBuffer);

  // Since puxStackBuffer and pxTaskBuffer parameters are not NULL,
  // it is impossible for xHandle to be null. This check is for
  // rigorous example demonstration.
  EFM_ASSERT(xHandle != NULL);

#else

  BaseType_t xReturned = pdFAIL;

  // Create Blink Task using dynamic memory allocation
  xReturned = xTaskCreate(task2_task,
                          "blink task",
                          BLINK_TASK_STACK_SIZE,
                          ( void * ) NULL,
                          tskIDLE_PRIORITY + 1,
                          &xHandle);

  // Unlike task creation using static allocation, dynamic task creation can very likely
  // fail due to lack of memory. Checking the return value is relevant.
  EFM_ASSERT(xReturned == pdPASS);

#endif
}

/*******************************************************************************
 * Blink task.
 ******************************************************************************/
static void task2_task(void *arg)
{
  (void)&arg;
  TimerHandle_t timer_handle;
  StaticTimer_t timer_buffer;
  timer_handle = xTimerCreateStatic("task 2 timer", // const char * const pcTimerName,
                  pdMS_TO_TICKS(2000),   // const TickType_t xTimerPeriod,
                  pdTRUE,               // const UBaseType_t uxAutoReload,
                  NULL,                     // void * const pvTimerID,
                  timer_callback,       // TimerCallbackFunction_t pxCallbackFunction
                  &timer_buffer);       // StaticTimer_t *pxTimerBuffer

  //Use the provided calculation macro to convert milliseconds to OS ticks
  const TickType_t xDelay = pdMS_TO_TICKS(100);
  xTimerStart(timer_handle, portMAX_DELAY);

  while (1) {
    //Wait for specified delay
    vTaskDelay(xDelay);
  }
}

void timer_callback(TimerHandle_t timer_handle)
{
  (void) timer_handle;
  if(sl_led_get_state(&sl_led_led1))
    {
      sl_led_turn_off(&sl_led_led1);
      xSemaphoreGive(mutex_handle);
    }
  else
    {
      if(xSemaphoreTake(mutex_handle, 0) == pdTRUE)
          {
            sl_led_turn_on(&sl_led_led1);
          }
    }

}
