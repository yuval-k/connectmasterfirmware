#include <OneWire.h>

#define NUM_POLES 20
#define TIMEOUT 1000

const byte CONNECT_READ_STATE = 0xBE;

bool pole_data[NUM_POLES][NUM_POLES];
unsigned long poles_deadline[NUM_POLES];

OneWire bus(10);  // on pin 10

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  bus.reset_search();
}

void loop() {

  // TODO dad last update time

  // each update data reads a pole
  update_data();
  send_data();

  unsigned long now = millis();

  // update local state,
  for (int i = 0; i < NUM_POLES; i++){
    if ( now > poles_deadline[i]){
      for (int j = 0; j < NUM_POLES; j++){
        pole_data[i][j] = false;
      }
    }
  }
  
}

void send_data() {
  for (int i = 0; i < NUM_POLES; i++){
    Serial.print('0' + i);
    for (int j = 0; j < NUM_POLES; j++){
      Serial.print(": ");
      if (pole_data[i][j]) {
        Serial.print('0' + j);
      }
    }
    Serial.println();
  }
}

void update_data() {
  // put your main code here, to run repeatedly:

  
  byte data[9];
  byte addr[8];
 

  if ( !bus.search(addr)) {
      Serial.print("# No more addresses.\n");
      bus.reset_search();
      return;
  }

  // we have a pole!
  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("# CRC is not valid!\n");
      return;
  }
  
  byte poleIndex = addr[1];
  
  bus.reset();
  bus.select(addr);
  bus.write(CONNECT_READ_STATE);
  
  for (int i = 0; i < 9; i++) {
    data[i] = bus.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  if ( OneWire::crc8(data, 8) != data[8]) {
      Serial.print("# CRC is not valid!\n");
      return;
  }
  poles_deadline[poleIndex] = millis() + TIMEOUT;

  // update local state,
  for (int i = 0; i < NUM_POLES; i++) {
    pole_data[poleIndex][i] = (0 != (data[i >> 3] & (1 <<( i & 0x7 ))));
  }
  
}
