#include "sms_z80a.h" 
#include "sms_vdp.h"
#include "sms_render.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序移植自网友ye781205的NES模拟器工程
//ALIENTEK STM32F407开发板   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/10/12
//版本：V1.0  			  
////////////////////////////////////////////////////////////////////////////////// 	 

// VDP context
SVDP *SMS_VDP;
u8 *VRam;		//[0x4000]
BYTE *cache;	//[0x20000];
BYTE *lut;		//[0x10000];
 
void SMS_frame(uint8_t zhen)
{
	static int32_t Cycle=0;
	int16_t RI_cnt=SMS_VDP->reg[10];
	for(SMS_VDP->line=0;SMS_VDP->line<192;SMS_VDP->line++)
	{
		//Imposta riga da ritornare per IN su Vertical Port
		SetRiga(SMS_VDP->line);
		//se Raster IRQ counter underflow
		if((--RI_cnt)<0)
		{
			// Imposta IRQ Pending e Reload Raster IRQ counter
			SMS_VDP->status |= 0x40;//SMS_VDP.pending = 1; // |=1; //  vdp->vint_pending = 1;
			RI_cnt=SMS_VDP->reg[10];//   vdp->left = vdp->reg[0x0A];
		}
		//Se Irq Pending & IRQ Raster abilitato RST38  
		if((SMS_VDP->status&0x40)&&(SMS_VDP->reg[0]&0x10))SMS_CPU_Irq();		
		//Esegue Z80 code per la durata di una riga
		Cycle=SMS_CPU_run(228+Cycle);		
		//Rendering della riga corrente 		
		if(zhen==0)render_line(SMS_VDP->line);//显示一行到LCD上面
	}	
	// Vsync IRQ
	SMS_VDP->status|=0x80;	// vdp->status |= 0x80;		
	for(SMS_VDP->line=192;SMS_VDP->line<=262;SMS_VDP->line++)
	{
		if(SMS_VDP->line<219)
			SetRiga(SMS_VDP->line);
		else
			SetRiga(SMS_VDP->line-6);
		//Gestisce IRQ  
		if(((SMS_VDP->reg[0]&0x10))||((SMS_VDP->status&0x80)&&(SMS_VDP->reg[1]&0x20)))SMS_CPU_Irq();
		//Esegue Z80 code per la durata di una riga
		Cycle=SMS_CPU_run(228+Cycle);
	}	
}

uint8_t VDP_init(void)
{    
	SMS_VDP->limit = 1;
	render_init();
	return 0;
}
 

// Write data to the VDP's control port
void vdp_ctrl_w(BYTE data)
{
	// Waiting for the reset of the command?
	if(SMS_VDP->pending == 0)
	{
		// Save data for later
		SMS_VDP->latch = data;
		// Set pending flag
		SMS_VDP->pending = 1;
	}else
	{
		// Clear pending flag
		SMS_VDP->pending = 0;
		// Extract code bits
		SMS_VDP->code = (data >> 6) & 3;
		// Make address
		SMS_VDP->addr = (data << 8 | SMS_VDP->latch) & 0x3FFF;
		// Read VRAM for code 0
		if(SMS_VDP->code == 0)
		{
			// Load buffer with current VRAM byte
			SMS_VDP->buffer = VRam[SMS_VDP->addr & 0x3FFF];
			// Bump address
			SMS_VDP->addr += 1;
		} 
		// VDP register write
		if(SMS_VDP->code == 2)
		{
			int r = (data & 0x0F);
			int d = SMS_VDP->latch;
			// Store register data
			SMS_VDP->reg[r] = d;
			// Update table addresses
			SMS_VDP->ntab = (SMS_VDP->reg[2] << 10) & 0x3800;
			SMS_VDP->satb = (SMS_VDP->reg[5] << 7) & 0x3F00;
		}
	}
} 
// Write data to the VDP's data port
void vdp_data_w(BYTE data)
{
	int index;
	// Clear the pending flag
	SMS_VDP->pending = 0;
	switch(SMS_VDP->code)
	{
		case 0: // VRAM write
		case 1: // VRAM write
		case 2: // VRAM write
			// Get current address in VRAM
			index = (SMS_VDP->addr & 0x3FFF);
			// Only update if data is new
			if(data != VRam[index])
			{
				// Store VRAM byte
				VRam[index] = data;
				// Mark patterns as dirty
				SMS_VDP->vram_dirty[(index >> 5)] = SMS_VDP->is_vram_dirty = 1;
			}
			break;
		case 3: // CRAM write 
			index = (SMS_VDP->addr & 0x1F);
			if(data != SMS_VDP->CRam[index])
			{
				SMS_VDP->CRam[index] = data;
				palette_sync(index);
			} 
			break;
	}
	// Bump the VRAM address
	SMS_VDP->addr = (SMS_VDP->addr + 1) & 0x3FFF;
} 
void VDP_write(uint8_t Value, uint16_t Port)
{
	if(Port &1)vdp_ctrl_w(Value);
	else vdp_data_w(Value);
}
uint8_t VDP_read(uint16_t Port)
{
	uint8_t temp=0;
	if (Port &1)//tmp=vdp_ctrl_r();
	{
		// Save the status flags
		temp = SMS_VDP->status;
		// Clear pending flag
		SMS_VDP->pending = 0;
		// Clear pending interrupt and sprite collision flags
		SMS_VDP->status &= ~(0x80 | 0x40 | 0x20);
		// return the old status flags
		return temp;
	}else// temp=vdp_data_r();
	{
		SMS_VDP->pending = 0;
		temp = SMS_VDP->buffer;
		SMS_VDP->buffer = VRam[SMS_VDP->addr & 0x3FFF];
		SMS_VDP->addr = (SMS_VDP->addr + 1) & 0x3FFF;
		return temp;
	} 
} 

