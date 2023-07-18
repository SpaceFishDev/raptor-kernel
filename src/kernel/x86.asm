bits 16

section _TEXT class=CODE

global __U4D ; needed for watcom again...
__U4D:
    shl edx, 16         
    mov dx, ax         
    mov eax, edx      
    xor edx, edx
    shl ecx, 16       
    mov cx, bx   
    div ecx     
    mov ebx, edx
    mov ecx, edx
    shr ecx, 16
    mov edx, eax
    shr edx, 16
ret

global __U4M
__U4M: ; this is needed for WATCOM to function
    shl edx, 16       
    mov dx, ax       
    mov eax, edx   
    shl ecx, 16         
    mov cx, bx          
    mul ecx             
    mov edx, eax      
    shr edx, 16
ret

global _x86_div64_32
_x86_div64_32:
    push bp            
    mov bp, sp          
    push bx
    mov eax, [bp + 8]  
    mov ecx, [bp + 12] 
    xor edx, edx
    div ecx            
    mov bx, [bp + 16]
    mov [bx + 4], eax
    mov eax, [bp + 4]   
    div ecx
    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx
    pop bx
    mov sp, bp
    pop bp
ret

global _x86_Video_WriteCharTeletype
_x86_Video_WriteCharTeletype:
    push bp            
    mov bp, sp          
    push bx
    mov bl, [bp + 4]
    mov ah, 0Eh
    mov al, [bp + 6]
    mov bh, [bp + 8]
    int 10h
    pop bx
    mov sp, bp
    pop bp
ret

global _x86_Disk_Reset
_x86_Disk_Reset:
    push bp           
    mov bp, sp          
    mov ah, 0
    mov dl, [bp + 4]  
    stc
    int 13h
    mov ax, 1
    sbb ax, 0      
    mov sp, bp
    pop bp
ret

global _x86_Disk_Write_Sector:
_x86_Disk_Write_Sector:
    push bp
    mov bp, sp
    push bx
    push es
    mov ah, 0x03
    mov dl, [bp + 4]
    mov ch, [bp + 6] 
    mov cl, [bp + 7]
    shl cl, 6
    mov al, [bp + 8]
    and al, 3Fh
    or cl, al
    mov dh, [bp + 10]
    mov al ,[bp + 12]
    mov bx, [bp + 16]
    mov es, bx
    mov bx, [bp + 14]
    mov ah, 0x03
    stc
    int 0x13
    mov ax, 1
    sbb ax, 0
    pop es
    pop bx
    mov sp, bp
    pop bp
ret

global _x86_Set_Video_Mode
_x86_Set_Video_Mode:
    push bp
    mov bp, sp
    mov bh, 0x00
    mov al, [bp + 4] ; mode
    mov ah, 0x00
    int 0x10
    mov sp, bp
    pop bp
ret
;void _cdecl x86_Put_Pixel(uint8_t color, uint16_t x, uint16_t y);
global _x86_Put_Pixel
_x86_Put_Pixel:
    push bp
    mov bp, sp
    mov al, [bp + 4] 
    mov bh, 0x00
    mov cx, [bp + 6]
    mov dx, [bp + 8]
    mov ah, 0Ch
    int 0x10
    mov sp, bp
    pop bp
ret

global _x86_Disk_Read
_x86_Disk_Read:
    push bp           
    mov bp, sp          
    push bx
    push es
    mov dl, [bp + 4]  
    mov ch, [bp + 6]   
    mov cl, [bp + 7]    
    shl cl, 6
    mov al, [bp + 8]    
    and al, 3Fh
    or cl, al
    mov dh, [bp + 10]      
    mov al, [bp + 12]
    mov bx, [bp + 16]   
    mov es, bx
    mov bx, [bp + 14]
    mov ah, 02h
    stc
    int 13h
    mov ax, 1
    sbb ax, 0              
    pop es
    pop bx
    mov sp, bp
    pop bp
ret

global _x86_Disk_GetDriveParams
_x86_Disk_GetDriveParams:
    push bp            
    mov bp, sp        
    push es
    push bx
    push si
    push di
    mov dl, [bp + 4]    
    mov ah, 08h
    mov di, 0         
    mov es, di
    stc
    int 13h
    mov ax, 1
    sbb ax, 0
    mov si, [bp + 6]    
    mov [si], bl
    mov bl, ch          
    mov bh, cl          
    shr bh, 6
    mov si, [bp + 8]
    mov [si], bx
    xor ch, ch          
    and cl, 3Fh
    mov si, [bp + 10]
    mov [si], cx
    mov cl, dh          
    mov si, [bp + 12]
    mov [si], cx
    pop di
    pop si
    pop bx
    pop es
    mov sp, bp
    pop bp
ret

global _x86_Set_Cursor_Pos
_x86_Set_Cursor_Pos:
    push bp
    mov bp, sp
    mov bh, 0x00
    mov dh, [bp + 6]
    mov dl, [bp + 4]
    mov ah, 0x02
    int 0x10
    mov sp, bp
    pop bp
ret

global _x86_Reboot
_x86_Reboot:
    mov ah , 0
    int 16h
    mov ah, 0x00
    mov bh, 0x00
    int 10h
    cli
    jmp 0xFFFF:0

global _inb
_inb:
    push bp
    mov bp, sp
    mov dx, [bp + 4] 
    in ax, dx
    mov sp, bp
    pop bp
ret

global _outb
_outb:
    push bp
    mov bp, sp
    mov dx, [bp + 4]
    mov al, [bp + 6]
    out dx, al
    mov sp, bp
    pop bp
ret

global _install_idt_element
_install_idt_element:
    push bp
    mov bp, sp
    
    mov ax, [bp + 4]
    mov dx, [bp + 6]
    mov bx, [bp + 8]
    mov [bx], ax 
    mov [bx+2], dx 

    mov sp, bp
    pop bp
ret

global _getDS
_getDS:
    push bp
    mov bp , sp
    mov ax, ds
    mov sp, bp
    pop bp
ret