 
#include "stm8s.h"
#include "milis.h"
//#include "stdio.h"
//#include "spse_stm8.h"
//#include "stm8_hd44780.h"
#include "swspi.h"

#define MX7219_NOP 0x0
#define MAX7219_DIG0 0x1
#define MAX7219_DIG1 0x2
#define MAX7219_DIG2 0x3
#define MAX7219_DIG3 0x4
#define MAX7219_DIG4 0x5
#define MAX7219_DIG5 0x6
#define MAX7219_DIG6 0x7
#define MAX7219_DIG7 0x8
#define MAX7219_DECMODE 0x9
#define MAX7219_INTENSITY 0xA
#define MAX7219_SCANLIM 0xB
#define MAX7219_SHUTDOWN 0xC
#define MAX7219_DISTEST 0xF

#define MAX7219_SHUTDOWN_NORMAL_MODE 0b1
#define MAX7219_DECMODE_DIG_ALL 0b11111111
#define MAX7219_DISTEST_OFF 0b0
#define MAX7219_SCANLIM_DIG_ALL 0b111


#define ENKODER_TLAC_A_GPIO GPIOA
#define ENKODER_TLAC_B_GPIO GPIOA
#define ENKODER_TLAC_GPIO GPIOE
//#define ENKODER_TLAC_GPIO GPIOA

#define ENKODER_TLAC_A_PIN GPIO_PIN_3
#define ENKODER_TLAC_B_PIN GPIO_PIN_4
#define ENKODER_TLAC_PIN GPIO_PIN_4
//#define ENKODER_TLAC_PIN GPIO_PIN_5

#define readA	GPIO_ReadInputPin(ENKODER_TLAC_A_GPIO,ENKODER_TLAC_A_PIN)
#define readB	GPIO_ReadInputPin(ENKODER_TLAC_B_GPIO,ENKODER_TLAC_B_PIN)


void init_enc(void);
void process_enc(void);
void init_timer(void);
void enc_choice_plus (void);
void enc_choice_minus (void);

uint8_t intensity=1;
uint8_t i;
volatile uint8_t hodnota_enkoderu=0;
volatile bool rotace=0;
volatile bool stisknuto=0,dlouhe_stisknuto=0;
bool konec_stisku=0;
uint32_t pocatek_stisku_cas=0;
uint32_t odpocet=0;
uint16_t zbytek=0;
volatile bool minule_stisk=0,ted_stisk=0;
volatile bool pocatek_stisku=0;
uint32_t bzucak=0;

uint8_t stav=0;
//uint8_t sekundy=0, minuty=0, hodiny=0;
volatile uint8_t rad=0;
volatile bool vyber=0;
volatile uint8_t cas[3]={
0,0,0
};

void main(void){
CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // taktovat MCU na 16MHz
init_milis(); // spustit časovač millis
swspi_init();
init_enc();
init_timer();
swspi_adressXdata(MAX7219_DECMODE,MAX7219_DECMODE_DIG_ALL);
swspi_adressXdata(MAX7219_SHUTDOWN,MAX7219_SHUTDOWN_NORMAL_MODE);
swspi_adressXdata(MAX7219_DISTEST,MAX7219_DISTEST_OFF);
swspi_adressXdata(MAX7219_SCANLIM,MAX7219_SCANLIM_DIG_ALL);
swspi_adressXdata(MAX7219_INTENSITY,intensity);
swspi_adressXdata(MAX7219_DIG6,0b1111);
swspi_adressXdata(MAX7219_DIG7,0b1111);
GPIO_Init(GPIOA,GPIO_PIN_6,GPIO_MODE_OUT_PP_LOW_SLOW);

//swspi_send_minutes(15);


  while (1){
		if(pocatek_stisku==1){
			pocatek_stisku_cas=milis();
			pocatek_stisku=0;
		}
		
		if (konec_stisku==1){
			if (milis()-pocatek_stisku_cas>999){
				dlouhe_stisknuto=1;
			}
			else{
				stisknuto=1;
			}
			konec_stisku=0;
		}
		
		switch(stav){
			case 0:
				GPIO_WriteLow(GPIOA,GPIO_PIN_6);
				if (stisknuto==1){
					if (vyber==0){vyber=1;}
					else{vyber=0;}
					stisknuto=0;
				}
				
				if (dlouhe_stisknuto==1){
					dlouhe_stisknuto=0;
					odpocet=milis();
					zbytek=0;
					stav=1;
				}
				
				if (vyber==0){
					swspi_toggle_slowly(rad,cas[rad]);
					if(rotace==1){
						swspi_send_time(cas[0],cas[1],cas[2]);
						rotace=0;
					}
				}
				if (vyber==1){
					swspi_toggle(rad,cas[rad]);
					if(rotace==1){
						swspi_send_time(cas[0],cas[1],cas[2]);
						rotace=0;
					}
				}
				break;
				
			case 1:
				GPIO_WriteLow(GPIOA,GPIO_PIN_6);
				if (milis()-(odpocet-zbytek)>999){
					cas[0]--;
					if (cas[0]>59){
						if (cas[1]!=0 || cas[2]!=0){
							cas[0]=59;
							cas[1]--;
							if (cas[1]>59){
								if (cas[2]!=0){
									cas[1]=59;
									cas[2]--;
									if (cas[2]>23){
										cas[23]=0;
									}
								}
								else{cas[1]=0;}
							}
						}
						else{cas[0]=0;}
					}
					if (cas[0]==0 && cas[1]==0 && cas[2]==0){
						stav=2;
						//GPIO_WriteHigh(GPIOG,GPIO_PIN_2);
						odpocet=milis();
						vyber=0;
						rad=0;
					}
					odpocet=milis();
					zbytek=milis()-odpocet;
					swspi_send_time(cas[0],cas[1],cas[2]);
				}
				
				if (stisknuto==1){
					stisknuto=0;
					stav=0;
					vyber=0;
					rad=0;
					cas[0]=0;
					cas[1]=0;
					cas[2]=0;
					swspi_send_time(cas[0],cas[1],cas[2]);
				}
				if (dlouhe_stisknuto==1){
					dlouhe_stisknuto=0;
					stav=0;
					vyber=0;
					rad=0;
				}
				break;
			case 2:
				if (milis()-bzucak>399){
					GPIO_WriteReverse(GPIOA,GPIO_PIN_6);
					bzucak=milis();
				}
				if (milis()-odpocet>4999){
					GPIO_WriteLow(GPIOA,GPIO_PIN_6);
					stav=0;
				}
				break;
		}
	}
}


INTERRUPT_HANDLER(TIM3_UPD_OVF_BRK_IRQHandler, 15)
 {
	 TIM3_ClearITPendingBit(TIM3_IT_UPDATE);
	 process_enc();
 }

void init_timer(void){
	TIM3_TimeBaseInit(TIM3_PRESCALER_16,1999);
	TIM3_ITConfig(TIM3_IT_UPDATE, ENABLE);
	TIM3_Cmd(ENABLE);
}


void init_enc(void){
GPIO_Init(ENKODER_TLAC_A_GPIO,ENKODER_TLAC_A_PIN,GPIO_MODE_IN_PU_NO_IT); //enkodér
GPIO_Init(ENKODER_TLAC_B_GPIO,ENKODER_TLAC_B_PIN,GPIO_MODE_IN_PU_NO_IT); //enkodér
GPIO_Init(ENKODER_TLAC_GPIO,ENKODER_TLAC_PIN,GPIO_MODE_IN_PU_NO_IT); //tlačítko enkodéru
}

void process_enc(void){
	static bool minuleA=0; // pamatuje si minulý stav vstupu A (nutné k detekování sestupné hrany)
	static bool minuleB=0; // pamatuje si minulý stav vstupu A (nutné k detekování sestupné hrany)
	// pokud je na vstupu A hodnota 0 a minule byla hodnota 1 tak jsme zachytili sestupnou hranu
	
		if (GPIO_ReadInputPin(ENKODER_TLAC_GPIO,ENKODER_TLAC_PIN)==RESET){
			ted_stisk=1;
		}
		else{
			ted_stisk=0;
		}
		
		if((ted_stisk==1) && (minule_stisk==0)){
			pocatek_stisku=1;
		}
		if(ted_stisk==0 && minule_stisk==1){
			konec_stisku=1;
		}
		
		
	
	
	if((GPIO_ReadInputPin(ENKODER_TLAC_B_GPIO,ENKODER_TLAC_B_PIN) != RESET) && minuleB==0 && GPIO_ReadInputPin(ENKODER_TLAC_A_GPIO,ENKODER_TLAC_A_PIN) == RESET){
		hodnota_enkoderu--;
		enc_choice_minus();
		rotace=1;
	}
	if((GPIO_ReadInputPin(ENKODER_TLAC_B_GPIO,ENKODER_TLAC_B_PIN) == RESET) && minuleB==1 && GPIO_ReadInputPin(ENKODER_TLAC_A_GPIO,ENKODER_TLAC_A_PIN) != RESET){
		hodnota_enkoderu--;
		enc_choice_minus();
		rotace=1;
	}

	if((GPIO_ReadInputPin(ENKODER_TLAC_A_GPIO,ENKODER_TLAC_A_PIN) != RESET) && minuleA==0 && GPIO_ReadInputPin(ENKODER_TLAC_B_GPIO,ENKODER_TLAC_B_PIN) == RESET){
		hodnota_enkoderu++;
		enc_choice_plus();
		rotace=1;
	}
	if((GPIO_ReadInputPin(ENKODER_TLAC_A_GPIO,ENKODER_TLAC_A_PIN) == RESET) && minuleA==1 && GPIO_ReadInputPin(ENKODER_TLAC_B_GPIO,ENKODER_TLAC_B_PIN) != RESET){
		hodnota_enkoderu++;
		enc_choice_plus();
		rotace=1;
	}
	
	if(GPIO_ReadInputPin(ENKODER_TLAC_A_GPIO,ENKODER_TLAC_A_PIN) != RESET){minuleA = 1;} // pokud je vstup A v log.1
	else{minuleA=0;}
	if(GPIO_ReadInputPin(ENKODER_TLAC_B_GPIO,ENKODER_TLAC_B_PIN) != RESET){minuleB = 1;} // pokud je vstup A v log.1
	else{minuleB=0;}

	minule_stisk=ted_stisk;
}


void enc_choice_minus (void){
	if (stav==0){
		//vyber++;
		if (vyber==0){rad--;}
		if (rad>2){rad=2;}
		if (vyber==1){
			cas[rad]--;
			if (cas[rad]>59){cas[rad]=59;}
			if (cas[2]>23){cas[2]=23;}
		}
	}
}


void enc_choice_plus(void){
	if (stav==0){
		if (vyber==0){rad++;}
		if (rad>2){rad=0;}
		if (vyber==1){
			cas[rad]++;
			if (cas[rad]>59){cas[rad]=0;}
			if (cas[2]>23){cas[2]=0;}
		}
	}
}


#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
