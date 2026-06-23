#include "FreeRTOS.h"
#include "task.h"
#include <unistd.h>

#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xstatus.h"

#define DELAY_5s	5000UL
#define DELAY_1s	1000UL


//Thread implementation
static void myTask1(void *);


//Task Handle
static TaskHandle_t th_myTask1;



int main( void )
{

	xil_printf( "My first FreeRTOS app\n" );


	xTaskCreate(myTask1,
			    (const char *) "myTask1",
				configMINIMAL_STACK_SIZE,
				NULL,
				tskIDLE_PRIORITY,
				&th_myTask1);


    // Start the Task
	vTaskStartScheduler();

	while(true);
}




static void myTask1(void* pvParameters)
{
 (void)pvParameters;
  unsigned int count = 0;

  while(1)
  {
	  vTaskDelay(pdMS_TO_TICKS(DELAY_5s));
	  count++;
	  //xil_printf("R5c1: Thread 2 counter value: %u \r\n", count);
  }
}
