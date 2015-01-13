#define DEBUG false
#define WAIT 80
#define PIN_BUZZER 12
#define PIN_LED 13

int byteRecibido = 0;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {

    // lee el byte entrante:
    byteRecibido = Serial.read();

    if (DEBUG) {
      Serial.print("Recibido: ");
      Serial.println(byteRecibido);
    }
    
    switch (byteRecibido) {
      case 'e':
      case 'E':
        digitalWrite(PIN_LED, HIGH);
        break;
      case 'a':
      case 'A':
        digitalWrite(PIN_LED, LOW);
        break;
      case 's':
      case 'S':
        tone(PIN_BUZZER, 261, WAIT);
        delay(WAIT);
        tone(PIN_BUZZER, 392, WAIT << 2); // *4
        break; 
    }
  }

}


