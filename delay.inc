.ifndef __6502_DELAY_INC__
.define __6502_DELAY_INC__

;
; There are two options.
;
; Inline Delay
;
; `delay_inline CYCLES` (between 2 and 325000 cycles)
;   Will inline between 2 and 26 bytes (~18 bytes on avarage)
;   that will consume CYCLES worth of cycles.
;
; Call Delay
;
; `delay_call CYCLES` (between 37 and 325000 cycles)
;   Will generate (always exactly 7 bytes):
;     ldx #..
;     ldy #..
;     jsr delay
;
; Must issue `delay_create_call` somewhere to declare the function.
; 

.define delay_2c nop
.define delay_3c bit z:$00

.macro delay_4c
    cmp z:$00,x
.endmacro

.macro delay_5c
    delay_2c
    delay_3c
.endmacro

.macro delay_6c
    delay_2c
    delay_4c
.endmacro

.macro delay_7c
    delay_3c
    delay_4c
.endmacro

.macro delay_inline N
    .local L0
    .local L1
    .local R

    R .set N

    .if R < 2
        .error "Must delay for at least two cycles."
    .endif
    .if R > 325000
        .error "Cant delay for more than 325000 cycles"
    .endif

    STATIC_C .set 25

    LOC_L0 .set *+4
    LOC_L1 .set LOC_L0+12

    ; Todo if we dont use the outer branch, we only need to relocate the inner...
    .if (R > STATIC_C) && ((LOC_L0 & $FF00) <> (LOC_L1 & $FF00))
        ; Move L0 into the same page
        MOV_DIST .set ($FF-(LOC_L0 & $FF))+1
        R .set R-(2*MOV_DIST)
        .repeat MOV_DIST
            delay_2c
        .endrep
    .endif

    EXTRA_C .set (R-STATIC_C) .MOD 5
    ITERATIONS .set ((R-STATIC_C) / 5)
    X_ITER .set (ITERATIONS+1) & $FF
    Y_ITER .set ((ITERATIONS>>8)+1) & $FF

    .if R < STATIC_C
        .repeat (R-2)/7
            delay_7c
            R .set R-7
        .endrep

        .if R & 1
            .if R >= 7
                delay_7c
                R .set R-7
            .elseif R=5
                delay_5c
                .exitmacro
            .elseif R=3
                delay_3c
                .exitmacro
            .endif
        .endif

        .repeat R / 6
            delay_6c
            R .set R-6
        .endrep
        ;R .set R-((R/6)*6)
        
        .if R = 6
            delay_6c
        .elseif R = 4
            delay_4c
        .elseif R = 2
            delay_2c
        .elseif R <> 0
            .error "I goofed up :o"
        .endif
        .exitmacro
    .endif

    ldx #X_ITER   ; 2
    ldy #Y_ITER   ; 2
L0:
    dex             ; 2
    bne L0          ; 2 (3)
    dex             ; 2
    dex             ; 2
    dex             ; 2
    delay_5c        ; 5
    dey             ; 2
    bne L0          ; 2 (3)
L1:
    .if EXTRA_C = 0
        delay_2c
    .elseif EXTRA_C = 1
        delay_3c
    .elseif EXTRA_C = 2
        delay_4c
    .elseif EXTRA_C = 3
        delay_5c
    .elseif EXTRA_C = 4
        delay_6c
    .else
        .error "Oops."
    .endif
.endmacro

.endif

.define RT_STATIC_C 27

.macro delay_call cycles
    .if cycles < (RT_STATIC_C+10)
        .error "Must delay at least 37 cycles. Use delay_inline for less resolution."
    .endif
    .if cycles > 325000
        .error "Cant delay for more than 325000 cycles"
    .endif

    R .set cycles
    R .set R-10
    OFFSET_C .set (R-RT_STATIC_C) .MOD 5

    .if OFFSET_C = 0
        BRANCH_OFF .set 0
    .elseif OFFSET_C = 1
        BRANCH_OFF .set 1
    .elseif OFFSET_C = 2
        BRANCH_OFF .set 2
    .elseif OFFSET_C = 3
        BRANCH_OFF .set 5
    .elseif OFFSET_C = 4
        BRANCH_OFF .set 8
    .else
        .error "Impossible."
    .endif

    ITERATIONS .set ((R-RT_STATIC_C) / 5)
    X_ITER .set (ITERATIONS+1) & $FF
    Y_ITER .set ((ITERATIONS>>8)+1) & $FF

    ldx #X_ITER
    ldy #Y_ITER
    jsr _delay_call_impl-BRANCH_OFF
.endmacro

.macro delay_create_call
    .local START
    .local L0
    .local DONE
START: 
_delay_call_4c:
    jmp _delay_call_1c
_delay_call_3c:
    jmp _delay_call_impl
_delay_call_2c:
    .byte $C9 ; (2 extra) cmp imm
_delay_call_1c:
    .byte $24 ; (1 extra) bit z:...
_delay_call_impl:
    nop
    ; always c&1==0
L0:
    dex       ; 2
    bne L0    ; 2/3
    dex       ; 2 (-5)
    dex       ; 2 (-10)
    dex       ; 2 (-15) => (-15 vs. 6+5)
    delay_5c  ; 4 -15 vs. 15
    dey       ; 2
    bne L0   ; 2/3
DONE:
    rts       ; 6

.if (START >> 8) <> (DONE >> 8)
    .error "You must create the delay function closer to a page boundary."
.endif
.endmacro


