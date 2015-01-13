int pinBuzzer = 12;
int numTonos = 10;
int tonos[] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440};
          //{mid C,  C#,   D,  D#,   E,   F,  F#,   G,  G#, A}


void setup() {
  for (int i = 0; i < numTonos; i++) {
    tone(pinBuzzer, tonos[i]);
    delay(500);
  }
  
  noTone(pinBuzzer);
}

void loop() {
}
