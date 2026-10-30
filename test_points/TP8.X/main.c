/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC16F1827
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"

//bytes comandos RX
#define SET_TX_OR_START 0x80
#define SET_PWM         0x81
#define SET_TEMP_GAIN   0x82
#define SET_LUM_GAIN    0x83
#define BUFFER_MAX      3

typedef union{
    uint16_t    v;
    struct{     //bytes alocados em little-endian
        uint8_t txL;    //primeiro o menor
        uint8_t txH;    //depois o maior
    };
    
}   uart_t;    // valor a ser transmitido

uint16_t    duty = 0,
            adc_read = 0,
            temp_gain = 1000,
            lum_gain = 1000;

uart_t      temp;
uart_t      lum;

uart_t      rc_data;

volatile uint8_t        bufferRx[BUFFER_MAX];
volatile uint8_t        countRx = 0;

void adjustTx(uart_t  *data){ //ajusta o dado para que possa ser enviado
    
    data->v     <<=  1;         //desloca todo o vetor 1 bit a esquerda
    data->txH  &= 0x7F;         //mascara o msb do byte mais significativo (normalmente n seria necessario)
    data->txL   >>=  1;         //desloca de volta o byte menos significativo 1 bit a direita
    data->txL  &= 0x7F;         //mascara o msb do byte menos significativo
}

void adjustRx(uart_t  *data){
    data->txL   <<= 1;           //desloca o byte menos significativo 1 bit a esquerda
    data->v     >>= 1;           //desloca o vetor inteiro1 bit a direita, "colando" o vetor todo de volta
}

uint16_t convertTemp(uint16_t rx, uint16_t gain){       //converte o valor do adc para celsius aplicando ganho
    
    uint32_t celsius = ((uint32_t)rx * gain)/1000;   // mult. por ponto fixo com 3 casa decimais, portanto dividir por
                                                // 1000 para obter resultado em um numero em ponto fixo 1 casa decimal.
    
                                                // ref = 1,023V para um adc de 10 bits = 1 bit por mV.
                                                // sensor: 10mV por celsius
                                                // ponto fixo 1 casa decimal: NAO precisa converter;
    
    return (uint16_t)celsius;
}

uint16_t convertLum(uint16_t  rx, uint16_t gain){       //converte o valor do adc em porcentagem de iluminacao aplicando ganho
    
    uint32_t percent = ((uint32_t)rx * 1000) / 1023;//multiplica o valor lido por 1000 para
                                                    //evitar perda de dados e ajustar o resultado para ponto
                                                    //fixo de 1 casa decimal. Depois, divide o valor
                                                    //pelo valor maximo de bits da conversao     
    
                      
    uint32_t result = (percent * gain) / 1000;      //aplica o ganho de ponto fixo 3 casas decimais e ajusta para
                                                    //ponto fixo 1 casa decimal ao dividir por 1000
    return (uint16_t)result;
}

void processarComando(){
    switch(bufferRx[0]){
        case SET_TX_OR_START:
            if(bufferRx[2] == 0x01){
                TMR1_StartTimer();

            } else if(bufferRx[2] == 0x00){
                TMR1_StopTimer();

            }
        break;
        
        case SET_PWM:
            rc_data.txH = bufferRx[1];
            rc_data.txL = bufferRx[2];
            
            adjustRx(&rc_data);
            
            if(rc_data.v < 1024){
                duty = rc_data.v;
            } else {
                duty = 1023;
            }
            
            EPWM2_LoadDutyValue(duty);
            
        break;
        
        case SET_TEMP_GAIN:
            rc_data.txH = bufferRx[1];
            rc_data.txL = bufferRx[2];
            
            adjustRx(&rc_data);
            
            if(rc_data.v < 16324){
                temp_gain = rc_data.v;
            }
            
        break;
        
        case SET_LUM_GAIN:
            rc_data.txH = bufferRx[1];
            rc_data.txL = bufferRx[2];
            
            adjustRx(&rc_data);
            
            if(rc_data.v < 16324){
                lum_gain = rc_data.v;
            }
            
        break;
    }
}

void sendTx(){
    
    adjustTx(&temp);
    adjustTx(&lum);    
    
    EUSART_Write(SET_TX_OR_START);
    EUSART_Write(temp.txH);
    EUSART_Write(temp.txL);
    EUSART_Write(lum.txH);
    EUSART_Write(lum.txL);
}

/*
                         Main application
 */

void main(void)
{
    // initialize the device
    SYSTEM_Initialize();

    TMR1_SetInterruptHandler(sendTx);
    TMR1_StopTimer();                   //inicializar com timer desligado
    
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();    
    
    while (1)
    {    
        if(EUSART_is_rx_ready()){
            uint8_t rxByte = EUSART_Read();

            if((rxByte & SET_TX_OR_START) != 0){
                countRx = 0;

                bufferRx[countRx] = rxByte;
                countRx++;

            } else {
                bufferRx[countRx] = rxByte;
                countRx++;
                if(countRx == BUFFER_MAX){
                    processarComando();
                    countRx = 0;
                }
            }
        }
        
        ADC_setRef(0);      //configura FVR do ADC para 1V
        __delay_us(10);     //tempo para estabilizacao da tensao de referencia
        adc_read = ADC_GetConversion(Temp);
        temp.v = convertTemp(adc_read, temp_gain);
        
        ADC_setRef(1);      //configura FVR do ADC para VDD
        __delay_us(10);     //tempo para estabilizacao da tensao de referencia
        adc_read = ADC_GetConversion(Lum);
        lum.v = convertLum(adc_read, lum_gain);
        
    }
}
/**
 End of File
*/