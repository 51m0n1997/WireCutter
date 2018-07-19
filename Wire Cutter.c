/*------------------------------------------------------------------------------------------------------------------/
*	Arduino Controlled Wire Cutter
*	Author: Simon Robinson
*	This is the code for an arduino controlled wire cutter that makes use of a keypad, LCD, stepper motor and 
*	solenoid valve to feed and cut heavy duty steel cable.
*
*	Credit goes to Mark Stanley and Alexander Brevig for writing the keypad library as well as Marco Schwartz 
*	for writing the LiquidCrystal_I2C library that I have used.
/------------------------------------------------------------------------------------------------------------------*/

// LiquidCrystal I2C - Version: Latest 
#include <LiquidCrystal_I2C.h>

// Keypad - Version: Latest 
#include <Key.h>
#include <Keypad.h>

//keypad variables
const byte Rows = 4; //number of rows on the keypad i.e. 4
const byte Cols = 3; //number of columns on the keypad i,e, 3

char keymap[Rows][Cols]=
{
{'1', '2', '3'},
{'4', '5', '6'},
{'7', '8', '9'},
{'*', '0', '#'}
};

byte rPins[Rows]= {8,7,6,5}; //Rows 0 to 3
byte cPins[Cols]= {4,3,2}; //Columns 0 to 2

Keypad kpd = Keypad(makeKeymap(keymap), rPins, cPins, Rows, Cols);//map the keypad to be used

//LCD Map
LiquidCrystal_I2C lcd(0x27,16,2);

//Stepper motor pins
#define pinDir 11
#define pinPul 10

//Solenoid valve pin
#define pinValve 12

//USER DEFINED DEFINITIONS: Change these to suit your specific system specifications
/*------------------------------------------------------------------------------------------------------------------*/
#define feedRadius 40 //radius of stepper motor gear in mm
#define stepsPerRev 1700 //how many loops the for loop requires to complete one revolution on the stepper motor
#define valveSwitchTime 2000 //how many milliseconds the valve is held open or closed before feeding resumes
#define stepDelay 200 //microsecond delay between each step of the motor, DECREASING this value will speed up the motor
/*------------------------------------------------------------------------------------------------------------------*/

//Other mathematical definitions
#define pi 3.1415926535897932384626433832795 //delicious
#define mmPerStep (feedRadius*2*pi/stepsPerRev) // How many mm of wire one step feeds

//Other variables
int state = 0;
int wire_length;
int wire_amount;
char input_arr[5];

void setup()
{
  LCD_init();
  pinMode(pinPul,OUTPUT); 
  pinMode(pinDir,OUTPUT);
  pinMode(pinValve,OUTPUT);
  digitalWrite(pinValve, HIGH);
  Serial.begin(9600);  // initializing serial monitor
}

void loop()
{
  char keypressed;
  int steps, i = 0;
  switch(state) 
  {
    case 0: //Home Screen State---------------------------------------------------------------------------------------------
        homeScreen();
      do {//wait for input
        keypressed = kpd.getKey();
        if(keypressed == '1') 
        { 
          state++;
        }
      } while(keypressed == NO_KEY);
      break;
    case 1: //select length of wire------------------------------------------------------------------------------------------
      wirelength();
      wire_length = input_handler();
      break;
    case 2: //select wire amount ----------------------------------------------------------------------------------------------
      wireAmount();
  	  wire_amount = input_handler();
      break;
    case 3: 
      confirm();//print confirm screen
      do {
        keypressed = kpd.getKey();
        if(keypressed == '*')
        {
          state--;
        } else if(keypressed == '#') {
          state++;
        }
      } while(keypressed != '#' && keypressed != '*');
    break;
    case 4: //begin cutting----------------------------------------------------------------------------------------------
      steps = (int)(wire_length/mmPerStep);
	    lcd.clear();
      lcd.print("Cutting");
      digitalWrite(pinDir, HIGH);
	  for(i = 0; i < wire_amount; i++) {
		  cutting(i+1);
  		for(int x = 0; x < steps; x++) {//feed the wire through
  			driveMotor(stepDelay + 50);
  		}
		delay(500);
  	digitalWrite(pinValve, LOW);
		delay(valveSwitchTime);
		digitalWrite(pinValve, HIGH);
		delay(valveSwitchTime);
	  } 
	  state++;
      break;
	case 5: //done cutting ----------------------------------------------------------------------------------------------
		lcd.clear();
		lcd.print("Done! # Continue");
		do {
			keypressed = kpd.getKey();
		}
		while(keypressed != '#');
		state = 0;
	  break;
  default: //error state
    lcd.clear();
    lcd.print("ERROR");
    delay(5000);
    break;
  } 
}

void LCD_init() //set up the LCD
{
  lcd.init();                      
  lcd.backlight();
  lcd.cursor();
  lcd.blink();
}

void homeScreen() //prints the homescreen
{
  lcd.clear();
  lcd.print("Menu");
  lcd.setCursor(0,1);
  lcd.print("1.Begin");
}

void wirelength() //prints the screen to enter the desired length of wires
{
  lcd.clear();
  lcd.print("Length:  * Back");
  lcd.setCursor(6,1);
  lcd.print("mm # Enter");
  lcd.setCursor(0,1);
}

void wireAmount() //prints the screen to enter the amount of desired wires
{
  lcd.clear();
  lcd.print("Amount:  * Back");
  lcd.setCursor(9,1);
  lcd.print("# Enter");
  lcd.setCursor(0,1);
}

void confirm() //prints the confirm screen
{
  lcd.clear();
  printNum(wire_amount);
  lcd.print("x");
  printNum(wire_length);
  lcd.print("mm");
  lcd.setCursor(0,1);
  lcd.print("*Back   #Confirm");
}

void cutting(int progress)//updates the LCD while the machine is cutting
{
  lcd.setCursor(0,1);
  printNum(progress);
  lcd.print("/");
  printNum(wire_amount);
}

int input_handler() //handles input for states 1 and 2 of the system
{
  char keypressed;
  int input_counter = 0;
  
  do {//wait for input
    keypressed = kpd.getKey();
    if(keypressed != NO_KEY) 
    { 
      if(input_counter != 4) 
      {
        input_arr[input_counter] = keypressed;
        input_counter++;
        lcd.print(keypressed);
      }
      input_arr[4] = '\0';
    }
  } while(keypressed != '#' && keypressed != '*');
  if(keypressed == '#') 
  {
    sscanf(input_arr,"%d", &input_counter);
    state++;
    return input_counter;
  } else {
    state--;
    return 0;
  }
}

void printNum(int num) //prints a number from a variable onto the LCD starting at the current cursor position
{ 
  int i = 0;
  sprintf(input_arr, "%d", num);
  while(input_arr[i] != '\0') {
    lcd.print(input_arr[i]);
    i++;
  }
}

void driveMotor(int delay) { //performs 1 step of the stepper motor. 
  digitalWrite(pinPul,HIGH); 
  delayMicroseconds(delay); 
  digitalWrite(pinPul,LOW); 
  delayMicroseconds(delay);
}
