#include <stdio.h>
#include "serial.h"
#include "lcdlib.h"
#include "s3c24xx.h"

int main()
{
    char c;
    
    uart0_init();   // ������115200��8N1(8������λ����У��λ��1��ֹͣλ)
    
    while (1)
    {
        printf("\r\n##### Test TFT LCD #####\r\n");
        printf("[1] TFT320240 8Bit\n\r");
        printf("[2] TFT320240 16Bit\n\r");
        printf("Enter your selection: ");

        c = getc();
        printf("%c\n\r", c);
        switch (c)
        {
            case '1':
            {
                Test_Lcd_Tft_8Bit_320240();
                break;
            }
            
            case '2':
            {
                Test_Lcd_Tft_16Bit_320240();
                break;
            }

            
            default: 
                break;
        }
    }
    
    return 0;
}
