//main.cpp
#include "uart.h"
#include <iostream>
#include <ostream>
#include <semaphore.h>

int main(void)
{
    Uart uart;
    int uart2_fd;
    int num;

    char rbuff[20];
    rbuff[0]='#';
    rbuff[1]='o';
    rbuff[2]='f';
    rbuff[3]='f';
    rbuff[4]='\n';


    uart2_fd =uart.uartOpen(UART2,O_RDWR | O_NONBLOCK | O_NOCTTY | O_NDELAY,BAUDRATE2);
    if(uart2_fd < 0)
        cout<<"uart init error"<<endl;
    else cout<<"uart init ready"<<endl;


    while(1)
    {
        sleep(1);
        num = uart.uartRead(uart2_fd,rbuff,20);
        if(num>0)
        {
            for(int i = 0;i < num;i++)
            {
                rbuff[i] = rbuff[i]+1;
            }
            uart.uartWrite(uart2_fd,rbuff,num);
            cout<<"uart write done"<<endl;
        }
    }
    return 0;
}