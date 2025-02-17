namespace higan {

Audio audio;
#include "stream.cpp"

auto Audio::reset(Interface* interface) -> void {
  this->interface = interface;
  streams.reset();
  channels = 0;
}

auto Audio::setFrequency(double frequency) -> void {
  this->frequency = frequency;
  for(auto& stream : streams) {
    stream->setFrequency(stream->inputFrequency, frequency);
  }
}

auto Audio::setVolume(double volume) -> void {
  this->volume = volume;
}

auto Audio::setBalance(double balance) -> void {
  this->balance = balance;
}

auto Audio::append(Stream& stream) -> void {
  if(streams.find(&stream)) return;
  streams.append(&stream);
  synchronize();
}

auto Audio::remove(Stream& stream) -> void {
  streams.removeByValue(&stream);
  synchronize();
}

auto Audio::synchronize() -> void {
  uint channels = 0;
  for(auto& stream : streams) {
    channels = max(channels, stream->channels.size());
  }
  this->channels = channels;
}

auto Audio::process() -> void {
  while(streams) {
    for(auto& stream : streams) {
      if(!stream->pending()) return;
    }

    double samples[channels];
    for(auto& sample : samples) sample = 0.0;

    for(auto& stream : streams) {
      double buffer[channels];
      uint length = stream->read(buffer), offset = 0;

      for(auto& sample : samples) {
        sample += buffer[offset];
        if(++offset >= length) offset = 0;
      }
    }

    for(auto c : range(channels)) {
      samples[c] = max(-1.0, min(+1.0, samples[c] * volume));
    }

    if(channels == 2) {
      if(balance < 0.0) samples[1] *= 1.0 + balance;
      if(balance > 0.0) samples[0] *= 1.0 - balance;
    }

    platform->audio(nullptr, samples, channels);
  }
}

}
