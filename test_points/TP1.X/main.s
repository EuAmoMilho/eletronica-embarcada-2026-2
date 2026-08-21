#include <xc.inc>

;definindo nomes dos vetores utilizados para as operacoes
global v1
global v2

;selecionando a area espelhada da memoria
PSECT udata_shr

;reservando espaço em bytes para cada variavel
v1:
    DS 1
v2: 
    DS 1

;define a area de reset e a area de codigo
psect resetVec, class=CODE, delta=2
resetVec:
    PAGESEL start
    goto start
    
psect code, class=CODE, delta=2
 
    start:
    BANKSEL PORTA	;selecionando registrador dos pinos A
    clrf PORTA		;limpando possiveis saidas
    
    BANKSEL ANSEL	;registrador que configura analog/digital
    clrf ANSEL		;
    clrf ANSELH		;todos os pinos digitais
    
    BANKSEL TRISA	;configurando os pinos de saida
    movlw 11100000B
    movwf TRISA
    bcf OPTION_REG, 7	;configurando pullup interno de PORTB
    clrf STATUS		;zerando flags e endereçamento direto                
			;de memoria
    
loop:
    clrwdt		; clearwatchdogtimer
    movf PORTB, W	; leitura de entradas
    movwf v1
    movf PORTC, W
    movwf v2
    
    movlw 0x0F		; mascara sobre os dados
    andwf v1, f
    andwf v2, f
    
    btfsc PORTA, 5
    goto adicao
    
    movf v2, W
    subwf v1, W
    movwf PORTA
    goto loop
    
    adicao:
    movf v1, W
    addwf v2, W
    movwf PORTA

    goto loop
    