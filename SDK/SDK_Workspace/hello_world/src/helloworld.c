/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 *
 *
 *
 */


#include <stdio.h>
#include <unistd.h>
#include "platform.h"
#include "xintc.h"
#include "xparameters.h"
#include "xio.h"
#include "vga_periph_mem.h"
#include "xil_exception.h"

XIntc Intc;
Xint32 offset = 0;
int interrupt_dir = 1;

unsigned char string_s[] = "LPRS 2\n";

void print(char *str);

void my_timer_interrupt_handler(void * baseaddr_p) {
	clear_text_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR+offset);
	if(offset >= 118 || offset <= -20)
	{
		interrupt_dir = (interrupt_dir == 1) ? -1 : 1; // promena smera.
	}
	print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR+offset, string_s, 6);
	offset += (Xint32)interrupt_dir;
}

void squareLeftRight()
{
	 int i, direction;
	 i = 0;
	 direction = 1;
	 while(1)
	 {
		clear_graphics_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);
		//if(condition(address, i, XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR))
		if(i >= 33 || i <= -33)
		{
			direction = (direction == 1) ? -1 : 1; // promena smera.
		}
	 	move_square(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, i);
	 	i += direction;
	 	delay(500000);
	 }
}

int main()
{
    init_platform();
    XStatus Status;

    VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x00, 0x0);// direct mode   0
    VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x04, 0x3);// display_mode  1
    VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x08, 0x1);// show frame      2
    VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x0C, 0x1);// font size       3
    VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0xFFFFFF);// foreground 4
    VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0x0000FF);// background color 5
    VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x18, 0xFF0000);// frame color      6
    VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x1C, 480);// v_sync_counter_tc 7
    VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x20, 0x1);// enable interrupt 8
    print("Hello World\n\r");

    clear_text_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);
    clear_graphics_screen(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);
    //draw_square(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR);
    set_cursor(500);
    //print_string(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR, string_s, 6);

	//move_text(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR,1,string_s,6,6);

	//squareLeftRight();

	//initialize interrupt controller
	Status = XIntc_Initialize (&Intc, XPAR_INTC_0_DEVICE_ID);
	if (Status != XST_SUCCESS)
		xil_printf ("\r\nInterrupt controller initialization failure");
	else
		xil_printf("\r\nInterrupt controller initialized");

	// Connect my_timer_interrupt_handler
	Status = XIntc_Connect (&Intc, XPAR_INTC_0_DEVICE_ID, (XInterruptHandler) my_timer_interrupt_handler,(void *)0);

	if (Status != XST_SUCCESS)
		xil_printf ("\r\nRegistering MY_TIMER Interrupt Failed");
	else
		xil_printf("\r\nMY_TIMER Interrupt registered");

	//start the interrupt controller in real mode
	Status = XIntc_Start(&Intc, XIN_REAL_MODE);
	//enable interrupt controller
	XIntc_Enable (&Intc, XPAR_INTC_0_DEVICE_ID);

	microblaze_enable_interrupts();

	while(1){
		;
	}

	cleanup_platform();
    return 0;
}
