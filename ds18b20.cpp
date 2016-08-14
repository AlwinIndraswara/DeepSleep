#include "deepSleep.h"

#define ONEWIRE_SEARCH    0
#include <OneWire.h>

static OneWire            ds(OW_PIN);

/* DS18x20 registers
 */
#define SERIAL_CHATTY
#define CONVERT_T         0x44
#define COPY_SCRATCHPAD   0x48
#define WRITE_SCRATCHPAD  0x4E
#define RECALL_EEPROM     0xB8
#define READ_SCRATCHPAD   0xBE
#define CHIP_DS18B20      0x10  // 16
#define CHIP_DS18S20      0x28  // 40

uint32_t time_read;       // us

static bool
ds18b20_convert(byte addr[8])
{
  ds.reset();
  if (!ds.reset()) return false;
  ds.select(addr);
  ds.write(CONVERT_T, 1);

  return true;
}

static float
ds18b20_read(byte addr[8]) 
{
  ds.reset();
  if (!ds.reset()) {
    ++rtcMem.failRead;
    return (86.0);
  }
  
  ds.select(addr);
  ds.write(READ_SCRATCHPAD);

  byte data[9]; //9
  for ( int i = 0; i < 9; i++){  //int ++i
    data[i] = ds.read(); 
  //  Serial.print(data[i]);
  //  Serial.print(" ");
  }

  if (OneWire::crc8(data, 9)) {
    ++rtcMem.failRead;
    return (87.0);
  } 
 // byte type_s;
 // type_s = 0;
  int16_t dCi = (data[1] << 8) | data[0];  // 12 bit temp
/*  if (type_s) {
    dCi = dCi << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      dCi = (dCi & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) dCi = dCi & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) dCi = dCi & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) dCi = dCi & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  } */
  return ((float)dCi / 16.0);
}



bool
read_temp(int n, byte addr[][8], float temp[])
{
  time_read = micros();

  for (int i = 0; i < n; i++) { //int 9=n ++i
    (void)ds18b20_convert(addr[i]);      // start next conversion
    delay(1000);
    temp[i] = ds18b20_read(addr[i]);     // read old conversion
  }

  time_read = micros() - time_read;

#ifdef SERIAL_CHATTY
  Serial.print("\nTemperature = ");
  Serial.print(temp[0]);
  Serial.println("C");
#endif
  return true;
}
