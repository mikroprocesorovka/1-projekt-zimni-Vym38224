#include "stm8s_conf.h"
#include "swspi.h"
#include "milis.h"

#define CS_L GPIO_WriteLow(CS_GPIO,CS_PIN)
#define CS_H GPIO_WriteHigh(CS_GPIO,CS_PIN)
#define CLK_L GPIO_WriteLow(CLK_GPIO,CLK_PIN)
#define    CLK_H GPIO_WriteHigh(CLK_GPIO,CLK_PIN)
#define DIN_L GPIO_WriteLow(DIN_GPIO,DIN_PIN)
#define DIN_H GPIO_WriteHigh(DIN_GPIO,DIN_PIN)

#define MX7219_NOP 0x00
#define MAX7219_DIG0 0x01
#define MAX7219_DIG1 0x02
#define MAX7219_DIG2 0x03
#define MAX7219_DIG3 0x04
#define MAX7219_DIG4 0x05
#define MAX7219_DIG5 0x06
#define MAX7219_DIG6 0x07
#define MAX7219_DIG7 0x08
#define MAX7219_DECMODE 0x09
#define MAX7219_INTENSITY 0x0A
#define MAX7219_SCANLIM 0x0B
#define MAX7219_SHUTDOWN 0x0C
#define MAX7219_DISTEST 0x0F

void swspi_init(void){
    GPIO_Init(CS_GPIO,CS_PIN,GPIO_MODE_OUT_PP_HIGH_FAST);
    GPIO_Init(CLK_GPIO,CLK_PIN,GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(DIN_GPIO,DIN_PIN,GPIO_MODE_OUT_PP_LOW_FAST);
}

void swspi_tx16(uint16_t data){//transmitting
    uint16_t maska=0b1<<15;//rotace doleva 15x //Ob1000000...
    CS_L;
    while(maska){
        if(maska & data){DIN_H;}//v případě nenulového součinu
        else{DIN_L;}
        CLK_H;//podle datasheetu
        maska = maska >> 1;//rotace bitu doprava
        CLK_L;
        }
    CS_H;
}


void swspi_send_time (uint8_t seconds,uint8_t minutes, uint8_t hours){
    uint8_t jednotky=0;
    uint8_t desitky=0;
    
    desitky=hours/10;
    jednotky=hours%10;
    jednotky=jednotky | 0b1<<7;
    swspi_adressXdata(MAX7219_DIG4,jednotky);
    swspi_adressXdata(MAX7219_DIG5,desitky);
    
    desitky=minutes/10;
    jednotky=minutes%10;
    jednotky=jednotky | 0b1<<7;
    swspi_adressXdata(MAX7219_DIG2,jednotky);
    swspi_adressXdata(MAX7219_DIG3,desitky);
    
    desitky=seconds/10;
    jednotky=seconds%10;
    jednotky=jednotky | 0b1<<7;
    swspi_adressXdata(MAX7219_DIG0,jednotky);
    swspi_adressXdata(MAX7219_DIG1,desitky);
}


void swspi_toggle(uint8_t rad, uint8_t hodnota_radu){
    static uint32_t last_time=0;
    uint8_t value;
    static bool stav;
    switch (stav){
        case 0:
            if (milis()-last_time>499){
                last_time=milis();
                stav=1;
                swspi_adressXdata(rad*2+1,0b10001111);
                swspi_adressXdata(rad*2+2,0b00001111);
            }
            break;
        case 1:
            if (milis()-last_time>499){
                last_time=milis();
                stav=0;
                value=hodnota_radu|0b1<<7;
                swspi_adressXdata(rad*2+1,hodnota_radu%10);
                value=hodnota_radu&0b0<<7;
                swspi_adressXdata(rad*2+2,hodnota_radu/10);
            }
    }
}



void swspi_toggle_slowly(uint8_t rad, uint8_t hodnota_radu){
    static uint32_t last_time=0;
    static bool stav;
    uint8_t value;
    switch (stav){
        case 0:
            if (milis()-last_time>999){
                last_time=milis();
                stav=1;
                swspi_adressXdata(rad*2+1,0b10001111);
                swspi_adressXdata(rad*2+2,0b00001111);
            }
            break;
        case 1:
            if (milis()-last_time>29){
                last_time=milis();
                stav=0;
                value=hodnota_radu|0b1<<7;
                swspi_adressXdata(rad*2+1,hodnota_radu%10);
                value=hodnota_radu&0b0<<7;
                swspi_adressXdata(rad*2+2,hodnota_radu/10);
            }
    }
}



void swspi_adressXdata(uint8_t adress, uint8_t data){
    uint16_t retezec;
    CS_L;
    
    retezec=adress<<8 | data;
    swspi_tx16(retezec);
}




void swspi_send_number(uint32_t number){
    uint16_t string;
    uint32_t delitel = 10000000;
    uint8_t i=0x0;
    uint8_t cislo;
    bool znak=0;
    
    
    
    for(i=0x8;i>=1;i--){ 
        
        cislo=number/delitel;
        if (cislo>0){znak=1;}
        
        if (znak==0){
            string=(i<<8) | 0b1111;
        }
        else{
            string=(i<<8) | (cislo);
        }
        
        
        swspi_tx16(string);
        
        number=number%delitel;
        delitel=delitel/10;
    }
}