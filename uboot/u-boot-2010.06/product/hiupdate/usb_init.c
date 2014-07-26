static int usb_stor_init(void)
{
	int ret = -1;

	if (usb_stop() < 0) {
		debug("usb_stop failed\n");
		return ret;
	}

	if (usb_init_debug() < 0) {
		debug("usb_init_debug failed\n");
		return ret;
	}

	wait_ms(1000);
	if (usb_init() < 0) {
		debug("usb_init failed!\n");
		return ret;
	}

	/*
	 * check whether a storage device is attached (assume that it's
	 * a USB memory stick, since nothing else should be attached).
	 */
	ret = usb_stor_scan(0);
	if (-1 == ret)
		debug("No USB device found. Not initialized!\n");

	return ret;
}

static void usb_stor_exit(void)
{
	usb_stop();
}

