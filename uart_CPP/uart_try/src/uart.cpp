//uart.c
#include "uart.h"

int Uart::uart1_only_fd = 0;
int Uart::uart2_only_fd = 0;
bool Uart::uart1_inited_flag = false;
bool Uart::uart2_inited_flag = false;

Uart::Uart()
{

}

int Uart::uartInit(int fd, speed_t buadrate)
{
    struct termios opt;

    tcgetattr(fd,&opt);

    cfsetispeed(&opt,buadrate);
    cfsetospeed(&opt,buadrate);   

    opt.c_cflag  |= CLOCAL | CREAD;
    opt.c_cflag &= ~CRTSCTS;
    opt.c_cflag &= ~CSIZE;
    opt.c_cflag |= CS8;
    opt.c_cflag &= ~PARENB;
    opt.c_cflag &= ~CSTOPB;
    opt.c_iflag &= ~INPCK;
    opt.c_iflag &= ~(ICRNL|BRKINT|ISTRIP);
    opt.c_iflag &= ~(IXON|IXOFF|IXANY);
    opt.c_oflag &= ~OPOST;

    opt.c_cflag = ~(ICANON | ECHO | ECHOE | ISIG);

    tcflush(fd,TCIFLUSH);
    if (tcsetattr(fd,TCSANOW,&opt) != 0)
    {
        return -1;
    }
    return 0;
}

int Uart::uartOpen(int port, int flag, speed_t buadrate)
{
    if(port == UART1)
    {
        if(uart1_inited_flag == false)
        {
            uart1_only_fd = open(UART1_DEV, flag);
            if (uart1_only_fd < 0)
            {
                return -1;
            }
            else
            {
                uartInit(uart1_only_fd,buadrate);
                uart1_inited_flag = true;
                return uart1_only_fd;
            }
        }
        else
        {
            return uart1_only_fd;
        }
    }
    else if (port == UART2)
    {
        if(uart2_inited_flag == false)
        {
            uart2_only_fd = open(UART2_DEV, flag);
            if (uart2_only_fd < 0)
            {
                return -1;
            }
            else
            {
                uartInit(uart2_only_fd,buadrate);
                uart2_inited_flag = true;
                return uart2_only_fd;
            }
        }
        else
        {
            return uart2_only_fd;
        }
    }
}

int Uart::uartWrite(int fd ,char *data,int num)
{
    int ret = -1;
    ret = write(fd,data,num);
    return ret;
}
int Uart::uartRead(int fd ,char *data,int num)
{
    int ret = -1;
    int i=0;
    char buf[1024] = {0};
    if(num > 1024)
    {
        ret = read(fd ,buf,1024);
    }
    else 
    {
        ret = read(fd, buf, num);
    }
    
    for (i=0;i<num;i++)
    {
        data[i]=buf[i];
    }
    return ret;
}

void Uart::uartClose(int fd)
{
    if(fd > 0)
        close(fd);
}

Uart::~Uart()
{
    
}