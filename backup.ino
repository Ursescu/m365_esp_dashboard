
int throttleInput = A2;
int stopInput = 5;
int breakInput = A3;

void setup() {
    Serial1.begin(115200);
    pinMode(throttleInput, INPUT);
    pinMode(breakInput, INPUT);
    pinMode(stopInput, INPUT);
}





#define SPEED_POS 7
#define BRAKE_POS 8

unsigned char motor[] = {0x55, 0xAA, 0x7, 0x20, 0x65, 0x0, 0x4, 0x00, 0x00, 0x0, 0x0, 0x0, 0x0};
unsigned char motor1[] = {0x55, 0xAA, 0x9, 0x20, 0x64, 0x0, 0x6, 0x00, 0x00, 0x0, 0x0, 0x72, 0x0, 0x0, 0x0};


void sendESCMotor(){
   
  uint16_t speed, breakS;
  speed = analogRead(throttleInput);
  breakS = analogRead(breakInput);
  uint8_t speed2 = map(speed, 170, 870, 0x26, 0xC2);
  uint8_t breakS2 = map(breakS, 170, 870, 0x26, 0xC2);
  
//  uint8_t breakS = 0x26;
//  if(digitalRead(stopInput)){
//    speed2 = 0x26;
//    breakS = 0x26;  
//  }
//  Serial.println("Salut");
//  Serial.println(speed2, HEX);
//  Serial.println(breakS2, HEX);

//
//  speed2 = 0x28;
//  breakS2 = 0x28;
  
  motor[SPEED_POS] = speed2;
  motor[BRAKE_POS] = breakS2;

  motor1[SPEED_POS] = speed2;
  motor1[BRAKE_POS] = breakS2;
//
//  Serial1.println(speed2, HEX);
//  
//  Serial1.println(breakS2, HEX);
//  Serial1.println("Salut");
  uint16_t calc = 0;
  for(int x = 2; x < sizeof(motor) - 2; x++) calc += motor[x];
  calc ^= 0xffff;
  motor[11] = (uint8_t)(calc&0xff);
  motor[12] = (uint8_t)((calc&0xff00) >> 8);
  
  calc = 0;
  for(int x = 2; x < sizeof(motor1) - 2; x++) calc += motor1[x];
  calc ^= 0xffff;
  motor1[13] = (uint8_t)(calc&0xff);
  motor1[14] = (uint8_t)((calc&0xff00) >> 8);

//  for(int x = 0; x < sizeof(motor1); x++){
//     Serial.println(motor1[x], HEX);
//  }
//  
  Serial1.write(motor, sizeof(motor));
  delay(10);
  Serial1.write(motor,sizeof(motor));
  delay(10);
  Serial1.write(motor,sizeof(motor));
  delay(10);
  Serial1.write(motor,sizeof(motor));
  delay(10);
  Serial1.write(motor1,sizeof(motor1));
}

uint8_t unlockStatus = 0;
void loop() {
    sendESCMotor();
}
