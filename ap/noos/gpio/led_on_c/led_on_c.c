#define GPBCON      (*(volatile unsigned long *)0x56000010)
#define GPBDAT      (*(volatile unsigned long *)0x56000014)

int main()
{
    GPBCON = 0x00000400;    // ����GPB5Ϊ�����, λ[11:10]=0b01
    GPBDAT = 0x00000000;    // GPB5���0��LED1����

    return 0;
}
