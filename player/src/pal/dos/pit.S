	.arch i8086,jumps
	.code16
	.intel_syntax noprefix

	.text

	.global	dos_pit_isr
	.type	dos_pit_isr, @function
dos_pit_isr:
    push    ax
    push    bx
    push    ds

    /* Load program data segment */
    mov     ax, word ptr cs:[dos_data_segment]
    mov     ds, ax

    /* Increment main counter */
    mov     ax, word ptr [dos_counter]
    mov     bx, word ptr [dos_counter+2]
    add     ax, 1
    adc     bx, 0
    mov     word ptr [dos_counter], ax
    mov     word ptr [dos_counter+2], bx

    /* Call BIOS ISR every 32nd time */
    and     ax, 0b11111
    jz      .call_bios
    mov     al, 0x20
    out     0x20, al
    jmp     .end
.call_bios:
	pushf
	call	dword ptr [dos_pit_original_isr]

.end:
    pop     ds
    pop     bx
    pop     ax
    iret

	.global	dos_data_segment
	.type	dos_data_segment, @object
dos_data_segment:
    .fill   2
