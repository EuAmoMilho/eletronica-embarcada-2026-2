CONFIG FOSC = INTRC_NOCLKOUT
CONFIG WDTE = ON
CONFIG MCLRE = ON
CONFIG LVP  = OFF

#include <xc.inc>

#define motor	PORTB, 1
#define sd1	PORTB, 4
#define sd2	PORTB, 5
#define display PORTC
    
global	voltas, Wt, STATUSt

PSECT udata_shr
voltas:
    DS 1
Wt:
    DS 1
STATUSt:
    DS 1
    
PSECT code, class=CODE, abs, delta=2
ORG 0x000
resetVec:
    PAGESEL start
    goto    start
    
ORG 0x004			;pagina p interrupcoes
    movwf   Wt
    swapf   STATUS, W
    movwf   STATUSt		;salvando contexto - swap nao afeta flags
    
    clrf    STATUS
    btfsc   TMR2IF
    goto    T2_ISR
    btfsc   T0IF
    goto    T0_ISR
    goto    ISR_END
    
; + interrupcao timer0
T0_ISR:
    movlw   0x38		;recarregando timer
    movwf   TMR0

    movlw   0xFE		;numero maximo de voltas da carga
    subwf   voltas, w		;compara voltas realizadas com no. max
    btfss   STATUS, 2
    incf    voltas, f		;incrementa voltas se voltas < 254

    movf    voltas, W
    xorlw   0xFE
    btfsc   STATUS, 2
    goto    idle
    bcf	    T0IF
    goto    ISR_END

idle:
    bcf     motor		;desliga motor se voltas == 254
    clrf    voltas		;zera o numero de voltas
    bcf	    T0IF
    goto    ISR_END

; + interrupcao timer2 (MUX)
T2_ISR:
    clrf    display		;limpa saida residual
    bcf	    TMR2IF
    btfss   sd1
    goto    ligasd1
ligasd2:			;nibble menos significativo
    bcf	    sd1
    bsf	    sd2
    movf    voltas, W
    call    LUT
    movwf   display
    goto    ISR_END
    
ligasd1:			;nibble mais significativo
    bcf	    sd2
    bsf	    sd1
    swapf   voltas, W
    call    LUT
    movwf   display
    goto    ISR_END
    
; restaurar contexto
ISR_END:
    swapf   STATUSt, W
    movwf   STATUS
    swapf   Wt, f
    swapf   Wt, W
    retfie
    
; + LUT digitos HEX
LUT:
    andlw   0x0F
    addwf   PCL, f
    retlw   00111111B
    retlw   00110000B
    retlw   01101101B
    retlw   01111001B
    retlw   01110010B
    retlw   01011011B
    retlw   01011111B
    retlw   00110001B
    retlw   01111111B
    retlw   01110011B
    retlw   01110111B
    retlw   01011110B
    retlw   01001100B
    retlw   01111100B
    retlw   01001111B
    retlw   01000111B
    
PSECT code, class=CODE, delta=2

start:
    BANKSEL PORTA
    clrf    PORTA
    clrf    PORTB
    clrf    PORTC   
    
    BANKSEL ANSEL
    clrf    ANSEL
    clrf    ANSELH
    
    BANKSEL TRISA
    movlw   00010000B
    movwf   TRISA
    movlw   11001101B
    movwf   TRISB
    movlw   10000000B
    movwf   TRISC

    movlw   11101111B		;fonte TMR0 pino RA4 - sem prescaler no
    				;TIMER 0, cada pulso incrementa direto
    movwf   OPTION_REG

    bsf     PIE1, 1		;ativa int do timer2
    movlw   0xFA
    movwf   PR2			;TIMER2 incrementa 250 vezes antes
    				;de incrementar o postscaler
				;periodo postscaler = 1ms
    
    clrf    STATUS
    movlw   01001101B		;1/10 postscaler: interrupcao a cada 10ms 
    				;prescaler 1:4, TMR2 periodo 4us 
    movwf   T2CON

    movlw   0x38		;carrega TIMER0 de modo a ativar a
    				;interrupcao a cada 200 pulsos
				;		(1 volta da carga)
    movwf   TMR0

    bcf	    T0IF
    bcf	    TMR2IF

    movlw   11100000B
    movwf   INTCON		;global, peripheral e TIMER0 interrupt

    movlw   0x07
    movwf   PORTA		;desliga LED

    bcf	    motor

    clrf    voltas

loop:
    clrwdt
    movf	voltas, W	;verifica se voltas == 0
    btfss	STATUS, 2
    bsf		motor		;se voltas != 0, liga motor

    goto loop    
