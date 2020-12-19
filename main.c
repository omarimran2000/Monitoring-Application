#include "msp.h"
#include <stdio.h>

#define BOTH_LED_OFF 0 
#define RED_LED_ON 1
#define RED_RGB_1_ON 2
#define RED_RGB_2_ON 3
#define DEBOUNCE_TIME 2000

void intialize_board(void);
void initialize_EUSCI(void);
void PORT1_IRQHandler(void);
void EUSCIA0_IRQHandler(void);
void increment(void);
void decrement(void);
void change_LED(void);
void UART0_OutString(char arr[]);
void UART0_OutChar(char c);

static int state = 0;

/*
   Function to initialize buttons,LED
*/
void intialize_board(void)
{
	//setting up button inputs and outputs 
	P1->SEL0 &=  (uint8_t)(~(((1 << 1) | (1 << 4)) | (1 << 0)));
	P1->SEL1 &=  (uint8_t)(~(((1 << 1) | (1 << 4)) | (1 << 0)));
	
	P2->SEL0 &= (uint8_t)(~((1 << 0) | (1 << 1)| (1<<2)));
	P2->SEL1 &= (uint8_t)(~((1 << 0) | (1 << 1)| (1<<2)));
	
	P1->REN |= (uint8_t)((((1 << 1) | (1 << 4))));
	P1->OUT |= (uint8_t)((((1 << 1) | (1 << 4))));
	P1->DIR &= (uint8_t)(~(((1 << 1) | (1 << 4))));
	
	P1->OUT &= (~(uint8_t)((1 << 0)));
	P1->DS  |= (uint8_t)((1 << 0));
	P1->DIR  |= (uint8_t)((1 << 0));
	
	P2->OUT &= (uint8_t)(~((1 << 0) | (1 << 1)| (1<<2)));
	P2->DS |= (uint8_t)((1 << 0) | (1 << 1)| (1<<2));
	P2->DIR |= (uint8_t)((1 << 0) | (1 << 1)| (1<<2));
	
	P1->IES |= (uint8_t)((((1 << 1) | (1 << 4))));
	P1->IFG &= (uint8_t)(~(((1 << 1) | (1 << 4))));
	P1->IE |=  (uint8_t)((((1 << 1) | (1 << 4))));
	
	//adding interupts to NVIC 
	NVIC_SetPriority(PORT1_IRQn, 2);
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);
	
	__ASM("CPSIE I"); 
}
/*
	Function to initalize the EUSCI 3
*/
void initialize_EUSCI(void)
{
	//UART configs based off example from CU Learn 
	
  CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
  CS->CTL0 = 0;                           // Reset tuning parameters
  CS->CTL0 = CS_CTL0_DCORSEL_3;           // Set DCO to 12MHz (nominal, center of 8-16MHz range)
  CS->CTL1 = CS_CTL1_SELA_2 |             // Select ACLK = REFO
            CS_CTL1_SELS_3 |                // SMCLK = DCO
            CS_CTL1_SELM_3;                 // MCLK = DCO
  CS->KEY = 0;                            // Lock CS module from unintended accesses

  // Configure UART pins
  P1->SEL0 |= BIT2 | BIT3;                // set 2-UART pin as secondary function

  // Configure UART
  EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
  EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | // Remain eUSCI in reset
            EUSCI_B_CTLW0_SSEL__SMCLK;      // Configure eUSCI clock source for SMCLK

  EUSCI_A0->BRW = 78;                     // Set Braud Rate 
	EUSCI_A0->MCTLW = (2 << EUSCI_A_MCTLW_BRF_OFS) |
            EUSCI_A_MCTLW_OS16;

	EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Initialize eUSCI
  EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;    // Clear eUSCI RX interrupt flag
  EUSCI_A0->IE |= EUSCI_A_IE_RXIE;        // Enable USCI_A0 RX interrupt	
	
	NVIC_SetPriority(EUSCIA0_IRQn, 2);
	NVIC_ClearPendingIRQ(EUSCIA0_IRQn);
	NVIC_EnableIRQ(EUSCIA0_IRQn);
	
}
/*
	Port 1 interupt 
*/
void PORT1_IRQHandler(void)
{
	if ((P1->IFG & (uint8_t)(1<<1))!=0)  //if up button is pressed 
	{
	 	int i = DEBOUNCE_TIME;
		while(i>0){i--;}
		
		P1->IFG &= ~(1<<1);
		
		increment();
		
	}
	else if ((P1->IFG & (uint8_t)(1<<4))!=0)  //if down button is pressed 
	{
		int i = DEBOUNCE_TIME;
		while(i>0){i--;}

		P1->IFG &= ~(1<<4);
		
		decrement();
	}
}
/*
	Interupt for when a key is pressed
*/
void EUSCIA0_IRQHandler(void)
{
	if(EUSCI_A0->RXBUF == 'n' || EUSCI_A0->RXBUF == 'N')
	{
		increment();
	}
	if(EUSCI_A0->RXBUF == 'p' || EUSCI_A0->RXBUF == 'P')
	{
		decrement();
	}
}
/*
	Function for if states need to be transitioned up 
*/
void increment()
{
		state++;
		state = state%4;
	  change_LED();
}
/*
	Function for if states need to be transitioned down 
*/
void decrement()
{
		if(state==0)
		{
			state=3;
		}
		else
		{
			state--;
		}
		change_LED();
}
/*
	Function for changing the LED when there has been a state change
*/
void change_LED()
{
	if(state==BOTH_LED_OFF)
	{
		UART0_OutString("STATE 1 ");
		P1->OUT &= (~(uint8_t)((1 << 0)));
		P2->OUT &= (uint8_t)(~((1 << 0) | (1 << 1)| (1<<2)));
	}
	else if(state == RED_LED_ON)
	{
		UART0_OutString("STATE 2 ");
		P1->OUT |= ((uint8_t)((1 << 0)));
		P2->OUT &= (uint8_t)(~((1 << 0) | (1 << 1)| (1<<2)));
	}
	else if(state==RED_RGB_1_ON)
	{
		UART0_OutString("STATE 3 ");
		P1->OUT |= ((uint8_t)((1 << 0)));
		P2->OUT &= (uint8_t)(~((1<<2)));
		P2->OUT |= (uint8_t)(((1 << 0) | (1 << 1)));
		
	}
	else if(state==RED_RGB_2_ON)
	{
		UART0_OutString("STATE 4  ");
		P1->OUT |= ((uint8_t)((1 << 0)));
		P2->OUT |= (uint8_t)(((1 << 0) | (1 << 1)| (1<<2)));
	}
}

/*
	Function to print out a string to the terminal  
*/
void UART0_OutString(char arr[])
{
	char *ptr;
	ptr = arr;

	while(*ptr!='\0')
	{
		while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));
	  UART0_OutChar(*ptr);
		ptr++;
	}
}
/*
	Function to print out one char to the terminal 
*/
void UART0_OutChar(char c)
{
	EUSCI_A0->TXBUF = c;
}
/*
	Main script 
*/
int main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; 
	intialize_board();
	initialize_EUSCI();
	while(1)
	{
		
	}
	return 0;
	
}

