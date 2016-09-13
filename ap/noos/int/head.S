@******************************************************************************
@ File��head.S
@ ���ܣ���ʼ���������ж�ģʽ������ģʽ��ջ�����ú��жϴ�����
@******************************************************************************       
   
.extern     main
.text 
.global _start 
_start:
@******************************************************************************       
@ �쳣�������������У���Reset��HandleIRQ�⣬�����쳣��û��ʹ��
@******************************************************************************       
    b   Reset

@ 0x04: δ����ָ����ֹģʽ��������ַ
HandleUndef:
    b   HandleUndef 
 
@ 0x08: ����ģʽ��������ַ��ͨ��SWIָ������ģʽ
HandleSWI:
    b   HandleSWI

@ 0x0c: ָ��Ԥȡ��ֹ���µ��쳣��������ַ
HandlePrefetchAbort:
    b   HandlePrefetchAbort

@ 0x10: ���ݷ�����ֹ���µ��쳣��������ַ
HandleDataAbort:
    b   HandleDataAbort

@ 0x14: ����
HandleNotUsed:
    b   HandleNotUsed

@ 0x18: �ж�ģʽ��������ַ
    b   HandleIRQ

@ 0x1c: ���ж�ģʽ��������ַ
HandleFIQ:
    b   HandleFIQ

Reset:                  
    ldr sp, =4096           @ ����ջָ�룬���¶���C����������ǰ��Ҫ���ջ
    bl  disable_watch_dog   @ �ر�WATCHDOG������CPU�᲻������
    
    msr cpsr_c, #0xd2       @ �����ж�ģʽ
    ldr sp, =3072           @ �����ж�ģʽջָ��

    msr cpsr_c, #0xd3       @ �������ģʽ
    ldr sp, =4096           @ ���ù���ģʽջָ�룬
                            @ ��ʵ��λ֮��CPU�ʹ��ڹ���ģʽ��
                            @ ǰ��ġ�ldr sp, =4096�����ͬ���Ĺ��ܣ��˾��ʡ��

    bl  init_led            @ ��ʼ��LED��GPIO�ܽ�
    bl  init_irq            @ �����жϳ�ʼ����������init.c��
    msr cpsr_c, #0x53       @ ����I-bit=0����IRQ�ж�
    
    ldr lr, =halt_loop      @ ���÷��ص�ַ
    ldr pc, =main           @ ����main����
halt_loop:
    b   halt_loop

HandleIRQ:
    sub lr, lr, #4                  @ ���㷵�ص�ַ
    stmdb   sp!,    { r0-r12,lr }   @ ����ʹ�õ��ļĴ���
                                    @ ע�⣬��ʱ��sp���ж�ģʽ��sp
                                    @ ��ʼֵ���������õ�3072
    
    ldr lr, =int_return             @ ���õ���ISR��EINT_Handle������ķ��ص�ַ  
    ldr pc, =EINT_Handle            @ �����жϷ���������interrupt.c��
int_return:
    ldmia   sp!,    { r0-r12,pc }^  @ �жϷ���, ^��ʾ��spsr��ֵ���Ƶ�cpsr
    
