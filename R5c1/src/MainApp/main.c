#include "FreeRTOS.h"
#include "task.h"
#include "xil_printf.h"

#define LPRINTF(fmt, ...) xil_printf("%s():%u " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LPERROR(fmt, ...) LPRINTF("ERROR: " fmt, ##__VA_ARGS__)

static TaskHandle_t idle_task;

static void idle_processing(void *unused_arg)
{
	(void)unused_arg;

	while (1)
		vTaskDelay(pdMS_TO_TICKS(1000));
}

int main(void)
{
	BaseType_t stat;

	xil_printf("Starting R5c1 without LED flashing\r\n");

	stat = xTaskCreate(idle_processing, (const char *)"IDLE", 1024,
			   NULL, 1, &idle_task);
	if (stat != pdPASS)
		LPERROR("cannot create IDLE task\r\n");

	vTaskStartScheduler();

	while (1)
		;

	return 0;
}
