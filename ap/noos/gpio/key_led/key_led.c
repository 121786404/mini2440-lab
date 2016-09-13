#define GPGCON      (*(volatile unsigned long *)0x56000060)
#define GPGDAT      (*(volatile unsigned long *)0x56000064)

#define GPBCON      (*(volatile unsigned long *)0x56000010)
#define GPBDAT      (*(volatile unsigned long *)0x56000014)

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
 */
#define GPG0_in     (0<<(0*2))
#define GPG3_in     (0<<(3*2))
#define GPG5_in     (0<<(5*2))
#define GPG6_in     (0<<(6*2))

#define GPG0_msk    (3<<(0*2))
#define GPG3_msk    (3<<(3*2))
#define GPG5_msk    (3<<(5*2))
#define GPG6_msk    (3<<(6*2))

int main()
{
        unsigned long dwDat;
        // LED1,LED2,LED3,LED4��Ӧ��4��������Ϊ���
        GPBCON &= ~(GPB5_msk | GPB6_msk | GPB7_msk | GPB8_msk);
        GPBCON |= GPB5_out | GPB6_out | GPB7_out | GPB8_out;
        
        // K1,K2,K3,K4��Ӧ��4��������Ϊ����
        GPGCON &= ~(GPG0_msk | GPG3_msk | GPG5_msk | GPG6_msk);
        GPGCON |= GPG0_in | GPG3_in | GPG5_in | GPG6_in;


        while(1){
            //��KnΪ0(��ʾ����)������LEDnΪ0(��ʾ����)
            dwDat = GPGDAT;             // ��ȡGPF�ܽŵ�ƽ״̬
        
            if (dwDat & (1<<0))        // K1û�а���
                GPBDAT |= (1<<5);       // LED1Ϩ��
            else    
                GPBDAT &= ~(1<<5);      // LED1����
                
            if (dwDat & (1<<3))         // K2û�а���
                GPBDAT |= (1<<6);       // LED2Ϩ��
            else    
                GPBDAT &= ~(1<<6);      // LED2����
    
			if (dwDat & (1<<5)) 	   // K3û�а���
				GPBDAT |= (1<<7);		// LED3Ϩ��
			else	
				GPBDAT &= ~(1<<7);		// LED3����
				
			if (dwDat & (1<<6)) 		// K4û�а���
				GPBDAT |= (1<<8);		// LED4Ϩ��
			else	
				GPBDAT &= ~(1<<8);		// LED4����
    }

    return 0;
}
