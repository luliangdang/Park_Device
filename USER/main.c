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

extern unsigned char SN[4]; 			//卡号
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
	
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
	LED_Init();		  		//初始化与LED连接的硬件接口
	LCD_Init();			   	//初始化LCD 	
	KEY_Init();				//按键初始化		
	BEEP_Init();
	RS485_Init(9600);	//初始化RS485
	InitRc522();			//初始化射频卡模块
	TIM3_Int_Init(4999,7199);//10Khz的计数频率，计数到5000为500ms
	TIM4_Int_Init(100,71);
	LED0=1;
	LED1=0;
 	POINT_COLOR=RED;//设置字体为红色
	LCD_ShowString(30,30,200,16,16,"RS485 TEST");
	LCD_ShowString(30,50,200,16,16,"Parking Address:");
 	POINT_COLOR=BLUE;//设置字体为蓝色
	addr = addrH*256+addrL;
	LCD_ShowxNum(158,50,addr,5,16,0);
	LCD_ShowString(30,70,200,16,16,"Send Data:");		//提示发送的数据
	LCD_ShowString(30,130,200,16,16,"Receive Data:");	//提示接收到的数据
 	
	while(1)
	{
		if(send_flag)					//读IC卡
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
						for(temp=0,i=0;i<6;i++)		//校验卡号及地址，不管数据溢出
							temp+=send[i+1];
						
						send[7]=temp/256;
						send[8]=temp%256;
						
						send[9]='I';
						
						for(i=0;i<5;i++)
							LCD_ShowxNum(30+i*32,90,send[i],3,16,0X80);	//显示数据
						for(i=0;i<5;i++)
							LCD_ShowxNum(30+i*32,110,send[i+5],3,16,0X80);	//显示数据
						RS485_Send_Data(send,10);//发送10个字节
						RS485_Send_Data(send,10);//发送10个字节
						TIM_Cmd(TIM3, ENABLE);  //使能TIMx
						time=0;
					}
				}
			}
		}
		if(send_flag==0 && time>250)					//读IC卡
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
						for(temp=0,i=0;i<6;i++)		//校验卡号及地址，不管数据溢出
							temp+=send[i+1];

						send[7]=temp/256;
						send[8]=temp%256;
						send[9]='O';
						
						for(i=0;i<5;i++)
							LCD_ShowxNum(30+i*32,90,send[i],3,16,0X80);	//显示数据
						for(i=0;i<5;i++)
							LCD_ShowxNum(30+i*32,110,send[i+5],3,16,0X80);	//显示数据
						RS485_Send_Data(send,10);//发送10个字节
						RS485_Send_Data(send,10);//发送10个字节
						TIM_Cmd(TIM3, ENABLE);  //使能TIMx
						time=0;
					}
				}
			}
		}
		if(receive_flag)//接收到有数据
		{
			receive_flag=0;
 			for(i=0;i<5;i++)
			{
				LCD_ShowxNum(30+i*32,150,receive_temp[i],3,16,0X80);	//显示数据
				printf("%c\r\n",receive_temp[i]);
			}
			switch(receive_temp[3])
			{
				case 0 :
				{
					RS485_Send_Data(send,10);
				}break;//发送10个字节
				case 1 :
				{
					send_flag=0;
					time=0;
					LED1=1;
					LED0=0;
					TIM_Cmd(TIM4, DISABLE);
					BEEP=0;
					TIM_Cmd(TIM3, DISABLE);//关闭TIMx	
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
					TIM_Cmd(TIM3, DISABLE);//关闭TIMx	
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
