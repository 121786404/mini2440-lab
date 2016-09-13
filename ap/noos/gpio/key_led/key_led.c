#define GPGCON      (*(volatile unsigned long *)0x56000060)
#define GPGDAT      (*(volatile unsigned long *)0x56000064)

#define GPBCON      (*(volatile unsigned long *)0x56000010)
#define GPBDAT      (*(volatile unsigned long *)0x56000014)

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
        // LED1,LED2,LED3,LED4对应的4根引脚设为输出
        GPBCON &= ~(GPB5_msk | GPB6_msk | GPB7_msk | GPB8_msk);
        GPBCON |= GPB5_out | GPB6_out | GPB7_out | GPB8_out;
        
        // K1,K2,K3,K4对应的4根引脚设为输入
        GPGCON &= ~(GPG0_msk | GPG3_msk | GPG5_msk | GPG6_msk);
        GPGCON |= GPG0_in | GPG3_in | GPG5_in | GPG6_in;


        while(1){
            //若Kn为0(表示按下)，则令LEDn为0(表示点亮)
            dwDat = GPGDAT;             // 读取GPF管脚电平状态
        
            if (dwDat & (1<<0))        // K1没有按下
                GPBDAT |= (1<<5);       // LED1熄灭
            else    
                GPBDAT &= ~(1<<5);      // LED1点亮
                
            if (dwDat & (1<<3))         // K2没有按下
                GPBDAT |= (1<<6);       // LED2熄灭
            else    
                GPBDAT &= ~(1<<6);      // LED2点亮
    
			if (dwDat & (1<<5)) 	   // K3没有按下
				GPBDAT |= (1<<7);		// LED3熄灭
			else	
				GPBDAT &= ~(1<<7);		// LED3点亮
				
			if (dwDat & (1<<6)) 		// K4没有按下
				GPBDAT |= (1<<8);		// LED4熄灭
			else	
				GPBDAT &= ~(1<<8);		// LED4点亮
    }

    return 0;
}
