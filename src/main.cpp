//------ Libraries
#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//------- Pins definition
#define SCK_PIN 13
#define MISO_PIN 12
#define MOSI_PIN 11
#define CSN_PIN 10
#define CE_PIN 9
#define Button1_PIN 6
#define Button2_PIN 5
#define Button3_PIN 4
#define Button4_PIN 3
#define PotX1_PIN A0
#define PotY1_PIN A1
#define PotX2_PIN A2
#define PotY2_PIN A3
#define Buzzer_PIN A4
#define VoltageSensor_PIN A5
#define Switch1_PIN 7
#define Switch2_PIN 8
#define Switch3_PIN A7
#define Switch4_PIN A6

//------- Radio data structure definition
typedef struct RADIO_DATA_STRUCT {
	byte switch1;
	byte switch2;
	byte switch3;
	byte switch4;
	byte button1;
	byte button2;
	byte button3;
	byte button4; 
	byte j1PotX;
	byte j1PotY;
	byte j2PotX;
	byte j2PotY;
} radio_data_struct;

//------- Radio variables
const uint64_t pipe = 0xE8E8F0F0E1LL;
RF24 radio(CE_PIN,CSN_PIN);
radio_data_struct data;

//------ Battery variables
#define BatteryMax 4.0
#define BatteryMin 3.2
unsigned long beeping_time, non_beeping_time, periode_time;
int beeping_counter;
bool buzzer_is_beeping;
double battery_voltage;

//------ Input variables
const byte input_pins[8] = {Switch1_PIN,Switch2_PIN,Switch3_PIN,Switch4_PIN,Button1_PIN,Button2_PIN,Button3_PIN,Button4_PIN};
const byte joystick_pins[4] = {PotX1_PIN,PotY1_PIN,PotX2_PIN,PotY2_PIN};
byte current_input_val[8];
int current_joystick_val[4];
byte* struct_members = &data.switch1;

//------ Input variables
unsigned long current_loop_time, last_loop_time;

//------ Function headers
void read_inputs();
void write_inputs();
void check_battery();
void print();

//======================================
//              SETUP
//======================================
void setup() {
	// Set Pin_Modes
	pinMode(Button1_PIN,INPUT_PULLUP);
	pinMode(Button2_PIN,INPUT_PULLUP);
	pinMode(Button3_PIN,INPUT_PULLUP);
	pinMode(Button4_PIN,INPUT_PULLUP);
	pinMode(Switch1_PIN,INPUT_PULLUP); 
	pinMode(Switch2_PIN,INPUT_PULLUP); 
	pinMode(Switch3_PIN,INPUT_PULLUP); 
	pinMode(Switch4_PIN,INPUT_PULLUP); 
	pinMode(PotX1_PIN,INPUT);
	pinMode(PotY1_PIN,INPUT);
	pinMode(PotX2_PIN,INPUT);
	pinMode(PotY2_PIN,INPUT);
	pinMode(VoltageSensor_PIN,INPUT); 
	pinMode(Buzzer_PIN,OUTPUT);

	// Inicialize radio 
	Serial.begin(115200);
	radio.begin();
	radio.openWritingPipe(pipe);
	radio.setChannel(90);
	radio.setAutoAck(false);
	radio.setDataRate(RF24_250KBPS);
	radio.setPALevel(RF24_PA_LOW);
	delay(1000);
}

//======================================
//               LOOP
//======================================
void loop() {
	current_loop_time = micros();
	if(current_loop_time - last_loop_time > 2000){
		read_inputs();
		write_inputs();
		check_battery();
		print();
		last_loop_time = current_loop_time;
	}
}

//======================================
//             Functions
//======================================
void read_inputs(){
	for(int i = 2; i < 4; i++){
		if(analogRead(input_pins[i]) > 400){
			current_input_val[i] = 1;
		}
		else{
			current_input_val[i] = 0;
		}
	}
	for(int i = 0; i < 2; i++){
		current_input_val[i] = digitalRead(input_pins[i]);
	}
	for(int i = 4; i < 8; i++){
		current_input_val[i] = digitalRead(input_pins[i]);
	}
	for(int i = 0; i < 4; i++){
		current_joystick_val[i] = analogRead(joystick_pins[i]);	
	}
}

void write_inputs(){
	for(int i = 0; i < 8; i++){
		struct_members[i] = current_input_val[i];
	}
	for(int i = 8; i < 12; i++){
		struct_members[i] = 0; //current_joystick_val[i-8];
	}
	radio.write(&data,sizeof(radio_data_struct));
}

void check_battery(){
	battery_voltage = map(analogRead(VoltageSensor_PIN),0,1023,0,500)/100.0;
	if(battery_voltage <= BatteryMin){
		if(millis() - periode_time > 30000 || beeping_counter < 3){
			if(millis() - periode_time > 30000){
				beeping_counter = 0;
				periode_time = millis();
			}
			if(!buzzer_is_beeping && millis() - non_beeping_time > 50){
				digitalWrite(Buzzer_PIN,HIGH);	
				buzzer_is_beeping = true;
				beeping_time = millis();
			}	
			if(buzzer_is_beeping && millis() - beeping_time > 200){
				digitalWrite(Buzzer_PIN,LOW);	
				buzzer_is_beeping = false;	
				non_beeping_time = millis();
				periode_time = millis();
				beeping_counter++;
			}	
		}
	}

}

void print(){
	for(int i = 0; i < 8; i++){
		Serial.print(current_input_val[i]);
		Serial.print(" , ");
	}

	for(int i = 0; i < 4; i++){
		Serial.print(current_joystick_val[i]);
		Serial.print(" , ");
	}

	Serial.println(battery_voltage);
}