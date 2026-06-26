#ifndef KR260DEMO_R5C0_SERVICE_HPP
#define KR260DEMO_R5C0_SERVICE_HPP

/*
 * R5 core 0 control service.
 *
 * R5c0 owns the board's single LED ("UF2_LED", bit 0x01 of AXI_GPIO_0) and runs
 * the heartbeat. It extends the comm-only ControlService with LED behaviour by
 * composing a LedController and overriding the LED hooks.
 */

#include "xparameters.h"

#include "kr260demo/control_service.hpp"
#include "kr260demo/led_controller.hpp"

class R5c0Service : public kr260demo::ControlService {
public:
	explicit R5c0Service(const kr260demo::CoreConfig &config)
		: kr260demo::ControlService(config),
		  led_(XPAR_AXI_GPIO_0_BASEADDR, /*led_mask=*/0x01u,
		       /*heartbeat_period_ms=*/200u)
	{
	}

	/* Initialise the LED GPIO. Call before starting the scheduler. */
	bool init_led() { return led_.init(); }

	/* Heartbeat task body. */
	void run_heartbeat() { led_.run_heartbeat(); }

protected:
	std::uint32_t on_set_led(std::uint8_t mode) override
	{
		return led_.set_mode(mode);
	}

	void on_fill_status(kr260demo_rpu_status_payload &status) override
	{
		led_.fill_status(status);
	}

private:
	kr260demo::LedController led_;
};

#endif /* KR260DEMO_R5C0_SERVICE_HPP */
