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
        printf("[1] TFT240320 8Bit\n\r");
        printf("[2] TFT240320 16Bit\n\r");
        printf("Enter your selection: ");

        c = getc();
        printf("%c\n\r", c);
        switch (c)
        {
            case '1':
            {
                Test_Lcd_Tft_8Bit_240320();
                break;
            }
            
            case '2':
            {
                Test_Lcd_Tft_16Bit_240320();
                break;
            }

            
            default: 
                break;
        }
    }
    
    return 0;
}
