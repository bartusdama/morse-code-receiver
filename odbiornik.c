#include "odbiornik.h"



int licznik_impulsow(float napiecie_niskie, float napiecie_wysokie, float wynik, int* impulsy, int impuls_roz)
{
	static int stan_wysoki = 0;
	static int stan_niski = 0;
	static int indeks = 0;
	int impuls_roz_2 = 0;
	
	if(wynik > napiecie_wysokie)
	{
				stan_wysoki +=1;
				
				if(stan_wysoki && stan_niski)
				{
					impulsy[indeks] = stan_niski;
					indeks +=1;

				}
				
				stan_niski = 0;
				
	}		
	else if(wynik < napiecie_niskie)
	{
			stan_niski +=1;
				
			if(stan_wysoki && stan_niski)
			{
					impulsy[indeks] = stan_wysoki;
					indeks +=1;

			}
				
				stan_wysoki= 0;
	}
	
	if( indeks > impuls_roz)
	{
			return -1;
	}
	

	if( (stan_niski > 10 * impulsy[indeks - 2]) && (indeks > 1) ) //drugim elementem tablicy moze byc kropka lub kreska (gdy ciag znakow zaczyna sie od t lub e)
	{																															//gdy drugim elementem tabicy jest kreska trzeba dluzej poczekac na dekodowanie bo stan niski musi wystapic az 10 razy 
				impuls_roz_2 = indeks -1;									// dluzej niz kreska
				
				indeks = 0;
				stan_niski = 0;
				stan_wysoki = 0;
				
				return impuls_roz_2;
	}
	else
	{
				return 0;
	}
			
	
}

int kodowanie_stanow(int* impulsy, int *identyfikatory, int impuls_roz_2) //
{
	volatile int i;
	int kreska;
	int kropka;
	int spacja;
	int tmp; //do okreslenia dlugosci poszczgolnych stanow
	

	//wyznaczanie ktory impuls to kreska, kropka lub spacja
	tmp = impulsy[2];
	for(i = 1 ; i <impuls_roz_2 ; i+=2)
	{
			if ((2*tmp) < impulsy[i])	//jesli w tablicy impulsow znajdzie wartosc co najmnej 2 razy wieksza(3 razy) 
			{																		//to znaczy ze tmp jest kropka
				kropka = tmp;
				break;
			}
			else if (tmp > (2*impulsy[i]))		//tutaj na odwrot 
			{
				kropka = impulsy[i];
				break;
			}
			else if(i == impuls_roz_2 -1) // jesli nie znajdze sie 2 razy wieksza lub mniejsza wartosc to znaczy
			{														// ze ciag sklada sie z samych liter 't'
				kropka = (int) (tmp / 3);
			}
	}
	kreska = 3 * kropka;
	spacja = 7 * kropka;
	
	
	int kreska_L = (int)(0.8*kreska);
	int kreska_H = (int)(1.2*kreska);
	
	int kropka_L = (int)(0.8*kropka);
	int kropka_H = (int)(1.2*kropka);
	
	int spacja_L = (int)(0.8*spacja);
	int spacja_H = (int)(1.2*spacja);
	
	for( i = 0 ; i < impuls_roz_2 ; i++ )
	{
		if( (impulsy[i+1] > kropka_L) && (impulsy[i+1] < kropka_H) ) 
		{
			identyfikatory[i] = 1;
		}
		
		else if( (impulsy[i+1] > kreska_L) && (impulsy[i+1] < kreska_H) )
		{
			identyfikatory[i] = 3;
		}

		else if( (impulsy[i+1] > spacja_L) && (impulsy[i+1] < spacja_H) )
		{
			identyfikatory[i] = 7;
		}
		
		else
		{
			identyfikatory[i] = 0;			
		}
		
	}
	
	for( i = 1; i< impuls_roz_2 ; i+=2)
	{
		identyfikatory[i] = - identyfikatory[i];
	}
	
	return 1;
	
}


int odkodowanie_na_znaki(char *zdekodowane,int *identyfikatory, int impuls_roz_2, char* znaki)
{
	int symbol_stanu = 0;
	int wartosc = 0;
	int mnoznik = 1;
	int tz_iterator = 0;
	
	for(int i=0 ; i<impuls_roz_2 ; i++)
		{

			symbol_stanu = identyfikatory[i];
			switch(symbol_stanu)
			{
				case 1:
					wartosc += (1 * mnoznik);
				
					if( i == (impuls_roz_2-1) )
					{
						zdekodowane[tz_iterator] = znaki[wartosc];
					}
					break;
				
				case 3:
					wartosc += (2 * mnoznik);
				
					if( i == (impuls_roz_2-1) )
					{
						zdekodowane[tz_iterator] = znaki[wartosc];
					}
					
					break;
				
				case -1:
					 mnoznik *= 2;
				   break;
				
				case -3:
					 zdekodowane[tz_iterator] = znaki[wartosc];
					 mnoznik = 1;
					 wartosc = 0;
					 tz_iterator ++;
					 break;
				
				case -7:
					 zdekodowane[tz_iterator] = znaki[wartosc];
					 tz_iterator ++;
					 zdekodowane[tz_iterator] = 0x20;
					 tz_iterator ++;
					 mnoznik = 1;
					 wartosc = 0;
						break;	
			}
				
		}
		
		return tz_iterator + 1;
		
}

