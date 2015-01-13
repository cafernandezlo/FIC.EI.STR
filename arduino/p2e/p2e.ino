#define dht_dpin A0

byte bGlobalErr;
byte datos_dht[5];

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
}

void loop() {
  leeDHT();
  switch(bGlobalErr) {
    case 0:
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
