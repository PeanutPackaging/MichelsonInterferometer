// Interferometer rig: live graph version for the Arduino IDE Serial Plotter
// Prints labelled values so each line on the graph is named.
// Floor ADXL345 0x53, Plate ADXL345 0x1D, Fringe OPT101 A0, Reference OPT101 A1
// Serial Plotter at 115200 baud

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
  if (Wire.endTransmission(false) != 0) return 0;
  Wire.requestFrom(addr, (uint8_t)1);
  return Wire.available() ? Wire.read() : 0;
}

bool setupSensor(uint8_t addr) {
  if (readReg(addr, 0x00) != 0xE5) return false;     // not found
  writeReg(addr, 0x31, 0x08);                        // full resolution, +/-2 g
  writeReg(addr, 0x2C, 0x0A);                        // 100 Hz
  writeReg(addr, 0x2D, 0x08);                        // measure
  return true;
}

void readXYZ(uint8_t addr, int16_t &x, int16_t &y, int16_t &z) {
  Wire.beginTransmission(addr);
  Wire.write(0x32);
  Wire.endTransmission(false);
  Wire.requestFrom(addr, (uint8_t)6);
  uint8_t b[6];
  for (uint8_t i = 0; i < 6; i++) b[i] = Wire.available() ? Wire.read() : 0;
  x = (int16_t)((b[1] << 8) | b[0]);
  y = (int16_t)((b[3] << 8) | b[2]);
  z = (int16_t)((b[5] << 8) | b[4]);
}

void plotSensor(bool ok, uint8_t addr, const char* name) {
  if (!ok) return;
  int16_t x, y, z;
  readXYZ(addr, x, y, z);
  Serial.print(name); Serial.print("X:"); Serial.print(x); Serial.print(',');
  Serial.print(name); Serial.print("Y:"); Serial.print(y); Serial.print(',');
  Serial.print(name); Serial.print("Z:"); Serial.print(z); Serial.print(',');
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(100000);
  delay(200);
  floorOK = setupSensor(FLOOR_ADDR);
  plateOK = setupSensor(PLATE_ADDR);
}

void loop() {
  plotSensor(floorOK, FLOOR_ADDR, "floor");
  plotSensor(plateOK, PLATE_ADDR, "plate");
  Serial.print("fringe:");    Serial.print(analogRead(A0)); Serial.print(',');
  Serial.print("reference:"); Serial.println(analogRead(A1));
  delay(50);                                         // 20 points per second
}