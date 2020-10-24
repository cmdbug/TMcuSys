#include "sms_z80a.h" 
#include <string.h>
//////////////////////////////////////////////////////////////////////////////////	 
//本程序移植自网友ye781205的NES模拟器工程
//ALIENTEK STM32F407开发板   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/10/12
//版本：V1.0  			  
////////////////////////////////////////////////////////////////////////////////// 	 


SMS_CPU80 *Z80A;

int32_t SMS_CPU_exec(int32_t cycle,void* cpu);

uint8_t*  Mempnt[64];						//pnt relativo a 1K
uint32_t  P_Tab[8];           				//Parity Tab
uint8_t*  Ram;               				//SMS_Ram               

int8_t SMS_CPU_Init(uint8_t* SMS_Ram, uint8_t* Ext_Ram, uint8_t* Ext_Rom, uint8_t RomBanksMask)
{
	uint32_t i,k,p;
	uint32_t* bb_p_tab;
	memset(Z80A,0,sizeof(SMS_CPU80));		//清零Z80A所有内容
	Ram=SMS_Ram;
	Z80A->XRAM=(uint32_t)Ext_Ram;		//Cartridge RAM
	Z80A->XROM=(uint32_t)Ext_Rom;	  	//Cartridge ROM 
	Z80A->RBM=RomBanksMask;	        	//And mask per n. banchi ROM 
	for(i=0;i<48;i++)Mempnt[i]=Ext_Rom;
	for(i=48;i<56;i++)
	{
		Mempnt[i]=SMS_Ram-0xc000;
		Mempnt[i+8]=SMS_Ram-0xE000;
	}
	Z80A->MP=(uint32_t)Mempnt;
	//Calcola indirizzo per bit banding su P_Tab每一点Calcola indirizzo条带P_Tab
	bb_p_tab=(uint32_t*)(((((uint32_t) P_Tab)&0x3ffff)<<5)|0x22000000);
	//Calcola parity bit 
	for(i=0;i<256;i++)
	{
		p=1;
		k=i;
		while(k)
		{
			if(k & 1)
			p^=1;
			k>>=1;
		}
		bb_p_tab[i]=p;
	}
	Z80A->PT=(uint32_t)bb_p_tab;
	Z80A->CT=0xff;
	Z80A->SP=0xf0;
	return 0;
} 
void SetRiga(uint8_t VDP_V)
{
	Z80A->VV=VDP_V;
} 
void SetController(uint8_t padstate)
{
	Z80A->CT=padstate;
} 
void SMS_CPU_Irq() //semplificata per solo RST38 
{
	uint16_t ad;
	// Se IFF HALT=1 lo azzera
	if(Z80A->IFF&0x80)Z80A->IFF &=0x7f;
	if(Z80A->IFF&0x02) //IFF1
	{
		Z80A->IFF&=0xFD;
		ad=(Z80A->SP>>8)|((Z80A->SP&0xff)<<8);
		ad--;
		Mempnt[ad>>10][ad]=Z80A->PC>>8;
		ad--;
		Mempnt[ad>>10][ad]=Z80A->PC&0xff;
		Z80A->SP=(ad>>8)|((ad&0xff)<<8);
		Z80A->PC=0x38;
	}  
}
int32_t SMS_CPU_run(int32_t cyc)
{
	return SMS_CPU_exec(cyc,&*Z80A);
}
// offset CPU fields 抵消CPU字段
#define ofsBC   0x00
#define ofsB    0x00
#define ofsC    0x01
#define ofsDE   0x02
#define ofsHL   0x04
#define ofsIX   0x08
#define ofsPC   0x0c
#define ofsSP   0x0e
#define ofsMP   0x10
#define ofsPT   0x14
#define ofsR6   0x06
#define ofsA    0x07
#define ofsPar  0x18
#define ofsR6i  0x26
#define ofsI    0x1c
#define ofsR    0x1d
#define ofsIFF  0x1e
#define ofsCT   0x28
#define ofsVV   0x29
#define ofsBS0  0x2a
#define ofsBS3  0x2b
#define ofsRBM  0x1f
#define ofsXROM 0x2c
#define ofsXRAM 0x30

//CM4 register alias
#define r_acc R0
#define r_jmp R1
#define r_ad  R2
#define r_res R3
#define r_op  R4
#define r_rp  R5
#define r_sr  R6
#define r_mp  R7
#define r_par R8
#define r_pc  R9
#define r_cyc R10
#define r_pnt R11
#define r_pt  R12

extern uint8_t VDP_read(uint16_t Port);
extern void VDP_write(uint8_t Value, uint16_t Port);
extern void SN76496Write(int data);
//esegue codice Z80 per max cycle
int32_t __asm SMS_CPU_exec(int32_t cycle,void* cpu)
{
  PRESERVE8                    
  
  PUSH	{r1-r12,lr}             // push all regs
  MOVS  r_cyc,r0
  MOV   r_rp,r1
  LDRB  r_acc,[r_rp,#ofsIFF]
  TST   r_acc,#0x80             // CPU Halt State
  BEQ   cpu_run
  ANDS  r_acc,r_cyc,#3          // se((cyc & 3)!=0) 
  SUBNE r_acc,#4                // cyc-=4
  POP	  {r1-r12,pc}             // exit
cpu_run
  LDRH  r_pc,[r_rp,#ofsPC]
  LDRB  r_acc,[r_rp,#ofsR6]
  // load r_sr con Z80 flag(r_acc)   S  Z  x  H  x  PV N  C  
  UBFX  r_res,r_acc,#4,#1       //   0  0  0  0  0  0  0  H
  BFI   r_acc,r_acc,#5,#1       //   S  Z  C  H  x  PV N  C
  BFI   r_acc,r_acc,#2,#3       //   S  Z  C  PV N  C  N  C
  BFI   r_acc,r_res,#3,#1       //   S  Z  C  PV H  C  N  C
  BFI   r_acc,r_acc,#1,#2       //   S  Z  C  PV H  N  C  C
  LSL   r_sr,r_acc,#24  
  LDR   r_mp,[r_rp,#ofsMP]  
  LDR   r_pt,[r_rp,#ofsPT]  
  LDR   r_par,[r_rp,#ofsPar]  
FetchNext
  AND   r_par,#0xfffffff8
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_op,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  BL    No_Prefix_Switch
  SUBS  r_cyc,#4
  BPL   FetchNext
run_exit
  //REV16 r_pc,r_pc
  STRH  r_pc,[r_rp,#ofsPC]
  // r_acc = r_sr in formato Z80 flag
  RBIT  r_acc,r_sr              // r_acc  x  x  N  H  PV C  Z  S
  BFI   r_sr,r_acc,#24,#6       // r_sr   S  Z  N  H  PV C  Z  S ......
  BFI   r_sr,r_acc,#23,#4       // r_sr   S  Z  N  H  H  PV C  Z ......  
  BFI   r_sr,r_acc,#20,#6       // r_sr   S  Z  C  H  H  PV N  H ......  
  BFI   r_sr,r_acc,#22,#3       // r_sr   S  Z  C  H  H  PV N  C ......  
  LSR   r_acc,r_sr,#24          // r_sr   S  Z  C  H  H  PV N  C
  AND   r_acc,#0xD7
  STRB  r_acc,[r_rp,#ofsR6]
  STR   r_par,[r_rp,#ofsPar]  
  MOV   R0,r_cyc
  POP	  {r1-r12,pc}

No_Prefix_Switch
  TBH   [PC, r_op, LSL #1] 
No_Prefix_Opcode
  DCI  ((NOP_4     - No_Prefix_Opcode)/2) //00 NOP            
  DCI  ((LD_BC_nn  - No_Prefix_Opcode)/2) //01 LD   BC,nn 
  DCI  ((ST_A_BC   - No_Prefix_Opcode)/2) //02 LD   (BC),A
  DCI  ((INC_BC    - No_Prefix_Opcode)/2) //03 INC  BC
  DCI  ((INC_B     - No_Prefix_Opcode)/2) //04 INC  B
  DCI  ((DEC_B     - No_Prefix_Opcode)/2) //05 DEC  B 
  DCI  ((LD_B_n    - No_Prefix_Opcode)/2) //06 LD   B,n
  DCI  ((RLCA      - No_Prefix_Opcode)/2) //07 RLCA 
  DCI  ((EX_AF     - No_Prefix_Opcode)/2) //08 EX   AF,AF' 
  DCI  ((ADD_HL_BC - No_Prefix_Opcode)/2) //09 ADD  HL,BC   
  DCI  ((LD_A_BC   - No_Prefix_Opcode)/2) //0A LD   A,(BC)  
  DCI  ((DEC_BC    - No_Prefix_Opcode)/2) //0B DEC  BC 
  DCI  ((INC_C     - No_Prefix_Opcode)/2) //0C INC  C 
  DCI  ((DEC_C     - No_Prefix_Opcode)/2) //0D DEC  C
  DCI  ((LD_C_n    - No_Prefix_Opcode)/2) //0E LD   C,n   
  DCI  ((RRCA      - No_Prefix_Opcode)/2) //0F RRCA 
  DCI  ((DJNZ      - No_Prefix_Opcode)/2) //10 DJNZ xx
  DCI  ((LD_DE_nn  - No_Prefix_Opcode)/2) //11 LD   DE,nn
  DCI  ((ST_A_DE   - No_Prefix_Opcode)/2) //12 LD   (DE),A 
  DCI  ((INC_DE    - No_Prefix_Opcode)/2) //13 INC  DE 
  DCI  ((INC_D     - No_Prefix_Opcode)/2) //14 INC  D
  DCI  ((DEC_D     - No_Prefix_Opcode)/2) //15 DEC  D
  DCI  ((LD_D_n    - No_Prefix_Opcode)/2) //16 LD   D,n 
  DCI  ((RLA       - No_Prefix_Opcode)/2) //17 RLA  
  DCI  ((JR        - No_Prefix_Opcode)/2) //18 JR   xx
  DCI  ((ADD_HL_DE - No_Prefix_Opcode)/2) //19 ADD  HL,DE 
  DCI  ((LD_A_DE   - No_Prefix_Opcode)/2) //1A LD   A,(DE)
  DCI  ((DEC_DE    - No_Prefix_Opcode)/2) //1B DEC  DE
  DCI  ((INC_E     - No_Prefix_Opcode)/2) //1C INC  E 
  DCI  ((DEC_E     - No_Prefix_Opcode)/2) //1D DEC  E
  DCI  ((LD_E_n    - No_Prefix_Opcode)/2) //1E LD   E,n
  DCI  ((RRA       - No_Prefix_Opcode)/2) //1F RRA
  DCI  ((JR_NZ     - No_Prefix_Opcode)/2) //20 JR   NZ,xx
  DCI  ((LD_HL_nn  - No_Prefix_Opcode)/2) //21 LD   HL,nn
  DCI  ((ST_HL_mm  - No_Prefix_Opcode)/2) //22 LD  (nn),HL
  DCI  ((INC_HL    - No_Prefix_Opcode)/2) //23 INC  HL
  DCI  ((INC_H     - No_Prefix_Opcode)/2) //24 INC  H
  DCI  ((DEC_H     - No_Prefix_Opcode)/2) //25 DEC  H 
  DCI  ((LD_H_n    - No_Prefix_Opcode)/2) //26 LD   H,n
  DCI  ((DAA       - No_Prefix_Opcode)/2) //27 DAA
  DCI  ((JR_Z      - No_Prefix_Opcode)/2) //28 JR   Z,xx
  DCI  ((ADD_HL_HL - No_Prefix_Opcode)/2) //29 ADD  HL,HL
  DCI  ((LD_HL_mm  - No_Prefix_Opcode)/2) //2A LD   HL,(nn)
  DCI  ((DEC_HL    - No_Prefix_Opcode)/2) //2B DEC  HL
  DCI  ((INC_L     - No_Prefix_Opcode)/2) //2C INC  L
  DCI  ((DEC_L     - No_Prefix_Opcode)/2) //2D DEC  L
  DCI  ((LD_L_n    - No_Prefix_Opcode)/2) //2E LD   L,n
  DCI  ((CPL       - No_Prefix_Opcode)/2) //2F CPL
  DCI  ((JR_NC     - No_Prefix_Opcode)/2) //30 JR   NC,xx
  DCI  ((LD_SP_nn  - No_Prefix_Opcode)/2) //31 LD   SP,nn
  DCI  ((ST_A_nn   - No_Prefix_Opcode)/2) //32 LD   (nn),A
  DCI  ((INC_SP    - No_Prefix_Opcode)/2) //33 INC  SP
  DCI  ((INC_HLm   - No_Prefix_Opcode)/2) //34 INC  (HL) 
  DCI  ((DEC_HLm   - No_Prefix_Opcode)/2) //35 DEC  (HL)
  DCI  ((ST_n_HL   - No_Prefix_Opcode)/2) //36 LD   (HL),n
  DCI  ((SCF       - No_Prefix_Opcode)/2) //37 SCF
  DCI  ((JR_C      - No_Prefix_Opcode)/2) //38 JR   C,xx
  DCI  ((ADD_HL_SP - No_Prefix_Opcode)/2) //39 ADD  HL,SP
  DCI  ((LD_A_mm   - No_Prefix_Opcode)/2) //3A LD   A,(mm)
  DCI  ((DEC_SP    - No_Prefix_Opcode)/2) //3B DEC  SP
  DCI  ((INC_A     - No_Prefix_Opcode)/2) //3C INC  A
  DCI  ((DEC_A     - No_Prefix_Opcode)/2) //3D DEC  A  
  DCI  ((LD_A_n    - No_Prefix_Opcode)/2) //3E LD   A,n
  DCI  ((CCF       - No_Prefix_Opcode)/2) //3F CCF
  DCI  ((LD_B_B    - No_Prefix_Opcode)/2) //40 LD   B,B 
  DCI  ((LD_B_C    - No_Prefix_Opcode)/2) //41 LD   B,C  
  DCI  ((LD_B_D    - No_Prefix_Opcode)/2) //42 LD   B,D
  DCI  ((LD_B_E    - No_Prefix_Opcode)/2) //43 LD   B,E
  DCI  ((LD_B_H    - No_Prefix_Opcode)/2) //44 LD   B,H 
  DCI  ((LD_B_L    - No_Prefix_Opcode)/2) //45 LD   B,L
  DCI  ((LD_B_HL   - No_Prefix_Opcode)/2) //46 LD   B,(HL)
  DCI  ((LD_B_A    - No_Prefix_Opcode)/2) //47 LD   B,A
  DCI  ((LD_C_B    - No_Prefix_Opcode)/2) //48 LD   C,B 
  DCI  ((LD_C_C    - No_Prefix_Opcode)/2) //49 LD   C,C  
  DCI  ((LD_C_D    - No_Prefix_Opcode)/2) //4A LD   C,D
  DCI  ((LD_C_E    - No_Prefix_Opcode)/2) //4B LD   C,E
  DCI  ((LD_C_H    - No_Prefix_Opcode)/2) //4C LD   C,H 
  DCI  ((LD_C_L    - No_Prefix_Opcode)/2) //4D LD   C,L
  DCI  ((LD_C_HL   - No_Prefix_Opcode)/2) //4E LD   C,(HL)
  DCI  ((LD_C_A    - No_Prefix_Opcode)/2) //4F LD   C,A
  DCI  ((LD_D_B    - No_Prefix_Opcode)/2) //50 LD   D,B 
  DCI  ((LD_D_C    - No_Prefix_Opcode)/2) //51 LD   D,C  
  DCI  ((LD_D_D    - No_Prefix_Opcode)/2) //52 LD   D,D
  DCI  ((LD_D_E    - No_Prefix_Opcode)/2) //53 LD   D,E
  DCI  ((LD_D_H    - No_Prefix_Opcode)/2) //54 LD   D,H 
  DCI  ((LD_D_L    - No_Prefix_Opcode)/2) //55 LD   D,L
  DCI  ((LD_D_HL   - No_Prefix_Opcode)/2) //56 LD   D,(HL)
  DCI  ((LD_D_A    - No_Prefix_Opcode)/2) //57 LD   D,A
  DCI  ((LD_E_B    - No_Prefix_Opcode)/2) //58 LD   E,B 
  DCI  ((LD_E_C    - No_Prefix_Opcode)/2) //59 LD   E,C  
  DCI  ((LD_E_D    - No_Prefix_Opcode)/2) //5A LD   E,D
  DCI  ((LD_E_E    - No_Prefix_Opcode)/2) //5B LD   E,E
  DCI  ((LD_E_H    - No_Prefix_Opcode)/2) //5C LD   E,H 
  DCI  ((LD_E_L    - No_Prefix_Opcode)/2) //5D LD   E,L
  DCI  ((LD_E_HL   - No_Prefix_Opcode)/2) //5E LD   E,(HL)
  DCI  ((LD_E_A    - No_Prefix_Opcode)/2) //5F LD   E,A
  DCI  ((LD_H_B    - No_Prefix_Opcode)/2) //60 LD   H,B 
  DCI  ((LD_H_C    - No_Prefix_Opcode)/2) //61 LD   H,C  
  DCI  ((LD_H_D    - No_Prefix_Opcode)/2) //62 LD   H,D
  DCI  ((LD_H_E    - No_Prefix_Opcode)/2) //63 LD   H,E
  DCI  ((LD_H_H    - No_Prefix_Opcode)/2) //64 LD   H,H 
  DCI  ((LD_H_L    - No_Prefix_Opcode)/2) //65 LD   H,L
  DCI  ((LD_H_HL   - No_Prefix_Opcode)/2) //66 LD   H,(HL)
  DCI  ((LD_H_A    - No_Prefix_Opcode)/2) //67 LD   H,A
  DCI  ((LD_L_B    - No_Prefix_Opcode)/2) //68 LD   L,B 
  DCI  ((LD_L_C    - No_Prefix_Opcode)/2) //69 LD   L,C  
  DCI  ((LD_L_D    - No_Prefix_Opcode)/2) //6A LD   L,D
  DCI  ((LD_L_E    - No_Prefix_Opcode)/2) //6B LD   L,E
  DCI  ((LD_L_H    - No_Prefix_Opcode)/2) //6C LD   L,H 
  DCI  ((LD_L_L    - No_Prefix_Opcode)/2) //6D LD   L,L
  DCI  ((LD_L_HL   - No_Prefix_Opcode)/2) //6E LD   L,(HL)
  DCI  ((LD_L_A    - No_Prefix_Opcode)/2) //6F LD   L,A
  DCI  ((ST_B_HL   - No_Prefix_Opcode)/2) //70 LD   (HL),B
  DCI  ((ST_C_HL   - No_Prefix_Opcode)/2) //71 LD   (HL),C 
  DCI  ((ST_D_HL   - No_Prefix_Opcode)/2) //72 LD   (HL),D
  DCI  ((ST_E_HL   - No_Prefix_Opcode)/2) //73 LD   (HL),E
  DCI  ((ST_H_HL   - No_Prefix_Opcode)/2) //74 LD   (HL),H
  DCI  ((ST_L_HL   - No_Prefix_Opcode)/2) //75 LD   (HL),L
  DCI  ((HALT      - No_Prefix_Opcode)/2) //76 HALT
  DCI  ((ST_A_HL   - No_Prefix_Opcode)/2) //77 LD   (HL),A
  DCI  ((LD_A_B    - No_Prefix_Opcode)/2) //78 LD   A,B 
  DCI  ((LD_A_C    - No_Prefix_Opcode)/2) //79 LD   A,C  
  DCI  ((LD_A_D    - No_Prefix_Opcode)/2) //7A LD   A,D
  DCI  ((LD_A_E    - No_Prefix_Opcode)/2) //7B LD   A,E
  DCI  ((LD_A_H    - No_Prefix_Opcode)/2) //7C LD   A,H 
  DCI  ((LD_A_L    - No_Prefix_Opcode)/2) //7D LD   A,L
  DCI  ((LD_A_HL   - No_Prefix_Opcode)/2) //7E LD   A,(HL)
  DCI  ((LD_A_A    - No_Prefix_Opcode)/2) //7F LD   A,A
  DCI  ((ADD_A_B   - No_Prefix_Opcode)/2) //80 ADD  A,B
  DCI  ((ADD_A_C   - No_Prefix_Opcode)/2) //81 ADD  A,C 
  DCI  ((ADD_A_D   - No_Prefix_Opcode)/2) //82 ADD  A,D
  DCI  ((ADD_A_E   - No_Prefix_Opcode)/2) //83 ADD  A,E
  DCI  ((ADD_A_H   - No_Prefix_Opcode)/2) //84 ADD  A,H
  DCI  ((ADD_A_L   - No_Prefix_Opcode)/2) //85 ADD  A,L
  DCI  ((ADD_A_HL  - No_Prefix_Opcode)/2) //86 ADD  A,(HL)
  DCI  ((ADD_A_A   - No_Prefix_Opcode)/2) //87 ADD  A,A
  DCI  ((ADC_A_B   - No_Prefix_Opcode)/2) //88 ADC  A,B
  DCI  ((ADC_A_C   - No_Prefix_Opcode)/2) //89 ADC  A,C 
  DCI  ((ADC_A_D   - No_Prefix_Opcode)/2) //8A ADC  A,D
  DCI  ((ADC_A_E   - No_Prefix_Opcode)/2) //8B ADC  A,E
  DCI  ((ADC_A_H   - No_Prefix_Opcode)/2) //8C ADC  A,H
  DCI  ((ADC_A_L   - No_Prefix_Opcode)/2) //8D ADC  A,L
  DCI  ((ADC_A_HL  - No_Prefix_Opcode)/2) //8E ADC  A,(HL)
  DCI  ((ADC_A_A   - No_Prefix_Opcode)/2) //8F ADC  A,A
  DCI  ((SUB_A_B   - No_Prefix_Opcode)/2) //90 SUB  A,B
  DCI  ((SUB_A_C   - No_Prefix_Opcode)/2) //91 SUB  A,C 
  DCI  ((SUB_A_D   - No_Prefix_Opcode)/2) //92 SUB  A,D
  DCI  ((SUB_A_E   - No_Prefix_Opcode)/2) //93 SUB  A,E
  DCI  ((SUB_A_H   - No_Prefix_Opcode)/2) //94 SUB  A,H
  DCI  ((SUB_A_L   - No_Prefix_Opcode)/2) //95 SUB  A,L
  DCI  ((SUB_A_HL  - No_Prefix_Opcode)/2) //96 SUB  A,(HL)
  DCI  ((SUB_A_A   - No_Prefix_Opcode)/2) //97 SUB  A,A
  DCI  ((SBC_A_B   - No_Prefix_Opcode)/2) //98 SBC  A,B
  DCI  ((SBC_A_C   - No_Prefix_Opcode)/2) //99 SBC  A,C 
  DCI  ((SBC_A_D   - No_Prefix_Opcode)/2) //9A SBC  A,D
  DCI  ((SBC_A_E   - No_Prefix_Opcode)/2) //9B SBC  A,E
  DCI  ((SBC_A_H   - No_Prefix_Opcode)/2) //9C SBC  A,H
  DCI  ((SBC_A_L   - No_Prefix_Opcode)/2) //9D SBC  A,L
  DCI  ((SBC_A_HL  - No_Prefix_Opcode)/2) //9E SBC  A,(HL)
  DCI  ((SBC_A_A   - No_Prefix_Opcode)/2) //9F SBC  A,A
  DCI  ((AND_A_B   - No_Prefix_Opcode)/2) //A0 AND  A,B
  DCI  ((AND_A_C   - No_Prefix_Opcode)/2) //A1 AND  A,C 
  DCI  ((AND_A_D   - No_Prefix_Opcode)/2) //A2 AND  A,D
  DCI  ((AND_A_E   - No_Prefix_Opcode)/2) //A3 AND  A,E
  DCI  ((AND_A_H   - No_Prefix_Opcode)/2) //A4 AND  A,H
  DCI  ((AND_A_L   - No_Prefix_Opcode)/2) //A5 AND  A,L
  DCI  ((AND_A_HL  - No_Prefix_Opcode)/2) //A6 AND  A,(HL)
  DCI  ((AND_A_A   - No_Prefix_Opcode)/2) //A7 AND  A,A
  DCI  ((XOR_A_B   - No_Prefix_Opcode)/2) //A8 XOR  A,B
  DCI  ((XOR_A_C   - No_Prefix_Opcode)/2) //A9 XOR  A,C 
  DCI  ((XOR_A_D   - No_Prefix_Opcode)/2) //AA XOR  A,D
  DCI  ((XOR_A_E   - No_Prefix_Opcode)/2) //AB XOR  A,E
  DCI  ((XOR_A_H   - No_Prefix_Opcode)/2) //AC XOR  A,H
  DCI  ((XOR_A_L   - No_Prefix_Opcode)/2) //AD XOR  A,L
  DCI  ((XOR_A_HL  - No_Prefix_Opcode)/2) //AE XOR  A,(HL)
  DCI  ((XOR_A_A   - No_Prefix_Opcode)/2) //AF XOR  A,A
  DCI  ((OR_A_B    - No_Prefix_Opcode)/2) //B0 OR  A,B
  DCI  ((OR_A_C    - No_Prefix_Opcode)/2) //B1 OR  A,C 
  DCI  ((OR_A_D    - No_Prefix_Opcode)/2) //B2 OR  A,D
  DCI  ((OR_A_E    - No_Prefix_Opcode)/2) //B3 OR  A,E
  DCI  ((OR_A_H    - No_Prefix_Opcode)/2) //B4 OR  A,H
  DCI  ((OR_A_L    - No_Prefix_Opcode)/2) //B5 OR  A,L
  DCI  ((OR_A_HL   - No_Prefix_Opcode)/2) //B6 OR  A,(HL)
  DCI  ((OR_A_A    - No_Prefix_Opcode)/2) //B7 OR  A,A
  DCI  ((CP_A_B    - No_Prefix_Opcode)/2) //B8 CP  A,B
  DCI  ((CP_A_C    - No_Prefix_Opcode)/2) //B9 CP  A,C 
  DCI  ((CP_A_D    - No_Prefix_Opcode)/2) //BA CP  A,D
  DCI  ((CP_A_E    - No_Prefix_Opcode)/2) //BB CP  A,E
  DCI  ((CP_A_H    - No_Prefix_Opcode)/2) //BC CP  A,H
  DCI  ((CP_A_L    - No_Prefix_Opcode)/2) //BD CP  A,L
  DCI  ((CP_A_HL   - No_Prefix_Opcode)/2) //BE CP  A,(HL)
  DCI  ((CP_A_A    - No_Prefix_Opcode)/2) //BF CP  A,A
  DCI  ((RET_NZ    - No_Prefix_Opcode)/2) //C0 RET  NZ
  DCI  ((POP_BC    - No_Prefix_Opcode)/2) //C1 POP  BC
  DCI  ((JP_NZ     - No_Prefix_Opcode)/2) //C2 JP   NZ,xx
  DCI  ((JP        - No_Prefix_Opcode)/2) //C3 JP   xx
  DCI  ((CALL_NZ   - No_Prefix_Opcode)/2) //C4 CALL NZ,xx
  DCI  ((PUSH_BC   - No_Prefix_Opcode)/2) //C5 PUSH BC
  DCI  ((ADD_A_n   - No_Prefix_Opcode)/2) //C6 ADD  A,n
  DCI  ((RST_00    - No_Prefix_Opcode)/2) //C7 RST  00
  DCI  ((RET_Z     - No_Prefix_Opcode)/2) //C8 RET  Z
  DCI  ((RET       - No_Prefix_Opcode)/2) //C9 RET
  DCI  ((JP_Z      - No_Prefix_Opcode)/2) //CA JP   Z,nn
  DCI  ((CB_Prefix - No_Prefix_Opcode)/2) //CB
  DCI  ((CALL_Z    - No_Prefix_Opcode)/2) //CC CALL Z,nn
  DCI  ((CALL      - No_Prefix_Opcode)/2) //CD CALL nn
  DCI  ((ADC_A_n   - No_Prefix_Opcode)/2) //CE ADC  A,n
  DCI  ((RST_08    - No_Prefix_Opcode)/2) //CF RST  08
  DCI  ((RET_NC    - No_Prefix_Opcode)/2) //D0 RET  NC
  DCI  ((POP_DE    - No_Prefix_Opcode)/2) //D1 POP  DE
  DCI  ((JP_NC     - No_Prefix_Opcode)/2) //D2 JP   NC,nn
  DCI  ((OUT_A_n   - No_Prefix_Opcode)/2) //D3 OUT  (n),A
  DCI  ((CALL_NC   - No_Prefix_Opcode)/2) //D4 CALL NC,nn
  DCI  ((PUSH_DE   - No_Prefix_Opcode)/2) //D5 PUSH DE
  DCI  ((SUB_A_n   - No_Prefix_Opcode)/2) //D6 SUB  A,n
  DCI  ((RST_10    - No_Prefix_Opcode)/2) //D7 RST  10
  DCI  ((RET_C     - No_Prefix_Opcode)/2) //D8 RET  C
  DCI  ((EXX       - No_Prefix_Opcode)/2) //D9 EXX
  DCI  ((JP_C      - No_Prefix_Opcode)/2) //DA JP   C,nn
  DCI  ((IN_A_n    - No_Prefix_Opcode)/2) //DB IN   A,(&00)
  DCI  ((CALL_C    - No_Prefix_Opcode)/2) //DC CALL C,nn
  DCI  ((DD_Prefix - No_Prefix_Opcode)/2) //DD
  DCI  ((SBC_A_n   - No_Prefix_Opcode)/2) //DE SBC  A,n
  DCI  ((RST_18    - No_Prefix_Opcode)/2) //DF RST  18
  DCI  ((RET_PO    - No_Prefix_Opcode)/2) //E0 RET  PO
  DCI  ((POP_HL    - No_Prefix_Opcode)/2) //E1 POP  HL
  DCI  ((JP_PO     - No_Prefix_Opcode)/2) //E2 JP   PO,nn
  DCI  ((EX_SP_HL  - No_Prefix_Opcode)/2) //E3 EX   (SP),HL
  DCI  ((CALL_PO   - No_Prefix_Opcode)/2) //E4 CALL PO,nn
  DCI  ((PUSH_HL   - No_Prefix_Opcode)/2) //E5 PUSH HL
  DCI  ((AND_A_n   - No_Prefix_Opcode)/2) //E6 AND  A,n
  DCI  ((RST_20    - No_Prefix_Opcode)/2) //E7 RST  20
  DCI  ((RET_PE    - No_Prefix_Opcode)/2) //E8 RET  PE
  DCI  ((JP_HL     - No_Prefix_Opcode)/2) //E9 JP   (HL)
  DCI  ((JP_PE     - No_Prefix_Opcode)/2) //EA JP   PE,nn
  DCI  ((EX_DE_HL  - No_Prefix_Opcode)/2) //EB EX   DE,HL
  DCI  ((CALL_PE   - No_Prefix_Opcode)/2) //EC CALL PE,nn
  DCI  ((ED_Prefix - No_Prefix_Opcode)/2) //ED
  DCI  ((XOR_A_n   - No_Prefix_Opcode)/2) //EE XOR  A,n
  DCI  ((RST_28    - No_Prefix_Opcode)/2) //EF RST  28
  DCI  ((RET_P     - No_Prefix_Opcode)/2) //F0 RET  P
  DCI  ((POP_AF    - No_Prefix_Opcode)/2) //F1 POP  AF
  DCI  ((JP_P      - No_Prefix_Opcode)/2) //F2 JP   P,nn
  DCI  ((DIRQ      - No_Prefix_Opcode)/2) //F3 DI
  DCI  ((CALL_P    - No_Prefix_Opcode)/2) //F4 CALL P,nn
  DCI  ((PUSH_AF   - No_Prefix_Opcode)/2) //F5 PUSH AF
  DCI  ((OR_A_n    - No_Prefix_Opcode)/2) //F6 OR   A,n
  DCI  ((RST_30    - No_Prefix_Opcode)/2) //F7 RST  30
  DCI  ((RET_M     - No_Prefix_Opcode)/2) //F8 RET  M
  DCI  ((LD_SP_HL  - No_Prefix_Opcode)/2) //F9 LD   SP,HL
  DCI  ((JP_M      - No_Prefix_Opcode)/2) //FA JP   M,nn
  DCI  ((EIRQ      - No_Prefix_Opcode)/2) //FB EI
  DCI  ((CALL_M    - No_Prefix_Opcode)/2) //FC CALL M,nn
  DCI  ((FD_Prefix - No_Prefix_Opcode)/2) //FD
  DCI  ((CP_A_n    - No_Prefix_Opcode)/2) //FE CP   A,n
  DCI  ((RST_38    - No_Prefix_Opcode)/2) //FF RST

//***********
// CB_Prefix
//***********
CB_Prefix     
  MOV   r_jmp,lr                // indirizzo ritorno
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_op,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  LSR   r_res,r_op,#3   
  CMP   r_res,#8                //opcode >= 0x40 ?  
  LSRGE r_res,#3            
  ORRGE r_res,#8                //res=(opcode/8)|8 
  AND   r_acc,r_op,#7           
  CMP   r_acc,#6
  BEQ   CB_Prefix_HL         
  BL    CB_Prefix_switch        // B C D E H L A
  SUBS  r_cyc,#4
  BX    r_jmp
CB_Prefix_HL                    // (HL) usa R6
  BL    GET_R6PN                // R6=(HL)
  CMP   r_res,#9                // BIT ?
  BEQ   CB_Prefix_HL_BIT
  BL    CB_Prefix_switch
  LDRB  r_acc,[r_rp,#ofsR6]
  BL    WRITE_RAM                
  SUBS  r_cyc,#4
  BX    r_jmp
CB_Prefix_HL_BIT
  BL    CB_Prefix_switch
  SUBS  r_cyc,#5
  BX    r_jmp

CB_Prefix_switch  
  AND   r_res,#0x0f
  TBH   [PC, r_res, LSL #1] 
CB_Prefix_Opcode
  DCI  ((RLC_R     - CB_Prefix_Opcode)/2) //CB00-CB07		RLC R
  DCI  ((RRC_R     - CB_Prefix_Opcode)/2) //CB08-CB0F   RRC R
  DCI  ((RL_R      - CB_Prefix_Opcode)/2) //CB10-CB17   RL R
  DCI  ((RR_R      - CB_Prefix_Opcode)/2) //CB18-CB1F		RR R
  DCI  ((SLA_R     - CB_Prefix_Opcode)/2) //CB20-CB27		SLA R
  DCI  ((SRA_R     - CB_Prefix_Opcode)/2) //CB28-CB2F		SRA R
  DCI  ((SLL_R     - CB_Prefix_Opcode)/2) //CB30-CB37	 *SLL R
  DCI  ((SRL_R     - CB_Prefix_Opcode)/2) //CB38-CB3F		SRL R
  DCI  ((PAD_CB    - CB_Prefix_Opcode)/2) //
  DCI  ((BIT_R     - CB_Prefix_Opcode)/2) //CB40-CB7F		BIT x,R
  DCI  ((RES_R     - CB_Prefix_Opcode)/2) //CB80-CBBF		RES x,R
  DCI  ((SET_R     - CB_Prefix_Opcode)/2) //CBC0-CBFF		SET x,R
  DCI  ((PAD_CB    - CB_Prefix_Opcode)/2) //
  DCI  ((PAD_CB    - CB_Prefix_Opcode)/2) //
  DCI  ((PAD_CB    - CB_Prefix_Opcode)/2) //
  DCI  ((PAD_CB    - CB_Prefix_Opcode)/2) //

PAD_CB  BX lr                   //(DEBUG Safe)

//***********
// xD Prefix
//***********
DD_Prefix
FD_Prefix
  LSR   r_op,#5                 // IX/IY select
  BFI   r_par,r_op,#1,#2        // 4/6
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_op,[r_pnt,r_pc]      // Get next opcode
  ADD   r_pc,#1
  MOVT  r_pc,#0
  SUB   r_cyc,#4
  TBH   [pc,r_op,lsl#1]  
xD_Prefix_Opcode
  DCI  ((NOP_4     - xD_Prefix_Opcode)/2) //00 NOP            
  DCI  ((LD_BC_nn  - xD_Prefix_Opcode)/2) //01 LD   BC,nn 
  DCI  ((ST_A_BC   - xD_Prefix_Opcode)/2) //02 LD   (BC),A
  DCI  ((INC_BC    - xD_Prefix_Opcode)/2) //03 INC  BC
  DCI  ((INC_B     - xD_Prefix_Opcode)/2) //04 INC  B
  DCI  ((DEC_B     - xD_Prefix_Opcode)/2) //05 DEC  B 
  DCI  ((LD_B_n    - xD_Prefix_Opcode)/2) //06 LD   B,n
  DCI  ((RLCA      - xD_Prefix_Opcode)/2) //07 RLCA 
  DCI  ((EX_AF     - xD_Prefix_Opcode)/2) //08 EX   AF,AF' 
  DCI  ((ADD_XX_BC - xD_Prefix_Opcode)/2) //09 ADD  XL,BC   
  DCI  ((LD_A_BC   - xD_Prefix_Opcode)/2) //0A LD   A,(BC)  
  DCI  ((DEC_BC    - xD_Prefix_Opcode)/2) //0B DEC  BC 
  DCI  ((INC_C     - xD_Prefix_Opcode)/2) //0C INC  C 
  DCI  ((DEC_C     - xD_Prefix_Opcode)/2) //0D DEC  C
  DCI  ((LD_C_n    - xD_Prefix_Opcode)/2) //0E LD   C,n   
  DCI  ((RRCA      - xD_Prefix_Opcode)/2) //0F RRCA 
  DCI  ((DJNZ      - xD_Prefix_Opcode)/2) //10 DJNZ xx
  DCI  ((LD_DE_nn  - xD_Prefix_Opcode)/2) //11 LD   DE,nn
  DCI  ((ST_A_DE   - xD_Prefix_Opcode)/2) //12 LD   (DE),A 
  DCI  ((INC_DE    - xD_Prefix_Opcode)/2) //13 INC  DE 
  DCI  ((INC_D     - xD_Prefix_Opcode)/2) //14 INC  D
  DCI  ((DEC_D     - xD_Prefix_Opcode)/2) //15 DEC  D
  DCI  ((LD_D_n    - xD_Prefix_Opcode)/2) //16 LD   D,n 
  DCI  ((RLA       - xD_Prefix_Opcode)/2) //17 RLA  
  DCI  ((JR        - xD_Prefix_Opcode)/2) //18 JR   xx
  DCI  ((ADD_XX_DE - xD_Prefix_Opcode)/2) //19 ADD  XX,DE 
  DCI  ((LD_A_DE   - xD_Prefix_Opcode)/2) //1A LD   A,(DE)
  DCI  ((DEC_DE    - xD_Prefix_Opcode)/2) //1B DEC  DE
  DCI  ((INC_E     - xD_Prefix_Opcode)/2) //1C INC  E 
  DCI  ((DEC_E     - xD_Prefix_Opcode)/2) //1D DEC  E
  DCI  ((LD_E_n    - xD_Prefix_Opcode)/2) //1E LD   E,n
  DCI  ((RRA       - xD_Prefix_Opcode)/2) //1F RRA
  DCI  ((JR_NZ     - xD_Prefix_Opcode)/2) //20 JR   NZ,xx
  DCI  ((LD_XX_nn  - xD_Prefix_Opcode)/2) //21 LD   XX,nn
  DCI  ((ST_XX_mm  - xD_Prefix_Opcode)/2) //22 LD  (nn),XX
  DCI  ((INC_XX    - xD_Prefix_Opcode)/2) //23 INC  XX
  DCI  ((INC_XH    - xD_Prefix_Opcode)/2) //24 INC  XH
  DCI  ((DEC_XH    - xD_Prefix_Opcode)/2) //25 DEC  XH 
  DCI  ((LD_XH_n   - xD_Prefix_Opcode)/2) //26 LD   XH,n
  DCI  ((DAA       - xD_Prefix_Opcode)/2) //27 DAA
  DCI  ((JR_Z      - xD_Prefix_Opcode)/2) //28 JR   Z,xx
  DCI  ((ADD_XX_XX - xD_Prefix_Opcode)/2) //29 ADD  XX,XX
  DCI  ((LD_XX_mm  - xD_Prefix_Opcode)/2) //2A LD   XX,(nn)
  DCI  ((DEC_XX    - xD_Prefix_Opcode)/2) //2B DEC  XX
  DCI  ((INC_XL    - xD_Prefix_Opcode)/2) //2C INC  XL
  DCI  ((DEC_XL    - xD_Prefix_Opcode)/2) //2D DEC  XL
  DCI  ((LD_XL_n   - xD_Prefix_Opcode)/2) //2E LD   XL,n
  DCI  ((CPL       - xD_Prefix_Opcode)/2) //2F CPL
  DCI  ((JR_NC     - xD_Prefix_Opcode)/2) //30 JR   NC,xx
  DCI  ((LD_SP_nn  - xD_Prefix_Opcode)/2) //31 LD   SP,nn
  DCI  ((ST_A_nn   - xD_Prefix_Opcode)/2) //32 LD   (nn),A
  DCI  ((INC_SP    - xD_Prefix_Opcode)/2) //33 INC  SP
  DCI  ((INC_XXd   - xD_Prefix_Opcode)/2) //34 INC  (XX+d) 
  DCI  ((DEC_XXd   - xD_Prefix_Opcode)/2) //35 DEC  (XX+d)
  DCI  ((ST_n_XXd  - xD_Prefix_Opcode)/2) //36 LD   (XX+d),n
  DCI  ((SCF       - xD_Prefix_Opcode)/2) //37 SCF
  DCI  ((JR_C      - xD_Prefix_Opcode)/2) //38 JR   C,xx
  DCI  ((ADD_XX_SP - xD_Prefix_Opcode)/2) //39 ADD  HL,SP
  DCI  ((LD_A_mm   - xD_Prefix_Opcode)/2) //3A LD   A,(mm)
  DCI  ((DEC_SP    - xD_Prefix_Opcode)/2) //3B DEC  SP
  DCI  ((INC_A     - xD_Prefix_Opcode)/2) //3C INC  A
  DCI  ((DEC_A     - xD_Prefix_Opcode)/2) //3D DEC  A  
  DCI  ((LD_A_n    - xD_Prefix_Opcode)/2) //3E LD   A,n
  DCI  ((CCF       - xD_Prefix_Opcode)/2) //3F CCF
  DCI  ((LD_B_B    - xD_Prefix_Opcode)/2) //40 LD   B,B 
  DCI  ((LD_B_C    - xD_Prefix_Opcode)/2) //41 LD   B,C  
  DCI  ((LD_B_D    - xD_Prefix_Opcode)/2) //42 LD   B,D
  DCI  ((LD_B_E    - xD_Prefix_Opcode)/2) //43 LD   B,E
  DCI  ((LD_B_XH   - xD_Prefix_Opcode)/2) //44 LD   B,XH 
  DCI  ((LD_B_XL   - xD_Prefix_Opcode)/2) //45 LD   B,XL
  DCI  ((LD_B_XXd  - xD_Prefix_Opcode)/2) //46 LD   B,(XX+d)
  DCI  ((LD_B_A    - xD_Prefix_Opcode)/2) //47 LD   B,A
  DCI  ((LD_C_B    - xD_Prefix_Opcode)/2) //48 LD   C,B 
  DCI  ((LD_C_C    - xD_Prefix_Opcode)/2) //49 LD   C,C  
  DCI  ((LD_C_D    - xD_Prefix_Opcode)/2) //4A LD   C,D
  DCI  ((LD_C_E    - xD_Prefix_Opcode)/2) //4B LD   C,E
  DCI  ((LD_C_XH   - xD_Prefix_Opcode)/2) //4C LD   C,XH 
  DCI  ((LD_C_XL   - xD_Prefix_Opcode)/2) //4D LD   C,XL
  DCI  ((LD_C_XXd  - xD_Prefix_Opcode)/2) //4E LD   C,(XX+d)
  DCI  ((LD_C_A    - xD_Prefix_Opcode)/2) //4F LD   C,A
  DCI  ((LD_D_B    - xD_Prefix_Opcode)/2) //50 LD   D,B 
  DCI  ((LD_D_C    - xD_Prefix_Opcode)/2) //51 LD   D,C  
  DCI  ((LD_D_D    - xD_Prefix_Opcode)/2) //52 LD   D,D
  DCI  ((LD_D_E    - xD_Prefix_Opcode)/2) //53 LD   D,E
  DCI  ((LD_D_XH   - xD_Prefix_Opcode)/2) //54 LD   D,XH 
  DCI  ((LD_D_XL   - xD_Prefix_Opcode)/2) //55 LD   D,XL
  DCI  ((LD_D_XXd  - xD_Prefix_Opcode)/2) //56 LD   D,(XX+d)
  DCI  ((LD_D_A    - xD_Prefix_Opcode)/2) //57 LD   D,A
  DCI  ((LD_E_B    - xD_Prefix_Opcode)/2) //58 LD   E,B 
  DCI  ((LD_E_C    - xD_Prefix_Opcode)/2) //59 LD   E,C  
  DCI  ((LD_E_D    - xD_Prefix_Opcode)/2) //5A LD   E,D
  DCI  ((LD_E_E    - xD_Prefix_Opcode)/2) //5B LD   E,E
  DCI  ((LD_E_XH   - xD_Prefix_Opcode)/2) //5C LD   E,XH 
  DCI  ((LD_E_XL   - xD_Prefix_Opcode)/2) //5D LD   E,XL
  DCI  ((LD_E_XXd  - xD_Prefix_Opcode)/2) //5E LD   E,(XX+d)
  DCI  ((LD_E_A    - xD_Prefix_Opcode)/2) //5F LD   E,A
  DCI  ((LD_XH_B   - xD_Prefix_Opcode)/2) //60 LD   XH,B 
  DCI  ((LD_XH_C   - xD_Prefix_Opcode)/2) //61 LD   XH,C  
  DCI  ((LD_XH_D   - xD_Prefix_Opcode)/2) //62 LD   XH,D
  DCI  ((LD_XH_E   - xD_Prefix_Opcode)/2) //63 LD   XH,E
  DCI  ((LD_XH_XH  - xD_Prefix_Opcode)/2) //64 LD   XH,XH 
  DCI  ((LD_XH_XL  - xD_Prefix_Opcode)/2) //65 LD   XH,XL
  DCI  ((LD_H_XXd  - xD_Prefix_Opcode)/2) //66 LD   H,(XX+d)
  DCI  ((LD_XH_A   - xD_Prefix_Opcode)/2) //67 LD   H,A
  DCI  ((LD_XL_B   - xD_Prefix_Opcode)/2) //68 LD   XL,B 
  DCI  ((LD_XL_C   - xD_Prefix_Opcode)/2) //69 LD   XL,C  
  DCI  ((LD_XL_D   - xD_Prefix_Opcode)/2) //6A LD   XL,D
  DCI  ((LD_XL_E   - xD_Prefix_Opcode)/2) //6B LD   XL,E
  DCI  ((LD_XL_XH  - xD_Prefix_Opcode)/2) //6C LD   XL,H 
  DCI  ((LD_XL_XL  - xD_Prefix_Opcode)/2) //6D LD   XL,L
  DCI  ((LD_L_XXd  - xD_Prefix_Opcode)/2) //6E LD   L,(XX+d)
  DCI  ((LD_XL_A   - xD_Prefix_Opcode)/2) //6F LD   XL,A
  DCI  ((ST_B_XXd  - xD_Prefix_Opcode)/2) //70 LD   (XX+d),B
  DCI  ((ST_C_XXd  - xD_Prefix_Opcode)/2) //71 LD   (XX+d),C 
  DCI  ((ST_D_XXd  - xD_Prefix_Opcode)/2) //72 LD   (XX+d),D
  DCI  ((ST_E_XXd  - xD_Prefix_Opcode)/2) //73 LD   (XX+d),E
  DCI  ((ST_H_XXd  - xD_Prefix_Opcode)/2) //74 LD   (XX+d),H
  DCI  ((ST_L_XXd  - xD_Prefix_Opcode)/2) //75 LD   (XX+d),L
  DCI  ((HALT      - xD_Prefix_Opcode)/2) //76 HALT
  DCI  ((ST_A_XXd  - xD_Prefix_Opcode)/2) //77 LD   (XX+d),A
  DCI  ((LD_A_B    - xD_Prefix_Opcode)/2) //78 LD   A,B 
  DCI  ((LD_A_C    - xD_Prefix_Opcode)/2) //79 LD   A,C  
  DCI  ((LD_A_D    - xD_Prefix_Opcode)/2) //7A LD   A,D
  DCI  ((LD_A_E    - xD_Prefix_Opcode)/2) //7B LD   A,E
  DCI  ((LD_A_XH   - xD_Prefix_Opcode)/2) //7C LD   A,XH 
  DCI  ((LD_A_XL   - xD_Prefix_Opcode)/2) //7D LD   A,XL
  DCI  ((LD_A_XXd  - xD_Prefix_Opcode)/2) //7E LD   A,(XX+d)
  DCI  ((LD_A_A    - xD_Prefix_Opcode)/2) //7F LD   A,A
  DCI  ((ADD_A_B   - xD_Prefix_Opcode)/2) //80 ADD  A,B
  DCI  ((ADD_A_C   - xD_Prefix_Opcode)/2) //81 ADD  A,C 
  DCI  ((ADD_A_D   - xD_Prefix_Opcode)/2) //82 ADD  A,D
  DCI  ((ADD_A_E   - xD_Prefix_Opcode)/2) //83 ADD  A,E
  DCI  ((ADD_A_XH  - xD_Prefix_Opcode)/2) //84 ADD  A,XH
  DCI  ((ADD_A_XL  - xD_Prefix_Opcode)/2) //85 ADD  A,XL
  DCI  ((ADD_A_XXd - xD_Prefix_Opcode)/2) //86 ADD  A,(XX+d)
  DCI  ((ADD_A_A   - xD_Prefix_Opcode)/2) //87 ADD  A,A
  DCI  ((ADC_A_B   - xD_Prefix_Opcode)/2) //88 ADC  A,B
  DCI  ((ADC_A_C   - xD_Prefix_Opcode)/2) //89 ADC  A,C 
  DCI  ((ADC_A_D   - xD_Prefix_Opcode)/2) //8A ADC  A,D
  DCI  ((ADC_A_E   - xD_Prefix_Opcode)/2) //8B ADC  A,E
  DCI  ((ADC_A_XH  - xD_Prefix_Opcode)/2) //8C ADC  A,XH
  DCI  ((ADC_A_XL  - xD_Prefix_Opcode)/2) //8D ADC  A,XL
  DCI  ((ADC_A_XXd - xD_Prefix_Opcode)/2) //8E ADC  A,(XX+d)
  DCI  ((ADC_A_A   - xD_Prefix_Opcode)/2) //8F ADC  A,A
  DCI  ((SUB_A_B   - xD_Prefix_Opcode)/2) //90 SUB  A,B
  DCI  ((SUB_A_C   - xD_Prefix_Opcode)/2) //91 SUB  A,C 
  DCI  ((SUB_A_D   - xD_Prefix_Opcode)/2) //92 SUB  A,D
  DCI  ((SUB_A_E   - xD_Prefix_Opcode)/2) //93 SUB  A,E
  DCI  ((SUB_A_XH  - xD_Prefix_Opcode)/2) //94 SUB  A,XH
  DCI  ((SUB_A_XL  - xD_Prefix_Opcode)/2) //95 SUB  A,XL
  DCI  ((SUB_A_XXd - xD_Prefix_Opcode)/2) //96 SUB  A,(XX+d)
  DCI  ((SUB_A_A   - xD_Prefix_Opcode)/2) //97 SUB  A,A
  DCI  ((SBC_A_B   - xD_Prefix_Opcode)/2) //98 SBC  A,B
  DCI  ((SBC_A_C   - xD_Prefix_Opcode)/2) //99 SBC  A,C 
  DCI  ((SBC_A_D   - xD_Prefix_Opcode)/2) //9A SBC  A,D
  DCI  ((SBC_A_E   - xD_Prefix_Opcode)/2) //9B SBC  A,E
  DCI  ((SBC_A_XH  - xD_Prefix_Opcode)/2) //9C SBC  A,XH
  DCI  ((SBC_A_XL  - xD_Prefix_Opcode)/2) //9D SBC  A,XL
  DCI  ((SBC_A_XXd - xD_Prefix_Opcode)/2) //9E SBC  A,(XXd)
  DCI  ((SBC_A_A   - xD_Prefix_Opcode)/2) //9F SBC  A,A
  DCI  ((AND_A_B   - xD_Prefix_Opcode)/2) //A0 AND  A,B
  DCI  ((AND_A_C   - xD_Prefix_Opcode)/2) //A1 AND  A,C 
  DCI  ((AND_A_D   - xD_Prefix_Opcode)/2) //A2 AND  A,D
  DCI  ((AND_A_E   - xD_Prefix_Opcode)/2) //A3 AND  A,E
  DCI  ((AND_A_XH  - xD_Prefix_Opcode)/2) //A4 AND  A,XH
  DCI  ((AND_A_XL  - xD_Prefix_Opcode)/2) //A5 AND  A,XL
  DCI  ((AND_A_XXd - xD_Prefix_Opcode)/2) //A6 AND  A,(XX+d)
  DCI  ((AND_A_A   - xD_Prefix_Opcode)/2) //A7 AND  A,A
  DCI  ((XOR_A_B   - xD_Prefix_Opcode)/2) //A8 XOR  A,B
  DCI  ((XOR_A_C   - xD_Prefix_Opcode)/2) //A9 XOR  A,C 
  DCI  ((XOR_A_D   - xD_Prefix_Opcode)/2) //AA XOR  A,D
  DCI  ((XOR_A_E   - xD_Prefix_Opcode)/2) //AB XOR  A,E
  DCI  ((XOR_A_XH  - xD_Prefix_Opcode)/2) //AC XOR  A,XH
  DCI  ((XOR_A_XL  - xD_Prefix_Opcode)/2) //AD XOR  A,XL
  DCI  ((XOR_A_XXd - xD_Prefix_Opcode)/2) //AE XOR  A,(XX+d)
  DCI  ((XOR_A_A   - xD_Prefix_Opcode)/2) //AF XOR  A,A
  DCI  ((OR_A_B    - xD_Prefix_Opcode)/2) //B0 OR  A,B
  DCI  ((OR_A_C    - xD_Prefix_Opcode)/2) //B1 OR  A,C 
  DCI  ((OR_A_D    - xD_Prefix_Opcode)/2) //B2 OR  A,D
  DCI  ((OR_A_E    - xD_Prefix_Opcode)/2) //B3 OR  A,E
  DCI  ((OR_A_XH   - xD_Prefix_Opcode)/2) //B4 OR  A,XH
  DCI  ((OR_A_XL   - xD_Prefix_Opcode)/2) //B5 OR  A,XL
  DCI  ((OR_A_XXd  - xD_Prefix_Opcode)/2) //B6 OR  A,(XX+d)
  DCI  ((OR_A_A    - xD_Prefix_Opcode)/2) //B7 OR  A,A
  DCI  ((CP_A_B    - xD_Prefix_Opcode)/2) //B8 CP  A,B
  DCI  ((CP_A_C    - xD_Prefix_Opcode)/2) //B9 CP  A,C 
  DCI  ((CP_A_D    - xD_Prefix_Opcode)/2) //BA CP  A,D
  DCI  ((CP_A_E    - xD_Prefix_Opcode)/2) //BB CP  A,E
  DCI  ((CP_A_XH   - xD_Prefix_Opcode)/2) //BC CP  A,XH
  DCI  ((CP_A_XL   - xD_Prefix_Opcode)/2) //BD CP  A,XL
  DCI  ((CP_A_XXd  - xD_Prefix_Opcode)/2) //BE CP  A,(XX+d)
  DCI  ((CP_A_A    - xD_Prefix_Opcode)/2) //BF CP  A,A
  DCI  ((RET_NZ    - xD_Prefix_Opcode)/2) //C0 RET  NZ
  DCI  ((POP_BC    - xD_Prefix_Opcode)/2) //C1 POP  BC
  DCI  ((JP_NZ     - xD_Prefix_Opcode)/2) //C2 JP   NZ,xx
  DCI  ((JP        - xD_Prefix_Opcode)/2) //C3 JP   xx
  DCI  ((CALL_NZ   - xD_Prefix_Opcode)/2) //C4 CALL NZ,xx
  DCI  ((PUSH_BC   - xD_Prefix_Opcode)/2) //C5 PUSH BC
  DCI  ((ADD_A_n   - xD_Prefix_Opcode)/2) //C6 ADD  A,n
  DCI  ((RST_00    - xD_Prefix_Opcode)/2) //C7 RST  00
  DCI  ((RET_Z     - xD_Prefix_Opcode)/2) //C8 RET  Z
  DCI  ((RET       - xD_Prefix_Opcode)/2) //C9 RET
  DCI  ((JP_Z      - xD_Prefix_Opcode)/2) //CA JP   Z,nn
  DCI  ((XCB_Prefix- xD_Prefix_Opcode)/2) //CB
  DCI  ((CALL_Z    - xD_Prefix_Opcode)/2) //CC CALL Z,nn
  DCI  ((CALL      - xD_Prefix_Opcode)/2) //CD CALL nn
  DCI  ((ADC_A_n   - xD_Prefix_Opcode)/2) //CE ADC  A,n
  DCI  ((RST_08    - xD_Prefix_Opcode)/2) //CF RST  08
  DCI  ((RET_NC    - xD_Prefix_Opcode)/2) //D0 RET  NC
  DCI  ((POP_DE    - xD_Prefix_Opcode)/2) //D1 POP  DE
  DCI  ((JP_NC     - xD_Prefix_Opcode)/2) //D2 JP   NC,nn
  DCI  ((OUT_A_n   - xD_Prefix_Opcode)/2) //D3 OUT  (n),A
  DCI  ((CALL_NC   - xD_Prefix_Opcode)/2) //D4 CALL NC,nn
  DCI  ((PUSH_DE   - xD_Prefix_Opcode)/2) //D5 PUSH DE
  DCI  ((SUB_A_n   - xD_Prefix_Opcode)/2) //D6 SUB  A,n
  DCI  ((RST_10    - xD_Prefix_Opcode)/2) //D7 RST  10
  DCI  ((RET_C     - xD_Prefix_Opcode)/2) //D8 RET  C
  DCI  ((EXX       - xD_Prefix_Opcode)/2) //D9 EXX
  DCI  ((JP_C      - xD_Prefix_Opcode)/2) //DA JP   C,nn
  DCI  ((IN_A_n    - xD_Prefix_Opcode)/2) //DB IN   A,(&00)
  DCI  ((CALL_C    - xD_Prefix_Opcode)/2) //DC CALL C,nn
  DCI  ((XDD_Prefix- xD_Prefix_Opcode)/2) //DD
  DCI  ((SBC_A_n   - xD_Prefix_Opcode)/2) //DE SBC  A,n
  DCI  ((RST_18    - xD_Prefix_Opcode)/2) //DF RST  18
  DCI  ((RET_PO    - xD_Prefix_Opcode)/2) //E0 RET  PO
  DCI  ((POP_XX    - xD_Prefix_Opcode)/2) //E1 POP  XX
  DCI  ((JP_PO     - xD_Prefix_Opcode)/2) //E2 JP   PO,nn
  DCI  ((EX_SP_XX  - xD_Prefix_Opcode)/2) //E3 EX   (SP),XX
  DCI  ((CALL_PO   - xD_Prefix_Opcode)/2) //E4 CALL PO,nn
  DCI  ((PUSH_XX   - xD_Prefix_Opcode)/2) //E5 PUSH XX
  DCI  ((AND_A_n   - xD_Prefix_Opcode)/2) //E6 AND  A,n
  DCI  ((RST_20    - xD_Prefix_Opcode)/2) //E7 RST  20
  DCI  ((RET_PE    - xD_Prefix_Opcode)/2) //E8 RET  PE
  DCI  ((JP_XX     - xD_Prefix_Opcode)/2) //E9 JP   (XX)
  DCI  ((JP_PE     - xD_Prefix_Opcode)/2) //EA JP   PE,nn
  DCI  ((EX_DE_HL  - xD_Prefix_Opcode)/2) //EB EX   DE,HL
  DCI  ((CALL_PE   - xD_Prefix_Opcode)/2) //EC CALL PE,nn
  DCI  ((ED_Prefix - xD_Prefix_Opcode)/2) //ED
  DCI  ((XOR_A_n   - xD_Prefix_Opcode)/2) //EE XOR  A,n
  DCI  ((RST_28    - xD_Prefix_Opcode)/2) //EF RST  28
  DCI  ((RET_P     - xD_Prefix_Opcode)/2) //F0 RET  P
  DCI  ((POP_AF    - xD_Prefix_Opcode)/2) //F1 POP  AF
  DCI  ((JP_P      - xD_Prefix_Opcode)/2) //F2 JP   P,nn
  DCI  ((DIRQ      - xD_Prefix_Opcode)/2) //F3 DI
  DCI  ((CALL_P    - xD_Prefix_Opcode)/2) //F4 CALL P,nn
  DCI  ((PUSH_AF   - xD_Prefix_Opcode)/2) //F5 PUSH AF
  DCI  ((OR_A_n    - xD_Prefix_Opcode)/2) //F6 OR   A,n
  DCI  ((RST_30    - xD_Prefix_Opcode)/2) //F7 RST  30
  DCI  ((RET_M     - xD_Prefix_Opcode)/2) //F8 RET  M
  DCI  ((LD_SP_XX  - xD_Prefix_Opcode)/2) //F9 LD   SP,XX
  DCI  ((JP_M      - xD_Prefix_Opcode)/2) //FA JP   M,nn
  DCI  ((EIRQ      - xD_Prefix_Opcode)/2) //FB EI
  DCI  ((CALL_M    - xD_Prefix_Opcode)/2) //FC CALL M,nn
  DCI  ((XFD_Prefix- xD_Prefix_Opcode)/2) //FD
  DCI  ((CP_A_n    - xD_Prefix_Opcode)/2) //FE CP   A,n
  DCI  ((RST_38    - xD_Prefix_Opcode)/2) //FF RST

XCB_Prefix
  MOV   r_jmp,lr
  BL    GET_R6PN                //R6=XX+d
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_pnt,[r_pnt,r_pc]      //r_pnt=opcode
  ADD   r_pc,#1
  MOVT  r_pc,#0
  LSR   r_res,r_pnt,#3   
  CMP   r_res,#8                //opcode >= 0x40 ?  
  LSRGE r_res,#3            
  ORRGE r_res,#8                //res=(opcode/8)|8 
  AND   r_op,r_pnt,#0xf8        //r_op=opcode(R6)
  ORR   r_op,#6
  BFI   r_pnt,r_res,#4,#4       //r_pnt 7-4=res
  BL    CB_Prefix_switch        //esegue opcode
  LSR   r_res,r_pnt,#4
  CMP   r_res,#9                // se BIT opcode esce 
  BXEQ  r_jmp                       
  LDRB  r_acc,[r_rp,#ofsR6]     // altrimenti copia
  AND   r_pnt,#0x07             // risultato in registro[opcode&7]
  STRB  r_acc,[r_rp,r_pnt]     
  BL    WRITE_RAM               // e in memoria  
  BX    r_jmp
  
XDD_Prefix
XFD_Prefix
  B DD_Prefix                   // xDxD ripete

//***********
// ED_Prefix
//***********
ED_Prefix
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_op,[r_pnt,r_pc]       // Get next opcode
  ADD   r_pc,#1
  MOVT  r_pc,#0
  SUB   r_cyc,#4
  LSR   r_res,r_op,#5
  TBB   [pc,r_res]
ED_Switch
  DCB  ((NOP_8    - ED_Switch)/2)  //00-1f
  DCB  ((NOP_8    - ED_Switch)/2)  //20-3f
  DCB  ((ED_40_7f - ED_Switch)/2)  //40-5f
  DCB  ((ED_40_7f - ED_Switch)/2)  //60-7f
  DCB  ((NOP_8    - ED_Switch)/2)  //80-9f
  DCB  ((ED_a0_bf - ED_Switch)/2)  //a0-bf
  DCB  ((NOP_8    - ED_Switch)/2)  //c0-df
  DCB  ((NOP_8    - ED_Switch)/2)  //e0-ff
NOP_8 
  SUB   r_cyc,#4
  BX    lr
ED_a0_bf
  SUB   r_res,r_op,#0x60
  B     ED_Select
ED_40_7f
  SUB   r_res,r_op,#0x40
ED_Select
  TBH   [pc,r_res,lsl#1]
ED_Prefix_Opcode  
  DCI  ((IN_B_C     - ED_Prefix_Opcode)/2) //ED40		IN B,(C)
  DCI  ((OUT_C_B    - ED_Prefix_Opcode)/2) //ED41		OUT (C),B
  DCI  ((SBC_HL_BC  - ED_Prefix_Opcode)/2) //ED42		SBC HL,BC
  DCI  ((ST_BC_mm   - ED_Prefix_Opcode)/2) //ED43  	LD (nn),BC
  DCI  ((NEG        - ED_Prefix_Opcode)/2) //ED44		NEG
  DCI  ((RETN       - ED_Prefix_Opcode)/2) //ED45		RETN
  DCI  ((IM0        - ED_Prefix_Opcode)/2) //ED46		IM 0
  DCI  ((LD_I_A     - ED_Prefix_Opcode)/2) //ED47		LD I,A
  DCI  ((IN_C_C     - ED_Prefix_Opcode)/2) //ED48		IN C,(C)
  DCI  ((OUT_C_C    - ED_Prefix_Opcode)/2) //ED49		OUT (C),C
  DCI  ((ADC_HL_BC  - ED_Prefix_Opcode)/2) //ED4A		ADC HL,BC
  DCI  ((LD_BC_mm   - ED_Prefix_Opcode)/2) //ED4B  	LD BC,(nn)
  DCI  ((NEG        - ED_Prefix_Opcode)/2) //ED4C		NEG*
  DCI  ((RETI       - ED_Prefix_Opcode)/2) //ED4D		RETI
  DCI  ((IM0        - ED_Prefix_Opcode)/2) //ED4E		IM 0*
  DCI  ((LD_R_A     - ED_Prefix_Opcode)/2) //ED4F		LD R,A
  DCI  ((IN_D_C     - ED_Prefix_Opcode)/2) //ED50		IN D,(C)
  DCI  ((OUT_C_D    - ED_Prefix_Opcode)/2) //ED51		OUT (C),D
  DCI  ((SBC_HL_DE  - ED_Prefix_Opcode)/2) //ED52		SBC HL,DE
  DCI  ((ST_DE_mm   - ED_Prefix_Opcode)/2) //ED53  	LD (nn),DE
  DCI  ((NEG        - ED_Prefix_Opcode)/2) //ED54		NEG*
  DCI  ((RETN       - ED_Prefix_Opcode)/2) //ED55		RETN*
  DCI  ((IM1        - ED_Prefix_Opcode)/2) //ED56		IM 1
  DCI  ((LD_A_I     - ED_Prefix_Opcode)/2) //ED57		LD A,I
  DCI  ((IN_E_C     - ED_Prefix_Opcode)/2) //ED58		IN E,(C)
  DCI  ((OUT_C_E    - ED_Prefix_Opcode)/2) //ED59		OUT (C),E
  DCI  ((ADC_HL_DE  - ED_Prefix_Opcode)/2) //ED5A		ADC HL,DE
  DCI  ((LD_DE_mm   - ED_Prefix_Opcode)/2) //ED5B  	LD DE,(nn)
  DCI  ((NEG        - ED_Prefix_Opcode)/2) //ED5C		NEG*
  DCI  ((RETN       - ED_Prefix_Opcode)/2) //ED5D		RETN*
  DCI  ((IM2        - ED_Prefix_Opcode)/2) //ED5E		IM 2
  DCI  ((LD_A_R     - ED_Prefix_Opcode)/2) //ED5F		LD A,R
  DCI  ((IN_H_C     - ED_Prefix_Opcode)/2) //ED60		IN H,(C)
  DCI  ((OUT_C_H    - ED_Prefix_Opcode)/2) //ED61		OUT (C),H
  DCI  ((SBC_HL_HL  - ED_Prefix_Opcode)/2) //ED62		SBC HL,HL
  DCI  ((ST_HL_mm1  - ED_Prefix_Opcode)/2) //ED63  	LD (nn),HL
  DCI  ((NEG        - ED_Prefix_Opcode)/2) //ED64		NEG*
  DCI  ((RETN       - ED_Prefix_Opcode)/2) //ED65		RETN*
  DCI  ((IM0        - ED_Prefix_Opcode)/2) //ED66		IM 0*
  DCI  ((RRD        - ED_Prefix_Opcode)/2) //ED67		RRD
  DCI  ((IN_L_C     - ED_Prefix_Opcode)/2) //ED68		IN L,(C)
  DCI  ((OUT_C_L    - ED_Prefix_Opcode)/2) //ED69		OUT (C),L
  DCI  ((ADC_HL_HL  - ED_Prefix_Opcode)/2) //ED6A		ADC HL,HL
  DCI  ((LD_HL_mm1  - ED_Prefix_Opcode)/2) //ED6B  	LD HL,(nn)
  DCI  ((NEG        - ED_Prefix_Opcode)/2) //ED6C		NEG*
  DCI  ((RETN       - ED_Prefix_Opcode)/2) //ED6D		RETN*
  DCI  ((IM0        - ED_Prefix_Opcode)/2) //ED6E		IM 0*
  DCI  ((RLD        - ED_Prefix_Opcode)/2) //ED6F		RLD
  DCI  ((IN_F_C     - ED_Prefix_Opcode)/2) //ED70		IN F,(C)* / IN (C)*
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //ED71		NOP
  DCI  ((SBC_HL_SP  - ED_Prefix_Opcode)/2) //ED72		SBC HL,SP
  DCI  ((ST_SP_mm   - ED_Prefix_Opcode)/2) //ED73  	LD (nn),SP	
  DCI  ((NEG        - ED_Prefix_Opcode)/2) //ED74		NEG*
  DCI  ((RETN       - ED_Prefix_Opcode)/2) //ED75		RETN*
  DCI  ((IM1        - ED_Prefix_Opcode)/2) //ED76		IM 1*
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //ED77		NOP
  DCI  ((IN_A_C     - ED_Prefix_Opcode)/2) //ED78		IN A,(C)
  DCI  ((OUT_C_A    - ED_Prefix_Opcode)/2) //ED79		OUT (C),A
  DCI  ((ADC_HL_SP  - ED_Prefix_Opcode)/2) //ED7A		ADC HL,SP
  DCI  ((LD_SP_mm   - ED_Prefix_Opcode)/2) //ED7B  	LD SP,(nn)
  DCI  ((NEG        - ED_Prefix_Opcode)/2) //ED7C		NEG*
  DCI  ((RETN       - ED_Prefix_Opcode)/2) //ED7D		RETN*
  DCI  ((IM2        - ED_Prefix_Opcode)/2) //ED7E		IM 2*
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //ED7F		NOP
  DCI  ((LDI        - ED_Prefix_Opcode)/2) //EDA0		LDI
  DCI  ((CPI        - ED_Prefix_Opcode)/2) //EDA1		CPI
  DCI  ((INI        - ED_Prefix_Opcode)/2) //EDA2		INI
  DCI  ((OUTI       - ED_Prefix_Opcode)/2) //EDA3		OUTI
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((LDD        - ED_Prefix_Opcode)/2) //EDA8		LDD
  DCI  ((CPD        - ED_Prefix_Opcode)/2) //EDA9		CPD
  DCI  ((IND        - ED_Prefix_Opcode)/2) //EDAA		IND
  DCI  ((OUTD       - ED_Prefix_Opcode)/2) //EDAB		OUTD
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((LDIR       - ED_Prefix_Opcode)/2) //EDB0		LDIR
  DCI  ((CPIR       - ED_Prefix_Opcode)/2) //EDB1		CPIR
  DCI  ((INIR       - ED_Prefix_Opcode)/2) //EDB2		INIR
  DCI  ((OTIR       - ED_Prefix_Opcode)/2) //EDB3		OTIR
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((LDDR       - ED_Prefix_Opcode)/2) //EDB8		LDDR
  DCI  ((CPDR       - ED_Prefix_Opcode)/2) //EDB9		CPDR
  DCI  ((INDR       - ED_Prefix_Opcode)/2) //EDBA		INDR
  DCI  ((OTDR       - ED_Prefix_Opcode)/2) //EDBB		OTDR
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP
  DCI  ((NOP_4      - ED_Prefix_Opcode)/2) //   		NOP

// GET in R6 (HL) (IX+d) o (IY+d) secondo bit 2 e 1 di r_par (00 HL 10 IX 11 IY 
// registri utilizzati r_pnt,r_ad (r_ad disponibile per eventuale successiva store)
GET_R6PN
  SUB   r_cyc,#3
  ANDS  r_pnt,r_par,#0x06       // flag prefisso DD/FD
  ADD   r_pnt,#ofsHL
  LDRH  r_ad,[r_rp,r_pnt]       // get IX o IY
  REV16 r_ad,r_ad
  BEQ   GET_R6AD                // se IX o IY
  SUB   r_cyc,#8
  LSR   r_pnt,r_pc,#10          // get d
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRSB r_pnt,[r_pnt,r_pc]      
  ADD   r_ad,r_pnt              // e somma ad r_ad
  ADD   r_pc,#1
  MOVT  r_pc,#0
GET_R6AD                        // R6=(r_ad)
  LSR   r_pnt,r_ad,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_pnt,[r_pnt,r_ad]
  STRB  r_pnt,[r_rp,#ofsR6]
  BX    lr

// imposta r_ad con (HL) o (XX+d)
GET_PN_AD
  SUB   r_cyc,#3
  ANDS  r_pnt,r_par,#0x06       // flag prefisso DD/FD
  ADD   r_pnt,#ofsHL
  LDRH  r_ad,[r_rp,r_pnt]       // get IX o IY
  REV16 r_ad,r_ad
  BXEQ  lr                      // se IX o IY
  SUB   r_cyc,#8
  LSR   r_pnt,r_pc,#10          // get d
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRSB r_pnt,[r_pnt,r_pc]      
  ADD   r_ad,r_pnt              // e somma ad r_ad
  ADD   r_pc,#1
  MOVT  r_pc,#0
  BX    lr

// imposta r_ad con (PC) e (PC+1)
GET_NN_AD
  LSR   r_pnt,r_pc,#10          // get ad_lo
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_ad,[r_pnt,r_pc]      
  ADD   r_pc,#1
  MOVT  r_pc,#0
  LSR   r_pnt,r_pc,#10          // get ad_lo
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_pnt,[r_pnt,r_pc]      
  ADD   r_pc,#1
  MOVT  r_pc,#0
  ORR   r_ad,r_pnt,lsl#8
  BX    lr

// imposta r_acc con (r_ad) e (r_ad+1) 
GET_WW_AD
  LSR   r_pnt,r_ad,#10          // get w_lo
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_acc,[r_pnt,r_ad]      
  ADD   r_ad,#1
  MOVT  r_ad,#0
  LSR   r_pnt,r_ad,#10          // get w_hi
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_pnt,[r_pnt,r_ad]      
  ORR   r_acc,r_pnt,lsl#8
  BX    lr


//***************************************************************
//ALU Opcode
//***************************************************************
  
ADD_A_n                         //C6 ADD  A,n
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_op,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  SUB   r_cyc,#3
  B     ADD_A_r_op
ADD_A_XL
ADD_A_XH
  AND   r_acc,r_par,#6
  AND   r_op,#5
  ADD   r_op,r_acc
  B     ADD_A_Xop
ADD_A_XXd
ADD_A_HL                        //86 ADD  A,(HL)
  MOV   r_jmp,lr
  BL    GET_R6PN
  MOV   lr,r_jmp
ADD_A_B                         //80 ADD  A,B
ADD_A_C                         //81 ADD  A,C 
ADD_A_D                         //82 ADD  A,D
ADD_A_E                         //83 ADD  A,E
ADD_A_H                         //84 ADD  A,H
ADD_A_L                         //85 ADD  A,L
ADD_A_A                         //87 ADD  A,A
  AND   r_op,#7   	            //Reg number
ADD_A_Xop
  LDRB  r_op,[r_rp,r_op]    		//Rn
ADD_A_r_op
  LDRB  r_acc,[r_rp,#7] 			  //Ld A
  LSL   r_res,r_acc,#24
  ADDS  r_res,r_op,lsl#24       //somma 
  MRS   r_sr,APSR               //get N Z C V
  LSR   r_res,#24
  STRB  r_res,[r_rp,#7]         //Store A
  EOR   r_acc,r_op              //test H flag
  EOR   r_acc,r_res
  AND   r_acc,#0x10
  BFI   r_sr,r_acc,#23,#5       //H N(=0) 
  BX    lr

ADC_A_n                         //CE ADC  A,n
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_op,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  SUB   r_cyc,#3
  B     ADC_A_r_op
ADC_A_XL
ADC_A_XH
  AND   r_acc,r_par,#6
  AND   r_op,#5
  ADD   r_op,r_acc
  B     ADC_A_Xop
ADC_A_XXd
ADC_A_HL                        //8E ADC  A,(HL)
  MOV   r_jmp,lr
  BL    GET_R6PN
  MOV   lr,r_jmp
ADC_A_B                         //88 ADC  A,B
ADC_A_C                         //89 ADC  A,C 
ADC_A_D                         //8A ADC  A,D
ADC_A_E                         //8B ADC  A,E
ADC_A_H                         //8C ADC  A,H
ADC_A_L                         //8D ADC  A,L
ADC_A_A                         //8F ADC  A,A
  
  AND   r_op,#7    	            //Reg number
ADC_A_Xop
  LDRB  r_op,[r_rp,r_op]   			//Rn
ADC_A_r_op
  LDRB  r_acc,[r_rp,#7] 			  //Ld A
  MOV   r_res,#0xffffffff
  BFI   r_res,r_acc,#24,#8   
  LSLS  r_sr,#3                 //C
  ADCS  r_res,r_op,lsl#24       //somma
  MRS   r_sr,APSR               //get N x C V
  LSRS  r_res,#24
  STRB  r_res,[r_rp,#7]         //Store A
  ORREQ r_sr,#0x40000000        //Z patch  
  EOR   r_acc,r_op              //test H flag
  EOR   r_acc,r_res
  AND   r_acc,#0x10
  BFI   r_sr,r_acc,#23,#5       //H N(=0) 
  BX    lr

SUB_A_n                         //D6 SUB  A,n
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_op,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  SUB   r_cyc,#3
  B     SUB_A_r_op
SUB_A_XL
SUB_A_XH
  AND   r_acc,r_par,#6
  AND   r_op,#5
  ADD   r_op,r_acc
  B     SUB_A_Xop
SUB_A_XXd
SUB_A_HL                        //96 SUB  A,(HL)
  MOV   r_jmp,lr
  BL    GET_R6PN
  MOV   lr,r_jmp
SUB_A_B                         //90 SUB  A,B
SUB_A_C                         //91 SUB  A,C 
SUB_A_D                         //92 SUB  A,D
SUB_A_E                         //93 SUB  A,E
SUB_A_H                         //94 SUB  A,H
SUB_A_L                         //95 SUB  A,L
SUB_A_A                         //97 SUB  A,A
  
  AND   r_op,#7    	            //Reg number
SUB_A_Xop
  LDRB  r_op,[r_rp,r_op]   			//Rn
SUB_A_r_op
  LDRB  r_acc,[r_rp,#7] 			  //Ld A
  LSL   r_res,r_acc,#24
  SUBS  r_res,r_op,lsl#24       //sub
  MRS   r_sr,APSR               //get N Z   DCB 0xC V
  LSR   r_res,#24
  STRB  r_res,[r_rp,#7]         //Store A
  EOR   r_acc,r_op              //test H flag
  EOR   r_acc,r_res
  AND   r_acc,#0x10
  BFI   r_sr,r_acc,#23,#5       //H  
  EOR   r_sr,#0x24000000        //C N(N=1)
  BX    lr

SBC_A_n                         //DE SBC  A,n
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_op,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  SUB   r_cyc,#3
  B     SBC_A_r_op
SBC_A_XL
SBC_A_XH
  AND   r_acc,r_par,#6
  AND   r_op,#5
  ADD   r_op,r_acc
  B     SBC_A_Xop
SBC_A_XXd                       
SBC_A_HL                        //9E SBC  A,(HL)
  MOV   r_jmp,lr
  BL    GET_R6PN
  MOV   lr,r_jmp
SBC_A_B                         //98 SBC  A,B
SBC_A_C                         //99 SBC  A,C 
SBC_A_D                         //9A SBC  A,D
SBC_A_E                         //9B SBC  A,E
SBC_A_H                         //9C SBC  A,H
SBC_A_L                         //9D SBC  A,L
SBC_A_A                         //9F SBC  A,A

  AND   r_op,#7    	            //Reg number
SBC_A_Xop
  LDRB  r_op,[r_rp,r_op]   			//Rn
SBC_A_r_op
  LDRB  r_acc,[r_rp,#7] 		    //Ld A
  LSL   r_res,r_acc,#24
  EOR   r_sr,#0x20000000  
  LSLS  r_sr,#3                 //C
  SBCS  r_res,r_op,lsl#24       //sub
  MRS   r_sr,APSR               //get N x   DCB 0xC V
  LSRS  r_res,#24
  STRB  r_res,[r_rp,#7]         //Store A
  ORREQ r_sr,#0x40000000        //Z patch   
  EOR   r_acc,r_op              //test H flag
  EOR   r_acc,r_res
  AND   r_acc,#0x10
  BFI   r_sr,r_acc,#23,#5       //H  
  EOR   r_sr,#0x24000000        //C N(=1)
  BX    lr

AND_A_n                         //E6 AND  A,n
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_op,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  SUB   r_cyc,#3
  B     AND_A_r_op
AND_A_XL
AND_A_XH
  AND   r_acc,r_par,#6
  AND   r_op,#5
  ADD   r_op,r_acc
  B     AND_A_Xop
AND_A_XXd
AND_A_HL                        //A6 AND  A,(HL)
  MOV   r_jmp,lr
  BL    GET_R6PN
  MOV   lr,r_jmp
AND_A_B                         //A0 AND  A,B
AND_A_C                         //A1 AND  A,C 
AND_A_D                         //A2 AND  A,D
AND_A_E                         //A3 AND  A,E
AND_A_H                         //A4 AND  A,H
AND_A_L                         //A5 AND  A,L
AND_A_A                         //A7 AND  A,A

  AND   r_op,#7    	            //Reg number
AND_A_Xop
  LDRB  r_op,[r_rp,r_op]   			//Rn
AND_A_r_op
  LDRB  r_acc,[r_rp,#7] 			  //Ld A
  AND   r_acc,r_op              //and
  STRB  r_acc,[r_rp,#7]         //Store A
  LSLS  r_res,r_acc,#24         //test N Z
  MRS   r_sr,APSR               //get N Z 
  LDR   r_res,[r_pt,r_acc,lsl#2]//get P
  MOV   r_acc,#0x10             //H=1 M=0
  ORR   r_acc,r_res,lsl#5       //C(=0) V(=P) H(=1) P(=0)
  BFI   r_sr,r_acc,#23,#7       //update sr
  BX    lr

XOR_A_n                         //EE XOR  A,n
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_op,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  SUB   r_cyc,#3
  B     XOR_A_r_op
XOR_A_XL
XOR_A_XH
  AND   r_acc,r_par,#6
  AND   r_op,#5
  ADD   r_op,r_acc
  B     XOR_A_Xop
XOR_A_XXd
XOR_A_HL                        //AE XOR  A,(HL)      
  MOV   r_jmp,lr
  BL    GET_R6PN
  MOV   lr,r_jmp
XOR_A_B                         //A8 XOR  A,B
XOR_A_C                         //A9 XOR  A,C 
XOR_A_D                         //AA XOR  A,D
XOR_A_E                         //AB XOR  A,E
XOR_A_H                         //AC XOR  A,H
XOR_A_L                         //AD XOR  A,L
XOR_A_A                         //AF XOR  A,A
  AND   r_op,#7    	            //Reg number
XOR_A_Xop  
  LDRB  r_op,[r_rp,r_op]   			//Rn
XOR_A_r_op
  LDRB  r_acc,[r_rp,#7] 			  //Ld A
  EOR   r_acc,r_op              //eor
  STRB  r_acc,[r_rp,#7]         //Store A
  LSLS  r_res,r_acc,#24         //test N Z
  MRS   r_sr,APSR               //get N Z 
  LDR   r_res,[r_pt,r_acc,lsl#2]//get P
  LSL   r_acc,r_res,#5          //C(=0) V(=P) H(=0) N(=0)
  BFI   r_sr,r_acc,#23,#7       //update sr
  BX    lr

OR_A_n                          //F6 OR   A,n
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_op,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  SUB   r_cyc,#3
  B     OR_A_r_op
OR_A_XL
OR_A_XH
  AND   r_acc,r_par,#6
  AND   r_op,#5
  ADD   r_op,r_acc
  B     OR_A_Xop
OR_A_XXd
OR_A_HL                         //B6 OR  A,(HL)
  MOV   r_jmp,lr
  BL    GET_R6PN
  MOV   lr,r_jmp
OR_A_B                          //B0 OR  A,B
OR_A_C                          //B1 OR  A,C 
OR_A_D                          //B2 OR  A,D
OR_A_E                          //B3 OR  A,E
OR_A_H                          //B4 OR  A,H
OR_A_L                          //B5 OR  A,L
OR_A_A                          //B7 OR  A,A

  AND   r_op,#7    	            //Reg number
OR_A_Xop  
  LDRB  r_op,[r_rp,r_op]   			//Rn
OR_A_r_op
  LDRB  r_acc,[r_rp,#7] 			  //Ld A
  ORR   r_acc,r_op              //or
  STRB  r_acc,[r_rp,#7]         //Store A
  LSLS  r_res,r_acc,#24         //test N Z
  MRS   r_sr,APSR               //get N Z 
  LDR   r_res,[r_pt,r_acc,lsl#2]//get P
  LSL   r_acc,r_res,#5          //C(=0) V(=P) H(=0) N(=0)
  BFI   r_sr,r_acc,#23,#7       //update sr
  BX    lr

CP_A_n                          //FE CP   A,n
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_op,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  SUB   r_cyc,#3
  B     CP_A_r_op
CP_A_XL
CP_A_XH
  AND   r_acc,r_par,#6
  AND   r_op,#5
  ADD   r_op,r_acc
  B     CP_A_Xop
CP_A_XXd
CP_A_HL                         //BE CP  A,(HL)
  MOV   r_jmp,lr
  BL    GET_R6PN
  MOV   lr,r_jmp
CP_A_B                          //B8 CP  A,B
CP_A_C                          //B9 CP  A,C 
CP_A_D                          //BA CP  A,D
CP_A_E                          //BB CP  A,E
CP_A_H                          //BC CP  A,H
CP_A_L                          //BD CP  A,L
CP_A_A                          //BF CP  A,A
  AND   r_op,#7    	            //Reg number
CP_A_Xop  
  LDRB  r_op,[r_rp,r_op]   			//Rn
CP_A_r_op
  LDRB  r_acc,[r_rp,#7] 			  //Ld A
  LSL   r_res,r_acc,#24
  SUBS  r_res,r_op,lsl#24       //sub
  MRS   r_sr,APSR               //get N Z   DCB 0xC V
  LSR   r_res,#24
  EOR   r_acc,r_op              //test H flag
  EOR   r_acc,r_res
  AND   r_acc,#0x10
  BFI   r_sr,r_acc,#23,#5       //H  
  EOR   r_sr,#0x24000000        //C N(N=1)
  BX    lr


//***************************************************************
// DEC/INC  
//***************************************************************
DEC_XXd                         //35 DEC  (XX+d)
DEC_HLm                         //35 DEC  (HL)
  MOV   r_jmp,lr  
  BL    GET_R6PN                // R6=(HL)/(IX+d)/(IY+d)
  BL    DEC_REG                 // DEC R6
  LDRB  r_acc,[r_rp,#ofsR6]     // Store R6
  BL    WRITE_RAM                                       
  BX    r_jmp
DEC_XH                          //25 DEC  XH 
DEC_XL                          //2D DEC  XL
  UBFX  r_op,r_op,#3,#3         //Reg number (4/5)
  AND   r_acc,r_par,#6      
  ADD   r_op,r_acc
  B     DEC_op
DEC_B                           //05 DEC  B 
DEC_C                           //0D DEC  C
DEC_D                           //15 DEC  D
DEC_E                           //1D DEC  E
DEC_H                           //25 DEC  H 
DEC_L                           //2D DEC  L
DEC_A                           //3D DEC  A  
DEC_REG
  UBFX  r_op,r_op,#3,#3         //Reg number
DEC_op
  LDRB  r_res,[r_rp,r_op]       //--
  LSL   r_acc,r_res,#24         
  SUBS  r_acc,#0x1000000
  LSR   r_acc,#24         
  STRB  r_acc,[r_rp,r_op]
  LSR   r_op,r_sr,#29           //C corrente
  MRS   r_sr,APSR               //get N Z x V 
  BFI   r_sr,r_op,#29,#1        //restore C
  LSLS  r_res,#28               //test H (Z=H)
  ORREQ r_res,#8
  ORR   r_res,#4
  BFI   r_sr,r_res,#24,#4       //H N(=1)
  BX    lr

INC_XXd                         //34 INC  (XX+d) 
INC_HLm                         //34 INC  (HL) 
  MOV   r_jmp,lr  
  BL    GET_R6PN                // R6=(HL)/(IX+d)/(IY+d)
  BL    INC_REG
  LDRB  r_acc,[r_rp,#ofsR6]     // Store R6
  BL    WRITE_RAM                                       
  BX    r_jmp
INC_XH                          //24 INC  XH
INC_XL                          //2C INC  XL
  UBFX  r_op,r_op,#3,#3         //Reg number (4/5)
  AND   r_acc,r_par,#6      
  ADD   r_op,r_acc
  B     INC_op
INC_B                           //04 INC  B
INC_C                           //0C INC  C 
INC_D                           //14 INC  D
INC_E                           //1C INC  E 
INC_H                           //24 INC  H
INC_L                           //2C INC  L
INC_A                           //3C INC  A
INC_REG
  UBFX  r_op,r_op,#3,#3         //Reg number
INC_op
  LDRB  r_acc,[r_rp,r_op]       //++
  LSL   r_acc,#24
  ADDS  r_acc,#0x1000000
  LSR   r_res,r_acc,#24         
  STRB  r_res,[r_rp,r_op]
  LSR   r_op,r_sr,#29           //C corrente
  MRS   r_sr,APSR               //get N Z x V 
  BFI   r_sr,r_op,#29,#1        //restore C
  LSLS  r_acc,#4                //test H (Z=H)
  ORREQ r_acc,#8
  BFI   r_sr,r_acc,#24,#4       //H N(=0)
  BX    lr

//***************************************************************
//Shift
//***************************************************************

RRA                             //1F RRA
  LDRB  r_acc,[r_rp,#ofsA] 			//r_acc=A
  UBFX  r_res,r_sr,#29,#1       //C corrente
  BFI   r_sr,r_acc,#29,#1       //C=A.0
  LSR   r_acc,#1                //z7654321
  BFI   r_acc,r_res,#7,#1       //C7654321
  AND   r_sr,#0xf0000000        //N=0 H=0
  STRB  r_acc,[r_rp,#ofsA]
  BX    lr

RLA                             //17 RLA  
  LDRB  r_acc,[r_rp,#ofsA]  		//r_acc=A
  UBFX  r_res,r_sr,#29,#1       //C corrente
  ORR   r_res,r_acc,lsl#1       //76543210C
  LSR   r_acc,#7
  BFI   r_sr,r_acc,#29,#1       //C=A.7
  AND   r_sr,#0xf0000000        //N=0 H=0
  STRB  r_res,[r_rp,#ofsA]
  BX    lr

RRCA                            //0F RRCA 
  LDRB  r_acc,[r_rp,#ofsA] 			//r_acc=A
  BFI   r_sr,r_acc,#29,#1       //C=A.0
  BFI   r_acc,r_acc,#8,#1       //076543210
  LSR   r_acc,#1                //07654321
  AND   r_sr,#0xf0000000        //N=0 H=0
  STRB  r_acc,[r_rp,#ofsA]
  BX    lr

RLCA                            //07 RLCA 
  LDRB  r_acc,[r_rp,#ofsA] 			//r_acc=A
  ORR   r_acc,r_acc,lsl#8       //7654321076543210
  LSR   r_acc,#7                //zzzzzzz765432107 
  BFI   r_sr,r_acc,#29,#1       //C=A.7
  AND   r_sr,#0xf0000000        //N=0 H=0
  STRB  r_acc,[r_rp,#ofsA]
  BX    lr


SLL_R
  AND   r_op,#7    	            //Reg number
  LDRB  r_acc,[r_rp,r_op]  			//r_acc=Rn
  LSLS  r_acc,#25
  ORR   r_acc,#0x01000000
  B     RR_exit

SRL_R
  AND   r_op,#7    	            //Reg number
  LDRB  r_acc,[r_rp,r_op]  			//r_acc=Rn
  BFI   r_acc,r_acc,#9,#1       //0z76543210
  BIC   r_acc,#1
  LSLS  r_acc,#23
  B     RR_exit

SLA_R
  AND   r_op,#7    	            //Reg number
  LDRB  r_acc,[r_rp,r_op]  			//r_acc=Rn
  LSLS  r_acc,#25
  B     RR_exit

SRA_R
  AND   r_op,#7    	            //Reg number
  LDRSB r_acc,[r_rp,r_op]  			//r_acc=Rn
  BFI   r_acc,r_acc,#9,#1       //0776543210
  BIC   r_acc,#1
  LSLS  r_acc,#23
  B     RR_exit

RR_R
  AND   r_op,#7    	            //Reg number
  LDRB  r_acc,[r_rp,r_op]  			//r_acc=Rn
  BFI   r_acc,r_acc,#9,#1       //0z76543210
  UBFX  r_res,r_sr,#29,#1       //C
  BFI   r_acc,r_res,#8,#1       //0C76543210
  BIC   r_acc,#1
  LSLS  r_acc,#23
  B     RR_exit

RL_R
  AND   r_op,#7    	            //Reg number
  LDRB  r_acc,[r_rp,r_op]  			//r_acc=Rn
  UBFX  r_res,r_sr,#29,#1       //C
  ORR   r_acc,r_res,r_acc,lsl#1 //76543210C
  B     ROT_exit

RRC_R
  AND   r_op,#7    	            //Reg number
  LDRB  r_acc,[r_rp,r_op]  			//r_acc=Rn
  ORR   r_acc,r_acc,lsl#8       //7654321076543210
  BFI   r_acc,r_acc,#9,#1       //7654320076543210
  LSR   r_acc,#1                //z765432007654321 
  B     ROT_exit

RLC_R
  AND   r_op,#7    	            //Reg number
  LDRB  r_acc,[r_rp,r_op]  			//r_acc=Rn
  ORR   r_acc,r_acc,lsl#8       //7654321076543210
  LSR   r_acc,#7                //zzzzzzz765432107 
ROT_exit  
  LSLS  r_acc,#24
RR_exit
  MRS   r_sr,APSR               //get N Z C
  LSR   r_acc,#24
  STRB  r_acc,[r_rp,r_op]       //Store Reg
  LDR   r_res,[r_pt,r_acc,lsl#2]//get P
  LSL   r_res,#4
  BFI   r_sr,r_res,#24,#5
  BX    lr

//***************************************************************
//SET/RES/BIT
//***************************************************************
SET_R
  MOV   r_res,#1                //1<<bit
  UBFX  r_acc,r_op,#3,#3
  LSL   r_res,r_acc
  AND   r_op,#7    	            //Reg number
  LDRB  r_acc,[r_rp,r_op]  			//r_acc=Rn
  ORR   r_acc,r_res
  STRB  r_acc,[r_rp,r_op]  			//Store Reg
  BX    lr

RES_R
  MOV   r_res,#1                //1<<bit
  UBFX  r_acc,r_op,#3,#3
  LSL   r_res,r_acc
  AND   r_op,#7    	            //Reg number
  LDRB  r_acc,[r_rp,r_op]  			//r_acc=Rn
  BIC   r_acc,r_res
  STRB  r_acc,[r_rp,r_op]  			//Store Reg
  BX    lr

BIT_R
  MOV   r_res,#1                //1<<bit
  UBFX  r_acc,r_op,#3,#3
  LSL   r_res,r_acc
  AND   r_op,#7    	            //Reg number
  LDRB  r_acc,[r_rp,r_op]  			//r_acc=Rn
  AND   r_acc,r_res
  LSLS  r_res,r_acc,#24         //Test N Z flag
  LSR   r_op,r_sr,#29           //C corrente
  MRS   r_sr,APSR               //get N Z x x 
  BFI   r_sr,r_op,#29,#1        //restore C
  LDR   r_res,[r_pt,r_acc,lsl#2]//get P
  LSL   r_res,#4
  ORR   r_res,#8                //H=1
  BFI   r_sr,r_res,#24,#5
  BX    lr

//***************************************************************
//JR JC CALL RST
//***************************************************************
JP_XX                           //xD E9 JP   (XX)
JP_HL                           //   E9 JP   (HL)
  AND   r_op,r_par,#6
  ADD   r_op,#ofsHL
  LDRH  r_pc,[r_rp,r_op]
  STRH  r_pc,[r_rp,#ofsPC]
  REV16 r_pc,r_pc
  SUBS  r_cyc,#4
  BX    lr

DJNZ                            //10 DJNZ xx
  SUB   r_cyc,#1
  LDRB  r_acc,[r_rp,#ofsBC]
  SUBS  r_acc,#1
  STRB  r_acc,[r_rp,#ofsBC]
  BNE   JR
  B     No_JR   

JR_NZ                           //20 JR   NZ,xx
  LSLS  r_res,r_sr,#2
  BCC   JR
  B     No_JR
JR_Z                            //28 JR   Z,xx
  LSLS  r_res,r_sr,#2
  BCS   JR
  B     No_JR
JR_NC                           //30 JR   NC,xx
  LSLS  r_res,r_sr,#3
  BCC   JR
  B     No_JR
JR_C                            //38 JR   C,xx
  LSLS  r_res,r_sr,#3
  BCS   JR
  B     No_JR
JR                              //18 JR   xx
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRSB r_acc,[r_pnt,r_pc]
  ADD   r_pc,#1
  ADD   r_pc,r_acc
  MOVT  r_pc,#0
  SUBS  r_cyc,#8
  BX    lr

No_JR  
  ADD   r_pc,#1
  MOVT  r_pc,#0
  SUBS  r_cyc,#3
  BX    lr

JP_NZ                           //C2 JP   NZ,xx
  LSLS  r_res,r_sr,#2
  BCC   JP
  B     No_JP
JP_Z                            //CA JP   Z,nn
  LSLS  r_res,r_sr,#2
  BCS   JP
  B     No_JP
JP_NC                           //D2 JP   NC,nn
  LSLS  r_res,r_sr,#3
  BCC   JP
  B     No_JP
JP_C                            //DA JP   C,nn
  LSLS  r_res,r_sr,#3
  BCS   JP
  B     No_JP
JP_PO                           //E2 JP   PO,nn
  LSLS  r_res,r_sr,#4
  BCC   JP
  B     No_JP
JP_PE                           //EA JP   PE,nn
  LSLS  r_res,r_sr,#4
  BCS   JP
  B     No_JP
JP_P                            //F2 JP   P,nn
  LSLS  r_res,r_sr,#1
  BCC   JP
  B     No_JP
JP_M                            //FA JP   M,nn
  LSLS  r_res,r_sr,#1
  BCS   JP
  B     No_JP

JP                              //C3 JP   xx
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_acc,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_pc,[r_pnt,r_pc]
  ORR   r_pc,r_acc,r_pc,lsl#8
  SUBS  r_cyc,#6
  BX    lr

No_JP  
  ADD   r_pc,#2
  MOVT  r_pc,#0
  SUBS  r_cyc,#6
  BX    lr
 
RST_00                          //C7 RST  00
RST_08                          //CF RST  08
RST_10                          //D7 RST  10
RST_18                          //DF RST  18
RST_20                          //E7 RST  20
RST_28                          //EF RST  28
RST_30                          //F7 RST  30
RST_38                          //FF RST
  REV16 r_res,r_pc
  AND   r_pc,r_op,#0x38
  ADD   r_cyc,#4                
  B     PUSH_r_res


CALL_NZ                         //C4 CALL NZ,xx
  LSLS  r_res,r_sr,#2
  BCC   CALL
  B     No_JP
CALL_Z                          //CC CALL Z,nn
  LSLS  r_res,r_sr,#2
  BCS   CALL
  B     No_JP
CALL_NC                         //D4 CALL NC,nn
  LSLS  r_res,r_sr,#3
  BCC   CALL
  B     No_JP
CALL_C                          //DC CALL C,nn
  LSLS  r_res,r_sr,#3
  BCS   CALL
  B     No_JP
CALL_PO                         //E4 CALL PO,nn
  LSLS  r_res,r_sr,#4
  BCC   CALL
  B     No_JP
CALL_PE                         //EC CALL PE,nn
  LSLS  r_res,r_sr,#4
  BCS   CALL
  B     No_JP
CALL_P                          //F4 CALL P,nn
  LSLS  r_res,r_sr,#1
  BCC   CALL
  B     No_JP
CALL_M                          //FC CALL M,nn
  LSLS  r_res,r_sr,#1
  BCS   CALL
  B     No_JP
CALL                            //CD CALL nn
  MOV   r_jmp,lr
  ADD   r_res,r_pc,#2
  MOVT  r_res,#0
  REV16 r_res,r_res
  BL    JP                      //10cyc
  ADD   r_cyc,#4                
  B     PUSH_call  

RET_NZ                          //C0 RET  NZ
  LSLS  r_res,r_sr,#2
  BCC   RET_CC
  B     No_RET
RET_Z                           //C8 RET  Z
  LSLS  r_res,r_sr,#2
  BCS   RET_CC
  B     No_RET
RET_NC                          //D0 RET  NC
  LSLS  r_res,r_sr,#3
  BCC   RET_CC
  B     No_RET
RET_C                           //D8 RET  C
  LSLS  r_res,r_sr,#3
  BCS   RET_CC
  B     No_RET
RET_PO                          //E0 RET  PO
  LSLS  r_res,r_sr,#4
  BCC   RET_CC
  B     No_RET
RET_PE                          //E8 RET  PE
  LSLS  r_res,r_sr,#4
  BCS   RET_CC
  B     No_RET
RET_P                           //F0 RET  P
  LSLS  r_res,r_sr,#1
  BCC   RET_CC
  B     No_RET
RET_M                           //F8 RET  M
  LSLS  r_res,r_sr,#1
  BCS   RET_CC
No_RET
  SUBS  r_cyc,#1
  BX    lr


PUSH_XX                         //E5 PUSH XX
  UBFX  r_op,r_par,#1,#2        // IX 2/IY 3
  ADD   r_op,#2                 //    4    5
  B     PUSH_op_x2
PUSH_AF                         //F5 PUSH AF
  RBIT  r_acc,r_sr              // r_acc  x  x  N  H  PV C  Z  S
  MOV   r_res,r_sr
  BFI   r_res,r_acc,#24,#6      // r_res  S  Z  N  H  PV C  Z  S ......
  BFI   r_res,r_acc,#23,#4      // r_res  S  Z  N  H  H  PV C  Z ......  
  BFI   r_res,r_acc,#20,#6      // r_res  S  Z  C  H  H  PV N  H ......  
  BFI   r_res,r_acc,#22,#3      // r_res  S  Z  C  H  H  PV N  C ......  
  LSR   r_acc,r_res,#24         // r_res  S  Z  C  H  H  PV N  C
  AND   r_acc,#0xD7
  STRB  r_acc,[r_rp,#ofsR6]
  LDRB  r_res,[r_rp,#ofsA]
  ORR   r_res,r_acc,lsl#8
  B     PUSH_r_res
PUSH_BC                         //C5 PUSH BC
PUSH_DE                         //D5 PUSH DE
PUSH_HL                         //E5 PUSH HL
  UBFX  r_op,r_op,#4,#2
PUSH_op_x2
  LDRH  r_res,[r_rp,r_op,lsl#1]
PUSH_r_res 
  MOV   r_jmp,lr
PUSH_call
  LDRH  r_ad,[r_rp,#ofsSP]
  REV16 r_ad,r_ad
  SUB   r_ad,#1
  MOVT  r_ad,#0
  MOV   r_acc,r_res
  BL    WRITE_RAM
  SUB   r_ad,#1
  MOVT  r_ad,#0
  LSR   r_acc,r_res,#8
  BL    WRITE_RAM
  REV16 r_ad,r_ad
  STRH  r_ad,[r_rp,#ofsSP]
  SUBS  r_cyc,#3
  BX    r_jmp

RET_CC
  SUB   r_cyc,#1
RET                             //C9 RET (pop PC)
  MOV   r_jmp,lr
  MOV   r_op,#ofsPC/2
  BL    POP_op_x2
  REV16 r_pc,r_res              //update r_pc
  BX    r_jmp
POP_XX                          //E1 POP  XX
  UBFX  r_op,r_par,#1,#2        // IX 2/IY 3
  ADD   r_op,#2                 //    4    5
  B     POP_op_x2
POP_AF                          //F1 POP  AF 
  MOV   r_jmp,lr
  BL    POP_af
  LDRH  r_acc,[r_rp,#ofsR6]
  REV16 r_acc,r_acc
  STRH  r_acc,[r_rp,#ofsR6]
  // load r_sr con Z80 flag(r_acc)   S  Z  x  H  x  PV N  C  
  UBFX  r_res,r_acc,#4,#1       //   0  0  0  0  0  0  0  H
  BFI   r_acc,r_acc,#5,#1       //   S  Z  C  H  x  PV N  C
  BFI   r_acc,r_acc,#2,#3       //   S  Z  C  PV N  C  N  C
  BFI   r_acc,r_res,#3,#1       //   S  Z  C  PV H  C  N  C
  BFI   r_acc,r_acc,#1,#2       //   S  Z  C  PV H  N  C  C
  LSL   r_sr,r_acc,#24  
  BX    r_jmp
POP_af
POP_BC                          //C1 POP  BC
POP_DE                          //D1 POP  DE
POP_HL                          //E1 POP  HL
  UBFX  r_op,r_op,#4,#2
POP_op_x2
  LDRH  r_ad,[r_rp,#ofsSP]
  REV16 r_ad,r_ad
  LSR   r_pnt,r_ad,#10          
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_acc,[r_pnt,r_ad]
  ADD   r_ad,#1
  MOVT  r_ad,#0
  LSR   r_pnt,r_ad,#10          
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_res,[r_pnt,r_ad]
  ADD   r_ad,#1
  MOVT  r_ad,#0
  ORR   r_res,r_acc,lsl#8  
  STRH  r_res,[r_rp,r_op,lsl#1]
  REV16 r_ad,r_ad
  STRH  r_ad,[r_rp,#ofsSP]
  SUBS  r_cyc,#6
  BX    lr

//***************************************************************
//16 bit ADD ADC SBC INC DEC
//***************************************************************
ADC_HL_SP                       //ED7A		ADC HL,SP
  MOV   r_op,#ofsSP/2
  B     ADC_HL_op
ADC_HL_BC                       //ED4A		ADC HL,BC
ADC_HL_DE                       //ED5A		ADC HL,DE
ADC_HL_HL                       //ED6A		ADC HL,HL
  UBFX  r_op,r_op,#4,#2
ADC_HL_op
  LDRH  r_op,[r_rp,r_op,lsl#1]  //reg da sommare
  MOVT  r_op,#0xffff
  REV   r_op,r_op
  LDRH  r_acc,[r_rp,#ofsHL]     //HL IX IY
  REV   r_acc,r_acc
  LSLS  r_sr,#3  
  ADCS  r_res,r_acc,r_op        //somma
  EOR   r_acc,r_op              //test H flag
  EOR   r_acc,r_res
  LSR   r_acc,#28               //r_acc.0=H
  LSL   r_acc,#1                //r_acc.1=H N=0
  MRS   r_sr,APSR
  BFI   r_sr,r_acc,#26,#2       //H N(=0)   
  REV   r_res,r_res
  STRH  r_res,[r_rp,#ofsHL]     //Store HL
  LSLS  r_res,#16
  ORREQ r_sr,#0x40000000        //Z patch    
  SUBS  r_cyc,#7
  BX    lr

SBC_HL_SP                       //ED72		SBC HL,SP
  MOV   r_op,#ofsSP/2
  B     SBC_HL_op
SBC_HL_BC                       //ED42		SBC HL,BC
SBC_HL_DE                       //ED52		SBC HL,DE
SBC_HL_HL                       //ED62		SBC HL,HL
  UBFX  r_op,r_op,#4,#2
SBC_HL_op
  LDRH  r_op,[r_rp,r_op,lsl#1]  //reg da sottrarre
  REV   r_op,r_op
  LDRH  r_acc,[r_rp,#ofsHL]     //HL IX IY
  REV   r_acc,r_acc
  EOR   r_sr,#0x20000000
  LSLS  r_sr,#3  
  SBCS  r_res,r_acc,r_op        //sottrae
  EOR   r_acc,r_op              //test H flag
  EOR   r_acc,r_res
  LSR   r_acc,#28               //r_acc.0=H
  MRS   r_sr,APSR
  BFI   r_sr,r_acc,#27,#1       //H   
  REV   r_res,r_res
  STRH  r_res,[r_rp,#ofsHL]     //Store HL
  LSLS  r_res,#16
  ORREQ r_sr,#0x40000000        //Z patch    
  ORR   r_sr,#0x4000000         //N=1
  EOR   r_sr,#0x20000000        //C
  SUBS  r_cyc,#7
  BX    lr

ADD_XX_XX                       //29 ADD  XX,XX
  AND   r_op,r_par,#6
  ADD   r_op,#ofsHL
  B     ADD_XX_op
ADD_XX_SP                       //39 ADD  HL,SP
ADD_HL_SP                       //39 ADD  HL,SP
  MOV   r_op,#ofsSP*8
ADD_XX_BC                       //09 ADD  XL,BC   
ADD_XX_DE                       //19 ADD  XX,DE 
ADD_HL_BC                       //09 ADD  HL,BC   
ADD_HL_DE                       //19 ADD  HL,DE 
ADD_HL_HL                       //29 ADD  HL,HL
  LSR   r_op,#3
ADD_XX_op
  AND   r_op,#0x1e
  LDRH  r_op,[r_rp,r_op]        //reg da sommare
  REV16 r_op,r_op
  AND   r_pnt,r_par,#6
  ADD   r_pnt,#ofsHL
  LDRH  r_acc,[r_rp,r_pnt]      //HL IX IY
  REV16 r_acc,r_acc
  ADD   r_res,r_acc,r_op        //somma
  EOR   r_acc,r_op              //test H flag
  EOR   r_acc,r_res
  AND   r_acc,#0x1000
  BFI   r_sr,r_acc,#15,#13      //H N(=0)   
  LSR   r_acc,r_res,#16         //C in bit 0
  BFI   r_sr,r_acc,#29,#1       //aggiorna C
  REV16 r_res,r_res
  STRH  r_res,[r_rp,r_pnt]      //Store HL IX IY
  SUBS  r_cyc,#7
  BX    lr

INC_XX                          //23 INC  XX
  AND   r_op,r_par,#6
  ADD   r_op,#ofsHL
  B     INC_XX_op
INC_SP                          //33 INC  SP
  MOV   r_op,#ofsSP*8
INC_BC                          //03 INC  BC
INC_DE                          //13 INC  DE 
INC_HL                          //23 INC  HL
  LSR   r_op,#3
INC_XX_op
  LDRH  r_acc,[r_rp,r_op]        
  REV16 r_acc,r_acc
  ADD   r_acc,#1
  REV16 r_acc,r_acc
  STRH  r_acc,[r_rp,r_op]        
  SUBS  r_cyc,#2
  BX    lr

DEC_XX                          //2B DEC  XX
  AND   r_op,r_par,#6
  ADD   r_op,#ofsHL
  B     DEC_XX_op
DEC_SP                          //3B DEC  SP
  MOV   r_op,#ofsSP*8
DEC_BC                          //0B DEC  BC 
DEC_DE                          //1B DEC  DE
DEC_HL                          //2B DEC  HL
  LSR   r_op,#3
  AND   r_op,#0x1e
DEC_XX_op
  LDRH  r_acc,[r_rp,r_op]        
  REV16 r_acc,r_acc
  SUB   r_acc,#1
  REV16 r_acc,r_acc
  STRH  r_acc,[r_rp,r_op]        
  SUBS  r_cyc,#2
  BX    lr


//***************************************************************
//LD 8bit
//***************************************************************
LD_B_B                          //40 LD   B,B 
LD_B_C                          //41 LD   B,C  
LD_B_D                          //42 LD   B,D
LD_B_E                          //43 LD   B,E
LD_B_H                          //44 LD   B,H 
LD_B_L                          //45 LD   B,L
LD_B_A                          //47 LD   B,A
LD_C_B                          //48 LD   C,B 
LD_C_C                          //49 LD   C,C  
LD_C_D                          //4A LD   C,D
LD_C_E                          //4B LD   C,E
LD_C_H                          //4C LD   C,H 
LD_C_L                          //4D LD   C,L
LD_C_A                          //4F LD   C,A
LD_D_B                          //50 LD   D,B 
LD_D_C                          //51 LD   D,C  
LD_D_D                          //52 LD   D,D
LD_D_E                          //53 LD   D,E
LD_D_H                          //54 LD   D,H 
LD_D_L                          //55 LD   D,L
LD_D_A                          //57 LD   D,A
LD_E_B                          //58 LD   E,B 
LD_E_C                          //59 LD   E,C  
LD_E_D                          //5A LD   E,D
LD_E_E                          //5B LD   E,E
LD_E_H                          //5C LD   E,H 
LD_E_L                          //5D LD   E,L
LD_E_A                          //5F LD   E,A
LD_H_B                          //60 LD   H,B 
LD_H_C                          //61 LD   H,C  
LD_H_D                          //62 LD   H,D
LD_H_E                          //63 LD   H,E
LD_H_H                          //64 LD   H,H 
LD_H_L                          //65 LD   H,L
LD_H_A                          //67 LD   H,A
LD_L_B                          //68 LD   L,B 
LD_L_C                          //69 LD   L,C  
LD_L_D                          //6A LD   L,D
LD_L_E                          //6B LD   L,E
LD_L_H                          //6C LD   L,H 
LD_L_L                          //6D LD   L,L
LD_L_A                          //6F LD   L,A
LD_A_B                          //78 LD   A,B 
LD_A_C                          //79 LD   A,C  
LD_A_D                          //7A LD   A,D
LD_A_E                          //7B LD   A,E
LD_A_H                          //7C LD   A,H  
LD_A_L                          //7D LD   A,L
LD_A_A                          //7F LD   A,A
  AND   r_res,r_op,#7           
  LDRB  r_acc,[r_rp,r_res]
  UBFX  r_op,r_op,#3,#3
  STRB  r_acc,[r_rp,r_op]
  BX    lr

LD_B_XH                         //44 LD   B,XH 
LD_B_XL                         //45 LD   B,XL
LD_C_XH                         //4C LD   C,XH 
LD_C_XL                         //4D LD   C,XL
LD_D_XH                         //54 LD   D,XH 
LD_D_XL                         //55 LD   D,XL
LD_E_XH                         //5C LD   E,XH 
LD_E_XL                         //5D LD   E,XL
LD_A_XH                         //7C LD   A,XH 
LD_A_XL                         //7D LD   A,XL
  AND   r_acc,r_par,#6
  AND   r_res,r_op,#5
  ADD   r_res,r_acc
  LDRB  r_acc,[r_rp,r_res]
  UBFX  r_op,r_op,#3,#3
  STRB  r_acc,[r_rp,r_op]
  BX    lr


LD_XH_XH                        //64 LD   XH,XH 
LD_XH_XL                        //65 LD   XH,XL
LD_XL_XH                        //6C LD   XL,XH 
LD_XL_XL                        //6D LD   XL,XL
  AND   r_acc,r_par,#6
  AND   r_res,r_op,#5
  ADD   r_res,r_acc
  B     LD_XLH_XLH
LD_XH_B                         //60 LD   XH,B 
LD_XH_C                         //61 LD   XH,C  
LD_XH_D                         //62 LD   XH,D
LD_XH_E                         //63 LD   XH,E
LD_XH_A                         //67 LD   XH,A
LD_XL_B                         //68 LD   XL,B 
LD_XL_C                         //69 LD   XL,C  
LD_XL_D                         //6A LD   XL,D
LD_XL_E                         //6B LD   XL,E
LD_XL_A                         //6F LD   XL,A
  AND   r_res,r_op,#7
LD_XLH_XLH
  LDRB  r_acc,[r_rp,r_res]
  AND   r_res,r_par,#6
  UBFX  r_op,r_op,#3,#3
  ADD   r_res,r_op
  STRB  r_acc,[r_rp,r_res]
  BX    lr


LD_B_XXd                        //46 LD   B,(XX+d)
LD_C_XXd                        //4E LD   C,(XX+d)
LD_D_XXd                        //56 LD   D,(XX+d)
LD_E_XXd                        //5E LD   E,(XX+d)
LD_H_XXd                        //66 LD   H,(XX+d)
LD_L_XXd                        //6E LD   L,(XX+d)
LD_A_XXd                        //7E LD   A,(XX+d)

LD_B_HL                         //46 LD   B,(HL)
LD_C_HL                         //4E LD   C,(HL)
LD_D_HL                         //56 LD   D,(HL)
LD_E_HL                         //5E LD   E,(HL)
LD_H_HL                         //66 LD   H,(HL)
LD_L_HL                         //6E LD   L,(HL)
LD_A_HL                         //7E LD   A,(HL)
  MOV   r_jmp,lr
  BL    GET_R6PN
  UBFX  r_op,r_op,#3,#3
  STRB  r_pnt,[r_rp,r_op]
  BX    r_jmp

LD_XH_n                         //26 LD   XH,n
LD_XL_n                         //2E LD   XL,n
  AND   r_res,r_par,#6
  UBFX  r_op,r_op,#3,#3
  ADD   r_op,r_res
  B     LD_XLH_n
LD_B_n                          //06 LD   B,n
LD_C_n                          //0E LD   C,n   
LD_D_n                          //16 LD   D,n 
LD_E_n                          //1E LD   E,n
LD_H_n                          //26 LD   H,n
LD_L_n                          //2E LD   L,n
LD_A_n                          //3E LD   A,n
  UBFX  r_op,r_op,#3,#3
LD_XLH_n
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_acc,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  STRB  r_acc,[r_rp,r_op]
  SUBS  r_cyc,#3                //-3 cicli
  BX    lr

LD_A_mm                         //3A LD   A,(mm)
  MOV   r_jmp,lr
  BL    GET_NN_AD
  SUB   r_cyc,#6
  B     LD_A_AD
LD_A_BC                         //0A LD   A,(BC)  
LD_A_DE                         //1A LD   A,(DE)
  MOV   r_jmp,lr
  LSR   r_op,#4
  LDRH  r_ad,[r_rp,r_op,lsl#1]
  REV16 r_ad,r_ad
LD_A_AD
  LSR   r_pnt,r_ad,#10  
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_acc,[r_pnt,r_ad]      
  STRB  r_acc,[r_rp,#ofsA]
  SUBS  r_cyc,#3
  BX    r_jmp

//***************************************************************
//LD 16bit
//***************************************************************

LD_XX_nn                        //21 LD   XX,nn
  AND   r_op,r_par,#6
  ADD   r_op,#ofsHL
  B     LD_XX_op
LD_SP_nn                        //31 LD   SP,nn
  MOV   r_op,#ofsSP*8
LD_BC_nn                        //01 LD   BC,nn 
LD_DE_nn                        //11 LD   DE,nn
LD_HL_nn                        //21 LD   HL,nn
  LSR   r_op,#3
LD_XX_op
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_acc,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_res,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  ORR   r_acc,r_res,r_acc,lsl#8 //swap
  STRH  r_acc,[r_rp,r_op]
  SUB   r_cyc,#6
  BX    lr


LD_SP_mm                        //ED7B LD SP,(nn)
  MOV   r_op,#ofsSP
  B     LD_rop_mm
LD_BC_mm                        //ED4B LD BC,(nn)
LD_DE_mm                        //ED5B LD DE,(nn)
LD_HL_mm1                       //ED6B LD HL,(nn)
  UBFX  r_op,r_op,#4,#2
  LSL   r_op,#1
  B     LD_rop_mm
LD_XX_mm                        //xD 2A LD XX,(nn)
LD_HL_mm                        //   2A LD HL,(mm)
  AND   r_op,r_par,#6
  ADD   r_op,#ofsHL
LD_rop_mm
  MOV   r_jmp,lr
  BL    GET_NN_AD               //r_ad=(pc+2)(pc+1)
  BL    GET_WW_AD               //r_acc=(r_ad)(r_ad+1)
  REV16 r_acc,r_acc
  STRH  r_acc,[r_rp,r_op]
  SUB   r_cyc,#12
  BX    r_jmp

LD_SP_XX                        //xD F9 LD   SP,XX
LD_SP_HL                        //   F9 LD   SP,HL
  AND   r_op,r_par,#6
  ADD   r_op,#ofsHL
  LDRH  r_acc,[r_rp,r_op]
  STRH  r_acc,[r_rp,#ofsSP]
  SUB   r_cyc,#2
  BX    lr


//***************************************************************
// Store 8bit
//***************************************************************
ST_B_XXd                        //70 LD   (XX+d),B
ST_C_XXd                        //71 LD   (XX+d),C 
ST_D_XXd                        //72 LD   (XX+d),D
ST_E_XXd                        //73 LD   (XX+d),E
ST_H_XXd                        //74 LD   (XX+d),H
ST_L_XXd                        //75 LD   (XX+d),L
ST_A_XXd                        //77 LD   (XX+d),A

ST_B_HL                         //70 LD   (HL),B
ST_C_HL                         //71 LD   (HL),C 
ST_D_HL                         //72 LD   (HL),D
ST_E_HL                         //73 LD   (HL),E
ST_H_HL                         //74 LD   (HL),H
ST_L_HL                         //75 LD   (HL),L
ST_A_HL                         //77 LD   (HL),A
  MOV   r_jmp,lr
  AND   r_op,#7
  LDRB  r_acc,[r_rp,r_op]
  BL    GET_PN_AD
  BL    WRITE_RAM
  ADD   r_cyc,#4
  BX    r_jmp

ST_A_nn                         //32 LD   (nn),A
  MOV   r_jmp,lr
  BL    GET_NN_AD
  SUB   r_cyc,#6
  B     ST_a_ad
ST_A_BC                         //02 LD   (BC),A
ST_A_DE                         //12 LD   (DE),A 
  MOV   r_jmp,lr
  LSR   r_op,#3
  LDRH  r_ad,[r_rp,r_op]
  REV16 r_ad,r_ad
ST_a_ad
  LDRB  r_acc,[r_rp,#ofsA]
  BL    WRITE_RAM
  ADD   r_cyc,#1
  BX    r_jmp

ST_n_XXd                        //xD 36 LD   (XX+d),n
  ADD   r_cyc,#3
ST_n_HL                         //   36 LD   (HL),n
  MOV   r_jmp,lr
  BL    GET_PN_AD
ST_pc_ad
  LSR   r_pnt,r_pc,#10
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_acc,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  BL    WRITE_RAM
  ADD   r_cyc,#1
  BX    r_jmp

//***************************************************************
// Store 16bit
//***************************************************************
ST_SP_mm                        //ED73 n n	LD (nn),SP	
  MOV   r_op,#ofsSP
  B     ST_rop_mm
ST_BC_mm                        //ED43 n n	LD (nn),BC
ST_DE_mm                        //ED53 n n	LD (nn),DE
ST_HL_mm1                       //ED63 n n	LD (nn),HL
  UBFX  r_op,r_op,#3,#3
  B     ST_rop_mm
ST_XX_mm                        //xD 22 LD (nn),XX
  SUB   r_cyc,#4
ST_HL_mm                        //    22 LD  (mm),HL
  AND   r_op,r_par,#6
  ADD   r_op,#ofsHL
ST_rop_mm
  MOV   r_jmp,lr
  BL    GET_NN_AD               //r_ad=(pc+2)(pc+1)
  LDRH  r_res,[r_rp,r_op]       //r_res=reg_16 0xLLHH
  REV16 r_acc,r_res
  BL    WRITE_RAM               //LL->(r_ad)
  ADD   r_ad,#1
  MOVT  r_ad,#0
  MOV   r_acc,r_res
  BL    WRITE_RAM               //HH->(r_ad+1)
  SUBS  r_cyc,#4
  BX    r_jmp

//***************************************************************
// ex
//***************************************************************
EX_SP_XX                        //xD E3 EX   (SP),XX
EX_SP_HL                        //   E3 EX   (SP),HL
  MOV   r_jmp,lr
  AND   r_op,r_par,#6
  ADD   r_op,#ofsHL
  LDRH  r_res,[r_rp,r_op]       //r_res=HL/IX/IY (LLHH)
  LDRH  r_ad,[r_rp,#ofsSP]
  REV16 r_ad,r_ad               //r_ad=SP
  LSR   r_pnt,r_ad,#10          //r_pnt=(SP)
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_pnt,[r_pnt,r_ad]            
  LSR   r_acc,r_res,#8          //r_acc=HL/IX/IY lo
  BFI   r_res,r_pnt,#8,#8       //HL/IX/IY lo=(SP)
  BL    WRITE_RAM
  ADD   r_ad,#1
  MOVT  r_ad,#0
  LSR   r_pnt,r_ad,#10          //r_pnt=(SP+1)
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_pnt,[r_pnt,r_ad]            
  AND   r_acc,r_res,#0xff       //r_acc=HL/IX/IY hi
  BFI   r_res,r_pnt,#0,#8       //HL/IX/IY lo=(SP+1)
  BL    WRITE_RAM
  STRH  r_res,[r_rp,r_op]       //aggiornam HL/IX/IY
  SUB   r_cyc,#7
  BX    r_jmp

EX_AF                           //08 EX   AF,AF' 
  // r_acc = r_sr in formato Z80 flag
  RBIT  r_acc,r_sr              // r_acc  x  x  N  H  PV C  Z  S
  BFI   r_sr,r_acc,#24,#6       // r_sr   S  Z  N  H  PV C  Z  S ......
  BFI   r_sr,r_acc,#23,#4       // r_sr   S  Z  N  H  H  PV C  Z ......  
  BFI   r_sr,r_acc,#20,#6       // r_sr   S  Z  C  H  H  PV N  H ......  
  BFI   r_sr,r_acc,#22,#3       // r_sr   S  Z  C  H  H  PV N  C ......  
  LSR   r_acc,r_sr,#24          // r_sr   S  Z  C  H  H  PV N  C
  AND   r_acc,#0xD7
  STRB  r_acc,[r_rp,#ofsR6]
  LDRH  r_res,[r_rp,#ofsR6]
  LDRH  r_acc,[r_rp,#ofsR6i]
  STRH  r_res,[r_rp,#ofsR6i]
  STRH  r_acc,[r_rp,#ofsR6]
  // load r_sr con Z80 flag(r_acc)   S  Z  x  H  x  PV N  C  
  UBFX  r_res,r_acc,#4,#1       //   0  0  0  0  0  0  0  H
  BFI   r_acc,r_acc,#5,#1       //   S  Z  C  H  x  PV N  C
  BFI   r_acc,r_acc,#2,#3       //   S  Z  C  PV N  C  N  C
  BFI   r_acc,r_res,#3,#1       //   S  Z  C  PV H  C  N  C
  BFI   r_acc,r_acc,#1,#2       //   S  Z  C  PV H  N  C  C
  LSL   r_sr,r_acc,#24  
  BX    lr

EXX                             //D9 EXX
  LDRH  r_res,[r_rp,#0x0]
  LDRH  r_acc,[r_rp,#0x20]
  STRH  r_res,[r_rp,#0x20]
  STRH  r_acc,[r_rp,#0x0]
  LDRH  r_res,[r_rp,#0x2]
  LDRH  r_acc,[r_rp,#0x22]
  STRH  r_res,[r_rp,#0x22]
  STRH  r_acc,[r_rp,#0x2]
  LDRH  r_res,[r_rp,#0x4]
  LDRH  r_acc,[r_rp,#0x24]
  STRH  r_res,[r_rp,#0x24]
  STRH  r_acc,[r_rp,#0x4]
  BX    lr

EX_DE_HL                        //EB EX   DE,HL
  LDRH  r_res,[r_rp,#2]
  LDRH  r_acc,[r_rp,#4]
  STRH  r_res,[r_rp,#4]
  STRH  r_acc,[r_rp,#2]
  BX    lr

//***************************************************************
// misc.
//***************************************************************
LDI                             //EDA0		LDI
  MOV   r_jmp,lr
  LDRH  r_op,[r_rp,#ofsHL]
  REV16 r_op,r_op
  ADD   r_res,r_op,#1
  LDRH  r_ad,[r_rp,#ofsDE]      
  REV16 r_ad,r_ad
  ADD   r_acc,r_ad,#1  
  B     LD_ID
LDD                             //EDA8		LDD
  MOV   r_jmp,lr
  LDRH  r_op,[r_rp,#ofsHL]
  REV16 r_op,r_op
  SUB   r_res,r_op,#1
  LDRH  r_ad,[r_rp,#ofsDE]      
  REV16 r_ad,r_ad
  SUB   r_acc,r_ad,#1  
LD_ID
  REV16 r_res,r_res
  STRH  r_res,[r_rp,#ofsHL]
  REV16 r_acc,r_acc
  STRH  r_acc,[r_rp,#ofsDE]
  LSR   r_pnt,r_op,#10          // r_acc=(HL)
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_acc,[r_pnt,r_op]      
  BL    WRITE_RAM               // (DE)=r_acc
  LDRH  r_ad,[r_rp,#ofsBC]
  REV16 r_ad,r_ad
  SUBS  r_ad,#1                 // BC--
  REV16 r_ad,r_ad
  STRH  r_ad,[r_rp,#ofsBC]
  AND   r_sr,#0xE0000000        // N=0 H=0
  ORRNE r_sr,#0x10000000        // PV=(BC!=0)
  SUB   r_cyc,#4
  BX    r_jmp
  
LDIR                            //EDB0		LDIR
  MOV   r_jmp,lr
  LDRH  r_op,[r_rp,#ofsHL]
  REV16 r_op,r_op
  LDRH  r_ad,[r_rp,#ofsDE]      
  REV16 r_ad,r_ad
  LDRH  r_res,[r_rp,#ofsBC]
  REV16 r_res,r_res
LDIR_LP  
  LSR   r_pnt,r_op,#10          // r_acc=(HL)
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_acc,[r_pnt,r_op]      
  BL    WRITE_RAM               // (DE)=r_acc
  ADD   r_op,#1
  MOVT  r_op,#0
  ADD   r_ad,#1
  MOVT  r_ad,#0
  SUBS  r_res,#1
  BEQ   LDR_END
  SUBS  r_cyc,#9
  BPL   LDIR_LP
  AND   r_sr,#0xE0000000        // V=0 N=0 H=0
  ORR   r_sr,#0x4000000         //Debug 
  SUB   r_pc,#2
  MOVT  r_pc,#0
  B     LDR_EXIT

LDDR
  MOV   r_jmp,lr
  LDRH  r_op,[r_rp,#ofsHL]
  REV16 r_op,r_op
  LDRH  r_ad,[r_rp,#ofsDE]      
  REV16 r_ad,r_ad
  LDRH  r_res,[r_rp,#ofsBC]
  REV16 r_res,r_res
LDDR_LP  
  LSR   r_pnt,r_op,#10          // r_acc=(HL)
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_acc,[r_pnt,r_op]      
  BL    WRITE_RAM               // (DE)=r_acc
  SUB   r_op,#1
  MOVT  r_op,#0
  SUB   r_ad,#1
  MOVT  r_ad,#0
  SUBS  r_res,#1
  BEQ   LDR_END
  SUBS  r_cyc,#9
  BPL   LDDR_LP
  SUB   r_pc,#2
  MOVT  r_pc,#0
  AND   r_sr,#0xE0000000        // V=0 N=0 H=0
  ORR   r_sr,#0x4000000         //Debug 
  B     LDR_EXIT
LDR_END
  SUB   r_cyc,#4
  AND   r_sr,#0xE0000000        // V=0 N=0 H=0
LDR_EXIT
  REV16 r_res,r_res
  STRH  r_res,[r_rp,#ofsBC]
  REV16 r_ad,r_ad
  STRH  r_ad,[r_rp,#ofsDE]
  REV16 r_op,r_op
  STRH  r_op,[r_rp,#ofsHL]
  BX    r_jmp
 

CPI                             //EDA1		CPI
CPD                             //EDA9		CPD
  UBFX  r_op,r_op,#2,#2         // 2  cpd / 0 cpi
  RSB   r_op,#1                 // -1 cpd / 1 cpi
  LDRB  r_acc,[r_rp,#7] 			  // Ld A
  LDRH  r_ad,[r_rp,#ofsHL]
  REV16 r_ad,r_ad
  LDRH  r_res,[r_rp,#ofsBC]
  REV16 r_res,r_res
  LSR   r_pnt,r_ad,#10          // r_pnt=(HL)
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_pnt,[r_pnt,r_ad]      
  ADD   r_ad,r_op               // r_ad --/++
  SUBS  r_res,#1
  B     CPR_END
CPIR                            //EDB1		CPIR
CPDR                            //EDB9		CPDR
  UBFX  r_op,r_op,#2,#2         // 2  cpd / 0 cpi
  RSB   r_op,#1                 // -1 cpd / 1 cpi
  LDRB  r_acc,[r_rp,#7] 			  // Ld A
  LDRH  r_ad,[r_rp,#ofsHL]
  REV16 r_ad,r_ad
  LDRH  r_res,[r_rp,#ofsBC]
  REV16 r_res,r_res
CPR_LP
  LSR   r_pnt,r_ad,#10          // r_pnt=(HL)
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_pnt,[r_pnt,r_ad]      
  ADD   r_ad,r_op               // r_ad --/++
  SUBS  r_res,#1
  BEQ   CPR_END                 // --BC==0 ->END
  CMP   r_pnt,r_acc             // (HL)==A ->END
  BEQ   CPR_END
  SUBS  r_cyc,#13
  BPL   CPR_LP                  // cyc-=13>0 ->loop
  SUB   r_pc,#2
  MOVT  r_pc,#0
  B     CPR_EXIT
CPR_END  
  SUB   r_cyc,#8
CPR_EXIT
  REV16 r_res,r_res
  STRH  r_res,[r_rp,#ofsBC]
  REV16 r_ad,r_ad
  STRH  r_ad,[r_rp,#ofsHL]
  UBFX  r_sr,r_sr,#29,#1        // C corrente
  SUB   r_op,r_acc,r_pnt
  EOR   r_acc,r_pnt             //test H flag
  EOR   r_acc,r_op
  AND   r_acc,#0x10
  ORR   r_acc,#0x08             //N=1
  ORR   r_acc,r_sr,lsl#6        //C 
  LSLS  r_op,#24
  MRS   r_sr,APSR               //get N Z   
  BFI   r_sr,r_acc,#23,#7       //update r_sr  
  CMP   r_res,#0
  ORRNE r_sr,#0x10000000        // PV=(BC!=0)
  BX    lr

CCF                             //3F CCF
  UBFX  r_res,r_sr,#29,#1       // C corrente
  BFI   r_sr,r_res,#27,#1       // in H
  AND   r_sr,#0xF8000000        // N=0
  EOR   r_sr,#0x20000000        // CPL C
  BX    lr

SCF                             //37 SCF
  AND   r_sr,#0xf0000000        // H=0 N=0
  ORR   r_sr,#0x20000000        // C=1
  BX    lr

RRD                             //ED67		RRD
RLD                             //ED6F		RLD
  MOV   r_jmp,lr
  LDRH  r_ad,[r_rp,#ofsHL]
  REV16 r_ad,r_ad
  LSR   r_pnt,r_ad,#10          // r_pnt=(HL)
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_pnt,[r_pnt,r_ad]      
  LDRB  r_res,[r_rp,#ofsA]      // r_res=A
  ANDS  r_op,#08
  BNE   RXD_RLD
  LSR   r_acc,r_pnt,#4          
  BFI   r_acc,r_res,#4,#4
  BFI   r_res,r_pnt,#0,#4
  B     RXD_end
RXD_RLD
  MOV   r_acc,r_res
  BFI   r_acc,r_pnt,#4,#4
  LSR   r_pnt,#4
  BFI   r_res,r_pnt,#0,#4
RXD_end
  STRB  r_res,[r_rp,#ofsA]      // store A
  BL    WRITE_RAM               // store(HL)
  AND   r_acc,r_sr,#0x20000000  // C corrente  
  LSLS  r_pnt,r_res,#24         // tst N Z
  MRS   r_sr,APSR               // get N Z   
  AND   r_sr,#0xc0000000 
  ORR   r_sr,r_acc              // update C
  LDR   r_acc,[r_pt,r_res,lsl#2]// get P
  BFI   r_sr,r_acc,#28,#1
  SUB   r_cyc,#6
  BX    r_jmp

CPL                             //2F CPL
  LDRB  r_acc,[r_rp,#ofsA]
  EOR   r_acc,#0xff
  STRB  r_acc,[r_rp,#ofsA]
  ORR   r_sr,#0x0c000000
  BX    lr
 
NEG                             //ED44		NEG
  MOV   r_res,#0
  LDRB  r_acc,[r_rp,#7] 			  //Ld A
  SUBS  r_res,r_acc,lsl#24      //0-A
  MRS   r_sr,APSR               //get N Z !C
  LSR   r_res,#24
  STRB  r_res,[r_rp,#7]         //Store A
  EOR   r_res,r_acc             //test H flag
  AND   r_res,#0x10
  CMP   r_acc,#0x80
  ORREQ r_res,#0x20             //V=(A==0x80)
  BFI   r_sr,r_res,#23,#6       //H  
  EOR   r_sr,#0x24000000        //C N(N=1)
  BX    lr

DAA                             //27 DAA
  LDRB  r_acc,[r_rp,#ofsA]
  MOV   r_res,#0
  TST   r_sr,#0x8000000         // H==1
  ORRNE r_res,#0x06
  AND   r_op,r_acc,#0x0f        // || A.lo > 0x09 
  CMP   r_op,#0x09
  ORRGT r_res,#0x06
  TST   r_sr,#0x20000000        // C==1
  ORRNE r_res,#0x60
  CMP   r_acc,#0x99             // || A > 0x99
  ORRGT r_res,#0x60
  TST   r_sr,#0x4000000         // N ? sub/add
  ADDEQ r_op,r_acc,r_res
  SUBNE r_op,r_acc,r_res
  ORRNE r_res,#0x08             // r_res.3=N
  EOR   r_acc,r_op              // test H
  AND   r_acc,#0x10
  ORR   r_res,r_acc             // r_res.4=H
  STRB  r_op,[r_rp,#ofsA]       // store A
  LSLS  r_op,#24                // tst N Z
  MRS   r_sr,APSR               // get N Z   
  BFI   r_sr,r_res,#23,#7       // update CxHN
  LDR   r_acc,[r_pt,r_res,lsl#2]// get P
  BFI   r_sr,r_acc,#28,#1
  BX    lr

LD_A_I      //ED57		LD A,I
  LDRB  r_acc,[r_rp,#ofsI]
  STRB  r_acc,[r_rp,#ofsA]
  AND   r_res,r_sr,#0x20000000  // C corrente  
  LSLS  r_acc,#24               // N Z 
  MRS   r_sr,APSR               // get N Z   
  LDRB  r_acc,[r_rp,#ofsIFF]
  BFI   r_res,r_acc,#28,#1      // V=IFF2
  BFI   r_sr,r_res,#0,#30
  BX    lr

LD_I_A      //ED47		LD I,A
  LDRB  r_acc,[r_rp,#ofsA]
  STRB  r_acc,[r_rp,#ofsI]
  BX    lr

LD_A_R                          //ED5F		LD A,R
  LDRB  r_acc,[r_rp,#ofsR]
  MRS   r_res,APSR               
  ADD   r_acc,r_res,lsr#25
  EOR   r_acc,r_cyc
  STRB  r_acc,[r_rp,#ofsR]
  STRB  r_acc,[r_rp,#ofsA]
  AND   r_res,r_sr,#0x20000000  // C corrente  
  LSLS  r_acc,#24               // N Z 
  MRS   r_sr,APSR               // get N Z   
  LDRB  r_acc,[r_rp,#ofsIFF]
  BFI   r_res,r_acc,#28,#1      // V=IFF2
  BFI   r_sr,r_res,#0,#30
  BX    lr

LD_R_A                          //ED4F		LD R,A
  LDRB  r_acc,[r_rp,#ofsA]
  STRB  r_acc,[r_rp,#ofsR]
  BX    lr

IM0                             //ED46		IM 0
IM1                             //ED56		IM 1
IM2                             //ED5E		IM 2
  LDRB  r_acc,[r_rp,#ofsIFF]
  AND   r_op,#0x18 
  BIC   r_acc,#0x18
  ORR   r_acc,r_op  
  STRB  r_acc,[r_rp,#ofsIFF]
  BX    lr

DIRQ                            //F3 DI
  LDRB  r_acc,[r_rp,#ofsIFF]
  BIC   r_acc,#0x03
  STRB  r_acc,[r_rp,#ofsIFF]
  BX    lr

EIRQ                            //FB EI
  LDRB  r_acc,[r_rp,#ofsIFF]
  ORR   r_acc,#0x03 
  STRB  r_acc,[r_rp,#ofsIFF]
  BX    lr

HALT                            //76 HALT
  LDRB  r_acc,[r_rp,#ofsIFF]
  ORR   r_acc,#0x80 
  STRB  r_acc,[r_rp,#ofsIFF]
  BX    lr

RETI                            //ED4D		RETI
RETN                            //ED45		RETN
  LDRB  r_acc,[r_rp,#ofsIFF]
  BFI   r_acc,r_acc,#1,#1 
  STRB  r_acc,[r_rp,#ofsIFF]
  B     RET

NOP_4                           //00 NOP            
  BX    lr



//***************************************************************
// MEMORY write 
//***************************************************************
WRITE_RAM                       // Ram[r_ad]=r_acc 
  SUB   r_cyc,#4
  CMP   r_ad,r_par,lsr#16       //r_par HI=RAM base address logico 
  BXCC  lr                      //r_pnt<ram_address:exit
  LSR   r_pnt,r_ad,#10          //Store
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  STRB  r_acc,[r_pnt,r_ad]
  SBFX  r_pnt,r_ad,#2,#14
  CMP   r_pnt,#-1               //Test >=FFFC (3fff Sign extd.)
  BXNE  lr
MempntSetup                     //Bank switch
  ANDS  r_pnt,r_ad,#0x03
  TBB   [pc,r_pnt]
MempntSwitch
  DCB  ((MemReg0 - MempntSwitch)/2)  
  DCB  ((MemReg1 - MempntSwitch)/2)  
  DCB  ((MemReg2 - MempntSwitch)/2)  
  DCB  ((MemReg3 - MempntSwitch)/2)  

MemReg1
  LDRB  r_pnt,[r_rp,#ofsRBM]
  AND   r_acc,r_pnt
  LDR   r_pnt,[r_rp,#ofsXROM]
  ADD   r_acc,r_pnt,r_acc,lsl#14
  MOV   r_pnt,#1
MemReg1Lp
  STR   r_acc,[r_mp,r_pnt,lsl#2]
  ADD   r_pnt,#1
  CMP   r_pnt,#16
  BNE   MemReg1Lp
  BX    lr

MemReg2
  LDRB  r_pnt,[r_rp,#ofsRBM]
  AND   r_acc,r_pnt
  LDR   r_pnt,[r_rp,#ofsXROM]
  ADD   r_acc,r_pnt,r_acc,lsl#14
  SUB   r_acc,#0x4000
  MOV   r_pnt,#16
MemReg2Lp
  STR   r_acc,[r_mp,r_pnt,lsl#2]
  ADD   r_pnt,#1
  CMP   r_pnt,#32
  BNE   MemReg2Lp
  BX    lr

MemReg3  
  STRB  r_acc,[r_rp,#ofsBS3]
  LDRB  r_pnt,[r_rp,#ofsBS0]
  TST   r_pnt,#0x08
  BNE   MemBank3Ram
MemBank3Rom  
  MOVT  r_par,#0xc000
  LDRB  r_pnt,[r_rp,#ofsRBM]
  AND   r_acc,r_pnt
  LDR   r_pnt,[r_rp,#ofsXROM]
  ADD   r_pnt,r_acc,lsl#14
  SUB   r_pnt,#0x8000
  B     MemBank3  

MemReg0
  STRB  r_acc,[r_rp,#ofsBS0]
  TST   r_acc,#0x08
  LDRBEQ r_acc,[r_rp,#ofsBS3]
  BEQ   MemBank3Rom
MemBank3Ram  
  MOVT  r_par,#0x8000
  LDR   r_pnt,[r_rp,#ofsXRAM]
  SUB   r_pnt,#0x4000
  TST   r_acc,#0x04
  SUBEQ r_pnt,#0x4000
MemBank3
  MOV   r_acc,#32
MemBank3Lp
  STR   r_pnt,[r_mp,r_acc,lsl#2]
  ADD   r_acc,#1
  CMP   r_acc,#48
  BNE   MemBank3Lp
  BX    lr

//***************************************************************
// I/O
//***************************************************************
INIR                            //EDB2		INIR
INDR                            //EDBA		INDR
  MOV   r_jmp,lr
  UBFX  r_op,r_op,#2,#2         // 2  indr / 0 inir
  RSB   r_op,#1                 // -1 indr / 1 indr
  LDRH  r_ad,[r_rp,#ofsHL]
  REV16 r_ad,r_ad
  LDRB  r_res,[r_rp,#ofsB] 			// r_res=B
INR_LP
  BL    IN_C                    // r_acc=IN(C)
  BL    WRITE_RAM               // (HL)=r_acc  
  ADD   r_ad,r_op               // r_ad --/++
  SUBS  r_res,#1                // se (--r_res)==0)) end
  BEQ   INR_END
  SUBS  r_cyc,#13
  BPL   INR_LP
  SUB   r_pc,#2
  MOVT  r_pc,#0
  B     INR_EXIT
INR_END  
  SUB   r_cyc,#8
INR_EXIT
  STRB  r_res,[r_rp,#ofsB]
  REV16 r_ad,r_ad
  STRH  r_ad,[r_rp,#ofsHL]      // aggiorna HL
  ORR   r_sr,#0x44000000        // Z=1 N=1
  BX    r_jmp


INI                             //EDA2		INI
IND                             //EDAA		IND
  MOV   r_jmp,lr
  BL    IN_C                    // r_acc=IN(C)
  LDRH  r_ad,[r_rp,#ofsHL]
  REV16 r_ad,r_ad
  BL    WRITE_RAM               // (HL)=r_acc  
  UBFX  r_op,r_op,#2,#2         // 2  ind / 0 ini
  RSB   r_op,#1                 // -1 ind / 1 ind
  ADD   r_ad,r_op               // r_ad --/++
  REV16 r_ad,r_ad
  STRH  r_ad,[r_rp,#ofsHL]      // aggiorna HL
  AND   r_sr,#0xbc000000        // Z=0
  LDRB  r_res,[r_rp,#ofsB] 			
  SUBS  r_res,#1
  STRB  r_res,[r_rp,#ofsB]
  ORREQ r_sr,#0x40000000        // Z=(B==1)
  ORR   r_sr,#0x4000000         // N=1
  BX    r_jmp

IN_A_n                          //DB IN   A,(&00)
  MOV   r_jmp,lr
  LSR   r_pnt,r_pc,#10          // r_pnt=n
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_pnt,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  BL    IN_N
  STRB  r_acc,[r_rp,#ofsA]
  BX    r_jmp

IN_B_C                          //ED40		IN B,(C)
IN_C_C                          //ED48		IN C,(C)
IN_D_C                          //ED50		IN D,(C)
IN_E_C                          //ED58		IN E,(C)
IN_H_C                          //ED60		IN H,(C)
IN_L_C                          //ED68		IN L,(C)
IN_F_C                          //ED70		IN F,(C)* / IN (C)*
IN_A_C                          //ED78		IN A,(C)
  MOV   r_jmp,lr
  BL    IN_C
  UBFX  r_op,r_op,#3,#3
  CMP   r_op,#6
  STRBNE r_acc,[r_rp,r_op]
IN_f_upd
  AND   r_res,r_sr,#0x20000000  // C 
  LSLS  r_acc,#24               // N Z 
  MRS   r_sr,APSR               // get N Z   
  BFI   r_sr,r_res,#0,#30       // update CxHN
  LDR   r_acc,[r_pt,r_res,lsl#2]// get P
  BFI   r_sr,r_acc,#28,#1
  BX    r_jmp

IN_C                            // Input port (C)
  LDRB  r_pnt,[r_rp,#ofsC]
IN_N                            // Input port r_pnt
  TST   r_pnt,#0x80             // 8x-Fx    
  BEQ   IN_cont                 //          
  TST   r_pnt,#0x40             // 8x-Bx VDP Read
  BEQ   VDP_READ              /*************************************************************************/
  MOV   r_acc,#0xff             
  ANDS  r_pnt,#1
  LDRBEQ r_acc,[r_rp,#ofsCT]    // Cx-Fx Controller Read
  BX    lr
IN_cont                     //case 0x7E: // V COUNTER return vdp_vcounter_r();
                            //case 0x7F: // H COUNTER return vdp_hcounter_r();
  MOV   r_acc,#0xff             // Default 0xff
  AND   r_pnt,#0x41
  CMP   r_pnt,#0x40             // 4x0-7x0 VDP V
  LDRBEQ r_acc,[r_rp,#ofsVV]
  BX    lr
VDP_READ
  PUSH  {r1-r12,lr}
  MOV   r0,r_pnt
  BL    __cpp(VDP_read)
  POP   {r1-r12,pc}
/*******************************************************************************/

OTIR                            //EDB3		OTIR
OTDR                            //EDBB		OTDR
  MOV   r_jmp,lr
  UBFX  r_op,r_op,#2,#2         // 2  indr / 0 inir
  RSB   r_op,#1                 // -1 indr / 1 indr
  LDRH  r_ad,[r_rp,#ofsHL]
  REV16 r_ad,r_ad
  LDRB  r_res,[r_rp,#ofsB] 			// r_res=B
OTR_LP
  LSR   r_pnt,r_ad,#10          // r_pnt=(HL)
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_acc,[r_pnt,r_ad]
  BL    OUT_C                   // OUT(c) r_acc
  ADD   r_ad,r_op               // r_ad --/++
  SUBS  r_res,#1                // se (--r_res)==0)) end
  BEQ   OTR_END
  SUBS  r_cyc,#13
  BPL   OTR_LP
  SUB   r_pc,#2
  MOVT  r_pc,#0
  B     OTR_EXIT
OTR_END  
  SUB   r_cyc,#8
OTR_EXIT
  STRB  r_res,[r_rp,#ofsB]
  REV16 r_ad,r_ad
  STRH  r_ad,[r_rp,#ofsHL]      // aggiorna HL
  ORR   r_sr,#0x44000000        // Z=1 N=1
  BX    r_jmp
/************************************************************************************/
OUTD                            //EDAB		OUTD
OUTI                            //EDA3		OUTI
  MOV   r_jmp,lr
  LDRH  r_ad,[r_rp,#ofsHL]
  REV16 r_ad,r_ad
  LSR   r_pnt,r_ad,#10          // r_pnt=(HL)
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_acc,[r_pnt,r_ad]
  BL    OUT_C                   // OUT(c) r_acc
  UBFX  r_op,r_op,#2,#2         // 2  ind / 0 ini
  RSB   r_op,#1                 // -1 ind / 1 ind
  ADD   r_ad,r_op               // r_ad --/++
  REV16 r_ad,r_ad
  STRH  r_ad,[r_rp,#ofsHL]      // aggiorna HL
  AND   r_sr,#0xbc000000        // Z=0
  LDRB  r_res,[r_rp,#ofsB] 			
  SUBS  r_res,#1
  STRB  r_res,[r_rp,#ofsB]
  ORREQ r_sr,#0x40000000        // Z=(B==1)
  ORR   r_sr,#0x4000000         // N=1
  BX    r_jmp

OUT_A_n                         //D3 OUT  (n),A
  LSR   r_pnt,r_pc,#10          // r_pnt=n
  LDR   r_pnt,[r_mp,r_pnt,lsl#2]
  LDRB  r_pnt,[r_pnt,r_pc]
  ADD   r_pc,#1
  MOVT  r_pc,#0
  LDRB  r_acc,[r_rp,#ofsA]
  B     OUT_N
OUT_C_B                         //ED41		OUT (C),B
OUT_C_C                         //ED49		OUT (C),C
OUT_C_D                         //ED51		OUT (C),D
OUT_C_E                         //ED59		OUT (C),E
OUT_C_H                         //ED61		OUT (C),H
OUT_C_L                         //ED69		OUT (C),L
OUT_C_A                         //ED79		OUT (C),A
  UBFX  r_op,r_op,#3,#3
  LDRB  r_acc,[r_rp,r_op]
OUT_C                           // Output port (C)
  LDRB  r_pnt,[r_rp,#ofsC]
OUT_N                           // Output port r_pnt
  AND   r_pnt,#0xCf             // 8x-Fx   &1100 1111  
  TST   r_pnt,#0x80           //            1000 0000
//  BXEQ  lr
  BEQ  SONUD              //则转移
/*****************************************************************/
  PUSH  {r1-r12,lr}
  MOV   r1,r_pnt
  BL    __cpp(VDP_write)
  POP   {r1-r12,pc}
/************************************************************************/
SONUD                     //SN76496_W
  TST   r_pnt,#0x40           //7x-7F            0100 0000	
  BXEQ  lr
  PUSH  {r1-r12,lr}
  BL    __cpp(SN76496Write)//SN76496Write
  POP   {r1-r12,pc}  
}
/************************************************************************************/
/*
Z80_OUT_C
;----------------------------------------------------------------------------
	mov addy,z80bc,lsr#16
	and addy,addy,#0xFF
;----------------------------------------------------------------------------
Z80_OUT
;----------------------------------------------------------------------------
	mov r1,addy,lsl#31
	orr r1,r1,addy,lsr#6
	ldr pc,[pc,r1,ror#29]
	DCD 0
OUT_Table
	DCD MemCtrl_SMS_W
	DCD IOCtrl_SMS_W
	DCD SN76496_W
	DCD SN76496_W
VDPdata_W_ptr
	DCD VDPdata_W
	DCD VDPctrl_W
	DCD ExternalIO_W
	DCD ExternalIO_W
*/

//void __fastcall OutZ80(WORD port, BYTE data)
//{
//  switch(port & 0xFF)
//  {
//    case 0x01: // GG SIO 
//    case 0x02:
//    case 0x03:
//    case 0x04:
//    case 0x05:
//      break;

//    case 0x06: // GG STEREO 
//      sms.psg_mask = (data & 0xFF);
//      if(VGM_File) Update_VGM_Dump(0x4F, 0, data);
//      break;

//    case 0x7E: // SN76489 PSG 
//    case 0x7F:
//      if(snd.init_flag) SN76496Write(data);
//      if(VGM_File) Update_VGM_Dump(0x50, 0, data);
//      break;

//    case 0xBE: // VDP DATA 
//      vdp_data_w(data);
//      break;

//    case 0xBD: // VDP CTRL  
//    case 0xBF:
//      vdp_ctrl_w(data);
//      break;

//    case 0xF0: // YM2413 
//      if(snd.init_flag) sms.ym_cur_reg = data;
//      break;

//    case 0xF1:
//      if(snd.init_flag) OPLL_writeReg(opll, sms.ym_cur_reg, data);
//      if(VGM_File) Update_VGM_Dump(0x51, sms.ym_cur_reg, data);
//      break;

//    case 0xF2: // YM2413 DETECT 
//      if(snd.init_flag) sms.port_F2 = (data & 1);
//      break;

//    case 0x3F:
//      sms.port_3F = ((data & 0x80) | (data & 0x20) << 1) & 0xC0;
//      if(sms.country == TYPE_DOMESTIC) sms.port_3F ^= 0xC0;
//      break;

//    case 0x3E: // memory enables (not suported yet)
//      break;
//  }
//}
