
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);

/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;
	p_tty->p_console->matchState      =    0;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->original_addr      =   disp_pos / 2;
		p_tty->p_console->cursor = p_tty->p_console->original_addr ;
		p_tty->p_console->current_start_addr =p_tty->p_console->original_addr ;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}

/*======================================================================*
			match
*======================================================================*/
PRIVATE void match(CONSOLE* p_con){
	u8* p_start = (u8*)(V_MEM_BASE + p_con->original_addr * 2);
	u8* p_match = (u8*)(V_MEM_BASE + p_con->match_start_cur * 2);
	int length  =  p_con->cursor - p_con->match_start_cur;
	int i,j;
	for( i=0;i<(p_con->match_start_cur -  p_con->original_addr  );i++){
		if(*(p_start+2*i+1) ==GREEN){
			*(p_start+2*i+1)  =  DEFAULT_CHAR_COLOR;
		}
		
	}
	for( i=0;i<=(p_con->match_start_cur -  p_con->original_addr  - length);i++){
		
		if(isMatch(  (p_start+2*i)  ,  p_match  , length)  ==  1 ){
			for( j=i;j<i+length;j++){
				*(p_start+2*j+1) = GREEN;
			}
		}
	}
}

int isMatch(char* str ,char * tar,int length){
	int i,result=1;
	for( i=0 ; i<length ; i++ ){
		if( *(str + i*2) != *(tar + i*2) ){
			result=0;
			break;
		}
	}
	return result;
}

/*======================================================================*
			   clear_console
 *======================================================================*/
PUBLIC void clear_console(CONSOLE* p_con){
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
	int i;

	for(i=1;p_con->cursor > p_con->original_addr ;i++){
		
		p_con->cursor--;
		*(p_vmem-2*i) = ' ';
		*(p_vmem-2*i+1) = DEFAULT_CHAR_COLOR;
		
	}

	set_cursor(p_con->cursor);

} 


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
	u8* p_start = (u8*)(V_MEM_BASE + p_con->original_addr * 2);
	int i,enterEnd;
	char  enterChar='\0';
	char  tabChar='\0';

  	if(0   ==  p_con->matchState){
	switch(ch) {

	case '\a':
		p_con->matchState=1;
		p_con->match_start_cur=p_con->cursor ;
		
		disable_irq(CLOCK_IRQ);
		break;	
	case '\n':
		if (p_con->cursor < p_con->original_addr +
		    p_con->v_mem_limit - SCREEN_WIDTH) {

			enterEnd= p_con->original_addr + SCREEN_WIDTH * 
				((p_con->cursor - p_con->original_addr) /
				 SCREEN_WIDTH + 1);

			for(i=0 ;i<enterEnd-p_con->cursor ;i++){
				*(p_vmem+i*2) = enterChar;
				*(p_vmem+i*2+1) = BLUE;
			}
			p_con->cursor =enterEnd;
		}
		break;
	case '\t':
		if (p_con->cursor < p_con->original_addr +
		    p_con->v_mem_limit ) {
			int step=4;
			int isEnd=1;
			if((p_con->cursor - p_con->original_addr) /SCREEN_WIDTH>0){
				step=0;
				for(i=0;i<(SCREEN_WIDTH - p_con->cursor%SCREEN_WIDTH);i++){
					if(*(p_vmem-SCREEN_WIDTH*2+i*2) ==' '||(*(p_vmem-SCREEN_WIDTH*2+i*2) ==tabChar && *(p_vmem-SCREEN_WIDTH*2+i*2+1) ==RED)){
						step++;
					}else{
						if(*(p_vmem-SCREEN_WIDTH*2+i*2) !=enterChar || *(p_vmem-SCREEN_WIDTH*2+i*2+1) !=BLUE){
							isEnd=0;
						}

						break;
					}
				}

				if(*(p_vmem-SCREEN_WIDTH*2) !=' '&&(*(p_vmem-SCREEN_WIDTH*2) !=tabChar||*(p_vmem-SCREEN_WIDTH*2+1) !=RED)
					&&(*(p_vmem-SCREEN_WIDTH*2) !=enterChar||*(p_vmem-SCREEN_WIDTH*2+1) !=BLUE)){
					step=4;
				}
			}
			if(isEnd==1){
				step=4;
			}

			for(i=0 ;i<step;i++){
				*(p_vmem+i*2) = tabChar;
				*(p_vmem+i*2+1) = RED;
				p_con->cursor ++;
			}
			
		}
		break;
	case '\b':
		if (p_con->cursor > p_con->original_addr) {
			
			if(enterChar==*(p_vmem-2) && BLUE==*(p_vmem-1) ){
				for(i=1;i<=SCREEN_WIDTH;i++){
				         if(enterChar==*(p_vmem-2*i) && BLUE==*(p_vmem-i*2+1) ){
					p_con->cursor--;
					*(p_vmem-2*i) = ' ';
					*(p_vmem-2*i+1) = DEFAULT_CHAR_COLOR;
				        }else{
				        	break;
				        }
				}

			}else if(tabChar==*(p_vmem-2) && RED==*(p_vmem-1) ){
				for(i=1;i<=SCREEN_WIDTH;i++){
				         if(tabChar==*(p_vmem-2*i) && RED==*(p_vmem-i*2+1) ){
					p_con->cursor--;
					*(p_vmem-2*i) = ' ';
					*(p_vmem-2*i+1) = DEFAULT_CHAR_COLOR;
				        }else{
				        	break;
				        }
				}
			}else{
				p_con->cursor--;
				*(p_vmem-2) = ' ';
				*(p_vmem-1) = DEFAULT_CHAR_COLOR;
			}
			
		}
		break;
	default:
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;
			*p_vmem++ = DEFAULT_CHAR_COLOR;
			p_con->cursor++;
		}
		break;
	}
	}else{
		switch(ch) {
		case  '\a':
			p_con->matchState=0;
			for( i=0;i<(p_con->match_start_cur -  p_con->original_addr  );i++){
				if(*(p_start+2*i+1) ==GREEN){
					*(p_start+2*i+1)  =  DEFAULT_CHAR_COLOR;
				}
		
			}
			for(i=1;p_con->cursor > p_con->match_start_cur;i++){
				p_con->cursor--;
				*(p_vmem-2*i) = ' ';
				*(p_vmem-2*i+1) = DEFAULT_CHAR_COLOR;
				
			}
			ticks = 0;
			enable_irq(CLOCK_IRQ);
			break;
		case  '\n':
			match(p_con);
			break;
		case  '\b':
			if (p_con->cursor > p_con->match_start_cur) {
				p_con->cursor--;
				*(p_vmem-2) = ' ';
				*(p_vmem-1) = DEFAULT_CHAR_COLOR;
			}
			break;
		default:
			if (p_con->cursor <p_con->original_addr + p_con->v_mem_limit - 1) {
				*p_vmem++ = ch;
				*p_vmem++ = GREEN;
				p_con->cursor++;
			}
			break;
		}

	}
	

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

