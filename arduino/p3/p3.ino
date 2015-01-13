#include <EEPROM.h>
#include "Wire.h"
#include <stdlib.h>

#define DS1307_TIMEKEEPER_REGISTERS 7 //Cantidad de registros de Fecha y Hora 
#define DS1307_BASE_ADDRESS 0x00 //Registro 00h 

#define DATA_FORMAT_SIZE 19 //Tamano del String para introducir la fecha

#define dht_dpin A0
#define DS1307_I2C_ADDRESS 0x68

#define RTC_PIN A5

#define DEBUG true

byte bGlobalErr;   // error global para el DHT11
byte datos_dht[5]; // para guardar los datos que almacena el DHT11

boolean full = false;  // para saber si ya se dio la vuelta
int index = 0;         // current index of EEPROM
int MAX_EEPROM = 1024; // size of EEPROM for ATmega328

unsigned long INIT_MILLIS;      // para tener que contar el inicio para saber cuando pasa el minuto
unsigned long MINUTE_IN_MILLIS = 60000; // un minuto expresado en milisegundos

void (*print_eeprom)(); // la funcion que se usara para imprimir


// Convertir valores decimales a valores BCD 
byte decToBcd(byte val) {
    return ((val / 10 * 16) + (val % 10)); 
} 

//Convertir valores BCD a valores decimales 
byte bcdToDec(byte val) {
    return ((val / 16 * 10) + (val % 16)); 
} 

// Se le pasa una posicion de la eeprom e imprime lo que hay en ella
void print_eeprom_data(int i) {
    Serial.print("EEPROM [");
    Serial.print(i, DEC);
    Serial.print("]\n\tH = ");
    Serial.print(EEPROM.read(i));
    Serial.print(".");
    Serial.print(EEPROM.read(i+1));
    Serial.print("% T = ");
    Serial.print(EEPROM.read(i+2));
    Serial.print(".");
    Serial.print(EEPROM.read(i+3));
    Serial.println("C");
    
    Serial.print("\t");
    Serial.print(EEPROM.read(i+4));
    Serial.print("-");
    Serial.print(EEPROM.read(i+5));
    Serial.print("-");
    Serial.print(EEPROM.read(i+6));
    Serial.print(" (");
    Serial.print(EEPROM.read(i+7));
    Serial.print(") ");
    Serial.print(EEPROM.read(i+8));
    Serial.print(":");
    Serial.print(EEPROM.read(i+9));
    Serial.print(":");
    Serial.println(EEPROM.read(i+10)); 
}

// Esta funcion imprime segun las posiciones de memoria
void print_eeprom_pos() {
  int i;
 
  for (i = 0; i < MAX_EEPROM; i += 11) {
    if (i >= index && !full) // Solo se imprime hasta donde esta
      break;
      
    print_eeprom_data(i);
  } 
 
}

// Esta funcion imprime segun la entrada mas vieja
void print_eeprom_date() {
    int i;
    
    if (full) {
      for (i = index; i < MAX_EEPROM; i += 11)
        print_eeprom_data(i); 
    }
    
    for (i = 0; i < index; i += 11)
      print_eeprom_data(i);
}

// *******************************************************
// La siguiente funcion realiza las siguientes tareas:
// * Configura la fecha y hora
// * Inicia el conteo del RTC
// * Configura la hora en modo 24h
// * OJO! La funcion no verifica los rangos de entrada
// *******************************************************
void setDate(byte segundo,    //0-59
             byte minuto,     //0-59
             byte hora,       //1-23
             byte diaSemana,  //1-7
             byte diaMes,     //1-28/29/30/31
             byte mes,        //1-12
             byte anho)       //0-99
{
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(DS1307_BASE_ADDRESS);
  Wire.write(decToBcd(segundo));    // 0 to bit 7 starts the clock
  Wire.write(decToBcd(minuto));
  Wire.write(decToBcd(hora));       // If you want 12 hour am/pm you need to set
                                   // bit 6 (also need to change readDateDs1307)
  Wire.write(decToBcd(diaSemana));
  Wire.write(decToBcd(diaMes));
  Wire.write(decToBcd(mes));
  Wire.write(decToBcd(anho));
  Wire.endTransmission();
}

// **************************************
// Obtiene la fecha y hora del DS1307
// **************************************
void getDate(byte *segundo,
             byte *minuto,
             byte *hora,
             byte *diaSemana,
             byte *diaMes,
             byte *mes,
             byte *anho)
{
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(DS1307_BASE_ADDRESS);
  Wire.endTransmission();
  
  Wire.requestFrom(DS1307_I2C_ADDRESS, DS1307_TIMEKEEPER_REGISTERS);
  
  *segundo   = bcdToDec(Wire.read() & 0x7f);
  *minuto    = bcdToDec(Wire.read());
  *hora      = bcdToDec(Wire.read() & 0x3f);
  *diaSemana = bcdToDec(Wire.read());
  *diaMes    = bcdToDec(Wire.read());
  *mes       = bcdToDec(Wire.read());
  *anho      = bcdToDec(Wire.read());
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
  // se configura el pin como entrada
  pinMode(RTC_PIN, INPUT);
  // se activa la resistencia interna del pull-up de 20k del pin
  digitalWrite(RTC_PIN, HIGH);
  
  Wire.begin();
  
  InicializaDHT();
  Serial.begin(9600); 
  
  delay(300);
  Serial.println("Iniciado sensor DHT11");

  delay(700);
  
  INIT_MILLIS = millis();
  if (DEBUG) {
    Serial.print  ("INIT_MILLIS: ");
    Serial.println(INIT_MILLIS);
  }
  
  print_eeprom = print_eeprom_pos;
  
}

boolean check_format(char *data) {
  //YYYY-MM-DD hh:mm:ss
  //0123456789012345678
  
  byte cnt;
  for (cnt = 0; cnt < DATA_FORMAT_SIZE; cnt++) {
    switch (cnt) {
      case 4:
      case 7:
        if (data[cnt] != '-')
          return false;
        break;
      case 10:
        if (data[cnt] != ' ')
          return false;
        break;
      case 13:
      case 16:
        if (data[cnt] != ':')
          return false;
        break;
      default:
        if (data[cnt] >= '0' && data[cnt] <= '9')
          continue;
        return false;
    } 
  }
    
  return true;
}

/*
De: http://mathforum.org/library/drmath/view/62324.html
 
Here we're defining
 
  k = day of month
  m = month number, taking Mar=1, ..., Dec=10, Jan=11, Feb=12
  d = last two digits of year, using the previous year for Jan and Feb
  c = first two digits of year
 
The formula is then
 
  f = k + [(13*m - 1)/5] + d + [d/4] + [c/4] - 2*c
*/
/* Calculate day of week in proleptic Gregorian calendar. Sunday == 0. */
byte getDayOfWeek(int year, byte month, byte day) {
	int adjustment, mm, yy;
 
	adjustment = (14 - month) / 12;
	mm = month + 12 * adjustment - 2;
	yy = year - adjustment;
	return (day + (13 * mm - 1) / 5 +
		yy + yy / 4 - yy / 100 + yy / 400) % 7;
}

boolean check_values(char *buff, byte *segundo, byte *minuto, byte *hora,
                     byte *diaSemana, byte *diaMes, byte *mes, int *anho) {
  //YYYY-MM-DD hh:mm:ss
  //0123456789012345678

Serial.println(buff);  
  buff[4] = buff[7] = buff[10] = buff[13] = buff[16] = '\0';

  *segundo = atoi(&buff[17]);
  *minuto  = atoi(&buff[14]);
  *hora    = atoi(&buff[11]);
  *diaMes  = atoi(&buff[8]);
  *mes     = atoi(&buff[5]);
  *anho    = atoi(&buff[0]);

/*
Serial.print(&buff[17]); Serial.print(" "); Serial.println(*segundo);
Serial.print(&buff[14]); Serial.print(" "); Serial.println(*minuto);
Serial.print(&buff[11]); Serial.print(" "); Serial.println(*hora);
Serial.print(&buff[8]);  Serial.print(" "); Serial.println(*diaMes);
Serial.print(&buff[5]);  Serial.print(" "); Serial.println(*mes);
Serial.print(&buff[0]);  Serial.print(" "); Serial.println(*anho);
*/
  if (!(*segundo < 60 && *minuto < 60 && *hora < 24))
    return false;
  
  if (*anho < 1700 || *anho > 2299)
    return false;

  if (*mes > 12 || *mes == 0)
    return false;

  if (*diaMes > 31 || *diaMes == 0)
    return false;

  *diaSemana = 1 + getDayOfWeek(*anho, *mes, *diaMes);
  
  return true;
}

// Funcion para mirar si el formato de la fecha es el correcto
// NO COMPRUEBA SI LOS VALORES SON LOS ADECUADOS
void established_data() {
  char buff[DATA_FORMAT_SIZE+1];
  
  byte cnt;
  
  Serial.println("Establecer fecha: 'YYYY-MM-DD hh:mm:ss'");
  Serial.println("Presione 'x' para abortar.");
  
  buff[DATA_FORMAT_SIZE] = '\0';
  for (cnt = 0; cnt < DATA_FORMAT_SIZE; cnt++) {
    while (!(Serial.available() > 0))
      ;
    buff[cnt] = Serial.read();
    if (buff[cnt] == 'x' || buff[cnt] == 'X') {
       Serial.println("Se ha abortado. No se establece la fecha.");
       return; 
    }
  }
  
  if (DEBUG) {
    Serial.print("Comprobar :");
    Serial.println(buff);
  }
  
  if (!check_format(buff)) {
    Serial.print  ("Formato incorrecto: ");
    Serial.println(buff);
    Serial.println("El formato tenia que ser 'YYYY-MM-DD hh:mm:ss'");
    return;
  }
  
  byte segundo, minuto, hora, diaSemana, diaMes, mes;
  int anho;
  if(!check_values(buff, &segundo, &minuto, &hora, &diaSemana, &diaMes, &mes, &anho)) {
    Serial.print("Fecha no valida.");
    return;
  }
  
  setDate(segundo, minuto, hora, diaSemana, diaMes, mes, byte(anho % 100));

}

void show_data() {
  byte segundo, minuto, hora, diaSemana, diaMes, mes, anho;
  
  getDate(&segundo, &minuto, &hora, &diaSemana, &diaMes, &mes, &anho);
  
  Serial.print("Dia de la semana: ");
  Serial.println(diaSemana, DEC);
  
  Serial.print(anho, DEC); Serial.print("-");
  Serial.print(mes, DEC); Serial.print("-");
  Serial.print(diaMes, DEC); Serial.print(" ");
  
  Serial.print(hora, DEC); Serial.print(":");
  Serial.print(minuto, DEC); Serial.print(":");
  Serial.println(segundo, DEC);
}

void read_data() {
  if (Serial.available() > 0) {
    // lee el byte entrante:
    int byteRecv = Serial.read();
    
    switch (byteRecv) {
      case 'd':
      case 'D':
        print_eeprom();
        
        index = 0;
        full = false;
        
        break;
       case 'e':
       case 'E':
         established_data();
         break;
       
       case 's':
       case 'S':
         show_data();
         break;
    }
  }
}

void save_on_eeprom() {
  int value;
  byte segundo, minuto, hora, diaSemana, diaMes, mes, anho;
  
  leeDHT();
  getDate(&segundo, &minuto, &hora, &diaSemana, &diaMes, &mes, &anho);
  
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
        
        Serial.print(anho, DEC); Serial.print("-");
        Serial.print(mes, DEC); Serial.print("-");
        Serial.print(diaMes, DEC); Serial.print(" (");
        Serial.print(diaSemana, DEC); Serial.print(")");
        Serial.print(hora, DEC); Serial.print(":");
        Serial.print(minuto, DEC); Serial.print(":");
        Serial.println(segundo, DEC);
      }
      //Empezamos desde el principio otra vez
      if (index + 11 > MAX_EEPROM) {
        full = true;
        index = 0; 
      }
      
      EEPROM.write(index    , datos_dht[0]);
      EEPROM.write(index + 1, datos_dht[1]);
      EEPROM.write(index + 2, datos_dht[2]);
      EEPROM.write(index + 3, datos_dht[3]);
      
      EEPROM.write(index + 4 , anho);
      EEPROM.write(index + 5 , mes);
      EEPROM.write(index + 6 , diaMes);
      EEPROM.write(index + 7 , diaSemana);
      EEPROM.write(index + 8 , hora);
      EEPROM.write(index + 9 , minuto);
      EEPROM.write(index + 10, segundo);
      
      index += 11;
      
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
  
}
