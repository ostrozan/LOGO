

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
#define IN1 A4
#define IN2 A3
#define IN3 A2
#define IN4 A1

#define RELE1 9
#define RELE2 8
#define OUT12V_1 7
#define OUT12V_2 6
#define OUT12V_3 5
#define OUT12V_4 4
void setup()
{

	Serial.begin(9600);
	
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}
	pinMode(IN1, INPUT);
	pinMode(IN2, INPUT);
	pinMode(IN3, INPUT);
	pinMode(IN4, INPUT);

	pinMode(RELE1, OUTPUT);
	pinMode(RELE2, OUTPUT);
	pinMode(OUT12V_1, OUTPUT);
	pinMode(OUT12V_2, OUTPUT);
	pinMode(OUT12V_3, OUTPUT);
	pinMode(OUT12V_4, OUTPUT);

   Serial.println("test");
}

void loop()
{

	if (digitalRead(IN1))digitalWrite(RELE1,HIGH);
	else digitalWrite(RELE1, LOW);
	if (digitalRead(IN2))digitalWrite(RELE2, HIGH);
	else digitalWrite(RELE2, LOW);
}
