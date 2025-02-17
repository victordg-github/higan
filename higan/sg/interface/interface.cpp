#include <sg/sg.hpp>

namespace higan::SG1000 {

Interface* interface = nullptr;

auto AbstractInterface::game() -> string {
  if(cartridge.node) {
    return cartridge.name();
  }

  return "(no cartridge connected)";
}

auto AbstractInterface::root() -> Node::Object {
  return system.node;
}

auto AbstractInterface::load(string tree) -> void {
  interface = this;
  system.load(Node::unserialize(tree));
}

auto AbstractInterface::power() -> void {
  system.power();
}

auto AbstractInterface::run() -> void {
  system.run();
}

auto AbstractInterface::save() -> void {
  system.save();
}

auto AbstractInterface::unload() -> void {
  save();
  system.unload();
}

auto AbstractInterface::serialize() -> serializer {
  system.runToSave();
  return system.serialize();
}

auto AbstractInterface::unserialize(serializer& s) -> bool {
  return system.unserialize(s);
}

}
