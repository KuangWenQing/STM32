#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "key.h"     
#include "exti.h"
#include "malloc.h"
#include "sdio_sdcard.h"     
#include "ff.h"  
#include "exfuns.h"    
#include "uc8088_spi.h"

u8 key_begin_flag = 0;
u8 key_stop_flag = 0;

 
/************************************************
�����ܣ��洢8088���ڴ�ӡ�����ݵ�SD��
************************************************/
//FATFS fs;													/* FatFs�ļ�ϵͳ���� */
FIL fnew;													/* �ļ����� */
FRESULT res_flash;                /* �ļ�������� */
UINT fnum;            					  /* �ļ��ɹ���д���� */
//BYTE ReadBuffer[1024]={0};        /* �������� */
BYTE WriteBuffer[] = "���д���ļ���123abc";         /* д������*/
u8 Buffer[SPI_BUF_LEN] = {0};

//int main(void)
// {	 
//	u8 one_char;
//	u16 tmp, i;
//	u32 wp, rp;
//	delay_init();	    	 //��ʱ������ʼ��	  
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
//	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200	
// 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
//	KEY_Init();					//��ʼ������
//	uc8088_init();		//8088��ʼ��
//	
//	printf("halt cpu\t");
//	rp = uc8088_read_u32(0x1a107018);
//	printf("read test = %x\r\n", rp);
//	tmp = uc8088_read_u8(0x1a107018);
//	printf("tmp = %x\r\n", tmp);
//	tmp = uc8088_read_u8(0x1a107019);
//	printf("tmp = %x\r\n", tmp);
//	tmp = uc8088_read_u8(0x1a10701a);
//	printf("tmp = %x\r\n", tmp);
//	 tmp = uc8088_read_u8(0x1a10701b);
//	printf("tmp = %x\r\n", tmp);
//	
//	wp = uc8088_read_u16(0x1a107018);
//	printf("wp = %x\r\n", wp);
//	wp = uc8088_read_u16(0x1a107019);
//	printf("wp = %x\r\n", wp);
//	wp = 0;

//	wp = uc8088_read_u32(Buf_addr);
//	rp = uc8088_read_u32(Buf_addr + 4);
//	printf("wp = %d,   rp = %d\r\n", wp, rp); 
//	LED0 = 0; 
//	while(1){
//		wp = uc8088_read_u32(Buf_addr);
//		rp = uc8088_read_u32(Buf_addr + 4);
//		
//		if (rp < wp){
//			tmp = uc8088_read_memory(Buf_addr + 8 + rp, Buffer, wp - rp);
//			if(tmp){
//				uc8088_write_u32(Buf_addr + 4, rp+tmp);
//				for(i=0; i<tmp; i+=4){
//					one_char = Buffer[i+3];
//					Buffer[i+3] = Buffer[i];
//					Buffer[i] = one_char;
//					one_char = Buffer[i+2];
//					Buffer[i+2] = Buffer[i+1];
//					Buffer[i+1] = one_char;
//				}
//				Buffer[tmp] = '\0';
//				printf("%s", Buffer);
//				//printf("rp = %d,   wp = %d,   i = %d\r\n", rp, wp, i);
//			}
//		}
//		else if((rp > wp)){
//			tmp = uc8088_read_memory(Buf_addr + 8 + rp, Buffer, SPI_BUF_LEN - rp);
//			i = uc8088_read_memory(Buf_addr + 8, Buffer + SPI_BUF_LEN - rp, wp);
//			tmp += i;
//			if (tmp){
//				uc8088_write_u32(Buf_addr + 4, i);
//				for(i=0; i<tmp; i+=4){
//					one_char = Buffer[i+3];
//					Buffer[i+3] = Buffer[i];
//					Buffer[i] = one_char;
//					one_char = Buffer[i+2];
//					Buffer[i+2] = Buffer[i+1];
//					Buffer[i+1] = one_char;
//				}
//				Buffer[tmp] = '\0';
//				printf("%s", Buffer);
//				//printf("rp = %d,   wp = %d,   i = %d\r\n", rp, wp, i+tmp);
//			}
//		}
////		printf("wp = %d,   rp = %d\r\n", wp, rp);
//		LED0=!LED0;//DS0��˸
//		Buffer[0] = '\0';
//		
//		delay_ms(50);
//	}
// }

 
 
 int main(void)
 {	 
	char USART_RX_BUF1[11], USART_RX_BUF2[11];

	u32 total,free;

	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200	
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	uc8088_init();		//8088��ʼ��
	EXTIX_Init(); //�ⲿ�жϳ�ʼ��
 	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��

	while(SD_Init())//��ⲻ��SD��
	{
		printf("SD Card Error!");
		delay_ms(200);					
		LED0=!LED0;//DS0��˸
	}
	
 	exfuns_init();							//Ϊfatfs��ر��������ڴ�				 
  f_mount(fs[0],"0:",1); 					//����SD�� 	
		  
	while(exf_getfree((u8*)'0', &total, &free))	//�õ�SD������������ʣ������
	{
		printf("SD Card Fatfs Error!\n");
		delay_ms(200);
		LED0=!LED0;//DS0��˸
	}													  			    
 					
	printf("SD Total Size: %d    MB\r\n", total>>10);	//��ʾSD�������� MB
	printf("SD  Free Size: %d    MB\r\n", free>>10);	//��ʾSD��ʣ������ MB	
	
	printf("\r\n******���������ļ�д�����... ******\r\n");	
	res_flash = f_open(&fnew, "0:gps_test_data\\out_test.log",FA_CREATE_ALWAYS | FA_WRITE );
	if ( res_flash == FR_OK )
	{
		printf("����/����FatFs��д�����ļ�.txt�ļ��ɹ������ļ�д�����ݡ�\r\n");
		LED0 = 1;
		LED1 = 1;
	}

	
	while(key_begin_flag == 0){
		LED0 = ~LED0;
		LED1 = ~LED1;
		delay_ms(300);
	}
	
	total = 0;
	while(1)
	{
		
		if (key_stop_flag)	// ��������
		{
			LED0 = 0;
			LED1 = 0;
			printf("�ļ�����ɹ�\r\n");
			while(1){
				delay_ms(1000);
				LED0 = ~LED0;
				LED1 = ~LED1;
			}
		}
//		if (write_slow_flag)	// BUF�е�����û���ü�д��SD��
//		{
//			/* ���ٶ�д���ر��ļ� */
//			f_close(&fnew);	
//			/* ����ʹ���ļ�ϵͳ��ȡ�������ļ�ϵͳ */
//			f_mount(NULL,"0:",1);
//		}

		
		// ���дBUF2 �Ѿ�д��
		if(USART_RX_cnt2 == USART_REC_LEN){
			res_flash = f_write(&fnew, USART_RX_BUF2, USART_REC_LEN, &fnum);
			if (res_flash==FR_OK && fnum == USART_REC_LEN){
				LED0 = 1;
				LED1 = 0;
				USART_RX_cnt2 = 0;
				//printf("write BUF2 OK, now cnt1 = %d\r\n", USART_RX_cnt1);
			}
			else
				printf("�����ļ�д��ʧ�ܣ�(res_flash = %d), fnum = %d\n",res_flash, fnum); 
		}
		else if(USART_RX_cnt1 == USART_REC_LEN){
			res_flash = f_write(&fnew, USART_RX_BUF1, USART_REC_LEN, &fnum);
			if (res_flash==FR_OK && fnum == USART_REC_LEN){
				LED0 = 0;
				LED1 = 1;
				USART_RX_cnt1 = 0;
				//printf("write BUF1 OK, now cnt2 = %d\r\n", USART_RX_cnt2);
			}
			else
				printf("�����ļ�д��ʧ�ܣ�(res_flash = %d), fnum = %d\n",res_flash, fnum); 
		}
	} 
}

//�ⲿ�ж�0������� 
void EXTI0_IRQHandler(void)
{
	delay_ms(10);//����
	if(WK_UP==1)	 	 //WK_UP����
	{				 
		if (key_stop_flag == 0)
		{
			/* ���ٶ�д���ر��ļ� */
			f_close(&fnew);	
			/* ����ʹ���ļ�ϵͳ��ȡ�������ļ�ϵͳ */
			f_mount(NULL,"0:",1);
		}
		
		LED0 = 1;
		LED1 = 1;
		key_stop_flag = 1;
	}
	EXTI_ClearITPendingBit(EXTI_Line0); //���LINE0�ϵ��жϱ�־λ  
}

void EXTI4_IRQHandler(void)
{
	delay_ms(10);//����
	if(KEY0==0)	 //����KEY0
	{
		
		key_begin_flag = 1;
	}		 
	EXTI_ClearITPendingBit(EXTI_Line4);  //���LINE4�ϵ��жϱ�־λ  
}
