struct System {
  Node::Object node;
  Node::Boolean fastBoot;

  struct Controls {
    Node::Object node;
    Node::Button up;
    Node::Button down;
    Node::Button left;
    Node::Button right;
    Node::Button a;
    Node::Button b;
    Node::Button option;
    Node::Button debug;
    Node::Button power;

    //controls.cpp
    auto load(Node::Object, Node::Object) -> void;
    auto poll() -> void;

    bool yHold = 0;
    bool upLatch = 0;
    bool downLatch = 0;
    bool xHold = 0;
    bool leftLatch = 0;
    bool rightLatch = 0;
  } controls;

  struct Video {
    Node::Video node;
    Node::Boolean interframeBlending;

    //video.cpp
    auto load(Node::Object, Node::Object) -> void;
    auto colorNeoGeoPocket(uint32) -> uint64;
    auto colorNeoGeoPocketColor(uint32) -> uint64;
  } video;

  enum class Model : uint { NeoGeoPocket, NeoGeoPocketColor };
  Memory::Readable<uint8> bios;

  inline auto model() const -> Model { return information.model; }
  inline auto frequency() const -> double { return 6'144'000; }

  //system.cpp
  auto run() -> void;
  auto runToSave() -> void;

  auto load(Node::Object) -> void;
  auto unload() -> void;
  auto save() -> void;
  auto power() -> void;

  //serialization.cpp
  auto serializeInit() -> void;
  auto serialize() -> serializer;
  auto unserialize(serializer&) -> bool;
  auto serializeAll(serializer&) -> void;
  auto serialize(serializer&) -> void;

private:
  struct Information {
    Model model = Model::NeoGeoPocket;
    natural serializeSize;
  } information;
};

extern System system;

auto Model::NeoGeoPocket() -> bool { return system.model() == System::Model::NeoGeoPocket; }
auto Model::NeoGeoPocketColor() -> bool { return system.model() == System::Model::NeoGeoPocketColor; }
