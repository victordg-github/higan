#include <cv/cv.hpp>

namespace higan::ColecoVision {

Scheduler scheduler;
System system;
#include "video.cpp"
#include "controls.cpp"
#include "serialization.cpp"

auto System::run() -> void {
  if(scheduler.enter() == Event::Frame) {
    vdp.refresh();
    controls.poll();
  }
}

auto System::runToSave() -> void {
  scheduler.enter(Scheduler::Mode::Serialize);
}

auto System::load(Node::Object from) -> void {
  if(node) unload();

  information = {};
  if(interface->name() == "ColecoVision") information.model = Model::ColecoVision;
  if(interface->name() == "ColecoAdam"  ) information.model = Model::ColecoAdam;

  higan::video.reset(interface);
  higan::audio.reset(interface);

  node = Node::append<Node::System>(nullptr, from, interface->name());

  regionNode = Node::append<Node::String>(node, from, "Region", "NTSC → PAL");
  regionNode->setAllowedValues({
    "NTSC → PAL",
    "PAL → NTSC",
    "NTSC",
    "PAL"
  });

  scheduler.reset();
  controls.load(node, from);
  video.load(node, from);
  vdp.load(node, from);
  cartridge.load(node, from);
  controllerPort1.load(node, from);
  controllerPort2.load(node, from);
}

auto System::save() -> void {
  if(!node) return;
  cartridge.save();
}

auto System::unload() -> void {
  if(!node) return;
  save();
  cartridge.port = {};
  controllerPort1.port = {};
  controllerPort2.port = {};
  node = {};
}

auto System::power() -> void {
  for(auto& setting : node->find<Node::Setting>()) setting->setLatch();

  auto setRegion = [&](string region) {
    if(region == "NTSC") {
      information.region = Region::NTSC;
      information.colorburst = Constants::Colorburst::NTSC;
    }
    if(region == "PAL") {
      information.region = Region::PAL;
      information.colorburst = Constants::Colorburst::PAL * 4.0 / 5.0;
    }
  };
  auto regions = regionNode->latch().split("→").strip();
  setRegion(regions.first());
  for(auto& requested : reverse(regions)) {
    if(requested == cartridge.region()) setRegion(requested);
  }

  if(auto fp = platform->open(node, "bios.rom", File::Read, File::Required)) {
    fp->read(bios, 0x2000);
  }

  cartridge.power();
  cpu.power();
  vdp.power();
  psg.power();
  scheduler.power(cpu);

  serializeInit();
}

}
