struct NECDSP : uPD96050, Thread {
  auto main() -> void;

  auto read(uint24 address, uint8 data) -> uint8;
  auto write(uint24 address, uint8 data) -> void;

  auto readRAM(uint24 address, uint8 data) -> uint8;
  auto writeRAM(uint24 address, uint8 data) -> void;

  auto unload() -> void;
  auto power() -> void;

  auto firmware() const -> vector<uint8>;
  auto serialize(serializer&) -> void;

  uint Frequency = 0;
};

extern NECDSP necdsp;
