.global Fix_SAM3XA
.type Fix_SAM3XA, %function
Fix_SAM3XA:
    /* fix stack pointer */
    ldr r0, =_sfixed
    ldr r0, [r0, #0]
    mov sp, r0

    /* set cortex-m3 reset values */
    ldr r0, =0xFFFFFFFF
    mov lr, r0

    /* Disable interrupts and configurable fault handlers (set PRIMASK) */
    cpsid i 

    /* Check for interrupts */
    mrs r0, ipsr
    cmp r0, #0
    bne clear_interrupt
    ldr r0 ,=Reset_Handler
    bx r0

/* Simulate Exception Return */
clear_interrupt:
    ldr r0, =0x01000000 // psr
    push {r0} 
    
    ldr r0, =Reset_Handler // pc
    push {r0}

    push {lr}
    mov r0 , r12
    push {r0}
    push {r3}
    push {r2}
    push {r1}
    push {r0}

    ldr r0, =0xFFFFFFF9 // EXC_RETURN
    bx r0
