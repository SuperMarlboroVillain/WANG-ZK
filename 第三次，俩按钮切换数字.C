#include <reg51.h>

sbit CLK = P3^0;
sbit LEFT_KEY  = P1^0;  // 左移键
sbit RIGHT_KEY = P1^1;  // 右移键

unsigned char LedBuff[4] = {0, 1, 2, 3};  // 初始显示0123
unsigned char LED_TAB[] = {0x3f,0x06,0x5b,0x4f,0x66,\
                           0x6d,0x7d,0x07,0x7f,0x6f,0x0}; 

unsigned char LED_Index = 0;
unsigned char mode = 0;  // 0=静态，1=左移，2=右移

unsigned int scroll_cnt = 0;  // 滚动计数器
#define SCROLL_TIME 250      // 滚动周期（单位ms），越大越慢

// 左移显示数据
void scroll_left() {
	unsigned char temp = LedBuff[0];
	LedBuff[0] = LedBuff[1];
	LedBuff[1] = LedBuff[2];
	LedBuff[2] = LedBuff[3];
	LedBuff[3] = temp;
}

// 右移显示数据
void scroll_right() {
	unsigned char temp = LedBuff[3];
	LedBuff[3] = LedBuff[2];
	LedBuff[2] = LedBuff[1];
	LedBuff[1] = LedBuff[0];
	LedBuff[0] = temp;
}

void delay_ms(unsigned int ms) {
	unsigned int i, j;
	for(i=0;i<ms;i++)
		for(j=0;j<123;j++);  // 粗略延时
}

int main(void)
{
	TMOD = 0x11;  // 定时器0和1方式1
	TH0 = 0xfc;
	TL0 = 0x18;

	TH1 = 0xf8;
	TL1 = 0x30;

	EA = 1;
	ET0 = 1;
	ET1 = 1;
	TR0 = 1;
	TR1 = 1;

	while(1)
	{
		// 检测左键
		if(LEFT_KEY == 0) {
			delay_ms(10);  // 简单消抖
			if(LEFT_KEY == 0) {
				mode = 1;  // 左移模式
			}
			while(LEFT_KEY == 0);  // 等待松开
		}

		// 检测右键
		if(RIGHT_KEY == 0) {
			delay_ms(10);
			if(RIGHT_KEY == 0) {
				mode = 2;  // 右移模式
			}
			while(RIGHT_KEY == 0);
		}

		// 根据模式滚动
		if(mode == 1 || mode == 2) {
			if(scroll_cnt >= SCROLL_TIME) {
				if(mode == 1)
					scroll_left();
				else
					scroll_right();
				scroll_cnt = 0;
			}
		}
	}
}

// 数码管动态扫描显示
void int_t0(void) interrupt 1
{
	unsigned char bit_cs[4] = {0xfe, 0xfd, 0xfb, 0xf7};

	TH0 = 0xfc;
	TL0 = 0x18;

	P0 = 0x00;
	P2 = bit_cs[LED_Index];
	CLK = 0;
	P0 = LED_TAB[LedBuff[LED_Index]];
	CLK = 1;

	LED_Index++;
	if(LED_Index == 4) LED_Index = 0;
}

// 定时器1中断，1ms执行一次
void int_t1(void) interrupt 3
{
	TH1 = 0xf8;
	TL1 = 0x30;

	scroll_cnt++;  // 用于控制滚动速度
}