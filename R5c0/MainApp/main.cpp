/*
 * KR260 RPU demo - R5 core 0 entry point.
 *
 * This file owns the FreeRTOS tasks and their handles. The rpmsg communication
 * lives in kr260demo::ControlService (../../common); R5c0Service adds the LED
 * behaviour for this core. R5c0 drives the board's single LED ("UF2_LED") and
 * runs the heartbeat.
 */

#include "FreeRTOS.h"
#include "task.h"

#include "kr260demo/control_service.hpp"
#include "r5c0_service.hpp"

static R5c0Service service(kr260demo::CoreConfig::current());

static TaskHandle_t comm_task_handle;
static TaskHandle_t led_task_handle;

static void comm_task(void *)
{
	service.run();
}

static void led_task(void *)
{
	service.run_heartbeat();
}

int main(void)
{
	if (!service.init_led())
		return -1;

	if (xTaskCreate(comm_task, "RPMSG", 2048, NULL, 2,
			&comm_task_handle) != pdPASS)
		return -1;

	if (xTaskCreate(led_task, "LED", 1024, NULL, 5,
			&led_task_handle) != pdPASS)
		return -1;

	vTaskStartScheduler();

	while (1)
		;

	return 0;
}
