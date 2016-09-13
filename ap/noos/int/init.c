/*
 * init.c: 进行一些初始化
 */ 

#include "s3c24xx.h"

/*
 * LED1,LED2,LED4对应GPB5、GPB6、GPB7、GPB8
 */
#define	GPB5_out	(1<<(5*2))
#define	GPB6_out	(1<<(6*2))
#define	GPB7_out	(1<<(7*2))
#define	GPB8_out	(1<<(8*2))

#define	GPB5_msk	(3<<(5*2))
#define	GPB6_msk	(3<<(6*2))
#define	GPB7_msk	(3<<(7*2))
#define	GPB8_msk	(3<<(8*2))

/*
 * K1,K2,K3,K4对应GPG0，GPG3，GPG5，GPG6
 * 中断引脚为EINT8，EINT11，EINT13，EINT14
 */
#define GPG0_int     (0x2<<(0*2))
#define GPG3_int     (0x2<<(3*2))
#define GPG5_int     (0x2<<(5*2))
#define GPG6_int     (0x2<<(6*2))

#define GPG0_msk    (3<<(0*2))
#define GPG3_msk    (3<<(3*2))
#define GPG5_msk    (3<<(5*2))
#define GPG6_msk    (3<<(6*2))

/*
 * 关闭WATCHDOG，否则CPU会不断重启
 */
void disable_watch_dog(void)
{
    WTCON = 0;  // 关闭WATCHDOG很简单，往这个寄存器写0即可
}

void init_led(void)
{
	// LED1,LED2,LED3,LED4对应的4根引脚设为输出
	GPBCON &= ~(GPB5_msk | GPB6_msk | GPB7_msk | GPB8_msk);
	GPBCON |= GPB5_out | GPB6_out | GPB7_out | GPB8_out;
}

/*
 * 初始化GPIO引脚为外部中断
 * GPIO引脚用作外部中断时，默认为低电平触发、IRQ方式(不用设置INTMOD)
 */ 
void init_irq( )
{
	// K1,K2,K3,K4对应的4根引脚设为中断功能
	GPGCON &= ~(GPG0_msk | GPG3_msk | GPG5_msk | GPG6_msk);
	GPGCON |= GPG0_int | GPG3_int | GPG5_int | GPG6_int;

    // 对于EINT8,11,13,14，需要在EINTMASK寄存器中使能它
    EINTMASK &= ~((1<<8) | (1<<11) | (1<<13) | (1<<14));
            
    /*
     * 看2440手册Figure 14-2. Priority Generating Block
     * EINT8~EINT23的优先级是一样的
     * 所以不用设置PRIORITY了
     */
//    PRIORITY = (PRIORITY & ((~0x01) | ~(0x3<<7)));

    // bit5是EINT8~EINT23的总开关
    INTMSK   &= ~(1<<5);
}




