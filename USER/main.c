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
//#include "uc8088_spi.h"

u8 key_begin_flag = 0;
u8 key_stop_flag = 0;

 
/************************************************
�����ܣ��洢8088���ڴ�ӡ�����ݵ�SD��
************************************************/
//FATFS fs;													/* FatFs�ļ�ϵͳ���� */
FIL fnew;													/* �ļ����� */
FRESULT res_flash;                /* �ļ�������� */
UINT fnum;            					  /* �ļ��ɹ���д���� */
 
 int main(void)
 {	 
	u32 total,free;
	
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200	
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	//uc8088_init();		//8088��ʼ��
	EXTIX_Init(); //�ⲿ�жϳ�ʼ��
 	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��

	//printf("���SD��\r\n");
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
 					
//	printf("SD Total Size: %d    MB\r\n", total>>10);	//��ʾSD�������� MB
//	printf("SD  Free Size: %d    MB\r\n", free>>10);	//��ʾSD��ʣ������ MB	
	
	res_flash = f_open(&fnew, "0:gps_test_data\\out_test.log",FA_CREATE_ALWAYS | FA_WRITE );
	if ( res_flash == FR_OK )
	{
//		printf("����/����FatFs��д�����ļ�.txt�ļ��ɹ���\r\n");
		LED0 = 1;
		LED1 = 1;
	}

	while(key_begin_flag == 0){
		LED0 = ~LED0;
		LED1 = ~LED1;
		delay_ms(300);
		//printf("please preess key0, which_BUF = %d\r\n", which_BUF);
	}
	
	which_BUF = 1;
	delay_ms(10);
	printf("$RST,ERASENAV\r\n");
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
		
		// ���дBUF1 �Ѿ�д��
		if(USART_RX_cnt1 >= USART_REC_LEN){			
				res_flash = f_write(&fnew, USART_RX_BUF1, USART_REC_LEN, &fnum);
				if (res_flash==FR_OK && fnum == USART_REC_LEN){
					LED0 = 0;
					LED1 = 1;
					//printf("1 OK");
				}
				else
					printf("�����ļ�д��ʧ�ܣ�(res_flash = %d), fnum = %d\n",res_flash, fnum); 
				
				USART_RX_cnt1 = 0;
		}
		
		if(USART_RX_cnt2 >= USART_REC_LEN){	
				res_flash = f_write(&fnew, USART_RX_BUF2, USART_REC_LEN, &fnum);
				if (res_flash==FR_OK && fnum == USART_REC_LEN){
						LED0 = 1;
						LED1 = 0;
						
						//printf("2 OK");
				}
				else
					printf("�����ļ�д��ʧ�ܣ�(res_flash = %d), fnum = %d\n",res_flash, fnum);
				
				USART_RX_cnt2 = 0;
				if (key_begin_flag == 1)
				{
						key_begin_flag = 0;
						printf("$SETDCXO,4026384389\r\n");
						delay_ms(10);
						printf("$SRP,-1557898,5327342,3132336\r\n");
				}
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
void EXTI3_IRQHandler(void)
{
	delay_ms(10);//����
	if(KEY1==0)	 //����KEY0
	{
		
		key_begin_flag = 1;
	}		 
	EXTI_ClearITPendingBit(EXTI_Line3);  //���LINE4�ϵ��жϱ�־λ  
}
