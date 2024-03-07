#include "uart.h"
#include "menu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void menu()
{
	UART_Println("\n");
	UART_Println("URUCHOMIONO PROGRAM DO DEKODOWANIA KODU MORSE'A");
	UART_Println("Do testowania programu potrzebne bedzie:");
	UART_Println("- nadajnik kodu Morse'a");
	UART_Println("Aby rozpoczac dekodowanie wpisz 'start'");
}



