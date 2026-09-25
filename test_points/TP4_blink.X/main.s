CONFIG FOSC = INTRC_NOCLKOUT	;desabilitando clock externo
CONFIG WDTE = ON		;ligando watchdog timer
CONFIG MCLRE = ON		;pino masterclear configurado para programar
CONFIG LVP  = OFF		;low level programming desabilitado

#include <xc.inc>
    
;definindo nomes para as portas dos LEDs
#define LedR	PORTA, 0
#define LedG	PORTA, 1
    
CTMR0 EQU 0x0B			;constante para TIMER0 - tempo estimado:
				;			 499,712ms
 
PSECT code, class=CODE, abs, delta=2
ORG 0x000
resetVec:
    PAGESEL start
    goto    start

PSECT code, class=CODE, delta=2
ORG 0x010
start:
    BANKSEL PORTA
    clrf    PORTA
    
    BANKSEL ANSEL
    clrf    ANSEL
    
    ;wdt oscilator freq: 31kHz
    BANKSEL WDTCON		;prescaler wdt 1x32728
    movlw   0x35		;periodo de aprox 1 seg
    movwf   WDTCON
    clrf    TMR0
    
    BANKSEL TRISA
    movlw   0xF8
    movwf   TRISA
    
    movlw   0x32		;500kHz
    movwf   OSCCON		;Fosc/4 = 125kHz
    
    movlw   0xC7		;PU desabilitado - int: rising edge
    				;T0Cs: Fosc/4 - LH tran - prescaler p T0
				;prescaler rate = 1:256 - T = 2,048ms
    movwf   OPTION_REG
        
    clrf    STATUS		;LED alterna quando pino da 
    movlw   0x07		;respectiva cor esta em LOW
    movwf   PORTA		;desliga todos
    
loop:
    bcf	    LedR		;liga led vermelho
    call    LdTMR		;carrega valor ao TIMER0 e entra no loop
    bsf	    LedR		;desliga LED saindo do loop
    clrwdt
    sleep			;entra em modo de baixo consumo
    nop
    
    bcf	    LedG		;liga led verde
    call    LdTMR		;carrega valor ao TIMER0 e entra no loop
    bsf	    LedG		;desliga LED saindo do loop
    clrwdt
    sleep			;entra em modo de baixo consumo
    nop
    
    goto    loop
    
LdTMR:
    movlw   CTMR0
    movwf   TMR0
    bcf	    T0IF		;zerando flag de interrupcao
    
TMRLOOP:
    clrwdt
    btfss   T0IF		;continuamente testa flag de interrupcao
    goto    TMRLOOP
    
    bcf	    T0IF		;zerando flag de interrupcao

    return
    


