// Interferometer rig test: two ADXL345 accelerometers (I2C) + two OPT101 photodiodes
// Floor ADXL345: SDO to GND  -> address 0x53
// Plate ADXL345: SDO to 3V3  -> address 0x1D
// Fringe OPT101 -> A0, Reference OPT101 -> A1
// Serial Monitor at 115200 baud

#include <Wire.h>

const uint8_t FLOOR_ADDR = 0x53;
const uint8_t PLATE_ADDR = 0x1D;

bool floorOK = false;
bool plateOK = false;

void writeReg(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

uint8_t readReg(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return 0;   // no reply from this address
  Wire.requestFrom(addr, (uint8_t)1);
  return Wire.available() ? Wire.read() : 0;
}

bool setupSensor(uint8_t addr, const char* name) {
  uint8_t id = readReg(addr, 0x00);                  // ADXL345 ID register, should be 0xE5
  Serial.print(name);
  Serial.print(" ADXL345 at 0x");
  Serial.print(addr, HEX);
  if (id != 0xE5) {
    Serial.print(": NOT FOUND (read 0x");
    Serial.print(id, HEX);
    Serial.println(")");
    return false;
  }
  Serial.println(": OK");
  writeReg(addr, 0x31, 0x08);                        // full resolution, +/-2 g (about 256 counts per g)
  writeReg(addr, 0x2C, 0x0A);                        // 100 Hz output rate
  writeReg(addr, 0x2D, 0x08);                        // start measuring
  return true;
}

void readXYZ(uint8_t addr, int16_t &x, int16_t &y, int16_t &z) {
  Wire.beginTransmission(addr);
  Wire.write(0x32);                                  // first data register
  Wire.endTransmission(false);
  Wire.requestFrom(addr, (uint8_t)6);
  uint8_t b[6];
  for (uint8_t i = 0; i < 6; i++) b[i] = Wire.available() ? Wire.read() : 0;
  x = (int16_t)((b[1] << 8) | b[0]);
  y = (int16_t)((b[3] << 8) | b[2]);
  z = (int16_t)((b[5] << 8) | b[4]);
}

void printSensor(bool ok, uint8_t addr) {
  if (!ok) {
    Serial.print("- - - ");
    return;
  }
  int16_t x, y, z;
  readXYZ(addr, x, y, z);
  Serial.print(x); Serial.print(' ');
  Serial.print(y); Serial.print(' ');
  Serial.print(z); Serial.print(' ');
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(100000);                             // 100 kHz, safer on long leads
  delay(200);

  Serial.println("--- Rig test ---");
  floorOK = setupSensor(FLOOR_ADDR, "Floor");
  plateOK = setupSensor(PLATE_ADDR, "Plate");
  Serial.println("floorX floorY floorZ plateX plateY plateZ A0 A1");
}

void loop() {
  printSensor(floorOK, FLOOR_ADDR);
  printSensor(plateOK, PLATE_ADDR);
  Serial.print(analogRead(A0)); Serial.print(' ');
  Serial.println(analogRead(A1));
  delay(100);                                        // 10 lines per second
}
