#include<xc.inc>

CONFIG FOSC = INTRC_NOCLKOUT
    
global fator_a	; fator de maior modulo
global fator_b  ;	   menor
global count	; indice para iteracoes
global cH	; MSB do produto
global cL	; lsb do produto
global i	; outro indice p operacao

sz	MACRO	reg, dest1, dest0
    movf	reg, W
    btfss	STATUS, 2
    goto	dest0
    movwf	cL
    movwf	cH
    goto	dest1
	ENDM
        
fconfig MACRO   Mreg, mreg
    movf	Mreg, W
    movwf	fator_a
    movf	mreg, W
    movwf	fator_b
	ENDM
	
PSECT udata_shr
 
fator_a:
    DS 1
fator_b:
    DS 1
count:
    DS 1
cH:
    DS 1
cL:
    DS 1
i:
    DS 1

PSECT resetVec, class=CODE, delta=2
resetVec:
    PAGESEL  start
    goto start
    
PSECT code, CLASS=CODE, delta=2
start:
    BANKSEL PORTA
    clrf PORTC
    clrf PORTA
    
    BANKSEL ANSEL
    clrf ANSEL
    clrf ANSELH
    
    BANKSEL TRISA
    clrf TRISC
    clrf TRISA
    bcf	 OPTION_REG, 7
    clrf STATUS
    
    ;;iniciar o FSR para ox20 
    ;INDF <- a
    ;bcf FSR, 7
    ;INDF <- b
setpointer:    
    movlw   0x20
    movwf   FSR
    
loop:
    
    clrwdt
    clrf    cH
    clrf    cL
    
    sz	    PORTB, result, ok1
ok1:
    sz	    PORTD, result, ok2
ok2:
    subwf   PORTB, W
    btfsc   STATUS, 0
    goto    Bmaior
    
Dmaior:
    fconfig	PORTD, PORTB
    goto seletor
Bmaior:
    fconfig	PORTB, PORTD

seletor:
    btfsc   RE0
    goto    m2
    call    m1
    
result:
    movf    cL, W ; little-endian
    movwf   PORTA
    movf    cH, W
    movwf   PORTC

save:
    bcf	    STATUS, 7
    bcf	    FSR,    7
    movf    fator_a, W
    movwf   INDF
    bsf	    FSR,    7
    movf    fator_b, W
    movwf   INDF
    bsf	    STATUS, 7
    movf    cL, W
    movwf   INDF
    bcf	    FSR,    7
    movf    cH, W
    movwf   INDF
    
    movlw   0x6F
    xorwf   FSR, W
    btfsc   STATUS, 2
    goto    setpointer
    incf    FSR
    
    goto    loop
    
    
    
m1:
    movf	fator_b, W
    movwf	count
    movf	fator_a, W

operacao1:
    movwf	cL
    decfsz	count, f
    goto	adicao
    return

adicao:
    addwf	fator_a, W
    movwf	cL
    btfsc	STATUS, 0
    incf	cH, f
    goto	operacao1

    
    
m2:
    movlw   0x08
    movwf   i
passo2:
    rrf	    fator_b, f
    movf    fator_a, W
    btfsc   STATUS, 0
    addwf   cH, f		;somar A com cH
    rrf	    cH, f
    rrf	    cL, f
    decfsz  i, f
    goto passo2
    goto result



