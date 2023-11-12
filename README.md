
# delay.inc - CA65 cycle delay macro

# About

Delay an exact number of 6502 cycles. Implements two different alternatives to delay.

## delay_inline

Using `delay_inline NUM_CYCLES` will inline between 2 and 26 bytes (~18 bytes on average).


## delay_call

Using `delay_call NUM_CYCLES` will emit:

```
ldx #..
ldy #..
jsr _delay_call_impl
```
To declare `_delay_call_impl` you must issue `delay_create_call` (consumes 21 bytes).

## Examples

```
.include "delay.inc"
.org $8000
delay_inline 123 
brk ; Reached at exactly cycle 123
```

```
.include "delay.inc"
.org $8000
delay_call 321
brk ; Reacted at exactly cycle 321
delay_create_call
```

