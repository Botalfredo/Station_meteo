/*
 * Epromm varablie location
 * 
 * volatile byte Random_face -> 0
 * unsigned char oled        -> 1
 * unsigned int vmax         -> 2 & 3
 * bool  mute = false        -> 4
 */

 void writeIntIntoEEPROM(int address, int number)
{ 
  byte byte1 = number >> 8;
  byte byte2 = number & 0xFF;
  EEPROM.write(address, byte1);
  EEPROM.write(address + 1, byte2);
}

int readIntFromEEPROM(int address)
{
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}
