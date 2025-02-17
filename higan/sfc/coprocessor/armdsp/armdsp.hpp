//ARMv3 (ARM60)

//note: this coprocessor uses the ARMv4 (ARM7TDMI) core as its base
//instruction execution forces ARM mode to remove ARMv4 THUMB access
//there is a possibility the ARMv3 supports 26-bit mode; but cannot be verified

struct ArmDSP : ARM7TDMI, Thread {
  #include "registers.hpp"

  auto boot() -> void;
  auto main() -> void;

  auto step(uint clocks) -> void override;
  auto sleep() -> void override;
  auto get(uint mode, uint32 address) -> uint32 override;
  auto set(uint mode, uint32 address, uint32 word) -> void override;

  auto read(uint24 address, uint8 data) -> uint8;
  auto write(uint24 address, uint8 data) -> void;

  auto unload() -> void;
  auto power() -> void;
  auto reset() -> void;  //soft reset

  auto firmware() const -> vector<uint8>;
  auto serialize(serializer&) -> void;

  uint Frequency;

  uint8 programROM[128 * 1024];
  uint8 dataROM[32 * 1024];
  uint8 programRAM[16 * 1024];
};

extern ArmDSP armdsp;
