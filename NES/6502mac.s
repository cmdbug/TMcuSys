
C EQU 0x01	;//6502 flags  6502标志
Z EQU 0x02
I EQU 0x04
D EQU 0x08
B EQU 0x10	;//(allways 1 except when IRQ pushes it)IRQ外部中断
R EQU 0x20	;//(locked at 1)
V EQU 0x40
N EQU 0x80



	MACRO		;//translate from 6502 PC to rom offset翻译从6502 PC ROM的偏移量
	encodePC
	and r1,m6502_pc,#0xE000	   ;//r9和0xe000按位与运算
	adr r2,memmap_tbl		   ;//把存储器映象地址加载到r2
;//	ldr r0,[r2,r1,lsr#11]	   ;//改过，加2行 
	lsr r0,r1,#11				;//>>11位	  r1/2048
	ldr r0,[r2,r0]				;//读取r2地址+r1偏移的数据到r0
	
	str r0,lastbank				;//保存6502PC从 ROM的最后偏移量 
	add m6502_pc,m6502_pc,r0	;//m6502_pc+r0
	MEND

	MACRO		;//pack 6502 flags into r0   6502标志包装到R0
	encodeP $extra
	and r0,cycles,#CYC_V+CYC_D+CYC_I+CYC_C
	tst m6502_nz,#0x80000000;//PSR_N
	orrne r0,r0,#N				;N
	tst m6502_nz,#0xff
	orreq r0,r0,#Z				;Z
	orr r0,r0,#$extra			;R(&B)
	MEND

	MACRO		;//;宏定义//unpack 6502 flags from r0	  解压缩6502从R0的标志
	decodeP
	bic cycles,cycles,#CYC_V+CYC_D+CYC_I+CYC_C
	and r1,r0,#V+D+I+C
	orr cycles,cycles,r1		;//VDIC
	bic m6502_nz,r0,#0xFD			;//r0 is signed
	eor m6502_nz,m6502_nz,#Z
	MEND		;//		  ;宏定义结束

	MACRO
	fetch $count			   ;//提取操作码	;$标号 宏名 $参数1，$参数2，...
;//---------------------------------------------------------------------
	ldr r0,clocksh			   ;//处理apu需要的时钟数
	add r0,r0,#$count
	str r0,clocksh

	ldr r1,opz            ;//获取代码跳转表地址
;//-------------------------------------------------------------------------
	subs cycles,cycles,#$count*256;//CYCLE=256 ;//	3*256 正数或0就执行下2条指令  
	ldrplb r0,[m6502_pc],#1			   ; //从存储器中加载字节到一个寄存器中	 r0=操作码
;	ldrpl pc,[m6502_optbl,r0,lsl#2]	  ;//r10 ********r0=r0x4***运行代码的地址**************************************
	ldrpl pc,[r1,r0,lsl#2]
	ldr pc,nexttimeout
	MEND

	MACRO						;//相同的提取，但它增加了进位（位0）
	fetch_c $count				;//same as fetch except it adds the Carry (bit 0) also.
;//---------------------------------------------------------------------
	ldr r0,clocksh				;//处理apu需要的时钟数
	add r0,r0,#$count
	str r0,clocksh	
	
	ldr r1,opz       ;//获取代码跳转表地址
;//-------------------------------------------------------------------------
	sbcs cycles,cycles,#$count*256;//CYCLE=256
	ldrplb r0,[m6502_pc],#1
;	ldrpl pc,[m6502_optbl,r0,lsl#2]
	ldrpl pc,[r1,r0,lsl#2]
	ldr pc,nexttimeout
	MEND

	MACRO
	clearcycles
	and cycles,cycles,#CYC_MASK		;Save CPU bits
	MEND

	MACRO
	readmemabs
	and r1,addy,#0xE000
	adr lr,%F0
;//	ldr pc,[m6502_rmem,r1,lsr#11]	;//in: addy,r1=addy&0xE000 (for rom_R)
	lsr r1,r1,#11				 ;//改过，加2行	 >>11
	ldr pc,[m6502_rmem,r1]
				
0				;//out: r0=val (bits 8-31=0 (LSR,ROR,INC,DEC,ASL)), addy preserved for RMW instructions
	MEND

	MACRO
	readmemzp
	ldrb r0,[cpu_zpage,addy]
	MEND

	MACRO
	readmemzpi
;//	ldrb r0,[cpu_zpage,addy,lsr#24]
	lsr r0,addy,#24				  ;//改过，加3行
	ldrb r0,[cpu_zpage,r0]

	MEND

	MACRO
	readmemzps
	ldrsb m6502_nz,[cpu_zpage,addy];RAM
	MEND

	MACRO
	readmemimm
	ldrb r0,[m6502_pc],#1  ;ROM
	MEND

	MACRO
	readmemimms
	ldrsb m6502_nz,[m6502_pc],#1
	MEND

	MACRO
	readmem
	[ _type = _ABS
		readmemabs
	]
	[ _type = _ZP
		readmemzp
	]
	[ _type = _ZPI
		readmemzpi
	]
	[ _type = _IMM
		readmemimm
	]
	MEND

	MACRO
	readmems
	[ _type = _ABS
		readmemabs
		orr m6502_nz,r0,r0,lsl#24
	]
	[ _type = _ZP
		readmemzps
	]
	[ _type = _IMM
		readmemimms
	]
	MEND


	MACRO
	writememabs
	and r1,addy,#0xe000
	adr r2,writemem_tbl
	adr lr,%F0
;//	ldr pc,[r2,r1,lsr#11]	;//in: addy,r0=val(bits 8-31=?)
	lsr r1,r1,#11				 ;//改过，加2行 >>11
	ldr pc,[r2,r1]

0				;out: r0,r1,r2,addy=?
	MEND

	MACRO
	writememzp
	strb r0,[cpu_zpage,addy]
	MEND

	MACRO
	writememzpi
;//	strb r0,[cpu_zpage,addy,lsr#24]
	lsr r1,addy,#24				 ;//改过，加2行	 >>24
	strb r0,[cpu_zpage,r1]


	MEND

	MACRO
	writemem			   ;//写内存
	[ _type = _ABS
		writememabs
	]
	[ _type = _ZP
		writememzp
	]
	[ _type = _ZPI
		writememzpi
	]
	MEND
;----------------------------------------------------------------------------

	MACRO					  ;///////////////////////////////// /////////////////////
	push16		;push r0
	mov r1,r0,lsr#8
	ldr r2,m6502_s
	strb r1,[r2],#-1
	orr r2,r2,#0x100
	strb r0,[r2],#-1
	strb r2,m6502_s
	MEND		;r1,r2=?

	MACRO
	push8 $x
	ldr r2,m6502_s
	strb $x,[r2],#-1
	strb r2,m6502_s
	MEND		;r2=?

	MACRO
	pop16		;pop m6502_pc
	ldrb r2,m6502_s
	add r2,r2,#2
	strb r2,m6502_s
	ldr r2,m6502_s
	ldrb r0,[r2],#-1
	orr r2,r2,#0x100
	ldrb m6502_pc,[r2]
	orr m6502_pc,m6502_pc,r0,lsl#8
	MEND		;r0,r1=?

	MACRO
	pop8 $x
	ldrb r2,m6502_s
	add r2,r2,#1
	strb r2,m6502_s
	orr r2,r2,#0x100
	ldrsb $x,[cpu_zpage,r2]		;signed for PLA & PLP

	MEND	;r2=?

;----------------------------------------------------------------------------
;doXXX: load addy, increment m6502_pc

	GBLA _type

_IMM	EQU 1						;immediate
_ZP		EQU 2						;zero page
_ZPI	EQU 3						;zero page indexed
_ABS	EQU 4						;absolute

	MACRO
	doABS                           ;absolute               $nnnn
_type	SETA      _ABS
	ldrb addy,[m6502_pc],#1
	ldrb r0,[m6502_pc],#1
	orr addy,addy,r0,lsl#8
	MEND

	MACRO
	doAIX                           ;absolute indexed X     $nnnn,X
_type	SETA      _ABS
	ldrb addy,[m6502_pc],#1
	ldrb r0,[m6502_pc],#1
	orr addy,addy,r0,lsl#8
	add addy,addy,m6502_x,lsr#24
;	bic addy,addy,#0xff0000 ;Base Wars needs this
	MEND

	MACRO
	doAIY                           ;absolute indexed Y     $nnnn,Y
_type	SETA      _ABS
	ldrb addy,[m6502_pc],#1
	ldrb r0,[m6502_pc],#1
	orr addy,addy,r0,lsl#8
	add addy,addy,m6502_y,lsr#24
;	bic addy,addy,#0xff0000 ;Tecmo Bowl needs this
	MEND

	MACRO
	doIMM                           ;immediate              #$nn
_type	SETA      _IMM
	MEND

	MACRO
	doIIX                           ;indexed indirect X     ($nn,X)
_type	SETA      _ABS
	ldrb r0,[m6502_pc],#1
	add r0,m6502_x,r0,lsl#24
	;//ldrb addy,[cpu_zpage,r0,lsr#24]	;//错误:指定的转变不允许
    lsr addy,r0,#24			  ;//改过，加2行 >>24
	ldrb addy,[cpu_zpage,addy]

	add r0,r0,#0x01000000
	;//ldrb r1,[cpu_zpage,r0,lsr#24]	;//R1,LSR#2;将R1中的内容右移2位
	lsr r1,r0,#24			  ;//改过，加2行
	ldrb r1,[cpu_zpage,r1]

	orr addy,addy,r1,lsl#8
	MEND

	MACRO
	doIIY                           ;indirect indexed Y     ($nn),Y
_type	SETA      _ABS
	ldrb r0,[m6502_pc],#1
;//	ldrb addy,[r0,cpu_zpage]! ;;在数据传送之前,将偏移量加到Rn 中,其结果作为传送数据的存储地址
                            ;//若使用后缀"!",则结果写回到Rn中
	ldrb addy,[r0,cpu_zpage]
	add r0,r0,cpu_zpage			;//////////////////////////////////////


	ldrb r1,[r0,#1]
	orr addy,addy,r1,lsl#8
	add addy,addy,m6502_y,lsr#24
;	bic addy,addy,#0xff0000 ;Zelda2 needs this
	MEND

	MACRO
	doZPI							;Zeropage indirect     ($nn)
_type	SETA      _ABS
	ldrb r0,[m6502_pc],#1
;//	ldrb addy,[r0,cpu_zpage]!;;在数据传送之前,将偏移量加到Rn 中,其结果作为传送数据的存储地址
                            ;//若使用后缀"!",则结果写回到Rn中
	ldrb addy,[r0,cpu_zpage]
	add r0,r0,cpu_zpage	
	
	
	ldrb r1,[r0,#1]
	orr addy,addy,r1,lsl#8
	MEND

	MACRO
	doZ                             ;zero page              $nn
_type	SETA      _ZP
	ldrb addy,[m6502_pc],#1
	MEND

	MACRO
	doZ2							;zero page              $nn
_type	SETA      _ZP
	ldrb addy,[m6502_pc],#2			;ugly thing for bbr/bbs
	MEND

	MACRO
	doZIX                           ;zero page indexed X    $nn,X
_type	SETA      _ZP
	ldrb addy,[m6502_pc],#1
	add addy,addy,m6502_x,lsr#24
	and addy,addy,#0xff ;Rygar needs this
	MEND

	MACRO
	doZIXf							;zero page indexed X    $nn,X
_type	SETA      _ZPI
	ldrb addy,[m6502_pc],#1
	add addy,m6502_x,addy,lsl#24
	MEND

	MACRO
	doZIY                           ;zero page indexed Y    $nn,Y
_type	SETA      _ZP
	ldrb addy,[m6502_pc],#1
	add addy,addy,m6502_y,lsr#24
	and addy,addy,#0xff
	MEND

	MACRO
	doZIYf							;zero page indexed Y    $nn,Y
_type	SETA      _ZPI
	ldrb addy,[m6502_pc],#1
	add addy,m6502_y,addy,lsl#24
	MEND

;----------------------------------------------------------------------------

	MACRO
	opADC
	readmem
	movs r1,cycles,lsr#1		;get C
	subcs r0,r0,#0x00000100
	adcs m6502_a,m6502_a,r0,ror#8
	mov m6502_nz,m6502_a,asr#24		;NZ
	orr cycles,cycles,#CYC_C+CYC_V	;Prepare C & V
	bicvc cycles,cycles,#CYC_V	;V
	MEND

	MACRO
	opAND
	readmem
	and m6502_a,m6502_a,r0,lsl#24
	mov m6502_nz,m6502_a,asr#24		;NZ
	MEND

	MACRO
	opASL
	readmem
	 add r0,r0,r0
	 orrs m6502_nz,r0,r0,lsl#24		;NZ
	 orr cycles,cycles,#CYC_C		;Prepare C
	writemem
	MEND

	MACRO
	opBIT
	readmem
	bic cycles,cycles,#CYC_V		;reset V
	tst r0,#V
	orrne cycles,cycles,#CYC_V		;V
	and m6502_nz,r0,m6502_a,lsr#24	;Z
	orr m6502_nz,m6502_nz,r0,lsl#24	;N
	MEND

	MACRO
	opCOMP $x			;A,X & Y
	readmem
	subs m6502_nz,$x,r0,lsl#24
	mov m6502_nz,m6502_nz,asr#24	;NZ
	orr cycles,cycles,#CYC_C	;Prepare C
	MEND

	MACRO
	opDEC
	readmem
	sub r0,r0,#1
	orr m6502_nz,r0,r0,lsl#24		;NZ
	writemem
	MEND

	MACRO
	opEOR
	readmem
	eor m6502_a,m6502_a,r0,lsl#24
	mov m6502_nz,m6502_a,asr#24		;NZ
	MEND

	MACRO
	opINC
	readmem
	add r0,r0,#1
	orr m6502_nz,r0,r0,lsl#24		;NZ
	writemem
	MEND

	MACRO
	opLOAD $x
	readmems
	mov $x,m6502_nz,lsl#24
	MEND

	MACRO
	opLSR
	[ _type = _ABS
		readmemabs
		movs r0,r0,lsr#1
		orr cycles,cycles,#CYC_C	;Prepare C
		mov m6502_nz,r0				;Z, (N=0)
		writememabs
	]
	[ _type = _ZP
		ldrb m6502_nz,[cpu_zpage,addy]
		movs m6502_nz,m6502_nz,lsr#1	;Z, (N=0)
		orr cycles,cycles,#CYC_C	;Prepare C
		strb m6502_nz,[cpu_zpage,addy]
	]
	[ _type = _ZPI
;//		ldrb m6502_nz,[cpu_zpage,addy,lsr#24]
		lsr m6502_nz,addy,#24				  ;//改过，加2行
		ldrb m6502_nz,[cpu_zpage,m6502_nz]

		movs m6502_nz,m6502_nz,lsr#1	;Z, (N=0)
		orr cycles,cycles,#CYC_C	;Prepare C
;//		strb m6502_nz,[cpu_zpage,addy,lsr#24]
		lsr r1,addy,#24				   ;//改过，加2行
		strb m6502_nz,[cpu_zpage,r1]
										 
	]
	MEND

	MACRO
	opORA
	readmem
	orr m6502_a,m6502_a,r0,lsl#24
	mov m6502_nz,m6502_a,asr#24
	MEND

	MACRO
	opROL
	readmem
	 movs cycles,cycles,lsr#1		;get C
	 adc r0,r0,r0
	 orrs m6502_nz,r0,r0,lsl#24		;NZ
	 adc cycles,cycles,cycles		;Set C
	writemem
	MEND

	MACRO
	opROR
	readmem
	 movs cycles,cycles,lsr#1		;get C
	 orrcs r0,r0,#0x100
	 movs r0,r0,lsr#1
	 orr m6502_nz,r0,r0,lsl#24		;NZ
	 adc cycles,cycles,cycles		;Set C
	writemem
	MEND

	MACRO
	opSBC
	readmem
	movs r1,cycles,lsr#1			;get C
	sbcs m6502_a,m6502_a,r0,lsl#24
	and m6502_a,m6502_a,#0xff000000
	mov m6502_nz,m6502_a,asr#24 		;NZ
	orr cycles,cycles,#CYC_C+CYC_V	;Prepare C & V
	bicvc cycles,cycles,#CYC_V		;V
	MEND

	MACRO
	opSTORE $x
	mov r0,$x,lsr#24
	writemem
	MEND
;----------------------------------------------------
	END
