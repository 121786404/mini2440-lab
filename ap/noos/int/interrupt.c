#include "s3c24xx.h"

void EINT_Handle()
{
    unsigned long oft = INTOFFSET;  /* oftӦ��Ϊ5, INTMSK��bit5��EINT8~EINT23���ܿ��� */
    unsigned long val = EINTPEND;

	/*
	 * K1,K2,K3,K4��ӦGPG0��GPG3��GPG5��GPG6
	 *            �� EINT8,EINT11,EINT13,EINT14
	 *            ���ǹ���INTMSK��bit5            
	 *            oft����5(��ӦINTMSK�Ĵ���)
	 * ��Ҫ��EINTPENDȷ���Ƿ��������ĸ��ж�
	 */

	GPBDAT |= (0xF<<5);   // ����LEDϨ��

	if (val & (1<<8))
	{
        // K1������
        GPBDAT &= ~(1<<5);      // LED1����
	}

	if (val & (1<<11))
	{
		// K2������
        GPBDAT &= ~(1<<6);      // LED2����
	}

	if (val & (1<<13))
	{
        // K3������
        GPBDAT &= ~(1<<7);      // LED3����
    }

	if (val & (1<<14))
	{
        // K4������
            GPBDAT &= ~(1<<8);      // LED4����
    }

    //���ж�
    EINTPEND = (1<<8) | (1<<11) | (1<<13) | (1<<14);
    
    SRCPND = 1<<oft;
    INTPND = 1<<oft;
}
