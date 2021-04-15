section .text

line_read:
  
  ; Read line header
  mov  ah, 3Fh                ; READ FROM FILE OR DEVICE
  mov  bx, [fslides]
  mov  cx, 32                 ; number of bytes to read
  mov  dx, buff
  int  21h
  jc   .error
  test ax, ax                 ; EOF
  jz   .eof
  push ax                     ; save number of bytes read
  
  ; Parse delay
  mov  si, dx
  call parse_uint16
  jc   .error
  
  ; Convert delay
  mov  dx, 10                 ; multiplier
  mul  dx                     ; DX:AX = AX * 10
  mov  bx, 549                ; divisor
  div  bx                     ; AX = DX:AX / 549
  mov  [line_delay], ax
  
  ; Get type
  cmp  byte [si], 'T'
  je   .text_line
  cmp  byte [si], 'B'
  je   .bitmap_line
  jmp  .error                 ; unknown line type

.text_line:
  mov  byte [line_type], 0
  jmp .text_bitmap_line_common

.bitmap_line:
  mov  byte [line_type], 1

.text_bitmap_line_common:
  call line_read_position

  mov  dx, si
  sub  dx, buff               ; DX = line header length
  pop  bx                     ; restore number of bytes read
  xor  cx, cx
  sub  dx, bx                 ; DX = offset (zero or negative)
  jz   .rewind
  not  cx                     ; CX = 0FFFFh if DX is negative
.rewind:
  mov  ax, 4201h              ; SET CURRENT FILE POSITION -> from current
  mov  bx, [fslides]
  int  21h
  jc   .error

  ; Read content
  call line_read_content
  jc  .error
  jmp .end

.eof:
  xor  ax, ax
  ret

.error:
  stc

.end:
  mov  ax, [line_length]
  ret


line_read_position:
  ; Parse row number
  inc  si
  call parse_uint16
  jc   .error
  mov  [line_y], ax

  ; Parse column number
  call parse_uint16
  jnc  .column_number
  cmp  byte [si], '<'
  je   .column_left
  cmp  byte [si], '^'
  je   .column_center
  cmp  byte [si], '>'
  je   .column_right
  mov  [line_x], ax
  jmp  .error
  
.column_number:
  mov  [line_x], ax
  jmp  .end
  
.column_left:
  inc  si
  mov  word [line_x], 0
  jmp  .end
  
.column_center:
  inc  si
  mov  word [line_x], 0FFF1h
  jmp  .end
  
.column_right:
  inc  si
  mov  word [line_x], 0FFF2h
  jmp  .end

.error:
  stc
.end:
  ret


line_read_content:
  mov  byte [line_length], 0

  mov  ah, 3Fh                ; READ FROM FILE OR DEVICE
  mov  bx, [fslides]
  mov  cx, LINE               ; number of bytes to read
  mov  dx, buff
  int  21h
  jc   .error
  test ax, ax                 ; EOF
  jz   .end

  xor  dx, dx
  mov  si, buff
  mov  di, line_content
  cld
.next:
  mov  bl, [si]
  cmp  bl, 0Dh
  je   .break
  mov  [di], bl
  inc  si
  inc  di
  inc  dx
  cmp  dx, LINE
  je   .error
  jmp  .next

.break:
  mov  byte [di], 0
  mov  [line_length], dx
  xor  cx, cx
  sub  dx, ax                 ; DX = offset
  add  dx, 2
  jz   .rewind
  not  cx                     ; CX = 0FFFFh if DX is negative
.rewind:
  mov  ax, 4201h              ; SET CURRENT FILE POSITION -> from current
  mov  bx, [fslides]
  int  21h
  jc   .error
  jmp  .end

.error:
  stc
.end:
  ret

%include "strings.asm"
