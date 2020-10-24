						 
globalptr	RN r10 ;//=wram_globals* ptr
;//cpu_zpage	RN r11 ;=CPU_RAM
;----------------------------------------------------------------------------


 MAP 0,globalptr		  ;//MAP 用于定义一个结构化的内存表的首地址
				;//6502.s	  ;//定义内存表的首地址为globalptr
opz #   4              ;opz # 256*4       ;//代码表地址					  
readmem_tbl # 8*4			  ;//8*4
writemem_tbl # 8*4			  ;//8*4
memmap_tbl # 8*4			 ;//存储器映象 ram+rom
cpuregs # 7*4				 ;//1208存放6502寄存器保存的开始地址
m6502_s # 4					 ;//
lastbank # 4				;//6502PC从 ROM的最后偏移量
nexttimeout # 4

rombase # 4			;//ROM开始地址
romnumber # 4		 ;// ROM大小  
rommask # 4		   ;//ROM掩膜	rommask=romsize-1

joy0data # 4	   ;//串行数据
joy1data # 4	   ;//手柄1串行数据

clocksh # 4    ;//执行的时钟数 apu用
cpunmif # 4      ;cpu中断标志
cpuirqf # 4      ;cpu中断标志 
;------------------------------------------------------------------------------------------


;// # 2 ;align					 

		END
