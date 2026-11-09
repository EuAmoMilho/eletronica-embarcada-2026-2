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

//  definicao de macros, tipos e estruturas

#define MAX_FILA        10
#define MAX_CARACTERES  20

enum    status  {vazia, agendado, atendido};

struct  cliente{
    char    nome[MAX_CARACTERES + 3];
    enum    status  situacao;
};

//  declaracao de variaveis e constantes

struct cliente clientes[MAX_FILA];

__eeprom struct cliente backup[MAX_FILA];

const   char    menu[] = "L - Exibe lista\nA - Agenda\nR - Apaga lista\nP - Proximo\n";

__eeprom uint8_t    indiceFila_eeprom = 0;
__eeprom uint8_t    indiceAtendidos_eeprom = 0;

uint8_t indiceFila = 0;
uint8_t indiceAtendidos = 0;

// funcoes

void    check(char string[]){
    string[0] = 'x';
}

void    enviarString(char string[]){
    for(int i = 0; string[i] != '\0'; i ++){
        EUSART_Write(string[i]);
    }
}

// funcoes para facilitar a escrita na eeprom, dado por dado da estrutura:

// Copia da RAM para a EEPROM
void salvar(uint8_t indice) {
    // Copia o array de caracteres (nome) byte a byte
    for(uint8_t j = 0; j < sizeof(clientes[indice].nome); j++) {
        backup[indice].nome[j] = clientes[indice].nome[j];
    }
    // Copia o status
    backup[indice].situacao = clientes[indice].situacao;
}

// Copia da EEPROM para a RAM
void carregar(uint8_t indice) {
    for(uint8_t j = 0; j < sizeof(clientes[indice].nome); j++) {
        clientes[indice].nome[j] = backup[indice].nome[j];
    }
    clientes[indice].situacao = backup[indice].situacao;
}

void receber_comando(uint8_t rxCmd){
    
    if(rxCmd == 'l' || rxCmd == 'L'){
        enviarString("Lista de Agendamentos:\n");
        
        if(indiceFila > 0){
            for(int i = 0; i < indiceFila; i++){
                carregar(i);
                
                enviarString(clientes[i].nome);
                EUSART_Write('\n');
                
            }
        }   else    {
            enviarString("Lista de agendamentos vazia.\n");
        }
        
    }   else if (rxCmd =='a' || rxCmd == 'A'){
        if(indiceFila < MAX_FILA){
            uint8_t rx_index = 2;
            enviarString("Digite o nome:\n");
            
            clientes[indiceFila].nome[0] = ' ';    // deixa espaço vazio para marcacao
            clientes[indiceFila].nome[1] = ' ';    // tab horizontal
            
            
            while (1){   
                // aguarda recebimento da mensagem
                while(!EUSART_is_rx_ready()){
                    __delay_us(10);
                    
                }
                
                // leitura de todo o buffer de recebimento
                uint8_t rxChar = EUSART_Read();

                if( ((rxChar == '\n' || rxChar == 13) && rx_index > 2) || rx_index >= MAX_CARACTERES-1){
                    clientes[indiceFila].nome[rx_index] = '\0';           // sinalizando final de string
                    clientes[indiceFila].situacao = agendado;
                    
                    salvar(indiceFila);
                    
                    enviarString("O agendamento foi realizado.\n");
                    indiceFila++;
                    indiceFila_eeprom = indiceFila;         // guardando o indice na memoria
                    
                    break;                 // cessar recebimento de nome

                    // verifica se o byte recebido e um caractere valido
                }   else if ( (rxChar >= 65 && rxChar <= 90) || (rxChar >= 97 && rxChar <= 122) || rxChar == ' '){
                    clientes[indiceFila].nome[rx_index] = rxChar;
                    rx_index++;

                }
                
            }
            
        }   else    {
            enviarString("Nao temos mais agendamentos disponiveis.\n");
        
        }
    }   else if (rxCmd == 'r' || rxCmd == 'R'){       // zera o indice da lista. na pratica, esvazia lista
                                            // pois os proximos agendamentos a sobrescreverao
        indiceFila = 0;
        indiceAtendidos = 0;
        
        indiceFila_eeprom = indiceFila;
        indiceAtendidos_eeprom = indiceAtendidos;
        
        for(int i = 0; i < MAX_FILA; i++){
            clientes[i].situacao = vazia;
        }        
        
        enviarString("Lista de agendamentos vazia.\n");
        
    }   else if (rxCmd == 'p' || rxCmd == 'P'){
        if(indiceAtendidos < indiceFila){
            clientes[indiceAtendidos].situacao = atendido;
            enviarString("Proximo:");
            enviarString(clientes[indiceAtendidos].nome);
            EUSART_Write('\n');
            
            check(clientes[indiceAtendidos].nome);
            
            salvar(indiceAtendidos);
            
            indiceAtendidos++;
            indiceAtendidos_eeprom = indiceAtendidos;
            
        }   else if (indiceFila == 0){
            enviarString("Lista de agendamentos vazia.\n");
            
        }   else    {
            enviarString("Todos os agendamentos foram atendidos.\n");
            
        }
        
    }   else    {
        enviarString(menu);
        
    }
}

/*
                         Main application
 */
void main(void)
{
    // initialize the device
    SYSTEM_Initialize();

    indiceFila = indiceFila_eeprom;
    indiceAtendidos = indiceAtendidos_eeprom;
    
    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();
    
    while (1)
    {
        if(EUSART_is_rx_ready()){
            uint8_t rxByte = EUSART_Read();
            
            receber_comando(rxByte);
        }
    }
}
/**
 End of File
*/