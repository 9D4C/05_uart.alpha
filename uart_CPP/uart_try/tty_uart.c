/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <getopt.h>
#include <linux/serial.h>
#define termios asmtermios
#include <asm/termios.h>
#undef termios
#include <termios.h>

extern int ioctl(int d, int request, ...);

static const char *device = "/dev/ttyTHS2";
static int speed = 115200;
static int hardflow = 0;
static int verbose = 0;
static FILE *fp;

/**
 * libtty_setcustombaudrate - set baud rate of tty device
 * @fd: device handle
 * @speed: baud rate to set
 *
 * The function return 0 if success, or -1 if fail.
 */
static int libtty_setcustombaudrate(int fd, int baudrate)
{
	struct termios2 tio;

	if (ioctl(fd, TCGETS2, &tio)) {
		perror("TCGETS2");
		return -1;
	}

	tio.c_cflag &= ~CBAUD;
	tio.c_cflag |= BOTHER;
	tio.c_ispeed = baudrate;
	tio.c_ospeed = baudrate;

	if (ioctl(fd, TCSETS2, &tio)) {
		perror("TCSETS2");
		return -1;
	}

	return 0;
}

/**
 * libtty_setopt - config tty device
 * @fd: device handle
 * @speed: baud rate to set
 * @databits: data bits to set
 * @stopbits: stop bits to set
 * @parity: parity to set
 * @hardflow: hardflow to set
 *
 * The function return 0 if success, or -1 if fail.
 */
static int libtty_setopt(int fd, int speed, int databits, int stopbits, char parity, char hardflow)
{
	struct termios newtio;
	struct termios oldtio;
	int i;

	bzero(&newtio, sizeof(newtio));
	bzero(&oldtio, sizeof(oldtio));

	if (tcgetattr(fd, &oldtio) != 0) {
		perror("tcgetattr");
		return -1;
	}
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	/* set data bits */
	switch (databits) {
	case 5:
		newtio.c_cflag |= CS5;
		break;
	case 6:
		newtio.c_cflag |= CS6;
		break;
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "unsupported data size\n");
		return -1;
	}

	/* set parity */
	switch (parity) {
	case 'n':
	case 'N':
		newtio.c_cflag &= ~PARENB; /* Clear parity enable */
		newtio.c_iflag &= ~INPCK;  /* Disable input parity check */
		break;
	case 'o':
	case 'O':
		newtio.c_cflag |= (PARODD | PARENB); /* Odd parity instead of even */
		newtio.c_iflag |= INPCK;	     /* Enable input parity check */
		break;
	case 'e':
	case 'E':
		newtio.c_cflag |= PARENB;  /* Enable parity */
		newtio.c_cflag &= ~PARODD; /* Even parity instead of odd */
		newtio.c_iflag |= INPCK;   /* Enable input parity check */
		break;
	case 'm':
	case 'M':
		newtio.c_cflag |= PARENB; /* Enable parity */
		newtio.c_cflag |= CMSPAR; /* Stick parity instead */
		newtio.c_cflag |= PARODD; /* Even parity instead of odd */
		newtio.c_iflag |= INPCK;  /* Enable input parity check */
		break;
	case 's':
	case 'S':
		newtio.c_cflag |= PARENB;  /* Enable parity */
		newtio.c_cflag |= CMSPAR;  /* Stick parity instead */
		newtio.c_cflag &= ~PARODD; /* Even parity instead of odd */
		newtio.c_iflag |= INPCK;   /* Enable input parity check */
		break;
	default:
		fprintf(stderr, "unsupported parity\n");
		return -1;
	}

	/* set stop bits */
	switch (stopbits) {
	case 1:
		newtio.c_cflag &= ~CSTOPB;
		break;
	case 2:
		newtio.c_cflag |= CSTOPB;
		break;
	default:
		perror("unsupported stop bits\n");
		return -1;
	}

	if (hardflow)
		newtio.c_cflag |= CRTSCTS;
	else
		newtio.c_cflag &= ~CRTSCTS;

	newtio.c_cc[VTIME] = 10; /* Time-out value (tenths of a second) [!ICANON]. */
	newtio.c_cc[VMIN] = 0;	 /* Minimum number of bytes read at once [!ICANON]. */

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &newtio) != 0) {
		perror("tcsetattr");
		return -1;
	}

	/* set tty speed */
	if (libtty_setcustombaudrate(fd, speed) != 0) {
		perror("setbaudrate");
		return -1;
	}

	return 0;
}

/**
 * libtty_open - open tty device
 * @devname: the device name to open
 *
 * In this demo device is opened blocked, you could modify it at will.
 */
static int libtty_open(const char *devname)
{
	int fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY);
	int flags = 0;

	if (fd < 0) {
		perror("open device failed");
		return -1;
	}

	flags = fcntl(fd, F_GETFL, 0);
	flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		printf("fcntl failed.\n");
		return -1;
	}

	if (isatty(fd) == 0) {
		printf("not tty device.\n");
		return -1;
	} else
		printf("tty device test ok.\n");

	return fd;
}

/**
 * libtty_sendbreak - uart send break
 * @fd: file descriptor of tty device
 *
 * Description:
 *  tcsendbreak() transmits a continuous stream of zero-valued bits for a specific duration, if the terminal
 *  is using asynchronous serial data transmission. If duration is zero, it transmits zero-valued bits for
 *  at least 0.25 seconds, and not more that 0.5 seconds. If duration is not zero, it sends zero-valued bits
 *  for some implementation-defined length of time.
 *
 *  If the terminal is not using asynchronous serial data transmission, tcsendbreak() returns without taking
 *  any action.
 */
static int libtty_sendbreak(int fd)
{
	return tcsendbreak(fd, 0);
}

/**
 * libtty_write - write data to uart
 * @fd: file descriptor of tty device
 *
 * The function return the number of bytes written if success, others if fail.
 */
static int libtty_write(int fd)
{
	int nwrite;
	char buf[64]={0xff,0xfe};
	int i;
	char data[4]={'o','f','f'};
	
	memset(buf+2, 0x00, 62);
	memcpy(buf+2, data, 3*sizeof(char));
	
	buf[strlen(buf)]=0xa;
	buf[strlen(buf)]=0xd;


	nwrite = write(fd, buf, strlen(buf));
	printf("wrote %d bytes already.\n", nwrite);
	

	return nwrite;
}

static void sig_handler(int signo)
{
	printf("capture sign no:%d\n", signo);
	if (fp != NULL) {
		fflush(fp);
		fsync(fileno(fp));
		fclose(fp);
	}
	exit(0);
}

int main(int argc, char *argv[])
{
	int fd;
	int ret;
	//char c;


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

	// test for write and read 
	// while (1) {
	// 	if (c != '\n')
	// 		printf("press b to send break,w to send a string,r to read data once,q to quit.\n");
	// 	scanf("%c", &c);
	// 	if (c == 'q')
	// 		break;
	// 	switch (c) {
	// 	case 'b':
	// 		ret = libtty_sendbreak(fd);
	// 		if (ret)
	// 			printf("libtty_sendbreak error: %d\n", ret);
	// 		break;
	// 	case 'w':
	// 		ret = libtty_write(fd);
	// 		if (ret <= 0)
	// 			printf("libtty_write error: %d\n", ret);
	// 		break;
	// 	default:
	// 		break;
	// 	}
	// 	}

}
