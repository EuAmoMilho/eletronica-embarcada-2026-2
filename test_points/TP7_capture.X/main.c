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
#include "mcc_generated_files/eccp1.h"

uint16_t    duty = 0;

volatile uint16_t   timer1_captured = 0;    // periodo entre capturas do timer1
volatile bool       new_capture_flag = false;// sinalizacao de nova leitura do
                                             // ccp1 modo captura

float               speed = 0.0f; // velocidade calculada

/*
                         Main application
 */
int main() {
    // initialize the device
    SYSTEM_Initialize();

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    while (1) {
        duty = ADC_GetConversion(Lum);
        if (duty >= 1023){
            duty = 1023;
        }
        EPWM2_LoadDutyValue(duty);
        
        if(new_capture_flag){
            uint16_t period;
            
            INTERRUPT_GlobalInterruptDisable();
            period = timer1_captured;
            new_capture_flag = false;
            INTERRUPT_GlobalInterruptEnable();
            
            if(period != 0){
                speed = 1.570796327f * 1000000 /((float)period * 2);  // (1/2 * PI) / (eccp_value * 2/1000000) [rad/s]
                                                                             //             isso pois o periodo de tmr1 e de 2us
            }
        }
        
        asm("NOP"); // ciclos de trabalho vazios para
                    // facilitar simulacao

            
    }

    return 0;
}
/**
 End of File
*/
