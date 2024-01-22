#ifndef UART_H
#define UART_H

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h> 

#define UART1_DEV "/dev/ttyS0"
#define UART2_DEV "/dev/ttyTHS2"

#define UART1 1
#define UART2 2

#define BAUDRATE1 B115200
#define BAUDRATE2 B115200

using namespace std;

class Uart
{
    public :
        ~Uart();
         Uart();
        int uartOpen(int port, int flag, speed_t buadrate);
        int uartWrite (int fd, char *data, int num );
        int uartRead  (int fd, char *data, int num );
        void uartClose(int fd);
    private :
        int uartInit(int fd, speed_t buadrate);

        static int uart1_only_fd;
        static int uart2_only_fd;
        static bool uart1_inited_flag;
        static bool uart2_inited_flag;
};

#endif//uart.h
