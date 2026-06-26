#ifndef KR260DEMO_OPENAMP_PLATFORM_HPP
#define KR260DEMO_OPENAMP_PLATFORM_HPP

/*
 * Thin RAII wrapper around the AMD OpenAMP/remoteproc platform glue
 * (common/openamp/platform_info.c). It owns the lifecycle of the remoteproc
 * instance and the rpmsg virtio device so that callers do not have to manage
 * the matching cleanup calls by hand.
 *
 * This layer is deliberately free of exceptions and dynamic dispatch so it is
 * safe to use from a bare-metal / FreeRTOS context. Failures are reported as
 * return values.
 */

#include <cstddef>
#include <cstdint>

struct rpmsg_device;

namespace kr260demo {

class OpenAmpPlatform {
public:
	OpenAmpPlatform() = default;
	~OpenAmpPlatform();

	OpenAmpPlatform(const OpenAmpPlatform &) = delete;
	OpenAmpPlatform &operator=(const OpenAmpPlatform &) = delete;

	/* Initialise libmetal + remoteproc. Returns true on success. */
	bool init();

	/*
	 * Create the rpmsg virtio device used to talk to the remote
	 * (Linux/APU) side. Returns nullptr on failure. The returned device is
	 * owned by the platform; release it with release_rpmsg_vdev().
	 */
	rpmsg_device *create_rpmsg_vdev();

	/* Tear down a device previously returned by create_rpmsg_vdev(). */
	void release_rpmsg_vdev(rpmsg_device *rpdev);

	/*
	 * Block servicing the given rpmsg device until the remote tears the
	 * vdev down (driver no longer OK). Returns 0 on a clean reset.
	 */
	int poll_until_reset(rpmsg_device *rpdev);

	bool initialised() const { return platform_ != nullptr; }
	void *raw() const { return platform_; }

private:
	void *platform_ = nullptr;
};

} // namespace kr260demo

#endif /* KR260DEMO_OPENAMP_PLATFORM_HPP */
