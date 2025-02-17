PortConnector::PortConnector(View* parent) : Panel(parent, Size{~0, ~0}) {
  setCollapsible().setVisible(false);

  renameAction.setIcon(Icon::Application::TextEditor).setText("Rename ...").onActivate([&] { eventRename(); });
  removeAction.setIcon(Icon::Action::Remove).setText("Delete ...").onActivate([&] { eventRemove(); });

  locationLabel.setMouseCursor(MouseCursor::Hand);
  locationLabel.setFont(Font().setBold());
  locationLabel.setForegroundColor({0, 0, 240});
  locationLabel.onMousePress([&](auto button) {
    if(button == Mouse::Button::Left) eventBrowse();
  });
  peripheralList.onActivate([&] { eventActivate(); });
  peripheralList.onChange([&] { eventChange(); });
  peripheralList.onContext([&] { eventContext(); });
  importButton.setText("Import ...").onActivate([&] { eventImport(); });
  acceptButton.setText("Create").onActivate([&] { eventActivate(); });
}

auto PortConnector::show() -> void {
  importButton.setVisible(port->type == "Cartridge");
  setVisible(true);
}

auto PortConnector::hide() -> void {
  setVisible(false);
  port = {};
}

auto PortConnector::refresh(higan::Node::Port port) -> void {
  this->port = port;

  auto path = port->property("path");
  if(!path) path = {emulator.system.data, port->type, "/"};
  if(path.beginsWith(Path::user())) {
    path.trimLeft(Path::user(), 1L);
    path.prepend("~/");
  }
  locationLabel.setText(path);
  peripheralList.reset();
  ListViewItem item{&peripheralList};
  item.setProperty("type", "nothing");
  item.setIcon(Icon::Action::Remove).setText("Nothing");

  if(string location = {emulator.system.templates, port->type, "/"}) {
    for(auto& name : directory::folders(location)) {
      ListViewItem item{&peripheralList};
      item.setProperty("type", "template");
      item.setProperty("location", {location, name});
      item.setProperty("path", location);
      item.setProperty("name", name.trimRight("/", 1L));
      item.setIcon(Icon::Action::Add).setText(name);
    }
  }

  auto location = port->property("path");
  if(!location) location = {emulator.system.data, port->type, "/"};
  if(location) {
    for(auto& name : directory::folders(location)) {
      ListViewItem item{&peripheralList};
      item.setProperty("type", "peripheral");
      item.setProperty("location", {location, name});
      item.setProperty("path", location);
      item.setProperty("name", name.trimRight("/", 1L));
      item.setIcon(Icon::Emblem::Folder).setText(name);
    }
  }

  peripheralList.doChange();
}

auto PortConnector::eventImport() -> void {
  if(auto location = execute("icarus", "--system", interface->name(), "--import").output) {
    refresh(port);
  }
}

auto PortConnector::eventActivate() -> void {
  if(auto item = peripheralList.selected()) {
    if(!port->hotSwappable && emulator.system.power) {
      auto response = MessageDialog()
      .setText("The peripheral currently connected to this port isn't hot-swappable.\n"
               "Removing it anyway may crash the emulated system.\n"
               "What would you like to do?")
      .setTitle("Warning").setAlignment(programWindow).question({"Force", "Power Off", "Cancel"});
      if(response == "Cancel") return;
      if(response == "Power Off") emulator.power(false);
    }

    auto name = item.property("name");

    if(item.property("type") == "nothing") {
      port->disconnect();
      nodeManager.refresh();
    }

    else if(item.property("type") == "template") {
      auto label = NameDialog()
      .setTitle({"Create New ", name})
      .setAlignment(programWindow)
      .create(item.text());
      if(!label) return;

      auto source = item.property("location");
      auto target = string{emulator.system.data, port->type, "/", label, "/"};
      if(directory::exists(target)) return (void)MessageDialog()
      .setText("A directory by this name already exists.")
      .setTitle("Error").setAlignment(programWindow).error();

      if(directory::copy(source, target)) {
        auto peripheral = port->allocate();
        peripheral->name = name;
        peripheral->setProperty("location", target);
        peripheral->setProperty("name", label);
        port->connect(peripheral);
        inputManager.bind();  //bind any inputs this peripheral may contain
        nodeManager.refresh();
      }
    }

    else if(item.property("type") == "peripheral") {
      auto location = item.property("location");
      auto connected = emulator.connected(location);

      //treat selecting the already-connected device as a no-op that aborts the port connection dialog
      if(connected && connected == port) return programWindow.show(home);

      if(connected) return (void)MessageDialog()
      .setText({"This peripheral is already connected to another port:\n\n", connected->name})
      .setTitle("Error").setAlignment(programWindow).error();

      auto peripheral = port->allocate();
      if(auto markup = file::read({location, "settings.bml"})) {
        peripheral = higan::Node::unserialize(markup);
      }
      peripheral->setProperty("location", location);
      peripheral->setProperty("name", item.property("name"));
      port->connect(peripheral);
      inputManager.bind();  //bind any inputs this peripheral may contain
      nodeManager.refresh();
    }
  }
}

auto PortConnector::eventChange() -> void {
  acceptButton.setVisible(false);

  if(auto item = peripheralList.selected()) {
    if(item.property("type") == "nothing") {
      acceptButton.setText("Disconnect").setVisible();
    }

    else if(item.property("type") == "template") {
      acceptButton.setText("Create").setVisible();
    }

    else if(item.property("type") == "peripheral") {
      acceptButton.setText("Connect").setVisible();
    }
  }

  controlLayout.resize();
}

auto PortConnector::eventContext() -> void {
  if(auto item = peripheralList.selected()) {
    if(item.property("type") == "peripheral") {
      contextMenu.setVisible();
    }
  }
}

auto PortConnector::eventBrowse() -> void {
  auto path = port->property("path");
  if(!path) path = {emulator.system.data, port->type, "/"};
  if(auto location = BrowserDialog()
  .setTitle({port->name, " Location"})
  .setPath(path)
  .setAlignment(programWindow).selectFolder()
  ) {
    port->setProperty("path", location);
    refresh(port);
  }
}

auto PortConnector::eventRename() -> void {
  if(auto item = peripheralList.selected()) {
    if(item.property("type") == "peripheral") {
      auto location = item.property("location");
      auto path = item.property("path");
      auto name = item.property("name");
      if(auto rename = NameDialog()
      .setAlignment(programWindow)
      .rename(name)
      ) {
        if(name == rename) return;

        if(directory::exists({path, rename})) return (void)MessageDialog()
        .setText("A directory by this name already exists.")
        .setTitle("Error").setAlignment(programWindow).error();

        if(!directory::rename({path, name}, {path, rename})) return (void)MessageDialog()
        .setText("Failed to rename directory.")
        .setTitle("Error").setAlignment(programWindow).error();

        //the location must be updated if this peripheral is already connected ...
        if(auto connector = emulator.connected(location)) {
          if(auto connected = connector->connected()) {
            connected->setProperty("location", {path, rename, "/"});
            connected->setProperty("name", rename);
            //the name will be updated in the node manager, so it must be refreshed:
            nodeManager.refresh();
          }
        }

        return refresh(port);
      }
    }
  }
}

auto PortConnector::eventRemove() -> void {
  if(auto item = peripheralList.selected()) {
    if(item.property("type") == "peripheral") {
      auto location = item.property("location");
      auto connected = emulator.connected(location);

      if(!port->hotSwappable && emulator.system.power && emulator.connected(location)) return (void)MessageDialog()
      .setText({"This peripheral is not hot-swappable and is already connected to a port:\n\n", connected->name})
      .setTitle("Error").setAlignment(programWindow).error();

      if(MessageDialog()
      .setText("Are you really sure you want to delete this peripheral?\n"
               "All data will be permanently lost!")
      .setTitle("Warning").setAlignment(programWindow).question() == "No"
      ) return;

      //must disconnect device before removing it, as disconnect will attempt to save data to disk
      if(connected) {
        port->disconnect();
        nodeManager.refresh();
      }

      if(!directory::remove(location)) return (void)MessageDialog()
      .setText("Failed to remove directory.")
      .setTitle("Error").setAlignment(programWindow).error();

      refresh(port);
    }
  }
}
