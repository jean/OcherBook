/*
 * Copyright (c) 2013, Chuck Coffing
 * OcherBook is released under the BSD 2-clause license.  See COPYING.
 */

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "clc/support/Logger.h"
#include "ocher/ocher.h"
#include "ocher/device/Device.h"

#define LOG_NAME "ocher.dev"


clc::Buffer Device::getMac()
{
	clc::Buffer mac;
	// TODO
	return mac;
}

clc::Buffer Device::getIp()
{
	clc::Buffer ip;
	// TODO
	return ip;
}

clc::Buffer Device::getVersion()
{
	clc::Buffer version;
	version.format("%d.%d.%d", OCHER_MAJOR, OCHER_MINOR, OCHER_PATCH);
	return version;
}

clc::Buffer Device::getBuildDate()
{
	clc::Buffer date(__DATE__);
	return date;
}

void Device::sleep()
{
#ifdef __linux__
//    if lsmod | grep -q sdio_wifi_pwr ; then
//        wlarm_le -i eth0 down
//        ifconfig eth0 down
//        /sbin/rmmod -r dhd
//        /sbin/rmmod -r sdio_wifi_pwr
//    fi

// "echo 1 > /sys/power/state-extended"
	const char* pwr = "/sys/power/state";
	int fd = open(pwr, O_WRONLY);
	if (fd == -1) {
		clc::Log::error(LOG_NAME, "%s: %s", pwr, strerror(errno));
	} else {
		write(fd, "mem", 3);
		close(fd);
		::sleep(1);  // TODO seems hackish, but don't want to return before enters sleep state
	}
#else
	::sleep(10);  // TODO for testing
#endif
}
