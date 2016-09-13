/*
 * FILE: lcdlib.c
 * ʵ��TFT LCD�Ĳ��Ժ���
 */

#include <stdio.h>
#include "lcddrv.h"
#include "framebuffer.h"


/* 
 * ��320x240,8bpp����ʾģʽ����TFT LCD
 */
void Test_Lcd_Tft_8Bit_320240(void)
{
    Lcd_Port_Init();                     // ����LCD����
    Tft_Lcd_Init(MODE_TFT_8BIT_320240);  // ��ʼ��LCD������
    Lcd_PowerEnable(0, 1);               // ����LCD_PWREN��Ч�������ڴ�LCD�ĵ�Դ
    Lcd_EnvidOnOff(1);                   // ʹ��LCD����������ź�

    Lcd_Palette8Bit_Init();     // ��ʼ����ɫ��
    ClearScr(0x0);              // ����
    printf("[TFT 64K COLOR(16bpp) LCD TEST]\n");

    printf("1. Press any key to draw line\n");
    getc();
    DrawLine(0  , 0  , 319, 0  , 0);    // ��ɫΪDEMO256pal[0]
    DrawLine(0  , 0  , 0  , 239, 1);    // ��ɫΪDEMO256pal[1]
    DrawLine(319, 0  , 319, 239, 2);    // ����
    DrawLine(0  , 239, 319, 239, 4);
    DrawLine(0  , 0  , 319, 239, 8);
    DrawLine(319, 0  , 0  , 239, 16);
    DrawLine(160, 0  , 160, 239, 32);
    DrawLine(0  , 120, 319, 120, 64);


    printf("2. Press any key to draw circles\n");
    getc();
    Mire();

    printf("3. Press any key to fill the screem with one color\n");
    getc();
    ClearScr(128);  //  �����ɫͼ����ɫΪDEMO256pal[128]

    printf("4. Press any key to fill the screem by temporary palette\n");
    getc(); 
    ClearScrWithTmpPlt(0x0000ff);       //  �����ɫͼ����ɫΪ��ɫ

    printf("5. Press any key to fill the screem by palette\n");
    getc();
    DisableTmpPlt();            // �ر���ʱ��ɫ��Ĵ���
    ChangePalette(0xffff00);    // �ı�������ɫ��Ϊ��ɫ�������ɫͼ��
    
    printf("6. Press any key stop the testing\n");
    getc();
    Lcd_EnvidOnOff(0);
}

/* 
 * ��320x240,16bpp����ʾģʽ����TFT LCD
 */
void Test_Lcd_Tft_16Bit_320240(void)
{
    Lcd_Port_Init();                     // ����LCD����
    Tft_Lcd_Init(MODE_TFT_16BIT_320240); // ��ʼ��LCD������
    Lcd_PowerEnable(0, 1);               // ����LCD_PWREN��Ч�������ڴ�LCD�ĵ�Դ
    Lcd_EnvidOnOff(1);                   // ʹ��LCD����������ź�

    ClearScr(0x0);  // ��������ɫ
    printf("[TFT 64K COLOR(16bpp) LCD TEST]\n");

    printf("1. Press any key to draw line\n");
    getc();
    DrawLine(0  , 0  , 319, 0  , 0xff0000);     // ��ɫ             
    DrawLine(0  , 0  , 0  , 239, 0x00ff00);     // ��ɫ             
    DrawLine(319, 0  , 319, 239, 0x0000ff);     // ��ɫ             
    DrawLine(0  , 239, 319, 239, 0xffffff);     // ��ɫ             
    DrawLine(0  , 0  , 319, 239, 0xffff00);     // ��ɫ             
    DrawLine(319, 0  , 0  , 239, 0x8000ff);     // ��ɫ             
    DrawLine(160, 0  , 160, 239, 0xe6e8fa);     // ��ɫ             
    DrawLine(0  , 120, 319, 120, 0xcd7f32);     // ��ɫ             

    printf("2. Press any key to draw circles\n");
    getc();
    Mire();

    printf("3. Press any key to fill the screem with one color\n");
    getc();
    ClearScr(0xff0000);             // ��ɫ

    printf("4. Press any key to fill the screem by temporary palette\n");
    getc();
    ClearScrWithTmpPlt(0x0000ff);   // ��ɫ

    printf("5. Press any key stop the testing\n");
    getc();
    Lcd_EnvidOnOff(0);
}


/* 
 * ��640x480,8bpp����ʾģʽ����TFT LCD
 */
void Test_Lcd_Tft_8Bit_640480(void)
{
    Lcd_Port_Init();                     // ����LCD����
    Tft_Lcd_Init(MODE_TFT_8BIT_640480);  // ��ʼ��LCD������
    Lcd_PowerEnable(0, 1);               // ����LCD_PWREN��Ч�������ڴ�LCD�ĵ�Դ
    Lcd_EnvidOnOff(1);                   // ʹ��LCD����������ź�

    Lcd_Palette8Bit_Init();     // ��ʼ����ɫ��
    ClearScr(0x0);              // ��������ɫ
    printf("[TFT 64K COLOR(16bpp) LCD TEST]\n");

    printf("1. Press any key to draw line\n");
    getc();
    DrawLine(0  , 0  , 639, 0  , 0);    // ��ɫΪDEMO256pal[0]
    DrawLine(0  , 0  , 0  , 479, 1);    // ��ɫΪDEMO256pal[1]
    DrawLine(639, 0  , 639, 479, 2);    // ����
    DrawLine(0  , 479, 639, 479, 4);
    DrawLine(0  , 0  , 639, 479, 8);
    DrawLine(639, 0  , 0  , 479, 16);
    DrawLine(320, 0  , 320, 479, 32);
    DrawLine(0  , 240, 639, 240, 64);

    printf("2. Press any key to draw circles\n");
    getc();
    Mire();

    printf("3. Press any key to fill the screem with one color\n");
    getc();
    ClearScr(128);  //  �����ɫͼ����ɫΪDEMO256pal[128]

    printf("4. Press any key to fill the screem by temporary palette\n");
    getc(); 
    ClearScrWithTmpPlt(0x0000ff);       //  �����ɫͼ����ɫΪ��ɫ
    
    printf("5. Press any key to fill the screem by palette\n");
    getc();
    DisableTmpPlt();            // �ر���ʱ��ɫ��Ĵ���
    ChangePalette(0xffff00);    // �ı�������ɫ��Ϊ��ɫ�������ɫͼ��
    
    printf("6. Press any key stop the testing\n");
    getc();
    Lcd_EnvidOnOff(0);
}

/* 
 * ��640x480,16bpp����ʾģʽ����TFT LCD
 */
void Test_Lcd_Tft_16Bit_640480(void)
{
    Lcd_Port_Init();                     // ����LCD����
    Tft_Lcd_Init(MODE_TFT_16BIT_640480); // ��ʼ��LCD������
    Lcd_PowerEnable(0, 1);               // ����LCD_PWREN��Ч�������ڴ�LCD�ĵ�Դ
    Lcd_EnvidOnOff(1);                   // ʹ��LCD����������ź�

    ClearScr(0x0);  // ��������ɫ
    printf("[TFT 64K COLOR(16bpp) LCD TEST]\n");

    printf("1. Press any key to draw line\n");
    getc();
    DrawLine(0  , 0  , 639, 0  , 0xff0000);     // ��ɫ
    DrawLine(0  , 0  , 0  , 479, 0x00ff00);     // ��ɫ
    DrawLine(639, 0  , 639, 479, 0x0000ff);     // ��ɫ
    DrawLine(0  , 479, 639, 479, 0xffffff);     // ��ɫ
    DrawLine(0  , 0  , 639, 479, 0xffff00);     // ��ɫ
    DrawLine(639, 0  , 0  , 479, 0x8000ff);     // ��ɫ
    DrawLine(320, 0  , 320, 479, 0xe6e8fa);     // ��ɫ
    DrawLine(0  , 240, 639, 240, 0xcd7f32);     // ��ɫ

    printf("2. Press any key to draw circles\n");
    getc();
    Mire();

    printf("3. Press any key to fill the screem with one color\n");
    getc();
    ClearScr(0xff0000);             // ��ɫ

    printf("4. Press any key to fill the screem by temporary palette\n");
    getc();
    ClearScrWithTmpPlt(0x0000ff);   // ��ɫ

    printf("5. Press any key stop the testing\n");
    getc();
    Lcd_EnvidOnOff(0);
}

