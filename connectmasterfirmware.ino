#include <Wire.h>

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))


#define NUM_POLES 20
#define TIMEOUT 1000

#define SEND_DATA_PERIOD 1000

const byte CONNECT_READ_STATE = 0xBE;

bool pole_data[NUM_POLES][NUM_POLES];
unsigned long poles_deadline[NUM_POLES];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Wire.setClock(100000);
  Wire.begin();
}

void loop() {
  static int index = 0;
  static unsigned long datasent_deadline = 0;
  
  unsigned long now = millis();
  
  index %= NUM_POLES;

  // TODO dad last update time

  // each update data reads a pole
  update_data(index);

  if (now > datasent_deadline) {
    datasent_deadline = now + SEND_DATA_PERIOD;
    send_data();
  }

  now = millis();

  // update local state,
  for (int i = 0; i < NUM_POLES; i++) {
    if ( now > poles_deadline[i]) {
      for (int j = 0; j < NUM_POLES; j++) {
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


  Wire.requestFrom(0x10+poleIndex, COUNT_OF(data));   // request 6 bytes from slave device #8
  int i;
  for(i = 0; (Wire.available()) && (i < COUNT_OF(data)); i++){
    data[i++] = Wire.read(); // receive a byte as character
  }

  if (i <  COUNT_OF(data)) {
    // we didn't receive all the data - no point of going forward...
    return;
  }


  if ( crc8(data, COUNT_OF(data)-1) != data[COUNT_OF(data)-1]) {
      Serial.print("# CRC is not valid!\n");
      return;
  }
  
  poles_deadline[poleIndex] = millis() + TIMEOUT;

  // update local state,
  for (int i = 0; i < NUM_POLES; i++) {
    pole_data[poleIndex][i] = (0 != (data[i >> 3] & (1 <<( i & 0x7 ))));
  }
  
}





byte crc8(const byte* data, short numBytes)
{
  byte crc = 0;

  while (numBytes--) {
    byte inbyte = *data++;
    for (byte i = 8; i; i--) {
      byte mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix) crc ^= 0x8C;
      inbyte >>= 1;
    }
  }
  return crc;
}

