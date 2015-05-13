// LCD.c - This .c file contains the declarations 
// for the LCD functions 
#include "c8051F120.h"
#include "Assign2.h"
#include "Methods.h"
#include "LCD.h"


void initLCD(){
	/* Power On */
	delay(30);

	P3 = FUNCTION_SET;
	pulse_Enable(5);

	cmdLCD(TWO_LINE_CONTROL);
	
	/* Display ON/OFF */
	cmdLCD(CONTROL);
	
	/* Entry Mode Set */
	cmdLCD(ENTRY_MODE_SET);
	
	cmdLCD(CLR_DISP);
	printLCD("    ELEC2700    ");
	delay(500);
	cmdLCD(CLR_DISP);

	MPB = 1;
	
	/* Now we can write data! YAH */
	
}


void printLCD(unsigned char *word){
	int fill = 0;
	int i = 0;
	while(*word){ /* print character by character till we reach the NULL character */
		fill++;
		writeLCD(*word++); /* increment pointer to the next character in the word */
		
	}
	

}

void writeLCD(unsigned char character){ /* Write a character to the LCD screen */
	/* Send first Nibble */
	P3 = 0;
	RS = 1;
	P3 = P3|(character>>4)&(0x0F); /* Upper nibble of character */
	pulse_Enable(1);
	
	/* Send last Nibble */
	P3 = 0;
	RS = 1;
	P3 = P3|(character)&(0x0F); 		/* Lower nibble of character */
	pulse_Enable(1);
	MPB = 1;
	RS = 0;
}


void writeIntLCD(unsigned char num){	
	static unsigned char temp_123 = 0;

	if(num>=100){ /* Highest decimal is zero */
		writeLCD((num/100) + 48); /* Highest decimal value */
		writeLCD((num/10)%10 + 48); /* Middle decimal value */
		writeLCD((num%10) + 48); /* Lowest decimal value */
		
	}
	else if(num>=10){
		writeLCD(' ');
		writeLCD((num/10)%10 + 48); /* Middle decimal value */
		writeLCD((num%10) + 48); /* Lowest decimal value */
	}
	else if(num<10){
		writeLCD(' ');
		writeLCD(' ');
		writeLCD((num%10) + 48); /* Lowest decimal value */
	}
	
}








void setCursor(unsigned char x, unsigned char y){
	unsigned char i;
	if(y==0){
	cmdLCD(START_LINE1);
	}
	else{
	cmdLCD(START_LINE2);
	}
	for(i = 0; i<x; i++){
		cmdLCD(MOVE_RIGHT);
	}
	

}

void pulse_Enable(unsigned char i){
	ENABLE = 1;
	delay(i);
	ENABLE = 0;
	delay(i);
}

void cmdLCD(unsigned char cmd){
		/* Send first Nibble */
	P3 = 0;
	P3 = P3|(cmd>>4)&(0x0F); /* Upper nibble of character */
	pulse_Enable(3);
	
	/* Send last Nibble */
	P3 = 0;
	P3 = P3|(cmd)&(0x0F); 		/* Lower nibble of character */
	pulse_Enable(3);
	MPB = 1;
}


