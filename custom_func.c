/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>

#include "r_smc_entry.h"
#include "platform.h"

#include "misc_config.h"
#include "custom_func.h"

#include "xmodem.h"
/*_____ D E C L A R A T I O N S ____________________________________________*/

struct flag_32bit flag_PROJ_CTL;
#define FLAG_PROJ_TIMER_PERIOD_1000MS                 	(flag_PROJ_CTL.bit0)
#define FLAG_PROJ_TRIG_BTN1                       	    (flag_PROJ_CTL.bit1)
#define FLAG_PROJ_TRIG_BTN2                 	        (flag_PROJ_CTL.bit2)
#define FLAG_PROJ_REVERSE3                  		    (flag_PROJ_CTL.bit3)
#define FLAG_PROJ_REVERSE4                              (flag_PROJ_CTL.bit4)
#define FLAG_PROJ_REVERSE5                              (flag_PROJ_CTL.bit5)
#define FLAG_PROJ_REVERSE6                              (flag_PROJ_CTL.bit6)
#define FLAG_PROJ_REVERSE7                              (flag_PROJ_CTL.bit7)


#define FLAG_PROJ_TRIG_1                                (flag_PROJ_CTL.bit8)
#define FLAG_PROJ_TRIG_2                                (flag_PROJ_CTL.bit9)
#define FLAG_PROJ_TRIG_3                                (flag_PROJ_CTL.bit10)
#define FLAG_PROJ_TRIG_4                                (flag_PROJ_CTL.bit11)
#define FLAG_PROJ_TRIG_5                                (flag_PROJ_CTL.bit12)
#define FLAG_PROJ_REVERSE13                             (flag_PROJ_CTL.bit13)
#define FLAG_PROJ_REVERSE14                             (flag_PROJ_CTL.bit14)
#define FLAG_PROJ_REVERSE15                             (flag_PROJ_CTL.bit15)

/*_____ D E F I N I T I O N S ______________________________________________*/

volatile unsigned long counter_tick = 0;
volatile unsigned long btn_counter_tick = 0;

#define BTN_PRESSED_LONG                                (2500)

#define	FLASH_AREA				                        0x5000  // app code addresss
/*_____ M A C R O S ________________________________________________________*/
#define UART1_RX_POLLING


/*_____ F U N C T I O N S __________________________________________________*/

unsigned long btn_get_tick(void)
{
	return (btn_counter_tick);
}

void btn_set_tick(unsigned long t)
{
	btn_counter_tick = t;
}

void btn_tick_counter(void)
{
	btn_counter_tick++;
    if (btn_get_tick() >= 60000)
    {
        btn_set_tick(0);
    }
}

unsigned long get_tick(void)
{
	return (counter_tick);
}

void set_tick(unsigned long t)
{
	counter_tick = t;
}

void tick_counter(void)
{
	counter_tick++;
    if (get_tick() >= 60000)
    {
        set_tick(0);
    }
}

void delay_ms(unsigned long ms)
{
	#if 1
    unsigned long tickstart = get_tick();
    unsigned long wait = ms;
	unsigned long tmp = 0;
	
    while (1)
    {
		if (get_tick() > tickstart)	// tickstart = 59000 , tick_counter = 60000
		{
			tmp = get_tick() - tickstart;
		}
		else // tickstart = 59000 , tick_counter = 2048
		{
			tmp = 60000 -  tickstart + get_tick();
		}		
		
		if (tmp > wait)
			break;
    }
	
	#else
	TIMER_Delay(TIMER0, 1000*ms);
	#endif
}

void _BusyWaitDelayMs(void)
{
    volatile uint32_t i;
	unsigned long tick_count = 0;

    BSP_DI();
    
    #if 0  // wait delay , 1.0562ms
    for (i = 0; i < 2000; i++) 
    {
        __nop();
    }
    #endif

    #if 0  // timer delay , 1.007ms @ clock src 2500KHz , fCLK/2^4 , 1ms
    R_Config_TAU0_0_Start();
    while(!(TCR00 == 0));
    R_Config_TAU0_0_Stop();
    #endif

    #if 1  // timer delay , 1.007ms @ clock src 2500KHz , fCLK/2^4 , 1ms
    tick_count = 1;	

	R_Config_TAU0_0_Start();
	while ((tick_count))
	{
		if (TMIF00 == 1)
		{
			TMIF00 = 0;
			R_Config_TAU0_0_Stop();
			if (--tick_count)
			{
				R_Config_TAU0_0_Start();
			}
		}
	}
    #endif

    BSP_EI();
}


void delayMearsurement_in_polling(void)
{
    unsigned char i = 0;
    for ( i = 0; i < 20 ; i++)
    {
        P7_bit.no3 = ~P7_bit.no3;// P73
        _BusyWaitDelayMs();
    }
}

// 115200 : 1 bytes (10bit) = 0.08ms
int boot_UART1_Get_withTimeOut(unsigned long timeout, unsigned char *data)
{
	unsigned long tick_count;

	tick_count = timeout;

    #if 0
	while (SRIF1 == 0)
	{
	    _BusyWaitDelayMs();
        tick_count--;
        if (tick_count == 0)
        {
            break;
        }
	}
    SRIF1 = 0;

	if (tick_count == 0)
	{
		*data = '0';
		return FALSE;
	}

    *data = SDR11L;
    #else
	R_Config_TAU0_0_Start();
	
	/* wait for a byte to arrive */
	while ((SRIF1 == 0) && (tick_count))
	{
		if (TMIF00 == 1)
		{
			TMIF00 = 0;
			R_Config_TAU0_0_Stop();
			if (--tick_count)
			{
				R_Config_TAU0_0_Start();
			}
		}
	}
	
	if (tick_count == 0)
	{
		*data = '0';
		return FALSE;
	}
		
	*data=(uint8_t) SDR11L;
	SRIF1 = 0;

    #endif
    
	return TRUE;
}

int boot_UART1_Get(void)
{
    unsigned char c = 0;
    // BSP_DI();

    // SRIF1 = 0;
    while (SRIF1 == 0);
    SRIF1 = 0;
    c = SDR11L;

    // BSP_EI();
    return c;
}

void boot_UART1_SendByte(int c)
{
    // BSP_DI();

    #if 0
    while (STIF1 == 0);
    STIF1 = 0;
    SDR10L = (unsigned char)c;
    #else
    STMK1 = 1U;    /* disable interrupt */
    SDR10L = (unsigned char)c;
    // STIF1 = 0U;    /* clear interrupt flag */  
    while(STIF1 == 0)
    {

    }
    STIF1 = 0U;    /* clear interrupt flag */  
    #endif

    // BSP_EI();
}


void boot_UART1_sendString(unsigned char __near * str)
{
    while (*str) {
        boot_UART1_SendByte(*str++);   // Send each character in the string
    }
}

void SendByteTest_in_polling(void)
{
    boot_UART1_SendByte(0x5A);
    boot_UART1_SendByte(0xA5);
    boot_UART1_sendString((unsigned char __near *)"11223344");
}

void Timer_1ms_IRQ(void)
{
    tick_counter();

    if ((get_tick() % 1000) == 0)
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 1;
    }

    if ((get_tick() % 50) == 0)
    {

    }	

    Button_Process_long_counter();
}


/*
    F24 target board
    LED1 connected to P66, LED2 connected to P67
*/
void LED_Toggle(void)
{
    // PIN_WRITE(6,6) = ~PIN_READ(6,6);
    // PIN_WRITE(6,7) = ~PIN_READ(6,7);
    P6_bit.no6 = ~P6_bit.no6;
    P6_bit.no7 = ~P6_bit.no7;
}

void loop(void)
{
	// static unsigned long LOG1 = 0;
    signed long i32Err = 0l;

    if (FLAG_PROJ_TIMER_PERIOD_1000MS)
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 0;

        // printf("log(timer):%4d\r\n",LOG1++);
        LED_Toggle();             
    }

    // Button_Process_in_polling();
    // SendByteTest_in_polling();

    if (FLAG_PROJ_TRIG_1)
    {
        FLAG_PROJ_TRIG_1 = 0;

        printf_tiny("(boot)entry Xmodem transfer\r\n");
        i32Err = Xmodem(FLASH_AREA);

        if(i32Err < 0)
        {
            printf_tiny("(boot)Xmodem transfer fail! 0x%02X," ,i32Err>>16);
            printf_tiny("0x%04X\r\n" ,i32Err&0xFFFF);
        }
        else
        {
            printf_tiny("(boot)Xomdem transfer done!\r\n");
            printf_tiny("(boot)Total trnasfer size is 0x%02X,", i32Err>>16);
            printf_tiny("0x%04X\r\n", i32Err&0xFFFF);

            //
            // In order to verify the checksum in the application, 
            // do CHIP_RST to enter bootloader again.
            //                
            // set_TimeoutFlag(1);
            printf_tiny("(boot)XMODEM : Perform RST...\r\n");
            const unsigned char ILLEGAL_ACCESS_ON = 0x80;
            IAWCTL |= ILLEGAL_ACCESS_ON;            // switch IAWEN on (default off)
            *(__far volatile char *)0x00000 = 0x00; //write illegal address 0x00000(RESET VECTOR)

        }       
    }
}


// F24 EVB , P137/INTP0 , set both edge 
void Button_Process_long_counter(void)
{
    if (FLAG_PROJ_TRIG_BTN2)
    {
        btn_tick_counter();
    }
    else
    {
        btn_set_tick(0);
    }
}

void Button_Process_in_polling(void)
{
    static unsigned char cnt = 0;

    if (FLAG_PROJ_TRIG_BTN1)
    {
        FLAG_PROJ_TRIG_BTN1 = 0;
        printf("BTN pressed(%d)\r\n",cnt);

        if (cnt == 0)   //set both edge  , BTN pressed
        {
            FLAG_PROJ_TRIG_BTN2 = 1;
        }
        else if (cnt == 1)  //set both edge  , BTN released
        {
            FLAG_PROJ_TRIG_BTN2 = 0;
        }

        cnt = (cnt >= 1) ? (0) : (cnt+1) ;
    }

    if ((FLAG_PROJ_TRIG_BTN2 == 1) && 
        (btn_get_tick() > BTN_PRESSED_LONG))
    {         
        printf("BTN pressed LONG\r\n");
        btn_set_tick(0);
        FLAG_PROJ_TRIG_BTN2 = 0;
    }
}

// F24 EVB , P137/INTP0
void Button_Process_in_IRQ(void)    
{
    FLAG_PROJ_TRIG_BTN1 = 1;
}

void UARTx_Process(unsigned char rxbuf)
{    
    if (rxbuf > 0x7F)
    {
        printf("invalid command\r\n");
    }
    else
    {
        printf("press:%c(0x%02X)\r\n" , rxbuf,rxbuf);   // %c :  C99 libraries.
        switch(rxbuf)
        {
            case '1':
                FLAG_PROJ_TRIG_1 = 1;
                break;
            case '2':
                FLAG_PROJ_TRIG_2 = 1;
                break;
            case '3':
                FLAG_PROJ_TRIG_3 = 1;
                break;
            case '4':
                FLAG_PROJ_TRIG_4 = 1;
                break;
            case '5':
                FLAG_PROJ_TRIG_5 = 1;
                break;

            case 'X':
            case 'x':
                RL78_soft_reset(7);
                break;
            case 'Z':
            case 'z':
                RL78_soft_reset(1);
                break;
        }
    }
}

/*
    Reset Control Flag Register (RESF) 
    BIT7 : TRAP
    BIT6 : 0
    BIT5 : 0
    BIT4 : WDCLRF
    BIT3 : 0
    BIT2 : 0
    BIT1 : IAWRF
    BIT0 : LVIRF
*/
void check_reset_source(void)
{
    /*
        Internal reset request by execution of illegal instruction
        0  Internal reset request is not generated, or the RESF register is cleared. 
        1  Internal reset request is generated. 
    */
    uint8_t src = RESF;
    printf("Reset Source <0x%08X>\r\n", src);

    #if 1   //DEBUG , list reset source
    if (src & BIT0)
    {
        printf("0)voltage detector (LVD)\r\n");       
    }
    if (src & BIT1)
    {
        printf("1)illegal-memory access\r\n");       
    }
    if (src & BIT2)
    {
        printf("2)EMPTY\r\n");       
    }
    if (src & BIT3)
    {
        printf("3)EMPTY\r\n");       
    }
    if (src & BIT4)
    {
        printf("4)watchdog timer (WDT) or clock monitor\r\n");       
    }
    if (src & BIT5)
    {
        printf("5)EMPTY\r\n");       
    }
    if (src & BIT6)
    {
        printf("6)EMPTY\r\n");       
    }
    if (src & BIT7)
    {
        printf("7)execution of illegal instruction\r\n");       
    }
    #endif

}

/*
    7:Internal reset by execution of illegal instruction
    1:Internal reset by illegal-memory access
*/
//perform sofware reset
void _reset_by_illegal_instruction(void)
{
    static const unsigned char illegal_Instruction = 0xFF;
    void (*dummy) (void) = (void (*)(void))&illegal_Instruction;
    dummy();
}
void _reset_by_illegal_memory_access(void)
{
    #if 1
    const unsigned char ILLEGAL_ACCESS_ON = 0x80;
    IAWCTL |= ILLEGAL_ACCESS_ON;            // switch IAWEN on (default off)
    *(__far volatile char *)0x00000 = 0x00; //write illegal address 0x00000(RESET VECTOR)
    #else
    signed char __far* a;                   // Create a far-Pointer
    IAWCTL |= _80_CGC_ILLEGAL_ACCESS_ON;    // switch IAWEN on (default off)
    a = (signed char __far*) 0x0000;        // Point to 0x000000 (FLASH-ROM area)
    *a = 0;
    #endif
}

void RL78_soft_reset(unsigned char flag)
{
    switch(flag)
    {
        case 7: // do not use under debug mode
            _reset_by_illegal_instruction();        
            break;
        case 1:
            _reset_by_illegal_memory_access();
            break;
    }
}

// retarget printf
int __far putchar(int c)
{
    // F24 , UART0
    STMK0 = 1U;    /* disable INTST0 interrupt */
    SDR00L = (unsigned char)c;
    while(STIF0 == 0)
    {

    }
    STIF0 = 0U;    /* clear INTST0 interrupt flag */
    return c;
}

void hardware_init(void)
{
    // const unsigned char indicator[] = "hardware_init";
    BSP_EI();
    R_Config_UART0_Start();         // UART0 , P15/TX , P16/RX
    R_Config_TAU0_1_Start();        // TIMER
    R_Config_INTC_INTP0_Start();    // BUTTON , P137 
    
    R_Config_UART1_Start();         // UART1 , P74/TX , P75/RX

    delayMearsurement_in_polling();
    STMK1 = 1U;    /* disable INTST1 interrupt */
    SRMK1 = 1U;    /* disable INTSR1 interrupt */

    // check_reset_source();
    printf("%s finish\r\n\r\n",__func__);
}
