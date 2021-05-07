// Lab10.c
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 1/16/2021 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* 
 Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// VCC   3.3V power to OLED
// GND   ground
// SCL   PD0 I2C clock (add 1.5k resistor from SCL to 3.3V)
// SDA   PD1 I2C data

//************WARNING***********
// The LaunchPad has PB7 connected to PD1, PB6 connected to PD0
// Option 1) do not use PB7 and PB6
// Option 2) remove 0-ohm resistors R9 R10 on LaunchPad
//******************************

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"
#include "SSD1306.h"
#include "Print.h"
#include "Random.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "TExaS.h"
#include "Switch.h"
//********************************************************************************
// debuging profile, pick up to 7 unused bits and send to Logic Analyzer
#define PB54                  (*((volatile uint32_t *)0x400050C0)) // bits 5-4
#define PF321                 (*((volatile uint32_t *)0x40025038)) // bits 3-1
// use for debugging profile
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PB5       (*((volatile uint32_t *)0x40005080)) 
#define PB4       (*((volatile uint32_t *)0x40005040)) 
// TExaSdisplay logic analyzer shows 7 bits 0,PB5,PB4,PF3,PF2,PF1,0 
// edit this to output which pins you use for profiling
// you can output up to 7 pins
void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|PF321|PB54; // sends at 10kHz
}
void ScopeTask(void){  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
// edit this to initialize which pins you use for profiling
void Profile_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x22;      // activate port B,F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 
  GPIO_PORTF_DEN_R |=  0x0E;   // enable digital I/O on PF3,2,1
  GPIO_PORTB_DIR_R |=  0x30;   // output on PB4 PB5
  GPIO_PORTB_DEN_R |=  0x30;   // enable on PB4 PB5  
}
//********************************************************************************
 
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
typedef enum {dead, alive} status_t;
struct sprite{
	int32_t x;
	int32_t y;
	int32_t vx;
	int32_t vy;
	int32_t frame;
	int32_t NumFrame;
	const uint8_t *image[4];
	status_t life;
};
typedef struct sprite sprite_t;
sprite_t Invaders[18];
sprite_t Missles[18];
sprite_t Player;
sprite_t EarthGuy;
int NeedToDraw; //1 means needs to draw and 0 means not need to draw
void Init(void){ int i;
	EarthGuy.x=50;
	EarthGuy.y=62;
	EarthGuy.frame=0;
	EarthGuy.NumFrame=3;
	EarthGuy.image[0]=AlienBossA;
	EarthGuy.image[1]=AlienBossB;
	EarthGuy.image[2]=Alien30pointB;
	EarthGuy.life=alive;
	Player.x=50;
	Player.y=8;
	Player.frame=0;
	Player.NumFrame=3;
	Player.image[0]=PlayerShip0;
	Player.life=alive;
//  for(i=0;i<6;i++){
//    Missles[i].x = 20*i;
//    Missles[i].y = 10;
//		if(i<3){
//			Missles[i].vx = 1;//ones on the left go right
//		}
//		if(i>3){
//			Missles[i].vx = -1;//ones on the right go left
//		}
//		Missles[i].vy = 1;
//    Missles[i].image = Alien10pointA;
//    Missles[i].life = alive;
//  }
  for(i=0;i<18;i++){
    Missles[i].life = dead;
		Missles[i].frame=0;
		Missles[i].NumFrame=3;
   }
	for(i=0;i<18;i++){
    Invaders[i].life = dead;
		Invaders[i].frame=0;
		Invaders[i].NumFrame=3;
   }
}
int32_t score=0;
int32_t Collisions (void){
//void Collisions (void){
	int i;uint32_t x1,y1, x2, y2; 
	score=0;
	x2=EarthGuy.x+8;
	y2=EarthGuy.y-4;
	for(i=0; i<18;i++){
		if(Missles[i].life==alive){
			x1=Missles[i].x;
			y1=Missles[i].y;
			if(((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))<9){
				EarthGuy.frame++;
				if(EarthGuy.frame==EarthGuy.NumFrame){
					EarthGuy.life=dead;
				}
				score+=100;
				//insert death sound effect
				Sound_InvadKill();
				return score;
			}
		}
		
	}
	x2=Player.x+8;
	y2=Player.y-4;
	for(i=0; i<18;i++){
		if(Invaders[i].life==alive){
			x1=Invaders[i].x;
			y1=Invaders[i].y;
			if(((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))<9){
				Player.life=dead;
				//insert death sound effect
				Sound_InvadKill();
				return score;
			}
		}
	}
	for(i=0; i<18;i++){
		if(Invaders[i].life==alive){
			x1=Invaders[i].x;
			y1=Invaders[i].y;
			for(int j=0; j<18; j++){
				if(Missles[j].life==alive){
					x2=Missles[j].x;
					y2=Missles[j].y;
				if(((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))<2){
					Missles[j].life=dead;
					Invaders[i].life=dead;
					score+=100;
				//insert death sound effect
					Sound_InvadKill();
				return score;
				}
				}
			}
		} 
	}
}
void Move(void){ int i; uint32_t adcData;
	if(Player.life==alive){
		NeedToDraw=1;
		adcData=ADC_In(); //is between 0 and 4096
		Player.x=((127-16)*adcData)/4096;
	}
	for (i=0; i<10; i++){
		if(Missles[i].life==alive){
			NeedToDraw=1;
			if(Missles[i].x<0){
				Missles[i].x=2;
				Missles[i].vx=-Missles[i].vx;
			}
			if(Missles[i].x>126){
				Missles[i].x=124;
				Missles[i].vx=-Missles[i].vx;
			}
			if((Missles[i].y<62)&&(Missles[i].y>0)){
				Missles[i].x+=Missles[i].vx;//movement left and right
				Missles[i].y+=Missles[i].vy;//movement up and down
			}else{
				Missles[i].life=dead;
			}
			}
	}
	for (i=0; i<10; i++){
		if(Invaders[i].life==alive){
			NeedToDraw=1;
			if(Invaders[i].x<0){
				Invaders[i].x=2;
				Invaders[i].vx=-Invaders[i].vx;
			}
			if(Invaders[i].x>126){
				Invaders[i].x=124;
				Invaders[i].vx=-Invaders[i].vx;
			}
			if((Invaders[i].y<62)&&(Invaders[i].y>0)){
				Invaders[i].x+=Invaders[i].vx;//movement left and right
				Invaders[i].y+=Invaders[i].vy;//movement up and down
			}else{
				Invaders[i].life=dead;
			}
			}
	}
}

void Fire(int32_t vx1, int32_t vy1){ int i;
	i=0;
	while(Missles[i].life==alive){
		i++;
		if(i==18) return;
	}
	Missles[i].x=Player.x+7;
	Missles[i].y=Player.y+4;
	Missles[i].vx=vx1;
	Missles[i].vy=vy1;
	Missles[i].image[0]=Laser0;
	Missles[i].life=alive;
	//add in sound
	Sound_Shoot();
}

void ReturnFire(int32_t vx1, int32_t vy1){ int i;
	i=0;
	while(Invaders[i].life==alive){
		i++;
		if(i==18) return;
	}
	Invaders[i].x=(Random()/2); //EarthGuy.x+7
	Invaders[i].y=EarthGuy.y-4;
	Invaders[i].vx=vx1; //(Random()/2)
	Invaders[i].vy=vy1;
	Invaders[i].image[0]=AlienBossA;
	Invaders[i].life=alive;
	//add in sound
}
void PauseGame(void){
	SSD1306_ClearBuffer();
	SSD1306_OutClear();
	SSD1306_OutString("The game has been paused, please press the left button to resume!");
	uint32_t play=0;
	while(play!=0x02){
		play=Switch_In()&0x02;
	}
	play=0;
	return;
}
void SysTick_Handler (void){ //every 50 ms
	uint32_t pauseplay=Switch_In()&0x02;
	if(pauseplay==0x02){
		PauseGame();
	}
	static uint32_t lastdown=0;
	uint32_t down=Switch_In()&0x04;
	if((down==0x04)&&(lastdown==0)){
		Fire(0,1);
	}
	if(Random()<12){
		ReturnFire(0,-1);
	}
	Move();
	Collisions();
	lastdown=down;
}

void Draw(void){ int i;
	SSD1306_ClearBuffer();
		if(Player.life==alive){
			SSD1306_DrawBMP(Player.x, Player.y, Player.image[0], 0,SSD1306_INVERSE);
		}
		if(EarthGuy.life==alive){
			SSD1306_DrawBMP(EarthGuy.x, EarthGuy.y, EarthGuy.image[EarthGuy.frame], 0,SSD1306_INVERSE);
		}
	for (i=0; i<10; i++){
		if(Missles[i].life==alive){
			SSD1306_DrawBMP(Missles[i].x, Missles[i].y, Missles[i].image[0], 0,SSD1306_INVERSE);
		}
	}
	for (i=0; i<10; i++){
		if(Invaders[i].life==alive){
			SSD1306_DrawBMP(Invaders[i].x, Invaders[i].y, Invaders[i].image[0], 0,SSD1306_INVERSE);
		}
	}
	SSD1306_OutBuffer();
	NeedToDraw=0;
}

void SysTick_Init20Hz (void);
	

int main(void){uint32_t time=0;
  DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();   
  Random_Init(1);
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
  SSD1306_ClearBuffer();
  SSD1306_DrawBMP(2, 62, SpaceInvadersMarquee, 0, SSD1306_WHITE);
  SSD1306_OutBuffer();
  //EnableInterrupts();
  Delay100ms(20);
	SSD1306_OutClear();
	SSD1306_ClearBuffer();
	//DisableInterrupts();
	SSD1306_OutString("Please Hold Left for English or Right for Spanish");
	Switch_Init();
	uint32_t buttonpress=0;
	while(buttonpress==0){
		buttonpress=Switch_In()&0x06;
	}
		Delay100ms(20);
		if (buttonpress==0x04){//for English
		SSD1306_OutClear();
		SysTick_Init20Hz();
		ADC_Init();
		Sound_Init();
		Switch_Init();
		Init();
		EnableInterrupts();
			//EarthGuy.life
		while((EarthGuy.life==alive)&&(Player.life==alive)){
			time++;
			if(NeedToDraw){
				Draw();
			}
		}
		DisableInterrupts();
		SSD1306_OutClear(); 
		if(Player.life==alive){
		SSD1306_OutString("You Win!");
		SSD1306_OutString("	Here is how ");	
		SSD1306_OutString("long it took you:");
		SSD1306_OutString("");
		time=time/5267;
		SSD1306_OutUDec(time);
		}else{
		SSD1306_OutString("Please try again!");	
		//SSD1306_OutString("Here is your score:");	
		//SSD1306_OutUDec(time);
		}
	}
	else if(buttonpress==0x02){//for Spanish
		SSD1306_OutClear();
		SysTick_Init20Hz();
		ADC_Init();
		Sound_Init();
		Switch_Init();
		Init();
		EnableInterrupts();
		while(EarthGuy.life==alive&&(Player.life==alive)){
			if(NeedToDraw){
				Draw();
			}
		}
		DisableInterrupts();
		SSD1306_OutClear(); 
		if(Player.life==alive){
		SSD1306_OutString("¡Tú ganas!");
		SSD1306_OutString("Este es el tiempo que le llevó:");
		time=time/5267;
		SSD1306_OutUDec(time);
	//	SSD1306_OutUDec(score);
		}else{
		SSD1306_OutString("¡Inténtalo de nuevo!");	
		//SSD1306_OutString("Aquí está tu puntuación:");	
		//SSD1306_OutUDec(score);
		}
	}
	while(1){};
  SSD1306_ClearBuffer();
  SSD1306_DrawBMP(47, 63, PlayerShip0, 0, SSD1306_WHITE); // player ship bottom
  SSD1306_DrawBMP(53, 55, Bunker0, 0, SSD1306_WHITE);

  SSD1306_DrawBMP(0, 9, Alien10pointA, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(20,9, Alien10pointB, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(40, 9, Alien20pointA, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(60, 9, Alien20pointB, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(80, 9, Alien30pointA, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(50, 19, AlienBossA, 0, SSD1306_WHITE);
  SSD1306_OutBuffer(); //takes 25 ms to happen
  Delay100ms(30);

  SSD1306_OutClear();  
  SSD1306_SetCursor(1, 1);
  SSD1306_OutString("GAME OVER");
  SSD1306_SetCursor(1, 2);
  SSD1306_OutString("Nice try,");
  SSD1306_SetCursor(1, 3);
  SSD1306_OutString("Earthling!");
  SSD1306_SetCursor(2, 4);
  while(1){
    Delay100ms(10);
    SSD1306_SetCursor(19,0);
    SSD1306_OutUDec2(time);
    time++;
    PF1 ^= 0x02;
  }
}

// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}

void SysTick_Init20Hz(void){
	NVIC_ST_CTRL_R=0;
	NVIC_ST_RELOAD_R=(80000000/20)-1;
	NVIC_ST_CURRENT_R=0;
	NVIC_ST_CTRL_R=7;
	NVIC_SYS_PRI3_R=(NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000;
}

