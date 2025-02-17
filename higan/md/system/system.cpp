#include <md/md.hpp>

namespace higan::MegaDrive {

Random random;
Scheduler scheduler;
System system;
#include "controls.cpp"
#include "video.cpp"
#include "serialization.cpp"

auto System::run() -> void {
  if(scheduler.enter() == Event::Frame) vdp.refresh();

  auto reset = controls.reset->value;
  controls.poll();
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

  tmss = Node::append<Node::Boolean>(node, from, "TMSS", false);

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
  vdp.load(node, from);
  cartridge.load(node, from);
  expansion.load(node, from);
  controllerPort1.load(node, from);
  controllerPort2.load(node, from);
  extensionPort.load(node, from);
}

auto System::unload() -> void {
  if(!node) return;
  save();
  cartridge.port = {};
  expansion.port = {};
  controllerPort1.port = {};
  controllerPort2.port = {};
  extensionPort.port = {};
  vdp.unload();
  mcd.unload();
  node = {};
}

auto System::save() -> void {
  if(!node) return;
  cartridge.save();
  expansion.save();
}

auto System::power(bool reset) -> void {
  for(auto& setting : node->find<Node::Setting>()) setting->setLatch();

  auto setRegion = [&](string region) {
    if(region == "NTSC-J") {
      information.region = Region::NTSCJ;
      information.frequency = Constants::Colorburst::NTSC * 15.0;
    }
    if(region == "NTSC-U") {
      information.region = Region::NTSCU;
      information.frequency = Constants::Colorburst::NTSC * 15.0;
    }
    if(region == "PAL") {
      information.region = Region::PAL;
      information.frequency = Constants::Colorburst::PAL * 12.0;
    }
  };
  auto regionsHave = regionNode->latch().split("→").strip();
  auto regionsWant = cartridge.bootable() ? cartridge.regions() : expansion.regions();
  setRegion(regionsHave.first());
  for(auto& have : reverse(regionsHave)) {
    for(auto& want : reverse(regionsWant)) {
      if(have == want) setRegion(have);
    }
  }
  information.megaCD = (bool)expansion.node;

  random.entropy(Random::Entropy::Low);

  cartridge.power();
  expansion.power();
  cpu.power(reset);
  apu.power(reset);
  vdp.power(reset);
  psg.power(reset);
  ym2612.power(reset);
  if(MegaCD()) mcd.power(reset);
  scheduler.power(cpu);

  serializeInit();
}

}
