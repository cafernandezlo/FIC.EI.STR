#include <Servo.h>

#define I_SEL 0
#define SEL (2+I_SEL)
#define HORIZ A2
#define VERT A1

#define ASK_CALIBRATE false
#define DEBUG false

#define DEBOUNCE 20 // Para evitar el efecto revote en el pulsador

#define STEP 100 // Paso minimo para contar el movimiento

//Estructura para indicar cuanto moverse
struct pos {
  int h;
  int v; 
};

// Los dos servos
Servo servoTilt, servoPan;
// Posicion en la que el joystick esta calibrado
int c_horiz, c_vert;

// para saber la ultima vez aue se pulso
volatile long ms = 0;

void calibrate() {
  if (ASK_CALIBRATE) {
    Serial.println("Se necesita calibrar el joystick.");
    Serial.println("Suelta el joystick y pulsa un boton para continuar.");
    while (Serial.available() <= 0)
      ;
    Serial.read();
  }
  
  c_horiz = analogRead(HORIZ);
  c_vert  = analogRead(VERT);
  
  if (ASK_CALIBRATE) {
    Serial.print("Calibrado (H = ");
    Serial.print(c_horiz, DEC);
    Serial.print(", V = ");
    Serial.print(c_vert, DEC);
    Serial.println(")");
  }
}

void f_sel() {
  if (ms - millis() > DEBOUNCE) {
    Serial.print(analogRead(HORIZ), DEC);
    Serial.print(",");
    Serial.println(analogRead(VERT), DEC); 
    ms = millis();
  } 
}

void setup () {
  pinMode(SEL, INPUT);
  // se activa la resistencia de pull up
  digitalWrite(SEL, HIGH);
  
  attachInterrupt(I_SEL, f_sel, FALLING);
  
  servoTilt.attach(6);
  servoPan.attach(5);
  servoTilt.write(90);
  servoPan.write(90);
  
  Serial.begin(9600);
  
  calibrate();
  Serial.println("setup done");
}

void move_servo(Servo servo, int n) {
  int value = servo.read();
 
  value += n;
  if (value > 180)
    value = 180;
  if (value < 0)
    value = 0;

  servo.write(value);
}

// Indica a donde se tiene que mover
void do_move() {
  int v1 = analogRead(HORIZ) - c_horiz;
  int v2 = analogRead(VERT)  - c_vert;
  
  struct pos pos;
  pos.h = v1 / STEP;
  pos.v = v2 / STEP;
  
  if (abs(v1) > STEP) {
    move_servo(servoPan, pos.h);
    if (DEBUG) {
      Serial.print("mover base: ");
      Serial.println(pos.h, DEC);
    }
  }
  if (abs(v2) > STEP) {
    move_servo(servoTilt, pos.v);
    if (DEBUG) {
      Serial.print("mover brazo: ");
      Serial.println(pos.v, DEC);
    }
  }
  
  // Tener que imprimir toda la informacion hace que exista cierto retraso
  // En el caso de no tener un delay se encuesta tan deprisa que parece que
  // los servos solo identifican la orden de moverte para un extremo
  // Por eso cuando no estamos en modo debug se usa delay para simular ese retraso  
  if (!DEBUG)
    delay(20);
}

void loop() {
  
  do_move();

}
