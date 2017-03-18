

//in1   A5
//in2   A4
//in3   A3
//in4   A2

//SW TX   32
//SW rX   33
// 485 SEL  34

//TX  20
//RX  21

//OUT 1   31
//OUT 2   30
//OUT 3   29
//OUT 4   28
//OUT 5   27
//OUT 6   26

//I2C SCL  25
//I2C SDA  24
//


#define RELE1 4
#define RELE2 5
#define OUT12V_1 6
#define OUT12V_2 7
#define OUT12V_3 8
#define OUT12V_4 9

char out = 9;
void setup()
{

	Serial.begin(9600);
	
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}
	pinMode(4, OUTPUT);
	pinMode( 5 , OUTPUT);
	pinMode(6, OUTPUT);
	pinMode(7, OUTPUT);
	pinMode(8, OUTPUT);
	pinMode(9, OUTPUT);
   Serial.println("test");
}

void loop()
{
	digitalWrite(RELE1, HIGH);
	delay(2000);
	Serial.println("R1");
    digitalWrite(RELE1, LOW);
	delay(2000);
	//if (digitalRead(A5)) {
	//	//Serial.println("R1"); digitalWrite(RELE1, HIGH);
	//}
	//else digitalWrite(RELE1, LOW);
	////if (digitalRead(A4))digitalWrite(RELE2, HIGH);
	//else digitalWrite(RELE2, LOW);
}
