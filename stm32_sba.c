// PIN LATCH  --------------------------------------------------------------------------------------------------------------------------


#include "stm32f10x.h"

void LAT_A (char xb, char bul) 
{ 
	if (bul == 0) GPIOA->BRR = 1 << xb;	
	if (bul == 1) GPIOA->BSRR= 1 << xb;
}
void LAT_B (char xb, char bul)
{
	if (bul == 0) GPIOB->BRR = 1 << xb;
	if (bul == 1) GPIOB->BSRR = 1 << xb;
}
void LAT_C (char xb, char bul)
{
	if (bul == 0) GPIOC->BRR = 1 << xb;
	if (bul == 1) GPIOC->BSRR= 1 << xb;
}
void LAT_D (char xb, char bul)
{
	if (bul == 0) GPIOD->BRR = 1 << xb;
	if (bul == 1) GPIOD->BSRR= 1 << xb;
}




// PIN PULL ----------------------------------------------------------------------------------------------------------------------------

void PULL_A (char xb, char bul) 
{ 
	if (bul == 0) GPIOA->ODR &= ~(1 << xb);	
	if (bul == 1) GPIOA->ODR |= 1 << xb; 
}
void PULL_B (char xb, char bul)
{
	if (bul == 0) GPIOB->ODR &= ~(1 << xb);
	if (bul == 1) GPIOB->ODR |= 1 << xb;
}
void PULL_C (char xb, char bul)
{
	if (bul == 0) GPIOC->ODR &= ~(1 << xb);
	if (bul == 1) GPIOC->ODR |= 1 << xb;
}
void PULL_D (char xb, char bul)
{
	if (bul == 0) GPIOD->ODR &= ~(1 << xb);
	if (bul == 1) GPIOD->ODR |= 1 << xb;
}





// PIN READ ----------------------------------------------------------------------------------------------------------------------------
	
	
int PORT_A (char xb)
{
	if ((GPIOA->IDR&1<<xb) > 0) return 1; else return 0;
	}
int PORT_B (char xb)
{
	if ((GPIOB->IDR&1<<xb) > 0) return 1; else return 0;
}
int PORT_C (char xb)
{
	if ((GPIOC->IDR&1<<xb) > 0) return 1; else return 0;
}
int PORT_D (char xb)
{
	if ((GPIOD->IDR&1<<xb) > 0) return 1; else return 0;
}


// PORT CONFIG -------------------------------------------------------------------------------------------------------------------------


void CONF_A (char pin, char cnf, char mode)
{
	if (pin <= 7) 
		{
		 GPIOA->CRL &= ~(15<<(pin*4)); // clear
		 GPIOA->CRL |= ((cnf<<2)+mode)<<pin*4; // set
		}
	
	if (pin >= 8) 
		{
		 GPIOA->CRH &= ~(15<<((pin-8)*4)); // clear 
		 GPIOA->CRH |= ((cnf<<2)+mode)<<(pin-8)*4; // set
	  }
	}

		
	
void CONF_B (char pin, char cnf, char mode)
{
	if (pin <= 7) 
		{
		 GPIOB->CRL &= ~(15<<(pin*4)); // clear
		 GPIOB->CRL |= ((cnf<<2)+mode)<<pin*4; // set
		}
	
	if (pin >= 8) 
		{
		 GPIOB->CRH &= ~(15<<((pin-8)*4)); // clear 
		 GPIOB->CRH |= ((cnf<<2)+mode)<<(pin-8)*4; // set
	  }
	}

void CONF_C (char pin, char cnf, char mode)
{
	if (pin <= 7) 
		{
		 GPIOC->CRL &= ~(15<<(pin*4)); // clear
		 GPIOC->CRL |= ((cnf<<2)+mode)<<pin*4; // set
		}
	
	if (pin >= 8) 
		{
		 GPIOC->CRH &= ~(15<<((pin-8)*4)); // clear 
		 GPIOC->CRH |= ((cnf<<2)+mode)<<(pin-8)*4; // set
	  }
	}	
			
	void CONF_D (char pin, char cnf, char mode)
{
	if (pin <= 7) 
		{
		 GPIOD->CRL &= ~(15<<(pin*4)); // clear
		 GPIOD->CRL |= ((cnf<<2)+mode)<<pin*4; // set
		}
	
	if (pin >= 8) 
		{
		 GPIOD->CRH &= ~(15<<((pin-8)*4)); // clear 
		 GPIOD->CRH |= ((cnf<<2)+mode)<<(pin-8)*4; // set
	  }
	}		
			
