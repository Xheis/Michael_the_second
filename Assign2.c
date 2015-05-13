/*_____________________________________________________________________________________________________________________

        Assignment:       Elec2700
        Module:         Assignment 2
        Author:         Chris Caelli & Tim Farnworth
        Date:           20/4/15

        Description:
        This program will emulate a MIDI keyboard and produce excellent music (probably)


        Notes:          
		Basic Structure is being implemented. The code is limited to strict C.
		Current c51.exe compiler compiles the program, however gives 5 warnings of "Uncalled Functions".
		This is okay as we're implimenting stub programs.
		
        Revisions:		v0.1 Alpha

_____________________________________________________________________________________________________________________*/


//--------------------------------------------------------------------------------------------------------------------
//                              Declarations
//--------------------------------------------------------------------------------------------------------------------

#include "c8051F120.h"
#include "Assign2.h"
#include "Methods.h"
#include "LCD.h"


/* Variables */
unsigned char gameState = 0;		/* Used for switching to and from the volume and play states	*/

//unsigned long old_time = 0;



//--------------------------------------------------------------------------------------------------------------------
//                              Abstract & Menu Methods
//--------------------------------------------------------------------------------------------------------------------


/*--------------------------------------------------------------------------------------------------------------------
        Function:         Main

        Description:      Main routine

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void main(void)
{

	General_Init();
	Oscillator_Init();
	Timer_Init();
	Voltage_Reference_Init();
	DAC_Init();
	Interrupts_Init();
	TR2 = 0;



	initLCD();
	
	


	P2 = 0x0;
	
	while(1)
	{	
		Check_State_Change();																											//STILL NEEDED AS OF 5:41PM 4/5/15
		
		//Execute the current state chosen
		switch (gameState)
		{
			case 0: /* Volume Menu */
				//Clear any previous visuals
				setCursor(0, 0);
				printLCD("Volume:");
				writeIntLCD(getVolume());

				setCursor(0, 1);
				printLCD("Octave:");
				writeIntLCD(getOctave());
				

				blink();

				Change_Volume();
				Display_Volume();
				Change_Octave();
				break;
			case 1: /* Play Menu */
				
				PORT1_TO_PLAY_TONE();
				break;
			}
		}
		
		//This is a debug counter that should never, EVER execute. So, delete it when you have no issues at the end.
//		while(1)
//		{	
//			debugcounter +=1;
//		}

}




/*--------------------------------------------------------------------------------------------------------------------
        Function:         General_Init

        Description:      Initialise ports, watchdog....

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void General_Init()
{
	WDTCN = 0xde;
	WDTCN = 0xad;
  	SFRPAGE = CONFIG_PAGE;
	P2MDOUT = 0xff;		// Need to make pushpull outputs to drive LEDs properly
	P3MDOUT = 0x7F;       // Outputs
	XBR2 = 0x40;
}







/*--------------------------------------------------------------------------------------------------------------------
        Function:         Check_State_Change

        Description:      Check the MPB state, and see if user wants to change state

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void Check_State_Change()
{
	if (MPB==0) /* Negative logic */
	{
		delay(25); //Calling a delay for 25ms
		if (MPB==0)
		{
			while (MPB==0);
			cmdLCD(CLR_DISP);
			gameState = (gameState+1)%2;	/* This will take the current state, add 1, and return the remainder. i.e. toggle between 1 and 0.*/
			//this won't work without the delay pls.
			if(gameState == 1){
				setCursor(0, 0);
					printLCD("   Play Mode    ");
					
					setCursor(0, 1);
					printLCD("V:");
					writeIntLCD(getVolume());

					setCursor(7, 1);
					printLCD("O:");
					writeIntLCD(getOctave());
					}
		}
	}
}

unsigned char getState(void){
	
	return(gameState);
}

void setState(unsigned char temp_state){
	P2 = 0; /* Clear port 2 in every state change */
	gameState = temp_state;
}

void blink(void){ /* Blink LED light at 1Hz */
	static unsigned long old_time = 0;
	if((millis() - old_time)>500){
		LD1 = ~LD1;
		old_time = millis();
	}
}