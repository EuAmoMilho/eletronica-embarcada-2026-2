#include<xc.inc>

CONFIG FOSC = INTRC_NOCLKOUT

global n
global d
global q
global r
    
global nibble
    
global i
    
PSECT udata_shr
 n:	    DS 1
 d:	    DS 1
 q:	    DS 1
 r:	    DS 1
 nibble:    DS 1
 i:	    DS 1
 address:   DS 1
    
PSECT resetVec, class=CODE, delta=2
resetVec:
    PAGESEL  start
    goto start
    
PSECT code, CLASS=CODE, abs, delta=2
 
start:
    
    ;limpar saidas
    BANKSEL PORTA
    clrf    PORTC
    clrf    PORTA
    bcf	    RE1
    
    ;todas as entradas digitais
    BANKSEL ANSEL
    clrf    ANSEL
    clrf    ANSELH
    
    BANKSEL TRISA
    clrf    TRISC	;config portas
    clrf    TRISA	;de display p saida

    bcf	    TRISE, 1	;limpando indicador
    			;de zero da saida

    bcf	    OPTION_REG, 7;config pull up portB

    clrf    STATUS	;limpando flags configurando
    			;pagina de memoria
    
loop:
    clrwdt
    
    bcf	    RE1
    
    movlw   0x20
    movwf   FSR
    
    clrf    q
    
    movf    PORTB, W
    movwf   n
    movwf   r
    movf    PORTD, W
    movwf   d
    
checkZ:			;verifica se um dos elementos e zero
			;em caso positivo, pula para o resultado
			;em que q = 0 e r = n.
			;tambem acende o LED em RE1.

    btfss   STATUS, 2
    goto    operation
    bsf	    RE1
    goto    result
    
operation:		;verifica se o numerador e maior que o
			;denominador. em caso negativo, tambem
			;pula para o resultado e exibe mesmo
			;resultado

    subwf   n, W
    btfss   STATUS, 0
    goto result
    
select:			;selecao de algoritmo para a divisao

    btfsc   RE0
    call    mode2
    btfss   RE0
    call    mode1
     
result:			;exibicao dos resultados
    movf    q, W
    movwf   PORTA
    movf    r, W
    movwf   PORTC
    goto    saveresp
    
mode1:			;algoritmo de subtracoes sucessivas
    
    movf    d, W
    subwf   r, W	;subtrai r - d.
    btfss   STATUS, 0	;caso r > d, incrementa o quociente e
    			;atualiza o valor do resto.

    return
    incf    q, f
    movwf   r
    movf    r, W
    btfsc   STATUS, 2
    return
    goto mode1		;repete ate que r < d, ou r = 0.
    
    
mode2:			;algoritmo de deslocamentos e subtracoes

    clrf    r
    movlw   0x08
    movwf   i		;8 iteracoes
    bcf	    STATUS, 0	;limpar flag da operacao anterior
    
subdesloc:
    rlf	    n, f
    rlf	    r, f	;carry de 'n' vai para 'r'
    
    movf    d, W
    subwf   r, W	;r - d

    btfsc   STATUS, 0
    movwf   r
    rlf	    q, f	;a partir de quando r > d, 'q' sera o 
    			;quociente com a rotacao do carry da 
			;operacao anterior

    decfsz  i, f
    goto    subdesloc	;repete ate completar as iteracoes
    return
    
saveresp:		;salva os resultados 
			;a partir de 0x20
    
    swapf   q, W	;MSB do quociente
    call    table
    movwf   INDF
    incf    FSR, f
    
    movf    q, W	;LSB do quociente
    call    table
    movwf   INDF
    incf    FSR, f
    
    swapf   r, W	;MSB do resto
    call    table
    movwf   INDF    
    incf    FSR, f
    
    movf    r, W	;LSB do resto
    call    table
    movwf   INDF
    
    goto    loop
    
    
ORG 0x060
table:			;LUT p ASCII
    andlw   0x0F
    addwf   PCL, f
    retlw   0x30
    retlw   0x31
    retlw   0x32
    retlw   0x33    
    retlw   0x34
    retlw   0x35
    retlw   0x36
    retlw   0x37
    retlw   0x38
    retlw   0x39
    retlw   0x41
    retlw   0x42
    retlw   0x43
    retlw   0x44
    retlw   0x45
    retlw   0x46
    
    
