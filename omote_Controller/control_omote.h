/* omote robotic controller 
   written by Oscar Liang
   blog.oscarliang.net
   
   Client - Robot side
*/


#include "WProgram.h"


#define MaxPacketNum 20
byte packets[MaxPacketNum];
byte pre_packets[MaxPacketNum];
byte Cur_Packet_Index = 0;


int pot1 = 0;
int pot2 = 0;
int pot3 = 0;
int pot4 = 0;

int js1_x = 512;
int js1_y = 512;
int js2_x = 512;
int js2_y = 512;

bool toggle1 = 0;
bool toggle2 = 0;
bool toggle3 = 0;
bool toggle4 = 0;

bool button3 = 0;
bool button4 = 0;
bool button5 = 0;
bool button6 = 0;



void DecodeButton(byte byte1, bool *bit1, bool *bit2, bool *bit3, bool *bit4, bool *bit5, bool *bit6, bool *bit7, bool *bit8){
// Usage: DecodeButton(buttonByte, &toggle1, &toggle2, &toggle3, &toggle4, &button3, &button4, &button5, &button6);
	byte sum = 0;

	*bit1 = byte1 >> 7;
	
	sum += (*bit1)*128;
	*bit2 = (byte1-sum) >> 6;
	
	sum += (*bit2)*64;
	*bit3 = byte1-sum >> 5;
	
	sum += (*bit3)*32;
	*bit4 = byte1-sum >> 4;
	
	sum += (*bit4)*16;
	*bit5 = byte1-sum >> 3;
	
	sum += (*bit5)*8;
	*bit6 = byte1-sum >> 2;
	
	sum += (*bit6)*4;
	*bit7 = byte1-sum >> 1;
	
	sum += (*bit7)*2;
	*bit8 = byte1-sum >> 0;

}


void DecodeButton2(byte byte1, byte *bit1, byte *bit2, byte *bit3, byte *bit4, byte *bit5, byte *bit6, byte *bit7, byte *bit8){
// Usage: DecodeButton(buttonByte, &toggle1, &toggle2, &toggle3, &toggle4, &button3, &button4, &button5, &button6);
	byte sum = 0;

	*bit1 = byte1 >> 7;
	
	sum += (*bit1)*128;
	*bit2 = (byte1-sum) >> 6;
	
	sum += (*bit2)*64;
	*bit3 = byte1-sum >> 5;
	
	sum += (*bit3)*32;
	*bit4 = byte1-sum >> 4;
	
	sum += (*bit4)*16;
	*bit5 = byte1-sum >> 3;
	
	sum += (*bit5)*8;
	*bit6 = byte1-sum >> 2;
	
	sum += (*bit6)*4;
	*bit7 = byte1-sum >> 1;
	
	sum += (*bit7)*2;
	*bit8 = byte1-sum >> 0;

}

void OM_Init(){
  
  Serial.begin(9600);
  delay(100);
  Serial.write(1);
  
}

bool OM_ReceivePacket(){

	if(Serial.available() > 0){

		
		while (Serial.available() > 0) {
			pre_packets[Cur_Packet_Index] = packets[Cur_Packet_Index]; // saving previous commands
			packets[Cur_Packet_Index++] = Serial.read();
			delay(2);
			
			// packets overflow, flag error
			if (Cur_Packet_Index >= MaxPacketNum){
				Serial.write(98);
				Serial.flush();
				break;
			}
		}
		
		switch (packets[0]){
		
				// full short
			case 0:	{
				
				// verfity we have received all data
				/*
				if(Cur_Packet_Index < 9){
					Serial.write(99);
					break;
				}
				*/
				
				pot1 = map(packets[1], 0, 255, 0, 1023);
				pot2 = map(packets[2], 0, 255, 0, 1023);
				pot3 = map(packets[3], 0, 255, 0, 1023);
				pot4 = map(packets[4], 0, 255, 0, 1023);
				
				js1_x = map(packets[5], 0, 255, 0, 1023);
				js1_y = map(packets[6], 0, 255, 0, 1023);
				js2_x = map(packets[7], 0, 255, 0, 1023);
				js2_y = map(packets[8], 0, 255, 0, 1023);
				
				DecodeButton(packets[9], &toggle1, &toggle2, &toggle3, &toggle4, &button3, &button4, &button5, &button6);
				
				break;
			}
			// full - long
			case 1: {
			
				pot1 = word(packets[1], packets[2]);
				pot2 = word(packets[3], packets[4]);
				pot3 = word(packets[5], packets[6]);
				pot4 = word(packets[7], packets[8]);
				
				js1_x = word(packets[9], packets[10]);
				js1_y = word(packets[11], packets[12]);
				js2_x = word(packets[13], packets[14]);
				js2_y = word(packets[15], packets[16]);
				
				DecodeButton(packets[17], &toggle1, &toggle2, &toggle3, &toggle4, &button3, &button4, &button5, &button6);
				
				break;
			}
			// sel - short
			case 2: {
			
				pot2 = map(packets[1], 0, 255, 0, 1023);
				
				js1_x = map(packets[2], 0, 255, 0, 1023);
				js1_y = map(packets[3], 0, 255, 0, 1023);
				
				DecodeButton(packets[4], &toggle1, &toggle2, &toggle3, &toggle4, &button3, &button4, &button5, &button6);
				
				break;
			}
			// sel - long
			case 3: {

				pot2 = word(packets[1], packets[2]);
				
				js1_x = word(packets[3], packets[4]);
				js1_y = word(packets[5], packets[6]);

				
				DecodeButton(packets[7], &toggle1, &toggle2, &toggle3, &toggle4, &button3, &button4, &button5, &button6);
				break;
			}
			default:
				;
			
		}
		
		
		// Reset Variables for next loop
		Cur_Packet_Index = 0;
		return true;
		
	}
	else{
	
		return false;
	
	}
	

}

void OM_SendFeedback(byte feedback){

	Serial.write(feedback);

}


