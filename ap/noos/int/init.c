/*
 * init.c: ����һЩ��ʼ��
 */ 

#include "s3c24xx.h"

/*
 * LED1,LED2,LED4��ӦGPB5��GPB6��GPB7��GPB8
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
 * K1,K2,K3,K4��ӦGPG0��GPG3��GPG5��GPG6
 * �ж�����ΪEINT8��EINT11��EINT13��EINT14
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
 * �ر�WATCHDOG������CPU�᲻������
 */
void disable_watch_dog(void)
{
    WTCON = 0;  // �ر�WATCHDOG�ܼ򵥣�������Ĵ���д0����
}

void init_led(void)
{
	// LED1,LED2,LED3,LED4��Ӧ��4��������Ϊ���
	GPBCON &= ~(GPB5_msk | GPB6_msk | GPB7_msk | GPB8_msk);
	GPBCON |= GPB5_out | GPB6_out | GPB7_out | GPB8_out;
}

/*
 * ��ʼ��GPIO����Ϊ�ⲿ�ж�
 * GPIO���������ⲿ�ж�ʱ��Ĭ��Ϊ�͵�ƽ������IRQ��ʽ(��������INTMOD)
 */ 
void init_irq( )
{
	// K1,K2,K3,K4��Ӧ��4��������Ϊ�жϹ���
	GPGCON &= ~(GPG0_msk | GPG3_msk | GPG5_msk | GPG6_msk);
	GPGCON |= GPG0_int | GPG3_int | GPG5_int | GPG6_int;

    // ����EINT8,11,13,14����Ҫ��EINTMASK�Ĵ�����ʹ����
    EINTMASK &= ~((1<<8) | (1<<11) | (1<<13) | (1<<14));
            
    /*
     * ��2440�ֲ�Figure 14-2. Priority Generating Block
     * EINT8~EINT23�����ȼ���һ����
     * ���Բ�������PRIORITY��
     */
//    PRIORITY = (PRIORITY & ((~0x01) | ~(0x3<<7)));

    // bit5��EINT8~EINT23���ܿ���
    INTMSK   &= ~(1<<5);
}




