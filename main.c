#include "MKL05Z4.h"
#include "ADC.h"
#include "frdm_bsp.h"
#include "pit.h"
#include "uart.h"
#include "odbiornik.h"
#include "menu.h"
#include <stdio.h>
#include <string.h>


char komenda[]="start";
uint8_t obecnyStanProgramu = 0;
int start = 0;  //po pierwszym pomiarze start jest ustawiany na 1, aby wyczyścić tablice z pomiarów
//-------------------------------------------------------------------------------------------------------------------------------------------------------
//tablica zamkow, kazdy znak posiada wartosc odpowiadajaceu mu indeksowi np. e=1, t=2 
char znaki[30]={' ','e','t','i','n','a','m','s','d','r','g' ,'u' ,'k' ,'w' ,'o' ,'h' ,'b' ,'l' ,'z' ,'f' ,'c' ,'p' ,' ' ,'v' ,'x' ,'?' ,'q' ,' ' ,'y' ,'j' }; 
int impulsy[300];
int identyfikatory[300];
char zdekodowane[30];
int impuls_roz = sizeof(impulsy) / sizeof(int);
//------------------------------------------------------------------------------------------------------------------------------------------------------
	
float volt_coeff = ((float)(((float)2.91) / 4095) ) * 1000;
uint8_t wynik_ok = 0;
float	wynik = 0;
int licznik = 0;  //zlicza ilosc impulsow w czasie trawanie okresu PIT
float niskie_napiecie = 450;
float wysokie_napiecie = 600;
//---------------------------------------------------------------------------------------------------------------------------------------------------------


int impuls_roz_2 = 0; // liczba stanow w tablicy impulsow
int zdekodowane_znaki = 0;			// liczba znakow w tablicy znakow	


char rx_buf[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	// bufor do obslugi danych odbieranych przez UART
int rx_buf_len = sizeof(rx_buf);
int rx_FULL = 0;


int wyswietlenie_kom = 1;
char sprintf_buf[64];

//-------------------------------------------------------------------------------------------
void ADC0_IRQHandler()
{	
	static uint16_t temp = 0;
	temp = ADC0->R[0];			// Odczyt danej i skasowanie flagi COCO
	if(!wynik_ok)				// Sprawdź, czy wynik skonsumowany przez pętlę główną
	{
		wynik += temp;				// Wyślij nową daną do pętli głównej
		licznik ++;

	}
	NVIC_EnableIRQ(ADC0_IRQn);
}
//-----------------------------------------------------------------------------------------------
void PIT_IRQHandler() {
		
	if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {

		PIT->CHANNEL[0].TFLG &= PIT_TFLG_TIF_MASK;

		wynik_ok = 1;
		wynik = wynik / licznik;
		
	}
	NVIC_ClearPendingIRQ(PIT_IRQn);
}
//---------------------------------------------------------------------------------------------------
void UART0_IRQHandler()
{
	rx_FULL = UART_Read(rx_FULL, rx_buf, rx_buf_len);
}
//--------------------------------------------------------------------------------------------------------


int main (void)
{
	uint8_t	kal_error;
	PIT_Init();
	
	kal_error=ADC_Init();
	if(kal_error)
	{	
		while(1);
	}
	
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(12);		// Pierwsze wyzwolenie przetwornika ADC0 w kanale 12 i odblokowanie przerwania
	UART_Init();
	menu();
	
	
	while(1)
	{
		switch (obecnyStanProgramu)
		{
//-------------------------------------------------------------------------------------------------------------------------------------------------------
			case 0:																																							//etap 1 - menu
			{	
					if( rx_FULL)
					{
						if(wyswietlenie_kom == 0)
						{
							wyswietlenie_kom = 1;
						}
						
						if(strcmp (rx_buf, komenda)==0)
						{
							UART_Println("Zaczynam odczyt kodu morse");
							
							obecnyStanProgramu = 1;
							
							if(start == 1)
							{
							memset(impulsy, 0, impuls_roz*sizeof(int));
							memset(identyfikatory, 0, impuls_roz*sizeof(int)); 
							memset(zdekodowane, 0, zdekodowane_znaki*sizeof(int)); 
							
							impuls_roz_2 = 0;
							zdekodowane_znaki = 0;
							}
							
						}			
						else
						{
							UART_Println("Nie rozumiem. Wpisz komende 'start'!");
						}

						rx_FULL=0;
					}
					break;
			}
//-------------------------------------------------------------------------------------------------------------------------------------------------------	
			case 1:																																						//etap 2 - wpisywanie impulsow do tablicy
			{	
					if(wynik_ok)
					{
						if(wyswietlenie_kom)
						{
							UART_Println("Nasluchiwanie...");
							wyswietlenie_kom = 0;
						}
						
						if(start == 0)
						{
							start = 1;
						}
						
						wynik = wynik*volt_coeff; //wynik ma byc jako napiecie
						
						
						impuls_roz_2 = licznik_impulsow(niskie_napiecie, wysokie_napiecie, wynik, impulsy, impuls_roz);
						
						if(impuls_roz_2 == 0)
						{
							licznik = 0;
							wynik = 0;
							wynik_ok = 0;
						}
						
						else if (impuls_roz_2 > 0)
						{
							UART_Println("Zakonczono nasluchiwanie");
							obecnyStanProgramu = 2;
						}		
					}
					break;
			}
//-------------------------------------------------------------------------------------------------------------------------------------------------------	
			case 2:																																							//etap 3 - wpisywanie impulsow do tablicy
			{
					int pd_stan = 0;
					pd_stan = kodowanie_stanow(impulsy,identyfikatory,impuls_roz_2);
				
					if(pd_stan == 1)	// dekodowanie pierwszego stopnia zakonczone
					{
							obecnyStanProgramu = 3;
					}
					break;
			}
//-------------------------------------------------------------------------------------------------------------------------------------------------------		
			case 3:																																								//etap 4 - dekodowanie
			{
					int i = 0;
					UART_Println("Odebrany sygnal:");
					zdekodowane_znaki = odkodowanie_na_znaki(zdekodowane, identyfikatory, impuls_roz_2, znaki);
					 for(i=0; i<zdekodowane_znaki; i++)
						{
								sprintf(rx_buf, "%c",zdekodowane[i]);
								UART_Print(rx_buf); 
						} 
					obecnyStanProgramu = 0;
					break;
			}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
		}
	}
}

