## Features
- [x] Add a reference counting system.
- [x] Add a Event system.
- [x] Add Window.
- [x] Add Vulkan.
- [x] Add ImGui Vulkan.
- [ ] Send each event to a queue instead of dispatching it immediately.
- [ ] Create a configuration manager and configuration file and add an option for the API.

- ### Vulkan
- [x] Create a delete queue.
- [x] Add a way to create and configurate a descriptor layout.

## Bugs:
- [ ] Fix and test project build on Linux.
- [ ] For whatever reason, the python requests module doesn't work on my Linux distribution.
- [ ] Custom reference doesn't work. Fix this later.

- ### Vulkan
- [ ] Fix vulkan error messages.
- [x] There is currently a bug where only the first draw call is being drawn, fix this later.