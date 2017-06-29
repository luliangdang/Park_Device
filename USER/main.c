#include "sys.h"
#include <string.h>
#include "delay.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "usart.h"	 
#include "rs485.h"
#include "rc522.h"
#include "beep.h"
#include "timer.h"

extern unsigned char SN[4]; 			//����
u8 send[10]={'S',addrH,addrL,0,0,0,0,0,0,'I'};

u8 receive_flag = 0;
u8 send_flag = 1;
u8 rs485_flag = 0;
u8 receive_temp[5];
u8 receive_ready = 1;

int main(void)
{	 
	u8 i=0;
	u16 temp;
	u8 fb[]={'S',addrH,addrL,0,0,0,0,addrH,addrL,'O'};
	u8 time=0;
	u8 sn[4];
	u8 id[4];
	u16 addr;
	
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
	LCD_Init();			   	//��ʼ��LCD 	
	KEY_Init();				//������ʼ��		
	BEEP_Init();
	RS485_Init(9600);	//��ʼ��RS485
	InitRc522();			//��ʼ����Ƶ��ģ��
	TIM3_Int_Init(4999,7199);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms
	TIM4_Int_Init(100,71);
	LED0=1;
	LED1=0;
 	POINT_COLOR=RED;//��������Ϊ��ɫ
	LCD_ShowString(30,30,200,16,16,"RS485 TEST");
	LCD_ShowString(30,50,200,16,16,"Parking Address:");
 	POINT_COLOR=BLUE;//��������Ϊ��ɫ
	addr = addrH*256+addrL;
	LCD_ShowxNum(158,50,addr,5,16,0);
	LCD_ShowString(30,70,200,16,16,"Send Data:");		//��ʾ���͵�����
	LCD_ShowString(30,130,200,16,16,"Receive Data:");	//��ʾ���յ�������
 	
	while(1)
	{
		if(send_flag)					//��IC��
		{
			ReadID();
			if(SN[0]!=0 && SN[1]!=0 && SN[2]!=0 && SN[3]!=0)
			{
				for(i=0;i<4;i++)
					sn[i]=SN[i];
				ReadID();
				if(sn[0]==SN[0] && sn[1]==SN[1] && sn[2]==SN[2] && sn[3]==SN[3])
				{
					if(!((id[0]==SN[0])&&(id[1]==SN[1])&&(id[2]==SN[2])&&(id[3]==SN[3])))
					{
						TIM_Cmd(TIM4, ENABLE);
						for(i=0;i<4;i++)
						{
							send[i+3]=SN[i];
							id[i]=SN[i];
							SN[i]=0;
						}
						for(temp=0,i=0;i<6;i++)		//У�鿨�ż���ַ�������������
							temp+=send[i+1];
						
						send[7]=temp/256;
						send[8]=temp%256;
						
						send[9]='I';
						
						for(i=0;i<5;i++)
							LCD_ShowxNum(30+i*32,90,send[i],3,16,0X80);	//��ʾ����
						for(i=0;i<5;i++)
							LCD_ShowxNum(30+i*32,110,send[i+5],3,16,0X80);	//��ʾ����
						RS485_Send_Data(send,10);//����10���ֽ�
						RS485_Send_Data(send,10);//����10���ֽ�
						TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx
						time=0;
					}
				}
			}
		}
		if(send_flag==0 && time>250)					//��IC��
		{
			ReadID();
			if(SN[0]!=0 && SN[1]!=0 && SN[2]!=0 && SN[3]!=0)
			{
				for(i=0;i<4;i++)
					sn[i]=SN[i];
				ReadID();
				if(sn[0]==SN[0] && sn[1]==SN[1] && sn[2]==SN[2] && sn[3]==SN[3])
				{
					if((id[0]==SN[0])&&(id[1]==SN[1])&&(id[2]==SN[2])&&(id[3]==SN[3]))
					{
						TIM_Cmd(TIM4, ENABLE);
						for(i=0;i<4;i++)
						{
							send[i+3]=SN[i];
							SN[i]=0;
						}
						for(temp=0,i=0;i<6;i++)		//У�鿨�ż���ַ�������������
							temp+=send[i+1];

						send[7]=temp/256;
						send[8]=temp%256;
						send[9]='O';
						
						for(i=0;i<5;i++)
							LCD_ShowxNum(30+i*32,90,send[i],3,16,0X80);	//��ʾ����
						for(i=0;i<5;i++)
							LCD_ShowxNum(30+i*32,110,send[i+5],3,16,0X80);	//��ʾ����
						RS485_Send_Data(send,10);//����10���ֽ�
						RS485_Send_Data(send,10);//����10���ֽ�
						TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx
						time=0;
					}
				}
			}
		}
		if(receive_flag)//���յ�������
		{
			receive_flag=0;
 			for(i=0;i<5;i++)
			{
				LCD_ShowxNum(30+i*32,150,receive_temp[i],3,16,0X80);	//��ʾ����
				printf("%c\r\n",receive_temp[i]);
			}
			switch(receive_temp[3])
			{
				case 0 :
				{
					RS485_Send_Data(send,10);
				}break;//����10���ֽ�
				case 1 :
				{
					send_flag=0;
					time=0;
					LED1=1;
					LED0=0;
					TIM_Cmd(TIM4, DISABLE);
					BEEP=0;
					TIM_Cmd(TIM3, DISABLE);//�ر�TIMx	
					for(i=0;i<4;i++)
					{
						send[i+3]=SN[i];
						SN[i]=0;
					}
				}break;
				case 2:
				{
					send_flag=1;
					LED1=0;
					LED0=1;
					delay_ms(50);
					for(i=0;i<10;i++)
						RS485_Send_Data(fb,10);
					for(i=0;i<4;i++)
					{
						send[i+3]=SN[i];
						SN[i]=0;
					}
				}break;
				case 3 :
				{
					send_flag=1;
					LED1=0;
					LED0=1;
					TIM_Cmd(TIM4, DISABLE);
					BEEP=0;
					TIM_Cmd(TIM3, DISABLE);//�ر�TIMx	
					for(i=0;i<4;i++)
					{
						send[i+3]=0;
						id[i]=0;
						SN[i]=0;
					}
					delay_ms(5000);
				}break;
				default :
				{
//					fb[3]=0;
//					RS485_Send_Data(fb,5);
				}break;
			}
 		}
		if(time<255)
		time++;
		delay_ms(25);
 }
}
