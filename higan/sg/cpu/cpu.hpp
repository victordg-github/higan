struct CPU : Z80, Z80::Bus, Thread {
  inline auto serializing() const -> bool override { return scheduler.serializing(); }

  //cpu.cpp
  auto main() -> void;
  auto step(uint clocks) -> void override;

  auto setNMI(bool value) -> void;
  auto setIRQ(bool value) -> void;

  auto power() -> void;

  //memory.cpp
  auto read(uint16 address) -> uint8 override;
  auto write(uint16 address, uint8 data) -> void override;

  auto in(uint16 address) -> uint8 override;
  auto out(uint16 address, uint8 data) -> void override;

  //serialization.cpp
  auto serialize(serializer&) -> void;

private:
  Memory::Writable<uint8> ram;

  struct State {
    bool nmiLine = 0;
    bool irqLine = 0;
  } state;
};

extern CPU cpu;
