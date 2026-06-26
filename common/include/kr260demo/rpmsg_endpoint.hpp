#ifndef KR260DEMO_RPMSG_ENDPOINT_HPP
#define KR260DEMO_RPMSG_ENDPOINT_HPP

/*
 * RAII wrapper around a single rpmsg endpoint plus a small C++ adapter so that
 * incoming messages are delivered to a virtual MessageSink instead of a raw C
 * callback. The endpoint is destroyed automatically on scope exit.
 */

#include <cstddef>
#include <cstdint>

#include <openamp/rpmsg.h>

namespace kr260demo {

/* Implemented by whoever wants to receive rpmsg traffic on an endpoint. */
class MessageSink {
public:
	virtual ~MessageSink() = default;

	/*
	 * Called from the rpmsg receive context. Return RPMSG_SUCCESS (0) to
	 * keep the buffer cycling; the data pointer is only valid for the
	 * duration of the call.
	 */
	virtual int on_message(const void *data, std::size_t len,
			       std::uint32_t src) = 0;

	/* Optional: notified when the remote tears the endpoint down. */
	virtual void on_unbind() {}
};

class RpmsgEndpoint {
public:
	RpmsgEndpoint() = default;
	~RpmsgEndpoint();

	RpmsgEndpoint(const RpmsgEndpoint &) = delete;
	RpmsgEndpoint &operator=(const RpmsgEndpoint &) = delete;

	/*
	 * Create a named endpoint on the given rpmsg device. Messages are
	 * routed to sink. Returns true on success.
	 */
	bool create(rpmsg_device *rdev, const char *service_name,
		    MessageSink &sink);

	void destroy();

	/* Send len bytes to dst. Returns the openamp status (>= 0 on success). */
	int send_to(const void *data, std::size_t len, std::uint32_t dst);

	bool valid() const { return sink_ != nullptr; }

private:
	static int rx_trampoline(rpmsg_endpoint *ept, void *data,
				 std::size_t len, std::uint32_t src, void *priv);
	static void unbind_trampoline(rpmsg_endpoint *ept);

	rpmsg_endpoint ept_{};
	MessageSink *sink_ = nullptr;
};

} // namespace kr260demo

#endif /* KR260DEMO_RPMSG_ENDPOINT_HPP */
