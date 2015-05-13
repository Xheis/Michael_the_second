#ifndef Methods
#define Methods

//--------------------------------------------------------------------------------------------------------------------
//                              Function prototypes
//--------------------------------------------------------------------------------------------------------------------

void set_Tone(unsigned short, unsigned char);
void theta_Manager(void);
unsigned short octave_Adjust(unsigned char, unsigned char);
char	combined_Sine(void);
void PORT1_TO_PLAY_TONE();
void UpdateLEDS();
void DAC_Init();
void Timer_Init();
void Voltage_Reference_Init();
void DAC_Sine_Wave();
void Oscillator_Init(void);
void Interrupts_Init(void);

void DAC_Multi_Sine_Wave();
void Set_Volume(unsigned char);
unsigned char delay_run(unsigned char);
void delay(unsigned short);
unsigned char getState(void);
void setState(unsigned char);
void Display_Volume(void);
void reset_Timer_0(void);
void update_millis(void);
unsigned short millis(void);
void millis_RESET(void);
void Change_Volume(void);
void Change_Octave(void);
void Display_Volume(void);
unsigned char getVolume(void);
unsigned char getOctave(void);
void blink(void);
void start_Fader(unsigned char j);
void run_Fader(void);
void end_Fader(unsigned char j);

#endif