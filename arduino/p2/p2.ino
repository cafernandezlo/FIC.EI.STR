#include <EEPROM.h>

#define dht_dpin A0

#define DEBUG true

byte bGlobalErr;   // error global para el DHT11
byte datos_dht[5]; // para guardar los datos que almacena el DHT11

boolean full = false;  // para saber si ya se dio la vuelta
int index = 0;         // current index of EEPROM
int MAX_EEPROM = 1024; // size of EEPROM for ATmega328
int byteRecv;          // caracter que se recibio por el puerto serie

unsigned long INIT_MILLIS;      // para tener que contar el inicio para saber cuando pasa el minuto
unsigned long MINUTE_IN_MILLIS = 60000;

void (*print_eeprom)(); // la funcion que se usara para imprimir


// Se le pasa una posicion de la eeprom e imprime lo que hay en ella
void print_eeprom_data(int i) {
    Serial.print("EEPROM [");
    Serial.print(i, DEC);
    Serial.print("] H = ");
    Serial.print(EEPROM.read(i));
    Serial.print(".");
    Serial.print(EEPROM.read(i+1));
    Serial.print("% T = ");
    Serial.print(EEPROM.read(i+2));
    Serial.print(".");
    Serial.print(EEPROM.read(i+3));
    Serial.println("C ");
}

// Esta funcion imprime segun las posiciones de memoria
void print_eeprom_pos() {
  int i;
 
  for (i = 0; i < MAX_EEPROM; i += 4) {
    if (i >= index && !full) // Solo se imprime hasta donde esta
      break;
      
    print_eeprom_data(i);
  } 
 
}

// Esta funcion imprime segun la entrada mas vieja
void print_eeprom_date() {
    int i;
    
    if (full) {
      for (i = index; i < MAX_EEPROM; i += 4)
        print_eeprom_data(i); 
    }
    
    for (i = 0; i < index; i += 4)
      print_eeprom_data(i);
}

void InicializaDHT() {
  //inicializacion del pin de datos
  pinMode(dht_dpin, OUTPUT);
  digitalWrite(dht_dpin, HIGH);
}

void leeDHT() {
  bGlobalErr = 0;
  byte dht_in;
  byte i;
  
  // se lleva a cabo el procedimiento de lectura:
  //se pone el pin a LOW e inmediatamente despues a HIGH
  digitalWrite(dht_dpin, LOW);
  delay(20);
  digitalWrite(dht_dpin, HIGH);
  delayMicroseconds(40);
  
  pinMode(dht_dpin, INPUT); //se configura como entrada el pin de datos del MDMT

  dht_in = digitalRead(dht_dpin); //...y se realiza la lectura del sensor dos
                                  //veces tan solo para verificar que funciona
  if (dht_in) {
    bGlobalErr=1;
    return;
  }
  
  delayMicroseconds(80);
  
  dht_in = digitalRead(dht_dpin);
  
  if (!dht_in) {
    bGlobalErr=2;
    return; 
  }
  
  delayMicroseconds(80);
  
  for (i = 0; i < 5; i++) // una vez verificado que funciona, se lee los 40 bits enviados por el DHT11
    datos_dht[i] = lee_datos_dht();
  
  
  pinMode(dht_dpin, OUTPUT); //se cambia el pin de datos a salida y se ponde a HIGH
  digitalWrite(dht_dpin, HIGH);
  
  //se verifica el checksum
  byte dht_check_sum = datos_dht[0] + datos_dht[1] + datos_dht[2] + datos_dht[3];
  if (datos_dht[4] != dht_check_sum) {
    bGlobalErr=3;
  }
}

byte lee_datos_dht() {
  byte i = 0;
  byte resultado = 0; 
  
  for (i = 0; i < 8; i++) {
    
    while (digitalRead(dht_dpin) == LOW);
    delayMicroseconds(30);
    if (digitalRead(dht_dpin) == HIGH)
      resultado |=(1<<(7-i));
      
    while (digitalRead(dht_dpin) == HIGH);
  }
  
  return resultado;
}



void setup() {
  InicializaDHT();
  Serial.begin(9600); 
  delay(300);
  Serial.println("Iniciado sensor DHT11");

  delay(700);
  
  INIT_MILLIS = millis();
  if (DEBUG) {
    Serial.print  ("INIT_MILLIS: ");
    Serial.println(INIT_MILLIS);
    Serial.print  ("MINUTE_IN_MILLIS: ");
    Serial.println(MINUTE_IN_MILLIS);
  }
  
  print_eeprom = print_eeprom_pos;
}

void read_data() {
  if (Serial.available() > 0) {
    // lee el byte entrante:
    byteRecv = Serial.read();
    
    switch (byteRecv) {
      case 'd':
      case 'D':
        print_eeprom();
        
        index = 0;
        full = false;
        
        break;
    }
  }
}

void save_on_eeprom() {
  int value;
  
  leeDHT();
  switch(bGlobalErr) {
    case 0:
      if (DEBUG) {
        Serial.print("To write start in :");
        Serial.println(index);
        
        Serial.print("Humedad = ");
        Serial.print(datos_dht[0], DEC);
        Serial.print(".");
        Serial.print(datos_dht[1], DEC);
        Serial.print("% ");
      
        Serial.print("temperature = ");
        Serial.print(datos_dht[2], DEC);
        Serial.print(".");
        Serial.print(datos_dht[3], DEC);
        Serial.println("C ");
      }
      //Empezamos desde el principio otra vez
      if (index + 4 > MAX_EEPROM) {
        full = true;
        index = 0; 
      }
      
      EEPROM.write(index    , datos_dht[0]);
      EEPROM.write(index + 1, datos_dht[1]);
      EEPROM.write(index + 2, datos_dht[2]);
      EEPROM.write(index + 3, datos_dht[3]);
      index += 4;
      
      break;
      
    case 1:
      Serial.println("Error1: la primera verificacion de inicializacion del DHT ha fallado.");
      break;

    case 2:
      Serial.println("Error2: la segunda verificacion de inicializacion del DHT ha fallado.");
      break;
    
    case 3:
      Serial.println("Error3: detectado fallo al almacenar los datos.");
      break;
    
    default:
      Serial.println("Error: codigo de error desconocido.");
      break;
  }
  
  delay(800);
}

void loop() {
  
  read_data();
  
  if (millis() - INIT_MILLIS >= MINUTE_IN_MILLIS) {
    INIT_MILLIS = millis();
    save_on_eeprom();
  }
  //Queda por controlar el desbordamiento que puede sufrir millis
 
  
  
}
