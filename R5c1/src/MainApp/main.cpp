/*
 * KR260 RPU demo - R5 core 1 entry point.
 *
 * This file owns the FreeRTOS task and its handle. The rpmsg communication
 * lives in kr260demo::ControlService (../../common); R5c1Service is a thin
 * subclass. R5c1 has no LED on the KR260 board, so there is no LED task.
 */

#include "FreeRTOS.h"
#include "task.h"

#include "kr260demo/control_service.hpp"
#include "r5c1_service.hpp"

static R5c1Service service(kr260demo::CoreConfig::current());

static TaskHandle_t comm_task_handle;

static void comm_task(void *)
{
	service.run();
}

int main(void)
{
	if (xTaskCreate(comm_task, "RPMSG", 2048, NULL, 2,
			&comm_task_handle) != pdPASS)
		return -1;

	vTaskStartScheduler();

	while (1)
		;

	return 0;
}
