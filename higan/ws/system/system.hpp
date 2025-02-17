struct System : IO {
  Node::Object node;

  enum class SoC : uint {
    ASWAN,
    SPHINX,
    SPHINX2,
  };

  enum class Model : uint {
    WonderSwan,
    WonderSwanColor,
    SwanCrystal,
    PocketChallengeV2,
    MamaMitte,
  };

  struct Controls {
    Node::Object node;

    //WonderSwan, WonderSwan Color, SwanCrystal, mamaMitte
    Node::Button y1;
    Node::Button y2;
    Node::Button y3;
    Node::Button y4;
    Node::Button x1;
    Node::Button x2;
    Node::Button x3;
    Node::Button x4;
    Node::Button b;
    Node::Button a;
    Node::Button start;
    Node::Button volume;

    //Pocket Challenge V2
    Node::Button up;
    Node::Button down;
    Node::Button left;
    Node::Button right;
    Node::Button pass;
    Node::Button circle;
    Node::Button clear;
    Node::Button view;
    Node::Button escape;

    //all models
    Node::Button power;

    //controls.cpp
    auto load(Node::Object, Node::Object) -> void;
    auto poll() -> void;

    bool xHold = 0;
    bool leftLatch = 0;
    bool rightLatch = 0;
  } controls;

  struct Video {
    Node::Video node;
    Node::Boolean colorEmulation;
    Node::Boolean interframeBlending;
    Node::String orientation;
    Node::Boolean showIcons;

    //video.cpp
    auto load(Node::Object, Node::Object) -> void;
    auto color(uint32) -> uint64;
  } video;

  struct Audio {
    Node::Audio node;
    Node::Boolean headphones;

    //audio.cpp
    auto load(Node::Object, Node::Object) -> void;
  } audio;

  inline auto abstract() const -> bool { return information.abstract; }
  inline auto model() const -> Model { return information.model; }
  inline auto soc() const -> SoC { return information.soc; }

  inline auto color() const -> bool { return io.color; }
  inline auto planar() const -> bool { return io.format == 0; }
  inline auto packed() const -> bool { return io.format == 1; }
  inline auto depth() const -> bool { return io.color && io.depth; }

  //system.cpp
  auto run() -> void;
  auto runToSave() -> void;

  auto load(Node::Object) -> void;
  auto unload() -> void;
  auto save() -> void;
  auto power() -> void;

  //io.cpp
  auto portRead(uint16 address) -> uint8 override;
  auto portWrite(uint16 address, uint8 data) -> void override;

  //serialization.cpp
  auto serializeInit() -> void;
  auto serialize() -> serializer;
  auto unserialize(serializer&) -> bool;
  auto serializeAll(serializer&) -> void;
  auto serialize(serializer&) -> void;

  struct Information {
    bool abstract = false;
    SoC soc = SoC::ASWAN;
    Model model = Model::WonderSwan;
    uint serializeSize = 0;
  } information;

  Memory::Readable<uint8> bootROM;
  EEPROM eeprom;

private:
  struct Registers {
    //$0060  DISP_MODE
    uint1 unknown0;
    uint1 unknown1;
    uint1 unknown3;
    uint1 format;
    uint1 depth;
    uint1 color;
  } io;
};

extern System system;

auto SoC::ASWAN() -> bool { return system.soc() == System::SoC::ASWAN; }
auto SoC::SPHINX() -> bool { return system.soc() == System::SoC::SPHINX; }
auto SoC::SPHINX2() -> bool { return system.soc() == System::SoC::SPHINX2; }

auto Model::WonderSwan() -> bool { return system.model() == System::Model::WonderSwan; }
auto Model::WonderSwanColor() -> bool { return system.model() == System::Model::WonderSwanColor; }
auto Model::SwanCrystal() -> bool { return system.model() == System::Model::SwanCrystal; }
auto Model::PocketChallengeV2() -> bool { return system.model() == System::Model::PocketChallengeV2; }
auto Model::MamaMitte() -> bool { return system.model() == System::Model::MamaMitte; }
