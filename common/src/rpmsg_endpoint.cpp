/*
 * RAII rpmsg endpoint implementation. See rpmsg_endpoint.hpp.
 */

#include "kr260demo/rpmsg_endpoint.hpp"

#include <metal/log.h>
#include <openamp/open_amp.h>

namespace kr260demo {

RpmsgEndpoint::~RpmsgEndpoint()
{
	destroy();
}

int RpmsgEndpoint::rx_trampoline(rpmsg_endpoint *ept, void *data,
				 std::size_t len, std::uint32_t src, void *priv)
{
	(void)ept;
	auto *sink = static_cast<MessageSink *>(priv);
	if (!sink)
		return RPMSG_SUCCESS;
	return sink->on_message(data, len, src);
}

void RpmsgEndpoint::unbind_trampoline(rpmsg_endpoint *ept)
{
	auto *sink = static_cast<MessageSink *>(ept->priv);
	if (sink)
		sink->on_unbind();
}

bool RpmsgEndpoint::create(rpmsg_device *rdev, const char *service_name,
			   MessageSink &sink)
{
	if (sink_)
		return false;

	int ret = rpmsg_create_ept(&ept_, rdev, service_name,
				   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
				   &RpmsgEndpoint::rx_trampoline,
				   &RpmsgEndpoint::unbind_trampoline);
	if (ret) {
		ML_ERR("rpmsg_create_ept(%s) failed: %d\r\n", service_name, ret);
		return false;
	}

	/*
	 * The receive callback is invoked with ept->priv; point it at the sink
	 * so the trampoline can recover the C++ object.
	 */
	ept_.priv = &sink;
	sink_ = &sink;
	return true;
}

void RpmsgEndpoint::destroy()
{
	if (sink_) {
		rpmsg_destroy_ept(&ept_);
		sink_ = nullptr;
	}
}

int RpmsgEndpoint::send_to(const void *data, std::size_t len,
			   std::uint32_t dst)
{
	if (!sink_)
		return -1;
	return rpmsg_sendto(&ept_, data, len, dst);
}

} // namespace kr260demo
