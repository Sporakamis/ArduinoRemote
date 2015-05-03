/* omote robotic controller 
   written by Oscar Liang
   blog.oscarliang.net
   
   Host - Remote controller side
*/


//=========================================================
//===================== Library =========================
//=========================================================

#include <LiquidCrystal.h>


//=========================================================
//===================== Pre Defined =======================
//=========================================================

const int LED_transmit_on_time = 100; // in millis
const int ConnectionTimeOut = 3000; // in millis
const byte Analog_Control_Init_Value = 512;

// --------- Mneu ------------

#define Num_Menu_Pages 3
byte Cur_Menu_Page = 0; // the current page number (used for different button functions for different pages)									
/*
Page Num   | Left Button | Right Button 
0 - Main   |             | Menu
1 - Menu   | Select      | Next

*/

byte Cur_Menu_Item = 0; // the current option
#define Num_Menu_Items 5
// List of menu page display text
String Menu_Items[Num_Menu_Items] = {"Exit",
									"Prot(full-short",
									"prot(full-long)",
									"prot(sel-short)",
									"prot(sel-long)"
									};
	
// --------- Protocol Type ---------
byte Cur_Protocol_Type = 0;


//=========================================================
//===================== Constants =========================
//=========================================================

const byte lcd_pin[7] = {22, 24, 26, 28, 30, 32, 34};
const byte led_pin[3] = {2, 3, 4};
const byte toggle_pin[4]  = {12, 11, 23, 25};
const byte button_pin[6] = {A8, A9, A10, A11, A12, A13};
const byte joystick_pin[4] = {A2, A3, A4, A5};
const byte joystick_button_pin[2] = {36, 38};
const byte pometer_pin[4] = {A0, A1, A14, A15};


//=========================================================
//===================== Global Variables =========================
//=========================================================

bool Connected = false;
bool Running = true; // false when controller is paused by user, transmission would stop.

int DisConTimer = 0; // disconnected timer
int LastConTime = 0; // the time of last communication

int pot1 = Analog_Control_Init_Value;
int pot2 = Analog_Control_Init_Value;
int pot3 = Analog_Control_Init_Value;
int pot4 = Analog_Control_Init_Value;

int js1_x = Analog_Control_Init_Value;
int js1_y = Analog_Control_Init_Value;
int js2_x = Analog_Control_Init_Value;
int js2_y = Analog_Control_Init_Value;
//byte jssw1 = digitalRead(36);
//byte jssw2 = digitalRead(38);

bool toggle1 = false;
bool toggle2 = false;
bool toggle3 = false;
bool toggle4 = false;

bool button1 = false;
bool button2 = false;

bool button3 = false;
bool button4 = false;
bool button5 = false;
bool button6 = false;

byte buttonByte = 0;

// ----------- Control Input value buffer --------
// Used to smooth out input value, and test button realease...
#define Buffer_Size 3
byte Cur_Buffer_Index = 0;


int p_pot1[Buffer_Size];
int p_pot2[Buffer_Size];
int p_pot3[Buffer_Size];
int p_pot4[Buffer_Size];

int p_js1_x[Buffer_Size];
int p_js1_y[Buffer_Size];
int p_js2_x[Buffer_Size];
int p_js2_y[Buffer_Size];


bool p_toggle1 = false;
bool p_toggle2 = false; 
bool p_toggle3 = false;
bool p_toggle4 = false;

bool p_button1 = false; // to test if these buttons were released
bool p_button2 = false; // to test if these buttons were released

bool p_button3 = false; // to test if these buttons were released
bool p_button4 = false; // to test if these buttons were released
bool p_button5 = false; // to test if these buttons were released
bool p_button6 = false; // to test if these buttons were released


LiquidCrystal lcd(lcd_pin[0], lcd_pin[1], lcd_pin[2], lcd_pin[3], lcd_pin[4], lcd_pin[5], lcd_pin[6]);

//=========================================================

/*
xrf

18 RX1 
19 TX1
Serial1
*/

//=========================================================


void TakeInputValues(){

	p_pot1[Cur_Buffer_Index] = analogRead(pometer_pin[0]);
	p_pot2[Cur_Buffer_Index] = analogRead(pometer_pin[1]);
	p_pot3[Cur_Buffer_Index] = analogRead(pometer_pin[2]);
	p_pot4[Cur_Buffer_Index] = analogRead(pometer_pin[3]);
	
	p_js1_x[Cur_Buffer_Index] = analogRead(joystick_pin[0]);
	p_js1_y[Cur_Buffer_Index] = analogRead(joystick_pin[1]);
	// because of the physical orientation of the joystick, 
	// we need to invert the direction of the values
	p_js2_x[Cur_Buffer_Index] = 1023 - analogRead(joystick_pin[2]);
	p_js2_y[Cur_Buffer_Index] = 1023 - analogRead(joystick_pin[3]);
	//byte jssw1 = digitalRead(36);
	//byte jssw2 = digitalRead(38);
	
	
	if(++Cur_Buffer_Index >= Buffer_Size){
		Cur_Buffer_Index = 0;
	}
	
	
	int sum_pot1 = 0;
	int sum_pot2 = 0;
	int sum_pot3 = 0;
	int sum_pot4 = 0;
	
	int sum_js1_x = 0;
	int sum_js1_y = 0;
	int sum_js2_x = 0;
	int sum_js2_y = 0;


	for (int i=0; i<Buffer_Size; i++){
	
		sum_pot1 += p_pot1[i];
		sum_pot2 += p_pot2[i];
		sum_pot3 += p_pot3[i];
		sum_pot4 += p_pot4[i];

		sum_js1_x += p_js1_x[i];
		sum_js1_y += p_js1_y[i];
		sum_js2_x += p_js2_x[i];
		sum_js2_y += p_js2_y[i];
	
	}
	
	pot1 = sum_pot1 / Buffer_Size;
	pot2 = sum_pot2 / Buffer_Size;
	pot3 = sum_pot3 / Buffer_Size;
	pot4 = sum_pot4 / Buffer_Size;

	js1_x = sum_js1_x / Buffer_Size;
	js1_y = sum_js1_y / Buffer_Size;
	js2_x = sum_js2_x / Buffer_Size;
	js2_y = sum_js2_y / Buffer_Size;
	
	
	// ---- button and toggles -------
	
	toggle1 = digitalRead(toggle_pin[0]);
	toggle2 = digitalRead(toggle_pin[1]);
	toggle3 = digitalRead(toggle_pin[2]);
	toggle4 = digitalRead(toggle_pin[3]);
	
	button3 = digitalRead(button_pin[2]);
	button4 = digitalRead(button_pin[3]);
	button5 = digitalRead(button_pin[4]);
	button6 = digitalRead(button_pin[5]);
	
	buttonByte = EncodeButton(toggle1, toggle2, toggle3, toggle4, button3, button4, button5, button6);
	

}

// convert 4 toggle and 4 buttons state into binary, and then convert binary to byte number for transmission
// expect bit1 - bit8 are zeros or ones
byte EncodeButton(bool bit1, bool bit2, bool bit3, bool bit4, bool bit5, bool bit6, bool bit7, bool bit8){

	byte sum = 0;
	sum += bit1 << 7;
	sum += bit2 << 6;
	sum += bit3 << 5;
	sum += bit4 << 4;
	sum += bit5 << 3;
	sum += bit6 << 2;
	sum += bit7 << 1;
	sum += bit8 << 0;
	
	return sum;
	
}



void setup(){

	// Init LCD Display
	lcd.begin(16, 2);
	lcd.setCursor(0,0);
    lcd.print("Initializing...");

	// start serial port at 9600 bps:
	Serial.begin(9600);
	Serial1.begin(9600);
	
	// LEDs
	pinMode(led_pin[0], OUTPUT);
	pinMode(led_pin[1], OUTPUT);
	pinMode(led_pin[2], OUTPUT);
	
	pinMode(toggle_pin[0], INPUT);
	pinMode(toggle_pin[1], INPUT);
	pinMode(toggle_pin[2], INPUT);
	pinMode(toggle_pin[3], INPUT);
	
	pinMode(joystick_button_pin[0], INPUT);
	pinMode(joystick_button_pin[1], INPUT);
	
	pinMode(button_pin[0], INPUT);
	pinMode(button_pin[1], INPUT);
	pinMode(button_pin[2], INPUT);
	pinMode(button_pin[3], INPUT);
	pinMode(button_pin[4], INPUT);
	pinMode(button_pin[5], INPUT);
	
	// Turn On All LED to indicate end of Init
	digitalWrite(led_pin[0], HIGH);
	digitalWrite(led_pin[1], HIGH);
	digitalWrite(led_pin[2], HIGH);
	delay(1000);
	digitalWrite(led_pin[0], HIGH); // light up Power LED
	digitalWrite(led_pin[1], LOW);
	digitalWrite(led_pin[2], LOW);
	
	
	Menu_LCD_Display();
	
	// init Buffer array
	for (int i=0; i<Buffer_Size; i++){
	
		p_pot1[i] = Analog_Control_Init_Value;
		p_pot2[i] = Analog_Control_Init_Value;
		p_pot3[i] = Analog_Control_Init_Value;
		p_pot4[i] = Analog_Control_Init_Value;

		p_js1_x[i] = Analog_Control_Init_Value;
		p_js1_y[i] = Analog_Control_Init_Value;
		p_js2_x[i] = Analog_Control_Init_Value;
		p_js2_y[i] = Analog_Control_Init_Value;
	
	}


}

// ==================== ==== =========================
// ==================== LOOP =========================
// ==================== ==== =========================


void loop(){

	//Debug();

	// Get Current Time
	int time = millis();

	p_button1 = button1;
	p_button2 = button2;
	
	// Check Menu Buttons for interrupts
	button1 = digitalRead(button_pin[0]);	// not used in remote control, only for menu
	button2 = digitalRead(button_pin[1]); // not used in remote control, only for menu
	
	if((button1 && !p_button1) || (button2 && !p_button2)){
		
		Menu_Action(button1, button2);
	
	}
	
	if (Running){
	
		if (Serial1.available() > 0) {
		
			
			Connected = true; // Update Connection Status
			DisConTimer = 0; // reset connection timer
			LastConTime = time;
			
			digitalWrite(led_pin[1], HIGH);
			digitalWrite(led_pin[2], HIGH);
			
			byte feedback = Serial1.read();
			
			// Feedback Signal...
			switch (feedback){
			
				// Go ahead signal
				case 1:	{
					
					TakeInputValues();
					
					switch(Cur_Protocol_Type){
					
						// full - short
						case 0:{
							
							byte _pot1 = map(pot1, 0, 1023, 0, 255);
							byte _pot2 = map(pot2, 0, 1023, 0, 255);
							byte _pot3 = map(pot3, 0, 1023, 0, 255);
							byte _pot4 = map(pot4, 0, 1023, 0, 255);
							
							byte _js1_x = map(js1_x, 0, 1023, 0, 255);
							byte _js1_y = map(js1_y, 0, 1023, 0, 255);
							byte _js2_x = map(js2_x, 0, 1023, 0, 255);
							byte _js2_y = map(js2_y, 0, 1023, 0, 255);
							
							Serial1.write(Cur_Protocol_Type); // sending command protocol type
							Serial1.write(_pot1);
							Serial1.write(_pot2);
							Serial1.write(_pot3);
							Serial1.write(_pot4); // 5
							
							Serial1.write(_js1_x); 
							Serial1.write(_js1_y);
							Serial1.write(_js2_x);
							Serial1.write(_js2_y);
							
							Serial1.write(buttonByte);
							
							break;
							
						}
						// full - long
						case 1:{
						
							byte _pot1_high = highByte(pot1);
							byte _pot1_low = lowByte(pot1);
							byte _pot2_high = highByte(pot2);
							byte _pot2_low = lowByte(pot2);
							byte _pot3_high = highByte(pot3);
							byte _pot3_low = lowByte(pot3);		
							byte _pot4_high = highByte(pot4);
							byte _pot4_low = lowByte(pot4);	
							
							byte _js1_x_high = highByte(js1_x);
							byte _js1_x_low = lowByte(js1_x);	
							byte _js1_y_high = highByte(js1_y);
							byte _js1_y_low = lowByte(js1_y);	
							byte _js2_x_high = highByte(js2_x);
							byte _js2_x_low = lowByte(js2_x);	
							byte _js2_y_high = highByte(js2_y);
							byte _js2_y_low = lowByte(js2_y);	
							
							
							Serial1.write(Cur_Protocol_Type); // sending command protocol type
							Serial1.write(_pot1_high);
							Serial1.write(_pot1_low);
							Serial1.write(_pot2_high);
							Serial1.write(_pot2_low); // 5
							Serial1.write(_pot3_high); 
							Serial1.write(_pot3_low);
							Serial1.write(_pot4_high);
							Serial1.write(_pot4_low);
							
							Serial1.write(_js1_x_high);
							Serial1.write(_js1_x_low);
							Serial1.write(_js1_y_high);
							Serial1.write(_js1_y_low);
							Serial1.write(_js2_x_high);
							Serial1.write(_js2_x_low);
							Serial1.write(_js2_y_high);
							Serial1.write(_js2_y_low);
							
							Serial1.write(buttonByte);
						
							break;
						}
						
						// sel - short
						case 2:{
						
							byte _pot2 = map(pot2, 0, 1023, 0, 255);

							byte _js1_x = map(js1_x, 0, 1023, 0, 255);
							byte _js1_y = map(js1_y, 0, 1023, 0, 255);

							
							Serial1.write(2); // sending command protocol type
							Serial1.write(_pot2);
							
							Serial1.write(_js1_x); 
							Serial1.write(_js1_y);
							
							Serial1.write(buttonByte);
						
							break;
						}
						
						// sel - long
						case 3:{
						
							byte _pot2_high = highByte(pot2);
							byte _pot2_low = lowByte(pot2);
							
							byte _js1_x_high = highByte(js1_x);
							byte _js1_x_low = lowByte(js1_x);	
							byte _js1_y_high = highByte(js1_y);
							byte _js1_y_low = lowByte(js1_y);	
							
							
							Serial1.write(Cur_Protocol_Type); // sending command protocol type

							Serial1.write(_pot2_high);
							Serial1.write(_pot2_low); 

							Serial1.write(_js1_x_high);
							Serial1.write(_js1_x_low);
							Serial1.write(_js1_y_high);
							Serial1.write(_js1_y_low);
							
							Serial1.write(buttonByte);
						
							break;
						}
					
					}
					
					break;
				}

				// Error: Un-identified protocol type
				case 97:	{
					lcd.clear();
					lcd.print("Error, Unknow Prot Type ...");
					
					digitalWrite(led_pin[2], HIGH);
					delay(2000);
					digitalWrite(led_pin[2], LOW);
					
					Menu_LCD_Display();
					
					break;
				}
				
				// Error: Packet Overflow
				case 98:	{
					lcd.clear();
					lcd.print("Error, Packet Overflow ...");
					
					digitalWrite(led_pin[2], HIGH);
					delay(2000);
					digitalWrite(led_pin[2], LOW);
					
					Menu_LCD_Display();
					
					break;
				}
				                                                                                                                                                                 
				// Error: Missing Packet
				case 99:	{
					lcd.clear();
					lcd.print("Received Error Feedback...");
					
					digitalWrite(led_pin[2], HIGH);
					delay(2000);
					digitalWrite(led_pin[2], LOW);
					
					Menu_LCD_Display();
					
					break;
				}
					
				// do nothing
				default:
					;
			
			}

		}
			
	}
	
	
	// Update Connection Status
	if(time - LastConTime > ConnectionTimeOut){
		Connected = false;
		digitalWrite(led_pin[1], LOW);
	}
	
	// Transmission LED
	if(time - LastConTime > LED_transmit_on_time){
		digitalWrite(led_pin[2], LOW);
	}
	
	// If we are disconnected, send connection invitation
	if (!Connected){
		delay(500);
		// send out re-connect request
		Serial1.write(1);
		delay(500);
		
	}

}



// ==================== ==== =========================
// ==================== Menu =========================
// ==================== ==== =========================

void Menu_Action(bool Left_Button, bool Right_Button){

	switch (Cur_Menu_Page){
		// Page: Main 
		case 0: {
			// Go to: Pause Page
			if (Left_Button){
				;
			}
			// Go to: Menu Page
			else if (Right_Button){
			
				Running = false;
				Cur_Menu_Page = 1;
				
			}
		
			break;
		}

		// Page: Menu 
		case 1: {
			// Select Option
			if (Left_Button){
				// Which menu option are we at?
				switch(Cur_Menu_Item){
					// Exit
					case 0:{
						Running = true;
						Cur_Menu_Page = 0;
						break;
					}
					// Prot(full-short)

					case 1:{
						Cur_Protocol_Type = 0;
						Cur_Menu_Item = 0;
						break;
					}
					// prot(full-long) 
					case 2:{
						Cur_Protocol_Type = 1;
						Cur_Menu_Item = 0;
						break;
					}
					
					// prot(sel-short)
					case 3:{
						Cur_Protocol_Type = 2;
						Cur_Menu_Item = 0;
						break;
					}
					
					// prot(sel-long)
					case 4:{
						Cur_Protocol_Type = 3;
						Cur_Menu_Item = 0;
						break;
					}
					
					default:
					;
				
				}
			}
			// Next Option
			else if (Right_Button){
			
				if (++Cur_Menu_Item >= Num_Menu_Items) 
					Cur_Menu_Item = 0;
				
			}
			break;
		}
		default: {
		
			Running = true;
			Cur_Menu_Page = 0;
			
		}
	
	}
	
	Menu_LCD_Display();

}

void Menu_LCD_Display(){

	switch(Cur_Menu_Page){
	
		case 0:{
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print("Running..");
			lcd.setCursor(0,1);
			lcd.print("<          Menu>");
			break;
		}
		case 1:{
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print(Menu_Items[Cur_Menu_Item]);
			lcd.setCursor(0,1);
			lcd.print("<Select     Next>");
			break;
		}

		default:
			;
	
	}




}



void Debug(){

	// in Debug Mode, stop re-connection by faking connected state.
	Connected = true;
	LastConTime = millis();

	TakeInputValues();


	Serial.print(pot1);
	Serial.print("  ");
	Serial.print(pot2);
	Serial.print("  ");
	Serial.print(pot3, DEC);
	Serial.print("  ");
	Serial.print(pot4, DEC);
	Serial.print("  ");
	
	Serial.print(js1_x);
	Serial.print("  ");
	Serial.print(js1_y, DEC);
	Serial.print("  ");
	Serial.print(js2_x, DEC);
	Serial.print("  ");
	Serial.print(js2_y, DEC);
	Serial.print("  ");
	
	Serial.print(toggle1, DEC);
	Serial.print("  ");
	Serial.print(toggle2, DEC);
	Serial.print("  ");
	Serial.print(toggle3, DEC);
	Serial.print("  ");
	Serial.print(toggle4, DEC);
	Serial.print("  ");
	
	
	Serial.print(button3, DEC);
	Serial.print("  ");
	Serial.print(button4, DEC);
	Serial.print("  ");
	Serial.print(button5, DEC);
	Serial.print("  ");
	Serial.print(button6, DEC);
	Serial.print("  ");

	Serial.println("  ");	
					
	delay(5);
}