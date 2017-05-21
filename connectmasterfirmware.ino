#include <OneWire.h>

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define NUM_POLES 20

const byte CONNECT_READ_STATE = 0xBE;

#define SEND_DATA_PERIOD 100
#define READ_POLE_PERIOD (100 / NUM_POLES)

bool pole_data[NUM_POLES][NUM_POLES];
unsigned long poles_updated[NUM_POLES];

OneWire bus(8);  // on pin 10

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  static int index = 0;
  static unsigned long datasent_deadline = 0;
  static unsigned long readpole_deadline = 0;

  unsigned long now = millis();
  // TODO dad last update time

  // each update data reads a pole
  if (now > readpole_deadline) {
    readpole_deadline = now + READ_POLE_PERIOD;
    update_data(index);
    ++index;
    index %= NUM_POLES;
  }

  if (now > datasent_deadline) {
    datasent_deadline = now + SEND_DATA_PERIOD;
    send_data();
  }


}

void send_data() {
  unsigned long now = millis();
  for (int i = 0; i < NUM_POLES; i++){
    
    Serial.print(now - poles_updated[i]);
    Serial.print(":");
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
//    Serial.print("# update_data!");
//    Serial.println(poleIndex);

  byte data[4];
  byte addr[8] = {0x33, poleIndex,0,0,0,0,0};
 
  addr[7] = OneWire::crc8(addr, 7);

  bus.reset();
  bus.select(addr);
  bus.write(CONNECT_READ_STATE);
//    Serial.print("#");

  for (int i = 0; i < COUNT_OF(data); i++) {
    data[i] = bus.read();
//    Serial.print(data[i], HEX);
//    Serial.print(" ");
  }
//    Serial.println();

  if ( OneWire::crc8(data, COUNT_OF(data)-1) != data[COUNT_OF(data)-1]) {
//      Serial.println("# CRC is not valid!");
      return;
  }
  poles_updated[poleIndex] = millis();

  // update local state,
  for (int i = 0; i < NUM_POLES; i++) {
    pole_data[poleIndex][i] = (0 != (data[i >> 3] & (1 <<( i & 0x7 ))));
  }
}