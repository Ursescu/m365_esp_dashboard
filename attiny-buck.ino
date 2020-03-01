void setup(){
    pinMode(PB1, OUTPUT);
    digitalWrite(PB1, LOW);
}

void loop(){
    digitalWrite(PB1, LOW);
    delay(500);
    digitalWrite(PB1, HIGH);
    delay(500);
}