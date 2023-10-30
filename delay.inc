.ifndef __6502_DELAY_INC__
.define __6502_DELAY_INC__

.define delay_2c nop
.define delay_3c bit z:$00

.macro delay_4c
    .local L0
L0:
    bit a:L0
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
    pha
    pla
.endmacro

.macro delay N
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
