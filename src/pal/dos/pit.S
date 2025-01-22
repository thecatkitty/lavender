	.arch i8086,jumps
	.code16
	.intel_syntax noprefix

	.text

	.global	__dos_pit_isr
	.type	__dos_pit_isr, @function
__dos_pit_isr:
    push    ax
    push    bx
    push    ds

    /* Load program data segment */
    mov     ax, word ptr cs:[__dos_ds]
    mov     ds, ax

    /* Increment main counter */
    mov     ax, word ptr [__dos_counter]
    mov     bx, word ptr [__dos_counter+2]
    add     ax, 1
    adc     bx, 0
    mov     word ptr [__dos_counter], ax
    mov     word ptr [__dos_counter+2], bx

    /* Call BIOS ISR every 32nd time */
    and     ax, 0b11111
    jz      .call_bios
    mov     al, 0x20
    out     0x20, al
    jmp     .end
.call_bios:
	pushf
	call	dword ptr [__dos_bios_isr]

.end:
    pop     ds
    pop     bx
    pop     ax
    iret

	.global	__dos_ds
	.type	__dos_ds, @object
__dos_ds:
    .fill   2
