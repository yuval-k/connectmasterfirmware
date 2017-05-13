#include <OneWire.h>

#define NUM_POLES 20
#define TIMEOUT 200

const byte CONNECT_READ_STATE = 0xBE;

#define SEND_DATA_PERIOD 500

bool pole_data[NUM_POLES][NUM_POLES];
unsigned long poles_deadline[NUM_POLES];

OneWire bus(8);  // on pin 10

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  static int index = 0;
  static unsigned long datasent_deadline = 0;

  index %= NUM_POLES;

  unsigned long now = millis();
  // TODO dad last update time

  // each update data reads a pole
  update_data(index);

  if (now > datasent_deadline) {
    datasent_deadline = now + SEND_DATA_PERIOD;
    send_data();
  }

  // update local state,
  for (int i = 0; i < NUM_POLES; i++){
    if ( now > poles_deadline[i]){
      for (int j = 0; j < NUM_POLES; j++){
        pole_data[i][j] = false;
      }
    }
  }

  ++index;
}

void send_data() {
  for (int i = 0; i < NUM_POLES; i++){
    Serial.print(i);
    for (int j = 0; j < NUM_POLES; j++){
      if (pole_data[i][j]) {
      Serial.print(":");
        Serial.print(j);
      }
    }
    Serial.println();
  }
}

void update_data(int poleIndex) {
  // put your main code here, to run repeatedly:

  byte data[9];
  byte addr[8] = {0x33, poleIndex,0,0,0,0,0};
 
  addr[7] = OneWire::crc8(addr, 7);

  bus.reset();
  bus.select(addr);
  bus.write(CONNECT_READ_STATE);
  
  for (int i = 0; i < 9; i++) {
    data[i] = bus.read();
//    Serial.print(data[i], HEX);
//    Serial.print(" ");
  }

  if ( OneWire::crc8(data, 8) != data[8]) {
//      Serial.print("# CRC is not valid!\n");
      return;
  }
  poles_deadline[poleIndex] = millis() + TIMEOUT;

  // update local state,
  for (int i = 0; i < NUM_POLES; i++) {
    pole_data[poleIndex][i] = (0 != (data[i >> 3] & (1 <<( i & 0x7 ))));
  }
}