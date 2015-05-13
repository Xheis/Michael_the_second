/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
---------				This is our Methods file that contains any 					---------
---------				computations we need																---------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/



//--------------------------------------------------------------------------------------------------------------------
//                              Declarations
//--------------------------------------------------------------------------------------------------------------------

#include "c8051F120.h"
#include "Assign2.h"
#include "Methods.h"
#include "LCD.h"


/*    Definitions    */
#define  	WAVE_RESOLUTION    		256   	// Our 256bit sine wave resolution
#define   	MAX_VOLUME        		16    	// 16 different volumes
#define   	MAX_FADER      			255    	// 256 different fading volumes
#define		FADING_TIME				1000
#define 	SINE_OFFSET     		128 		// DC offset for sin wave
#define 	DEFAULT_OCTAVE			4
#define 	DEFAULT_VOLUME			15			// Defualt volume. 0-15. 0=> mute
#define		NUM_NOTES						14



/* */

typedef struct {
	unsigned char on : 1;   /* 1 bit for setting and clearing */
	unsigned char fad_value;
} bitbang;



/*    Global Variables        */
unsigned short milliseconds = 0;
volatile unsigned short theta[NUM_NOTES] = {0};		
volatile unsigned short d_theta[NUM_NOTES] = {0};		//Our d_theta variable does...
unsigned char 	data 		num_active_keys = 0; /* The number of keys which are currently being pressed */
volatile unsigned char   data    volume = 	DEFAULT_VOLUME; 	/* Volume 0-15. 0=> mute, 15=> max */
unsigned char   data    octave = 	DEFAULT_OCTAVE; 	/* Set inital octave */
bitbang			fader[NUM_NOTES] = {0,0};		/* this is for reseting the fader 0=>fader has been reset. 1=> fader is currently running */




/*	Specific Tones and their frequencies		C		 D		E		 F	  G		 A		B	 	 C#	  D#   E   F#   G#   A#    B			*/
unsigned short	 code	tone[]	=	{262,	294, 330,	349, 392,	440, 494, 277, 311, 330, 370, 415, 466, 494};



/* 	Arrays		*/
const char    code    sin[] = { 
                                    /* DAC voltages for 8-bit, 16 volume sine wave */
                                    /* ------------------------------------------------------------------------ */
                                    
                                    #include "HarmonySine8.csv"    /* 256 piece sine wave */
                                };

const unsigned char    code    fader_values[] = { 
                                    
                                    #include "E:\University of Newcastle\2015 - Semster 1\ELEC2700\Assignments\Assignment 2\temp\fader.csv"    /* 256 piece decay */
                                };


const unsigned char	code delay_HB[] = {
																		#include "delay_HB.csv"
};

const unsigned char	code delay_LB[] = {
																		#include "delay_LB.csv"
};
								





//--------------------------------------------------------------------------------------------------------------------
//                              Functions & Methods
//--------------------------------------------------------------------------------------------------------------------


/*		Voltage_Reference_Init	*/
/*--------------------------------------------------------------------------------------------------------------------
        Function:         Voltage_Reference_Init

        Description:      Initialise voltage references (Needed for DAC)

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void Voltage_Reference_Init()
{
    SFRPAGE   = ADC0_PAGE;
    REF0CN    = 0x02;
}














/*		Oscillator_Init			*/
/*--------------------------------------------------------------------------------------------------------------------
        Function:         Oscillator_Init

        Description:      Initialise the system Clock at a faster rate  (Needed for DAC)

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void Oscillator_Init()
{
    int i = 0;
    SFRPAGE   = CONFIG_PAGE;
    OSCICN    = 0x83;
    CCH0CN    &= ~0x20;
    SFRPAGE   = LEGACY_PAGE;
    FLSCL     = 0xB0;
    SFRPAGE   = CONFIG_PAGE;
    CCH0CN    |= 0x20;
    PLL0CN    |= 0x01;
    PLL0DIV   = 0x01;
    PLL0FLT   = 0x01;
    PLL0MUL   = 0x04;
    for (i = 0; i < 15; i++);  // Wait 5us for initialization
    PLL0CN    |= 0x02;
    while ((PLL0CN & 0x10) == 0);
    CLKSEL    = 0x02;
   
}












/*		Timer_Init				*/
/*--------------------------------------------------------------------------------------------------------------------
        Function:         Timer_Init

        Description:      Initialise timer ports and registers

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void Timer_Init()
{
	/* Timer 1 and Timer 0 */
	SFRPAGE   = TIMER01_PAGE;
    TCON      = 0x40;
    TMOD      = 0x11;
    CKCON     = 0x02;

   // SFRPAGE   = TIMER01_PAGE;	/* Initialize Timer0 and Timer1 */
    //TCON      = 0x50;
    //TMOD      = 0x11;
    //CKCON     = 0x0A;
	
	TL1       = delay_LB[0];
    TH1       = delay_HB[0];
	
	reset_Timer_0();

	/* Timer 2 */
    SFRPAGE   = TMR2_PAGE; /* 2^14 sample rate */
    TMR2CN    = 0x04;
    TMR2CF    = 0x0A;
    RCAP2L    = 0x29;
    RCAP2H    = 0xFA;
}











/*		DAC_Init				*/
/*--------------------------------------------------------------------------------------------------------------------
        Function:         DAC_Init

        Description:      Initialise DAC0. 

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void DAC_Init()
{
	//We want this function to set up our DAC for general use. We will use the SetDAC() method for specifically readying the DAC for playing notes
	SFRPAGE   = DAC0_PAGE;
    DAC0CN    = 0x84; 
}


/*		Interrupts_Init			*/
/*--------------------------------------------------------------------------------------------------------------------
        Function:         Interrupts_Init

        Description:      Initialise interrupts

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void Interrupts_Init()
{
		IE = 0x0; 	/* Clear the register */
		EA = 1; 		/* Enable global interupts */
		ET2 = 1;		/* Enable timer2 interrupt */
		ET0 = 1;		/* Enable timer0 interrupt */

}
















/*		Timer2_ISR				*/
/*--------------------------------------------------------------------------------------------------------------------
        Function:         Timer2_ISR

        Description:      

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void Timer2_ISR (void) interrupt 5
{
    DAC_Multi_Sine_Wave();
    TF2 = 0;        // Reset Interrupt

}












/*		DAC_Sine_Wave	--- DEPRECIATED		*/
/*--------------------------------------------------------------------------------------------------------------------
        Function:         DAC_Sine_Wave

        Description:      Use to play a single note at a time

        Revisions:				DEPRECIATED. USED ONLY FOR DEBUGGING. IT SHOWS ALL THE CALCULATIONS WE USE AND WILL BE 
													LEFT AS AN EDUCATIONAL TOOL FOR FUTURE UPGRADES OR REVISIONS.

--------------------------------------------------------------------------------------------------------------------*/
	///* Run through sine wave */
	//void DAC_Sine_Wave(void){
	//		unsigned char i = (unsigned char)((theta[0]&0xFF00)>>8);
	//		DAC0H = SINE_OFFSET + volume*(sin[i])/MAX_VOLUME;        /*    Update the voltage in the DAC    */
	//    theta[0] = theta[0] + d_theta[0];    /* Due to sine wave being 8 bit, the char overflow will bring state back to 0 */
	//}













/*		Set_Volume				*/
/*--------------------------------------------------------------------------------------------------------------------
        Function:         Set_Volume

        Description:      Sets the volume of the wave

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
	void Set_Volume(unsigned char i){
    if(i>MAX_VOLUME){    /* Adjust volume if it's value is too large */
        i = MAX_VOLUME;
    }
    volume = i;
}






/*		delay				*/
/*--------------------------------------------------------------------------------------------------------------------
        Function:         delay

        Description:      Creates a delay for any application e.g. debouncing

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void delay(unsigned short delay_len){

	unsigned short oldtime = millis(); /* Store the current time */

	while((millis()-oldtime)<delay_len); /* wait for time to pass */
}	

		




/*		mirror_binary			*/
/*--------------------------------------------------------------------------------------------------------------------
        Function:         mirror_binary

        Description:      Used to mirror the byte used for the LEDs on the Perif. Board.
													i.e. LED8 becomes the MSB, whereas it is usually the LSB
        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
unsigned char mirror_binary(unsigned char num){
	char i;
	unsigned char temp = 0;
	for(i=0; i<8; i++){
		temp += (((num>>i)&0x01)<<(7-i));
	}
	return(temp);
}














/*		PORT1_TO_PLAY_TONE			*/
/*--------------------------------------------------------------------------------------------------------------------
        Function:         PORT1_TO_PLAY_TONE

        Description:      This is our main input handling function that is looped in the gameloop.
													It handles all PORT1 inputs (i.e. Notes)
        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void PORT1_TO_PLAY_TONE(void){
	unsigned char alteredPort;
	unsigned char button_i;
	unsigned char i,j;
	unsigned short tone;
	unsigned char buttons_active = 0;
	UpdateLEDS(); /* Display the notes pushed on the keyboard */
	
	for(i = 1; i<8; i++){ 	/* Check buttons 1 to 7 inclusive */
		alteredPort = ~(P1);				/* Take Complement of the port */
		button_i = (alteredPort>>i)&0x01; /* Move value to the right then increment by one then take the modulo to check whether button is active */
		/* button_i ACTIVE HIGH */
		buttons_active += button_i; /* Count the number of active notes */
		
		j = i-1; /* Normalize for pointing to an array */
		if(button_i){/* a key has been pressed */
			TR2 = 1;		/* Run timer2 if a button is set */
			run_Fader();
			
			if(~PB1){
				LD1 = 1;
				end_Fader(j);
				set_Tone(0, j); /* Turn off note... This sets the d_theta to 0. We can therefore check the d_thetas for whether a note is being played */
				theta[j] = 0;
				j = j+7; /* Adjust j so it will point to the sharp of the note */
			}
			else /* Sharp has not been pressed */
			{
				
				LD1 = 0;
				end_Fader(j+7);
				set_Tone(0, j+7); /* Turn off note... This sets the d_theta to 0. We can therefore check the d_thetas for whether a note is being played */
				theta[j+7] = 0;		/* Reset theta position */
			}

			start_Fader(j);
			tone = octave_Adjust(octave, j);
			set_Tone(tone, j);

			
		}else{ /* if button has not been pressed */
			end_Fader(j); /* End faders for notes if no button is pressed */
			end_Fader(j+7);
			
			set_Tone(0, j); /* Turn off note... This sets the d_theta to 0. We can therefore check the d_thetas for whether a note is being played */
			theta[j] = 0;		/* Reset theta position */
			//We also want to reset the sharp buttons
			set_Tone(0, j+7); /* Turn off note... This sets the d_theta to 0. We can therefore check the d_thetas for whether a note is being played */
			theta[j+7] = 0;		/* Reset theta position */
			
		}
	}
	if(buttons_active==0){
		TR2 = 0; /* If no buttons have been pressed, the turn off the timer */
	}
	num_active_keys = buttons_active; /* list the number of active buttons */
}










/*		octave_Adjust			*/
/*--------------------------------------------------------------------------------------------------------------------
        Function:         octave_Adjust

        Description:      octave_Adjust allows us to choose both an Octave and key, and will return Dtheta(i) for the
													correct value. This is used every key.
        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
unsigned short octave_Adjust(unsigned char OCT, unsigned char piano_key_select)
{		
		char move = OCT - DEFAULT_OCTAVE;
		unsigned short altered_FREQ = tone[piano_key_select];
		
		if(move>0){
			altered_FREQ = altered_FREQ<<(move); /* multiply by 2^move */
		}else if(move<0){
			move = -move; /* make move positive */
			altered_FREQ = altered_FREQ>>(move); /* divide by 2^move */
		}
		
    return(altered_FREQ);
}














/*--------------------------------------------------------------------------------------------------------------------
        Function:         combined_Sine

        Description:      Performs all wave calculations and manipulations, and will output a multi-key, multi-tone 
													sine wave.

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
char	combined_Sine(void){

	unsigned char i,j, index_fader; 

	short sine_combined = 0; /* The combined value of the notes */

	if(num_active_keys!=0){

		for(i = 0; i< NUM_NOTES; i++){

			j = (unsigned char)(((theta[i])>>8)&0xFF); /* Alter theta value for point to a value between 0x0 and 0xff */
			index_fader = fader[i].fad_value;
			sine_combined += (sin[j]);//*fader_values[index_fader])/MAX_FADER;	
							/* Add fader control here */
			theta[i] = theta[i] + d_theta[i]; /* Increment theta */


		}
		return((char)(sine_combined/num_active_keys)); /* Return the new DAC Value */
	}
	else{
		return(0);
	}
}









/*--------------------------------------------------------------------------------------------------------------------
        Function:         set_Tone

        Description:      Set tone allows us to choose both an Octave and Note from our lookup table

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void set_Tone(unsigned short frequency, unsigned char note_select)
{
    d_theta[note_select] = frequency; /* Why do we have to multiply by 8??? Should no the timer be consistent */

}








/*--------------------------------------------------------------------------------------------------------------------
        Function:         UpdateLEDs 

        Description:      Used to show in-play button presses

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void UpdateLEDS()
{
	P2 = ~P1; /* Invert Port1 and display it on P2 leds */
}








/*--------------------------------------------------------------------------------------------------------------------
        Function:         DAC_Multi_Sine_Wave

        Description:      This is our interupt method that determines the wave(s) that are generated through the DAC

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void DAC_Multi_Sine_Wave(void){
    DAC0H = SINE_OFFSET + volume*combined_Sine()/MAX_VOLUME;        /*    Update the voltage in the DAC    */
        /* Due to sine wave being 8 bit, the char overflow will bring state back to 0 */
}








/*--------------------------------------------------------------------------------------------------------------------
        Function:         Display_Volume

        Description:      This is our interupt method that determines the wave(s) that are generated through the DAC

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void Display_Volume()
{
	//Make LED 5,6,7 & 8 display the volume
	//	MSB=LED5;LSB=LED8
	char i = LD1;
	unsigned char	tempVolume = volume;
	tempVolume = mirror_binary(tempVolume);
	tempVolume = (0xF0&tempVolume);
	P2 = tempVolume;
	LD1 =i;
}

//unsigned char mirror_binary(unsigned char num){
//	char i;
//	unsigned char temp = 0;

//	for(i=0; i<8; i++){
//		temp += (((num>>i)&0x01)<<(7-i));
//	}

//	return(temp);
//}










/*--------------------------------------------------------------------------------------------------------------------
        Function:         Change_Volume

        Description:      Sets the Volume whilst in game menu

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void Change_Volume()
{
	if (~PB2)
	{
		delay(25);
		while (~PB2){
			blink();
		}
		if (volume < 15)
		{
			volume++;
		}
	}
	else if (~PB1)
	{
		delay(25);
		while (~PB1){
			blink();
		}

		
		if (volume > 0)
		{
			volume--;
		}
	}
}


unsigned char getVolume(void){
	return(volume);
}




/*--------------------------------------------------------------------------------------------------------------------
        Function:         Change_Octave

        Description:      Sets the Volume whilst in game menu

        Revisions:

--------------------------------------------------------------------------------------------------------------------*/
void Change_Octave()
{
	if (~PB4)
	{
		delay(25);
		while (~PB4){
			blink();
		}
		if (octave < 8)
		{
			octave++;
		}
	}
	else if (~PB3)
	{
		delay(25);
		while (~PB3){
			blink();
		}

		
		if (octave > 0)
		{
			octave--;
		}
	}
}





/* Interrupt for a millisecond timer */
void update_millis(void) interrupt 1
{
	milliseconds++;
	reset_Timer_0();
}

void reset_Timer_0(void){
	TF0 			= 0;  /* Clear flag */
	TL0       		= 0x06;	/* Top up for a 1 millisecond delay */
  	TH0       		= 0xF8;
	TR0 			= 1; 		/* Enable Timer */
}

unsigned short millis(){
	return(milliseconds);
}

void millis_RESET(void){
	milliseconds = 0;
}



unsigned char getOctave(void){
	return(octave);
}


void start_Fader(unsigned char j){
	
	if(fader[j].on==0){ /* If the fading instance was not already on, turn it on */
		fader[j].on==1; /* Turn on fader */
		fader[j].fad_value = 0; /* Initialize fader */
	}
}

void end_Fader(unsigned char j){ /* terminate the fader */
	fader[j].on==0;
}

void run_Fader(void){

	static unsigned short old_time = 0;
	unsigned char i;

	if((millis() - old_time)>(20)){
		old_time = millis();
		for(i = 0; i<NUM_NOTES; i++){ /* update all notes */
			fader[i].fad_value++;
			if(fader[i].fad_value>250){
				fader[i].fad_value = 250; /* Hold fader value */
			} 
		}
	}
}