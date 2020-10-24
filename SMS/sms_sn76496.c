#include "sms_sn76496.h"  
#include "sms_main.h"  
//////////////////////////////////////////////////////////////////////////////////	 
//本程序移植自网友ye781205的NES模拟器工程
//ALIENTEK STM32F407开发板   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/10/12
//版本：V1.0  			  
////////////////////////////////////////////////////////////////////////////////// 	 

//#pragma warning(disable:4244)

#define MAX_OUTPUT  0x7FFF
#define STEP        0x10000
#define FB_WNOISE   0x12000
#define FB_PNOISE   0x08000
#define NG_PRESET   0x0F35

t_SN76496 *SN76496;
u16 * psg_buffer;		//音频数据缓存,大小为:SNBUF_size*2字节


void SN76496Write(int data)
{
	if (data & 0x80)
	{
		int r = (data & 0x70) >> 4;
		int c = r/2;

		SN76496->LastRegister = r;
		SN76496->Register[r] = (SN76496->Register[r] & 0x3f0) | (data & 0x0f);
		switch (r)
		{
			case 0:	/* tone 0 : frequency */
			case 2:	/* tone 1 : frequency */
			case 4:	/* tone 2 : frequency */
				SN76496->Period[c] = SN76496->UpdateStep * SN76496->Register[r];
				if (SN76496->Period[c] == 0) SN76496->Period[c] = SN76496->UpdateStep;
				if (r == 4)
				{
					/* update noise shift frequency */
					if ((SN76496->Register[6] & 0x03) == 0x03)
						SN76496->Period[3] = 2 * SN76496->Period[2];
				}
				break;
			case 1:	/* tone 0 : volume */
			case 3:	/* tone 1 : volume */
			case 5:	/* tone 2 : volume */
			case 7:	/* noise  : volume */
				SN76496->Volume[c] = SN76496->VolTable[data & 0x0f];
				break;
			case 6:	/* noise  : frequency, mode */
				{
					int n = SN76496->Register[6];
					SN76496->NoiseFB = (n & 4) ? FB_WNOISE : FB_PNOISE;
					n &= 3;
					/* N/512,N/1024,N/2048,Tone #3 output */
					SN76496->Period[3] = (n == 3) ? 2 * SN76496->Period[2] : (SN76496->UpdateStep << (5+n));

					/* reset noise shifter */
					SN76496->RNG_A = NG_PRESET;
					SN76496->Output[3] = SN76496->RNG_A & 1;
				}
				break;
		}
	}else
	{
		int r = SN76496->LastRegister;
		int c = r/2;

		switch (r)
		{
			case 0:	/* tone 0 : frequency */
			case 2:	/* tone 1 : frequency */
			case 4:	/* tone 2 : frequency */
				SN76496->Register[r] = (SN76496->Register[r] & 0x0f) | ((data & 0x3f) << 4);
				SN76496->Period[c] = SN76496->UpdateStep * SN76496->Register[r];
				if (SN76496->Period[c] == 0) SN76496->Period[c] = SN76496->UpdateStep;
				if (r == 4)
				{
					/* update noise shift frequency */
					if ((SN76496->Register[6] & 0x03) == 0x03)
						SN76496->Period[3] = 2 * SN76496->Period[2];
				}
				break;
		}
	}
}
void SN76496Update(short *buffer,int length, unsigned char mask)
{
	int i, j;
	int buffer_index = 0;

	/* If the volume is 0, increase the counter */
	for (i = 0;i < 4;i++)
		{
		if (SN76496->Volume[i] == 0)
		{
			/* note that I do count += length, NOT count = length + 1. You might think */
			/* it's the same since the volume is 0, but doing the latter could cause */
			/* interferencies when the program is rapidly modulating the volume. */
			if (SN76496->Count[i] <= length*STEP) SN76496->Count[i] += length*STEP;
		}
	}
	while (length > 0)
	{
		int vol[4];
		unsigned int out;
		int left; 
		/* vol[] keeps track of how long each square wave stays */
		/* in the 1 position during the sample period. */
		vol[0] = vol[1] = vol[2] = vol[3] = 0; 
		for (i = 0;i < 3;i++)
		{
			if (SN76496->Output[i]) vol[i] += SN76496->Count[i];
			SN76496->Count[i] -= STEP;
			/* Period[i] is the half period of the square wave. Here, in each */
			/* loop I add Period[i] twice, so that at the end of the loop the */
			/* square wave is in the same status (0 or 1) it was at the start. */
			/* vol[i] is also incremented by Period[i], since the wave has been 1 */
			/* exactly half of the time, regardless of the initial position. */
			/* If we exit the loop in the middle, Output[i] has to be inverted */
			/* and vol[i] incremented only if the exit status of the square */
			/* wave is 1. */
			while (SN76496->Count[i] <= 0)
			{
				SN76496->Count[i] +=SN76496->Period[i];
				if (SN76496->Count[i] > 0)
				{
					SN76496->Output[i] ^= 1;
					if (SN76496->Output[i]) vol[i] += SN76496->Period[i];
					break;
				}
				SN76496->Count[i] += SN76496->Period[i];
				vol[i] += SN76496->Period[i];
			}
			if (SN76496->Output[i]) vol[i] -= SN76496->Count[i];
		} 
		left = STEP;
		do
		{
			int nextevent;
			if (SN76496->Count[3] < left) nextevent =SN76496->Count[3];
			else nextevent = left;
			if (SN76496->Output[3]) vol[3] += SN76496->Count[3];
			SN76496->Count[3] -= nextevent;
			if (SN76496->Count[3] <= 0)
			{
				if (SN76496->RNG_A & 1) SN76496->RNG_A ^= SN76496->NoiseFB;
				SN76496->RNG_A >>= 1;
				SN76496->Output[3] = SN76496->RNG_A & 1;
				SN76496->Count[3] += SN76496->Period[3];
				if (SN76496->Output[3]) vol[3] += SN76496->Period[3];
			}
			if (SN76496->Output[3]) vol[3] -= SN76496->Count[3];
			left -= nextevent;
		} while (left > 0);
		out = 0;
		for(j = 0; j < 4; j += 1)
		{
			int k = vol[j] * SN76496->Volume[j];
			if(mask & (1 << (4+j))) out += k;
		} 
		if(out > MAX_OUTPUT * STEP) out = MAX_OUTPUT * STEP;
		buffer[buffer_index] = out / STEP; 
		/* Next sample set */
		buffer_index += 1;
		length--;
	}
}
void SN76496_set_clock(int clock)
{
	SN76496->UpdateStep = ((double)STEP * SN76496->SampleRate * 16) / clock;
}
void SN76496_set_gain(int gain)
{
	int i;
	double out;//双精度浮点型
	gain &= 0xff;
	out = MAX_OUTPUT / 3;
	while (gain-->0)out *= 1.023292992;
	for (i = 0;i < 15;i++)
	{
		if (out > MAX_OUTPUT / 3) SN76496->VolTable[i] = MAX_OUTPUT / 3;
		else SN76496->VolTable[i] = out;
        out /= 1.258925412;
	}
	SN76496->VolTable[15] = 0;
}
int SN76496_init(int clock,int volume,int sample_rate)
{
	int i;

	SN76496->SampleRate = sample_rate;
	SN76496_set_clock(clock);
	for (i = 0;i < 4;i++) SN76496->Volume[i] = 0;
	SN76496->LastRegister = 0;
	for (i = 0;i < 8;i+=2)
	{
		SN76496->Register[i] = 0;
		SN76496->Register[i + 1] = 0x0f;	/* volume = 0 */
	}
	for (i = 0;i < 4;i++)
	{
		SN76496->Output[i] = 0;
		SN76496->Period[i] = SN76496->Count[i] = SN76496->UpdateStep;
	}
	SN76496->RNG_A = NG_PRESET;
	SN76496->Output[3] = SN76496->RNG_A & 1;
	SN76496_set_gain((volume >> 8) & 0xFF);
	return 0;
} 
//////////////////////////////////////////////////////////////////////////////////////////
//对外接口函数
u8 sms_audio_init(void)
{ 	 
	//Set up SN76489 emulation
	SN76496_init(MASTER_CLOCK, 0xFF, Sound_Rate);//  60  262 
	//音频输出初始化
 	sms_sound_open(Sound_Rate);
	return 0;
} 
// update de som
void sms_update_Sound(void)
{ 
    SN76496Update((short*)psg_buffer, SNBUF_size, 0xFF); 
	sms_apu_fill_buffer(0,psg_buffer);   	
}

///////////////////////////////////////////////////////////////////////////////////////////

