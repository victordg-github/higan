auto System::Video::load(Node::Object parent, Node::Object from) -> void {
  node = Node::append<Node::Video>(parent, from, "Video");
  from = Node::scan(parent = node, from);

  if(MasterSystem::Model::MasterSystem()) {
    node->type    = "CRT";
    node->width   = 256;
    node->height  = 240;
    node->scaleX  = 1.0;
    node->scaleY  = 1.0;
    node->aspectX = 8.0;
    node->aspectY = 7.0;
    node->colors  = 1 << 6;
    node->color   = [&](auto index) { return colorMasterSystem(index); };
  }

  if(MasterSystem::Model::GameGear()) {
    node->type   = "LCD";
    node->width  = 160;
    node->height = 144;
    node->scaleX = 1.0;
    node->scaleY = 1.0;
    node->colors = 1 << 12;
    node->color  = [&](auto index) { return colorGameGear(index); };

    interframeBlending = Node::append<Node::Boolean>(parent, from, "Interframe Blending", true, [&](auto value) {
      vdp.screen.setInterframeBlending(value);
    });
    interframeBlending->dynamic = true;
  }
}

auto System::Video::colorMasterSystem(uint32 color) -> uint64 {
  uint2 B = color >> 4;
  uint2 G = color >> 2;
  uint2 R = color >> 0;

  uint64 r = image::normalize(R, 2, 16);
  uint64 g = image::normalize(G, 2, 16);
  uint64 b = image::normalize(B, 2, 16);

  return r << 32 | g << 16 | b << 0;
}

auto System::Video::colorGameGear(uint32 color) -> uint64 {
  uint4 B = color >> 8;
  uint4 G = color >> 4;
  uint4 R = color >> 0;

  uint64 r = image::normalize(R, 4, 16);
  uint64 g = image::normalize(G, 4, 16);
  uint64 b = image::normalize(B, 4, 16);

  return r << 32 | g << 16 | b << 0;
}
