#include <fc/fc.hpp>

namespace higan::Famicom {

Random random;
Scheduler scheduler;
System system;
#include "controls.cpp"
#include "video.cpp"
#include "serialization.cpp"

auto System::run() -> void {
  if(scheduler.enter() == Event::Frame) ppu.refresh();

  auto reset = controls.reset->value;
  platform->input(controls.reset);
  if(!reset && controls.reset->value) power(true);
}

auto System::runToSave() -> void {
  scheduler.enter(Scheduler::Mode::Serialize);
}

auto System::load(Node::Object from) -> void {
  if(node) unload();

  information = {};

  higan::video.reset(interface);
  higan::audio.reset(interface);

  node = Node::append<Node::System>(nullptr, from, interface->name());

  regionNode = Node::append<Node::String>(node, from, "Region", "NTSC-J → NTSC-U → PAL");
  regionNode->setAllowedValues({
    "NTSC-J → NTSC-U → PAL",
    "NTSC-U → NTSC-J → PAL",
    "PAL → NTSC-J → NTSC-U",
    "PAL → NTSC-U → NTSC-J",
    "NTSC-J",
    "NTSC-U",
    "PAL"
  });

  scheduler.reset();
  controls.load(node, from);
  video.load(node, from);
  ppu.load(node, from);
  cartridge.load(node, from);
  controllerPort1.load(node, from);
  controllerPort2.load(node, from);
}

auto System::unload() -> void {
  if(!node) return;
  save();
  cartridge.unload();
  controllerPort1.port = {};
  controllerPort2.port = {};
  ppu.unload();
  node = {};
}

auto System::save() -> void {
  if(!node) return;
  cartridge.save();
}

auto System::power(bool reset) -> void {
  for(auto& setting : node->find<Node::Setting>()) setting->setLatch();

  auto setRegion = [&](string region) {
    if(region == "NTSC-J") {
      information.region = Region::NTSCJ;
      information.frequency = Constants::Colorburst::NTSC * 6.0;
    }
    if(region == "NTSC-U") {
      information.region = Region::NTSCU;
      information.frequency = Constants::Colorburst::NTSC * 6.0;
    }
    if(region == "PAL") {
      information.region = Region::PAL;
      information.frequency = Constants::Colorburst::PAL * 6.0;
    }
  };
  auto regionsHave = regionNode->latch().split("→").strip();
  setRegion(regionsHave.first());
  for(auto& have : reverse(regionsHave)) {
    if(have == cartridge.region()) setRegion(have);
  }

  random.entropy(Random::Entropy::Low);

  cartridge.power();
  cpu.power(reset);
  apu.power(reset);
  ppu.power(reset);
  scheduler.power(cpu);

  serializeInit();
}

}
