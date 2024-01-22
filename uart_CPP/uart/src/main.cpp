#include "tty_uart.hpp"
#include <iostream>
#include <ostream>
#include <semaphore.h>

int main(int argc, char *argv[])
{
	signal(SIGINT, sig_handler);

	fd = libtty_open(device);
	if (fd < 0) {
		printf("libtty_open: %s error.\n", device);
		exit(0);
	}

	ret = libtty_setopt(fd, speed, 8, 1, 'n', hardflow);
	if (ret != 0) {
		printf("libtty_setopt error.\n");
		exit(0);
	}

	//test for write and read 
	while (1) {
		if (c != '\n')
			printf("press b to send break,w to send a string,r to read data once,q to quit.\n");
		scanf("%c", &c);
		if (c == 'q')
			break;
		switch (c) {
		case 'b':
			ret = libtty_sendbreak(fd);
			if (ret)
				printf("libtty_sendbreak error: %d\n", ret);
			break;
		case 'w':
			ret = libtty_write(fd);
			if (ret <= 0)
				printf("libtty_write error: %d\n", ret);
			break;
		default:
			break;
		}
		}

}