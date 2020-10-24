	INCLUDE equates.s

	
	IMPORT NES_RAM
	IMPORT NES_SRAM	
	IMPORT CPU_reset
	IMPORT romfile ;from main.c
	IMPORT cpu_data	   ; 6502.s	
	IMPORT op_table
		
	EXPORT cpu6502_init
	EXPORT map67_
	EXPORT map89_
	EXPORT mapAB_
	EXPORT mapCD_
	EXPORT mapEF_
;----------------------------------------------------------------------------
 AREA rom_code, CODE, READONLY
;	THUMB
   PRESERVE8   
;----------------------------------------------------------------------------
cpu6502_init PROC
;----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}
		
    ldr r10,=cpu_data	;读取地址
	ldr r11,=NES_RAM	;r11=cpu_zpage
	
        ldr r11,[r11]       ;NES_RAM用了指针 
;*******************************************************		
		str r11,memmap_tbl             ;NES_RAM用了指针
		str r11,memmap_tbl+4
		str r11,memmap_tbl+8
  
		ldr r0,=NES_SRAM              ;NES_SRAM用了指针
		ldr r0,[r0]
		str r0,memmap_tbl+12
;**********************************************************************			
		
	ldr r0,=op_table   ;获取代码跳转表地址
	str r0,opz         ;保存

	ldr r0,=romfile		 
	ldr r0,[r0]			 ;R0现在指向ROM映像（包括头）
	add r3,r0,#16		;r3现在指向rom镜像(不包括头）
	str r3,rombase		;设置rom基地址
						;r3=rombase til end of loadcart so DON'T FUCK IT UP
	mov r2,#1
	ldrb r1,[r3,#-12]	; 16kB PROM的数目  	 2
	rsb r0,r2,r1,lsl#14	 ;romsize=X*16KB	 <<14 逆向减法指令	 r0=0x7fff
	str r0,rommask		;rommask=promsize-1	 32768-1	
;------------------------------------------------------------------------------------	
	mov r9,#0		;(消除任何encodePC的映射器*初始化过程中的错误)
	str r9,lastbank		;6502PC从 ROM的最后偏移量写0

	mov r0,#0			;默认rom映射
	bl map89AB_			;89AB=1st 16k
	mov r0,#-1
	bl mapCDEF_			;CDEF=last 16k
;----------------------------------------------------------------------------

    ldrb r1,[r3,#-10]		;get mapper#
	ldrb r2,[r3,#-9]
	tst r2,#0x0e			;long live DiskDude!
	and r1,r1,#0xf0
	and r2,r2,#0xf0
	orr r0,r2,r1,lsr#4
	movne r0,r1,lsr#4		;ignore high nibble if header looks bad	忽略高四位，如果头看起来很糟糕
                            ;r0=mapper号
;----------------------------------------------------------------------------	
    
	ldr r0,=Mapper_W
	str r0,writemem_tbl+16
	str r0,writemem_tbl+20
	str r0,writemem_tbl+24
	str r0,writemem_tbl+28
;------------------------------------------------------------------------------

	bl CPU_reset		;reset everything else
	ldmfd sp!,{r4-r11,lr}
	bx lr
	ENDP

;----------------------------------------------------------------------------
map67_	;rom paging.. r0=page#
;----------------------------------------------------------------------------
	ldr r1,rommask
	and r0,r1,r0,lsl#13
	ldr r1,rombase
	add r0,r1,r0
	sub r0,r0,#0x6000
	str r0,memmap_tbl+12
	b flush
;----------------------------------------------------------------------------
map89_	;rom paging.. r0=page# ROM分页
;----------------------------------------------------------------------------
	ldr r1,rombase			 ;rom开始地址
	sub r1,r1,#0x8000
	ldr r2,rommask
	and r0,r2,r0,lsl#13
	add r0,r1,r0
	str r0,memmap_tbl+16
	b flush
;----------------------------------------------------------------------------
mapAB_
;----------------------------------------------------------------------------
	ldr r1,rombase
	sub r1,r1,#0xa000
	ldr r2,rommask
	and r0,r2,r0,lsl#13
	add r0,r1,r0
	str r0,memmap_tbl+20
	b flush
;----------------------------------------------------------------------------
mapCD_
;----------------------------------------------------------------------------
	ldr r1,rombase
	sub r1,r1,#0xc000
	ldr r2,rommask
	and r0,r2,r0,lsl#13
	add r0,r1,r0
	str r0,memmap_tbl+24
	b flush
;----------------------------------------------------------------------------
mapEF_
;----------------------------------------------------------------------------
	ldr r1,rombase
	sub r1,r1,#0xe000
	ldr r2,rommask
	and r0,r2,r0,lsl#13
	add r0,r1,r0
	str r0,memmap_tbl+28
	b flush
;----------------------------------------------------------------------------
map89AB_
;----------------------------------------------------------------------------
	ldr r1,rombase		   ;rom基地址（不包括头）
	sub r1,r1,#0x8000
	ldr r2,rommask
	and r0,r2,r0,lsl#14
	add r0,r1,r0
	str r0,memmap_tbl+16
	str r0,memmap_tbl+20
flush		;update m6502_pc & lastbank
	ldr r1,lastbank
	sub r9,r9,r1
	and r1,r9,#0xE000	   ;//r9和0xe000按位与运算
	adr r2,memmap_tbl		   ;//把存储器映象地址加载到r2
	lsr r1,r1,#11				;//>>11位	  r1/2048
	ldr r0,[r2,r1]				;//读取r2地址+r1偏移的数据到r0

	str r0,lastbank				;//保存6502PC从 ROM的最后偏移量 
	add r9,r9,r0	;//m6502_pc+r0
	orr lr,#0x01		;lr最低位置1防止进入arm状态
	bx lr

;----------------------------------------------------------------------------
mapCDEF_
;----------------------------------------------------------------------------
	ldr r1,rombase
	sub r1,r1,#0xc000
	ldr r2,rommask
	and r0,r2,r0,lsl#14
	add r0,r1,r0
	str r0,memmap_tbl+24
	str r0,memmap_tbl+28
	b flush
;----------------------------------------------------------------------------
map89ABCDEF_
;----------------------------------------------------------------------------
	ldr r1,rombase
	sub r1,r1,#0x8000
	ldr r2,rommask
	and r0,r2,r0,lsl#15
	add r0,r1,r0
	str r0,memmap_tbl+16
	str r0,memmap_tbl+20
	str r0,memmap_tbl+24
	str r0,memmap_tbl+28
	b flush
;*************************************************************************************
     IMPORT asm_Mapper_Write;
Mapper_W	
;-------------------------------------------
	stmfd sp!,{r3,lr}	;LR 寄存器放栈
	mov r1,r12
	bl asm_Mapper_Write
	ldmfd sp!,{r3,lr}
	orr lr,#0x01		;lr最低位置1防止进入arm状态
	bx lr
;	nop
;---------------------------------------------------------------------------------------

    END





