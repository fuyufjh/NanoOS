#include "common.h"
#include "string.h"

int int_to_dec(int x, char* str);
int int_to_hex(unsigned x, char* str);

void vfprintf(void (*printer)(char), const char *ctl, void **args) {
	const char *str = ctl;
    int i=0;
    for (;*str != '\0'; str++)
    {
        if (*str != '%')
            printer(*str);
        else
        {
            char buffer[13];
            char *p;
            switch (*(++str))
            {
            case 'd':
                int_to_dec((int)args[i++],buffer);
                p=&buffer[0];
                for (; *p != '\0'; p++) printer(*p);
                break;
            case 'x':
                int_to_hex((int)args[i++],buffer);
                p=&buffer[0];
                for (; *p != '\0'; p++) printer(*p);
                break;
            case 's':
                p=(char*)args[i++];
                for (; *p != '\0'; p++) printer(*p);
                break;
            case 'c':
                printer((char)(int)args[i++]);
            }
        }
    }
}

extern void serial_printc(char);

/* __attribute__((__noinline__))  here is to disable inlining for this function to avoid some optimization problems for gcc 4.7 */
void __attribute__((__noinline__))
printk(const char *ctl, ...) {
	void **args = (void **)&ctl + 1;
	vfprintf(serial_printc, ctl, args);
}


int int_to_dec(int x, char* str)
{
    if (x==0)
    {
        str[0]='0';
        str[1]='\0';
        return 1;
    }
    if (x==0x80000000)
    {
        strcpy(str,"-2147483648");
        return 11;
    }
    int n=0;
    int digit[12];
    int symbol=0;
    if (x<0)
    {
        x=-x;
        symbol=1;
    }
    int i;
    for (i=0;x;i++)
    {
        digit[i]=x%10;
        x /= 10;
        n++;
    }
    char* ptr= &(str[0]);
    if (symbol) *(ptr++)='-';
    for (i=n-1;i>=0;i--)
    {
        *(ptr++) = (char)digit[i]+'0';
    }
    *ptr='\0';
    return symbol?n+1:n;
}

int int_to_hex(unsigned x, char* str)
{
    if (x==0)
    {
        str[0]='0';
        str[1]='\0';
        return 1;
    }
    char t[8];
    char* ptr = &t[0];
    while (x)
    {
        unsigned d=x & 0xf;
        if (d<10) *(ptr++)=(char)d+'0';
        else if (d<16) *(ptr++)=(char)d-10+'A';
        x >>= 4;
    }
    ptr--;
    int i=0;
    for (i=0;ptr>=t;ptr--)
    {
        str[i++]=*ptr;
    }
    str[i]='\0';
    return i;
}

