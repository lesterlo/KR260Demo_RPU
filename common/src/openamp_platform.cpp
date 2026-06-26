/*
 * RAII wrapper implementation. See openamp_platform.hpp.
 */

#include "kr260demo/openamp_platform.hpp"

#include <openamp/open_amp.h>
#include <openamp/virtio.h>

#include "platform_info.h"

namespace kr260demo {

OpenAmpPlatform::~OpenAmpPlatform()
{
	if (platform_) {
		platform_cleanup(platform_);
		platform_ = nullptr;
	}
}

bool OpenAmpPlatform::init()
{
	if (platform_)
		return true;

	if (platform_init(0, nullptr, &platform_) != 0) {
		platform_ = nullptr;
		return false;
	}
	return true;
}

rpmsg_device *OpenAmpPlatform::create_rpmsg_vdev()
{
	if (!platform_)
		return nullptr;

	return platform_create_rpmsg_vdev(platform_, 0, VIRTIO_DEV_DEVICE,
					  nullptr, nullptr);
}

void OpenAmpPlatform::release_rpmsg_vdev(rpmsg_device *rpdev)
{
	if (platform_ && rpdev)
		platform_release_rpmsg_vdev(rpdev, platform_);
}

int OpenAmpPlatform::poll_until_reset(rpmsg_device *rpdev)
{
	if (!platform_ || !rpdev)
		return -1;

	struct rproc_plat_info arg;
	arg.rpdev = rpdev;
	arg.rproc = static_cast<struct remoteproc *>(platform_);
	return platform_poll_on_vdev_reset(&arg);
}

} // namespace kr260demo
