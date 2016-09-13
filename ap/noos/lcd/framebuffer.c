/*
 * FILE: framebuffer.c
 * ʵ����framebuffer�ϻ��㡢���ߡ���ͬ��Բ�������ĺ���
 */

#include "framebuffer.h"

extern unsigned int fb_base_addr;
extern unsigned int bpp;
extern unsigned int xsize;
extern unsigned int ysize;

/* 
 * ����
 * ���������
 *     x��y : ��������
 *     color: ��ɫֵ
 *         ����16BPP: color�ĸ�ʽΪ0xAARRGGBB (AA = ͸����),
 *     ��Ҫת��Ϊ5:6:5��ʽ
 *         ����8BPP: colorΪ��ɫ���е�����ֵ��
 *     ����ɫȡ���ڵ�ɫ���е���ֵ
 */
void PutPixel(UINT32 x, UINT32 y, UINT32 color)
{
    UINT8 red,green,blue;

    switch (bpp){
        case 16:
        {
            UINT16 *addr = (UINT16 *)fb_base_addr + (y * xsize + x);
            red   = (color >> 19) & 0x1f; // 5 BIT
            green = (color >> 10) & 0x3f; // 6 bit
            blue  = (color >>  3) & 0x1f; // 5 bit
            color = (red << 11) | (green << 5) | blue; // ��ʽ5:6:5
            *addr = (UINT16) color;
            break;
        }
        
        case 8:
        {
            UINT8 *addr = (UINT8 *)fb_base_addr + (y * xsize + x);
            *addr = (UINT8) color;
            break;
        }

        default:
            break;
    }
}

/* 
 * ����
 * ���������
 *     x1��y1 : �������
 *     x2��y2 : �յ�����
 *     color  : ��ɫֵ
 *         ����16BPP: color�ĸ�ʽΪ0xAARRGGBB (AA = ͸����),
 *     ��Ҫת��Ϊ5:6:5��ʽ
 *         ����8BPP: colorΪ��ɫ���е�����ֵ��
 *     ����ɫȡ���ڵ�ɫ���е���ֵ
 */
void DrawLine(int x1,int y1,int x2,int y2,int color)
{
    int dx,dy,e;
    dx=x2-x1; 
    dy=y2-y1;
    
    if(dx>=0)
    {
        if(dy >= 0) // dy>=0
        {
            if(dx>=dy) // 1/8 octant
            {
                e=dy-dx/2;
                while(x1<=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1+=1;e-=dx;}   
                    x1+=1;
                    e+=dy;
                }
            }
            else        // 2/8 octant
            {
                e=dx-dy/2;
                while(y1<=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1+=1;e-=dy;}   
                    y1+=1;
                    e+=dx;
                }
            }
        }
        else           // dy<0
        {
            dy=-dy;   // dy=abs(dy)

            if(dx>=dy) // 8/8 octant
            {
                e=dy-dx/2;
                while(x1<=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1-=1;e-=dx;}   
                    x1+=1;
                    e+=dy;
                }
            }
            else        // 7/8 octant
            {
                e=dx-dy/2;
                while(y1>=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1+=1;e-=dy;}   
                    y1-=1;
                    e+=dx;
                }
            }
        }   
    }
    else //dx<0
    {
        dx=-dx;     //dx=abs(dx)
        if(dy >= 0) // dy>=0
        {
            if(dx>=dy) // 4/8 octant
            {
                e=dy-dx/2;
                while(x1>=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1+=1;e-=dx;}   
                    x1-=1;
                    e+=dy;
                }
            }
            else        // 3/8 octant
            {
                e=dx-dy/2;
                while(y1<=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1-=1;e-=dy;}   
                    y1+=1;
                    e+=dx;
                }
            }
        }
        else           // dy<0
        {
            dy=-dy;   // dy=abs(dy)

            if(dx>=dy) // 5/8 octant
            {
                e=dy-dx/2;
                while(x1>=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1-=1;e-=dx;}   
                    x1-=1;
                    e+=dy;
                }
            }
            else        // 6/8 octant
            {
                e=dx-dy/2;
                while(y1>=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1-=1;e-=dy;}   
                    y1-=1;
                    e+=dx;
                }
            }
        }   
    }
}

/* 
 * ����ͬ��Բ
 */
void Mire(void)
{
    UINT32 x,y;
    UINT32 color;
    UINT8 red,green,blue,alpha;

    for (y = 0; y < ysize; y++)
        for (x = 0; x < xsize; x++){
            color = ((x-xsize/2)*(x-xsize/2) + (y-ysize/2)*(y-ysize/2))/64;
            red   = (color/8) % 256;
            green = (color/4) % 256;
            blue  = (color/2) % 256;
            alpha = (color*2) % 256;

            color |= ((UINT32)alpha << 24);
            color |= ((UINT32)red   << 16);
            color |= ((UINT32)green << 8 );
            color |= ((UINT32)blue       );

            PutPixel(x,y,color);
        }
}

/* 
 * ����Ļ��ɵ�ɫ
 * ���������
 *     color: ��ɫֵ
 *         ����16BPP: color�ĸ�ʽΪ0xAARRGGBB (AA = ͸����),
 *     ��Ҫת��Ϊ5:6:5��ʽ
 *         ����8BPP: colorΪ��ɫ���е�����ֵ��
 *     ����ɫȡ���ڵ�ɫ���е���ֵ
 */
void ClearScr(UINT32 color)
{   
    UINT32 x,y;
    
    for (y = 0; y < ysize; y++)
        for (x = 0; x < xsize; x++)
            PutPixel(x, y, color);
}

