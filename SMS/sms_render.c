#include "ili93xx.h"
#include "sms_vdp.h"
#include "sms_render.h"
#include <string.h>
//////////////////////////////////////////////////////////////////////////////////	 
//本程序移植自网友ye781205的NES模拟器工程
//ALIENTEK STM32F407开发板   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/10/12
//版本：V1.0  			  
////////////////////////////////////////////////////////////////////////////////// 	 

#define LSB_FIRST 0	//大端模式

//#pragma intrinsic(memcpy, memset) // fazendo a memcpy e memset inline

// master roda em 8 bits, o gens em 16, este buffer cont閙 a linha em 8 bits
const DWORD atex[4] =
{
    0x00000000,
    0x10101010,
    0x20202020,
    0x30303030,
};

// Macros to access memory 32-bits at a time (from MAME's drawgfx.c)
#define read_dword(address) *(DWORD *)address
#define write_dword(address,data) *(DWORD *)address=data

// Initialize the rendering data
void render_init(void)
{
	int bx, sx, b, s, bp, bf, sf, c;
	// Generate 64k of data for the look up table
	for(bx = 0; bx < 0x100; bx += 1)
	{
		for(sx = 0; sx < 0x100; sx += 1)
		{
			// Background pixel
			b  = (bx & 0x0F);
			// Background priority
			bp = (bx & 0x20) ? 1 : 0;
			// Full background pixel + priority + sprite marker
			bf = (bx & 0x7F);
			// Sprite pixel
			s  = (sx & 0x0F);
			// Full sprite pixel, w/ palette and marker bits added
			sf = (sx & 0x0F) | 0x10 | 0x40;
			// Overwriting a sprite pixel ?
			if(bx & 0x40)
			{
				// Return the input
				c = bf;
			}
			else
			{
				// Work out priority and transparency for both pixels
				c = bp ? b ? bf : s ? sf : bf : s ? sf : bf;
			}
			// Store result
			lut[(bx << 8) | (sx)] = c;
		}
	} 
	render_reset();
}

// Reset the rendering data
void render_reset(void)
{
	int i; 
	// Clear palette
	for(i = 0; i < SMS_PALETTE_SIZE; i += 1)
	{ 
		palette_sync(i); 
	} 
	// Invalidate pattern cache
	SMS_VDP->is_vram_dirty = 1; 
}

// Draw the Master System background
void render_bg_sms(int line)
{
	int locked = 0;
	int v_line = (line + SMS_VDP->reg[9]) % 224;
	int v_row  = (v_line & 7) << 3;
	int hscroll = ((SMS_VDP->reg[0] & 0x40) && (line < 0x10)) ? 0 : (0x100 - SMS_VDP->reg[8]);
	int column = 0;
	WORD attr;
	WORD *nt = (WORD *)&VRam[SMS_VDP->ntab + ((v_line >> 3) << 6)];
	int nt_scroll = (hscroll >> 3);
	int shift = (hscroll & 7);
	DWORD atex_mask;
	DWORD *cache_ptr;
	DWORD *linebuf_ptr = (DWORD *)&SMS_VDP->linebuf[0 - shift];

	// Draw first column (clipped)
	if(shift)
	{
		int x, c, a;
 		attr = nt[(column + nt_scroll) & 0x1F];
#ifndef LSB_FIRST
		attr = (((attr & 0xFF) << 8) | ((attr & 0xFF00) >> 8));
#endif
		a = (attr >> 7) & 0x30;
		for(x = shift; x < 8; x += 1)
		{
			c = cache[((attr & 0x7FF) << 6) | (v_row) | (x)];
			SMS_VDP->linebuf[(0 - shift) + (x)  ] = ((c) | (a));
		} 
		column += 1;
	}
	// Draw a line of the background
	for(; column < 32; column += 1)
	{
		// Stop vertical scrolling for leftmost eight columns
		if((SMS_VDP->reg[0] & 0x80) && (!locked) && (column >= 24))
		{
			locked = 1;
			v_row = (line & 7) << 3;
			nt = (WORD *)&VRam[((SMS_VDP->reg[2] << 10) & 0x3800) + ((line >> 3) << 6)];
		}
		// Get name table attribute word
		attr = nt[(column + nt_scroll) & 0x1F];
#ifndef LSB_FIRST
		attr = (((attr & 0xFF) << 8) | ((attr & 0xFF00) >> 8));
#endif
		// Expand priority and palette bits
		atex_mask = atex[(attr >> 11) & 3];
		// Point to a line of pattern data in cache
		cache_ptr = (DWORD *)&cache[((attr & 0x7FF) << 6) | (v_row)];
		// Copy the left half, adding the attribute bits in
		write_dword( &linebuf_ptr[(column << 1)] , read_dword( &cache_ptr[0] ) | (atex_mask));
		// Copy the right half, adding the attribute bits in
		write_dword( &linebuf_ptr[(column << 1) | (1)], read_dword( &cache_ptr[1] ) | (atex_mask));
	}
	// Draw last column (clipped)
	if(shift)
	{
		int x, c, a;
		unsigned char *p = &SMS_VDP->linebuf[(0 - shift)+(column << 3)];//***********************************
		attr = nt[(column + nt_scroll) & 0x1F];
#ifndef LSB_FIRST
		attr = (((attr & 0xFF) << 8) | ((attr & 0xFF00) >> 8));
#endif
		a = (attr >> 7) & 0x30;
		for(x = 0; x < shift; x += 1)
		{
			c = cache[((attr & 0x7FF) << 6) | (v_row) | (x)];
			p[x] = ((c) | (a));
		}
	}
}  
// Draw sprites
void render_obj(int line)
{
	int i;
	// Sprite count for current line (8 max.)
	int count = 0;
	// Sprite dimensions
	int width = 8;
	int height = (SMS_VDP->reg[1] & 0x02) ? 16 : 8;
	// Pointer to sprite attribute table
	BYTE *st = (BYTE *)&VRam[SMS_VDP->satb];
	// Adjust dimensions for double size sprites
	if(SMS_VDP->reg[1] & 0x01)
	{
		width *= 2;
		height *= 2;
	}
	// Draw sprites in front-to-back order
	for(i = 0; i < 64; i += 1)
	{
		// Sprite Y position
		int yp = st[i];
		// End of sprite list marker?
		if(yp == 208) return;
		// Actual Y position is +1
		yp += 1;
		// Wrap Y coordinate for sprites > 240
		if(yp > 240) yp -= 256;
		// Check if sprite falls on current line
		if((line >= yp) && (line < (yp + height)))
		{
			BYTE *linebuf_ptr;
			// Width of sprite
			int start = 0;
			int end = width;
			// Sprite X position
			int xp = st[0x80 + (i << 1)];
			// Pattern name
			int n = st[0x81 + (i << 1)];
			// Bump sprite count
			count += 1;
			// Too many sprites on this line ?
			if((SMS_VDP->limit) && (count == 9)) return;
			// X position shift
			if(SMS_VDP->reg[0] & 0x08) xp -= 8;
			// Add MSB of pattern name
			if(SMS_VDP->reg[6] & 0x04) n |= 0x0100;
			// Mask LSB for 8x16 sprites
			if(SMS_VDP->reg[1] & 0x02) n &= 0x01FE;
			// Point to offset in line buffer
			linebuf_ptr = (BYTE *)&SMS_VDP->linebuf[xp];
			// Clip sprites on left edge
			if(xp < 0) start = (0 - xp);
			// Clip sprites on right edge
			if((xp + width) > 256) end = (256 - xp);
			// Draw double size sprite
			if(SMS_VDP->reg[1] & 0x01)
			{
				int x;
				BYTE *cache_ptr = (BYTE *)&cache[(n << 6) | (((line - yp) >> 1) << 3)];
				// Draw sprite line
				for(x = start; x < end; x += 1)
				{
					// Source pixel from cache
					BYTE sp = cache_ptr[(x >> 1)];
					// Only draw opaque sprite pixels
					if(sp)
					{
						// Background pixel from line buffer
						BYTE bg = linebuf_ptr[x];
						// Look up result
						linebuf_ptr[x] = lut[(bg << 8) | (sp)];
						// Set sprite collision flag
						if(bg & 0x40) SMS_VDP->status |= 0x20;
					}
				}
			}
			else // Regular size sprite (8x8 / 8x16)
			{
				int x;
				BYTE *cache_ptr = (BYTE *)&cache[(n << 6) | ((line - yp) << 3)];
				// Draw sprite line
				for(x = start; x < end; x += 1)
				{
					// Source pixel from cache
					BYTE sp = cache_ptr[x];
					// Only draw opaque sprite pixels
					if(sp)
					{
						// Background pixel from line buffer
						BYTE bg = linebuf_ptr[x];
						// Look up result
						linebuf_ptr[x] = lut[(bg << 8) | (sp)];
						// Set sprite collision flag
						if(bg & 0x40) SMS_VDP->status |= 0x20;
					}
				}
			}
		}
	}
} 
// Update pattern cache with modified tiles
void update_cache(void)
{
	int i, x, y, c;
	int b0, b1, b2, b3;
	int i0, i1, i2, i3;
	if(!SMS_VDP->is_vram_dirty) return;
	SMS_VDP->is_vram_dirty = 0;
	for(i = 0; i < 0x200; i += 1)
	{
		if(SMS_VDP->vram_dirty[i])
		{
			SMS_VDP->vram_dirty[i] = 0;
			for(y = 0; y < 8; y += 1)
			{
				b0 = VRam[(i << 5) | (y << 2) | (0)];
				b1 = VRam[(i << 5) | (y << 2) | (1)];
				b2 = VRam[(i << 5) | (y << 2) | (2)];
				b3 = VRam[(i << 5) | (y << 2) | (3)];
				for(x = 0; x < 8; x += 1)
				{
					i0 = (b0 >> (x ^ 7)) & 1;
					i1 = (b1 >> (x ^ 7)) & 1;
					i2 = (b2 >> (x ^ 7)) & 1;
					i3 = (b3 >> (x ^ 7)) & 1;
					c = (i3 << 3 | i2 << 2 | i1 << 1 | i0);
					cache[0x00000 | (i << 6) | ((y  ) << 3) | (x)] = c;
					cache[0x08000 | (i << 6) | ((y  ) << 3) | (x ^ 7)] = c;
					cache[0x10000 | (i << 6) | ((y ^ 7) << 3) | (x)] = c;
					cache[0x18000 | (i << 6) | ((y ^ 7) << 3) | (x ^ 7)] = c;
				}
			}
		}
	}
}
// Update a palette entry
void palette_sync(int index)
{
	int r, g, b;
	r = ((SMS_VDP->CRam[index] >> 0) & 3) << 6;
	g = ((SMS_VDP->CRam[index] >> 2) & 3) << 6;
	b = ((SMS_VDP->CRam[index] >> 4) & 3) << 6;
	SMS_VDP->SMS_Palette[index] = MAKE_PIXEL(r, g, b);
}

extern u8 sms_xoff;	//显示在x轴方向的偏移量(实际显示宽度=256-2*sms_xoff)
void render_line(int line)
{
	uint16_t i;
	u16 sx,ex;
	if(line >= 192)return;  
	/* Update pattern cache更新调色板 */
	update_cache();
	/* Blank line (full width) */
	if(!(SMS_VDP->reg[1] & 0x40))
	{
		memset(SMS_VDP->linebuf, BACKDROP_COLOR, 256);
	}else
	{
		/* Draw background */
		render_bg_sms(line);
		/* Draw sprites */
		render_obj(line);
		/* Blank leftmost column of display空白的最左边的列显示 */
		if(SMS_VDP->reg[0] & 0x20)
		{
			memset(SMS_VDP->linebuf, BACKDROP_COLOR, 8);
		}
	}  
	sx=sms_xoff;
	ex=256-sms_xoff; 	
	if(lcddev.width==480)//480*800的屏幕,显示480*384大小
	{
		for(i=sx;i<ex;i++)
		{ 
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值 		
		}	
		for(i=sx;i<ex;i++)
		{ 
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
			i++;
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
 			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];//得到颜色值
		}			
	}else
	{
		for(i=sx;i<ex;i++)
		{ 
			TFTLCD->LCD_RAM=SMS_VDP->SMS_Palette[SMS_VDP->linebuf[i]&0x1F];
		}
	}
}



