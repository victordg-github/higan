auto System::serializeInit() -> void {
  serializer s;

  uint signature = 0;
  char version[16] = {};
  char description[512] = {};

  s.integer(signature);
  s.array(version);
  s.array(description);

  serializeAll(s);
  information.serializeSize = s.size();
}

auto System::serialize() -> serializer {
  serializer s(information.serializeSize);

  uint signature = 0x31545342;
  char version[16] = {};
  char description[512] = {};
  memory::copy(&version, (const char*)SerializerVersion, SerializerVersion.size());

  s.integer(signature);
  s.array(version);
  s.array(description);

  serializeAll(s);
  return s;
}

auto System::unserialize(serializer& s) -> bool {
  uint signature = 0;
  char version[16] = {};
  char description[512] = {};

  s.integer(signature);
  s.array(version);
  s.array(description);

  if(signature != 0x31545342) return false;
  if(string{version} != SerializerVersion) return false;

  power();
  serializeAll(s);
  return true;
}

auto System::serializeAll(serializer& s) -> void {
  system.serialize(s);
  cpu.serialize(s);
  ppu.serialize(s);
  apu.serialize(s);
  cartridge.serialize(s);
  iram.serialize(s);
}

auto System::serialize(serializer& s) -> void {
  eeprom.serialize(s);

  s.integer(io.unknown0);
  s.integer(io.unknown1);
  s.integer(io.unknown3);
  s.integer(io.format);
  s.integer(io.depth);
  s.integer(io.color);
}
