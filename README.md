# KR260Demo_RPU

FreeRTOS firmware for the two Cortex-R5 cores (R5c0, R5c1) of the KR260
demo. Each core runs an OpenAMP/rpmsg **control service** that the Linux/APU
side drives with the `apu-rpu-ctl` utility (`KR260Demo_APU`).

## Layout

```
common/                         shared, core-agnostic code
  include/kr260demo/
    rpu_control_protocol.h       wire protocol (kept byte-identical to APU copy)
    openamp_platform.hpp         RAII over remoteproc/rpmsg-vdev lifecycle
    rpmsg_endpoint.hpp           RAII rpmsg endpoint + MessageSink adapter
    control_service.hpp          comm-only base service: CoreConfig + dispatch
    led_controller.hpp           reusable LED/heartbeat helper (optional)
  openamp/                       AMD OpenAMP platform glue (C, from Vitis)
    platform_info.[ch], rsc_table.[ch], helper.c, zynqmp_r5_a53_rproc.c
  src/                           C++ library implementation
    openamp_platform.cpp, rpmsg_endpoint.cpp, control_service.cpp,
    led_controller.cpp
R5c0/MainApp/{main.cpp, r5c0_service.hpp}   core-0: tasks + LED service
R5c1/src/MainApp/{main.cpp, r5c1_service.hpp}   core-1: task + no-LED service
```

## Architecture

The communication library is a layered, board-agnostic core; per-core
behaviour and FreeRTOS task ownership live with each core.

1. **`OpenAmpPlatform`** — RAII wrapper around `platform_init`/`platform_cleanup`
   and the rpmsg virtio device create/release/poll cycle.
2. **`RpmsgEndpoint`** — RAII endpoint that routes incoming frames to a
   `MessageSink` (C++ virtual interface) instead of a raw C callback.
3. **`ControlService`** — **base class, communication only**: framing,
   validation, and dispatch of `PING` / `GET_STATUS` / `SET_LED`. It owns no
   GPIO and creates no tasks. Per-core behaviour is supplied by overriding the
   virtual hooks `on_set_led()`, `on_fill_status()` and `handle_custom()`. Its
   blocking `run()` is the body of a task created by `main.cpp`.
4. **`LedController`** — a separate, optional helper that owns the AXI GPIO LED
   and the heartbeat loop. It is deliberately *not* part of the comm core.

Each core has its **own service subclass** and its `main.cpp` **owns the
FreeRTOS tasks and handles**:

- **`R5c0Service`** composes a `LedController` and overrides the LED hooks;
  `R5c0/MainApp/main.cpp` creates the RPMSG task and the LED task.
- **`R5c1Service`** is a thin subclass using the base defaults (no LED);
  `R5c1/src/MainApp/main.cpp` creates only the RPMSG task. Because the base has
  no GPIO dependency, R5c1 links **no** GPIO code at all.

## LED

The KR260 board exposes a **single** LED ("UF2_LED") to the RPU, on a 1-bit AXI
GPIO (`XPAR_AXI_GPIO_0`, base `0xb0000000`, bit `0x01`). Only **R5c0** drives it
(heartbeat). **R5c1 has no LED**: a `SET_LED` request to R5c1 is accepted as an
ACK no-op and its status LED fields read zero.

## Per-core identity

`kr260demo::CoreConfig` (resolved at compile time from `XPAR_CPU_ID`) carries
the communication identity; LED parameters are constructor arguments to R5c0's
`LedController`.

| Core  | Service name      | LED            | Heartbeat | Shared-mem PA |
|-------|-------------------|----------------|-----------|---------------|
| R5c0  | `mncos-r5c0-ctrl` | UF2_LED (`0x01`) | 1000 ms | `0x09860000`  |
| R5c1  | `mncos-r5c1-ctrl` | none           | —         | `0x09e60000`  |

Service names and shared-mem regions match the device-tree reserved-memory
contract and what the APU utility looks up.

## Wire protocol

`common/include/kr260demo/rpu_control_protocol.h` is the single source of truth
for the on-wire frame and **must stay byte-for-byte identical** to
`KR260Demo_APU/include/kr260demo/rpu_control_protocol.h`. Frame =
`kr260demo_rpu_msg_header` (magic `0x4d525055`, version 1) followed by an
optional payload, max 256 bytes.

## Extending a core

Add new message types by overriding `handle_custom()` in that core's service
subclass (e.g. `R5c0Service`):

```cpp
protected:
    bool handle_custom(const kr260demo_rpu_msg_header &req,
                       const void *payload, std::uint16_t len,
                       std::uint32_t src) override {
        if (req.type == MY_MSG_FOO) {
            /* ... */
            send_response(&req, src, MY_MSG_FOO_ACK,
                          KR260_RPU_STATUS_OK, nullptr, 0);
            return true;
        }
        return false;   // base replies BAD_TYPE
    }
```

## Building

Both cores are standard Vitis CMake/Ninja projects. Sources and include dirs
are wired through each core's `src/UserConfig.cmake` (`USER_COMPILE_SOURCES` /
`USER_INCLUDE_DIRECTORIES`); only R5c0 lists `led_controller.cpp`. The BSP must
provide `open_amp`, `metal` and `xilmailbox` (already enabled).

> Note: each core's `src/CMakeLists.txt` intentionally does **not** force
> `CMAKE_C_COMPILER` to the C++ compiler — the `common/openamp` glue is C and
> must be compiled as C. The CXX linker is still selected because the
> application sources are C++.

## Testing from the APU

```
apu-rpu-ctl ping   all          # both pong
apu-rpu-ctl status all          # R5c0 heartbeat rises; R5c1 heartbeat=0, led=off
apu-rpu-ctl led    r5c0 heartbeat   # UF2_LED blinks
apu-rpu-ctl led    all  on          # both ACK (R5c1 no-op)
```
