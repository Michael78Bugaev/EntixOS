#include <port_based.h>

unsigned char   inb(unsigned short port)
{
    unsigned char result;
	__asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return (result);
}


void    outb(unsigned short port, unsigned char data)
{
	__asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}


unsigned char   inw(unsigned short port)
{
    unsigned short result;
    __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return (result);
}


void outw(unsigned short port, unsigned short data)
{
    __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}