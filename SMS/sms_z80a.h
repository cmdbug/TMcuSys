#include <stdint.h>
#ifndef __SMS_Z80A_H
#define __SMS_Z80A_H
//////////////////////////////////////////////////////////////////////////////////	 
//本程序移植自网友ye781205的NES模拟器工程
//ALIENTEK STM32F407开发板   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/10/12
//版本：V1.0  			  
////////////////////////////////////////////////////////////////////////////////// 	 

typedef struct _SMS_CPU80
{
	uint8_t   B;    //00 B
	uint8_t   C;    //01 C
	uint8_t   D;    //02 D  
	uint8_t   E;    //03 E  
	uint8_t   H;    //04 H  
	uint8_t   L;    //05 L
	uint8_t   F;    //06 F
	uint8_t   A;    //07 A
	uint16_t  IX;   //08 IX
	uint16_t  IY;   //0a IY
	uint16_t  PC;   //0c PC
	uint16_t  SP;   //0e SP
	uint32_t  MP;   //10 indirizzo 1K Memory Pointer array
	uint32_t  PT;   //14 indirizzo Parity Table bit bandingindirizzo平价表位条带
	uint32_t  PAR;  //18 HI: CPU ram base address, LO: work variable
	uint8_t   R;    //1c R
	uint8_t   I;    //1d I
	uint8_t   IFF;  //1e IFF
	uint8_t   RBM;  //1f ROM Bank Mask (numero bank -1)
	uint8_t   Bi;   //20 B'
	uint8_t   Ci;   //21 C'
	uint8_t   Di;   //22 D'  
	uint8_t   Ei;   //23 E'  
	uint8_t   Hi;   //24 H'  
	uint8_t   Li;   //25 L'
	uint8_t   Fi;   //26 F'
	uint8_t   Ai;   //27 A'
	uint8_t   CT;   //28 Controller port value控制器端口值
	uint8_t   VV;   //29 VDP_Vertical value 
	uint8_t   BS0;  //2a Bank Switch 0 (FFFC)
	uint8_t   BS3;  //2b Bank Switch 3 (FFFF)
	uint32_t  XROM; //2c Cartridge ROM base address
	uint32_t  XRAM; //30 Cartridge RAM base address
//  uint8_t*  Mempnt[64];		//pnt relativo a 1K
//  uint32_t  P_Tab[8];         //Parity Tab
//  uint8_t*  Ram;  
}SMS_CPU80;
extern SMS_CPU80 *Z80A;


int32_t SMS_CPU_run(int32_t cycle);  //esegue codice CPU finche' (cycle)>0
void    SMS_CPU_Irq(void);           //RST38
void    SMS_CPU_Reset(void);         //Reset
int8_t  SMS_CPU_Init(uint8_t* SMS_Ram, uint8_t* Ext_Ram, uint8_t* Ext_Rom, uint8_t RomBanksMask);
void    Z80A_del_ram(void);
void    SetRiga(uint8_t VDP_V);
void    SetController(uint8_t padstate);

#endif
