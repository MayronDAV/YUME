#pragma once

#include "YUME/Core/application.h"
#include "YUME/Core/layer.h"
#include "YUME/Core/log.h"

// Events
// --------------------------
#include "YUME/Events/event.h"
#include "YUME/Events/application_event.h"
#include "YUME/Events/key_event.h"
#include "YUME/Events/mouse_event.h"
// --------------------------

// Input
// --------------------------
#include "YUME/Core/input.h"
#include "YUME/Core/key_codes.h"
#include "YUME/Core/mouse_codes.h"
// --------------------------

// Utils
// --------------------------
#include "YUME/Core/timestep.h"
#include "YUME/Utils/timer.h"
#include "YUME/Utils/scoped_timer.h"
#include "YUME/Utils/clock.h"
// --------------------------

// Scene
// --------------------------
#include "YUME/Scene/scene.h"
#include "YUME/Scene/entity.h"
#include "YUME/Scene/entity_manager.h"
#include "YUME/Scene/Component/components.h"
#include "YUME/Scene/Component/components_2D.h"
#include "YUME/Scene/Component/components_3D.h"
// --------------------------

// Renderer
// --------------------------
#include "YUME/Renderer/graphics_context.h"
#include "YUME/Renderer/renderer_command.h"
#include "YUME/Renderer/renderer_api.h"
#include "YUME/Renderer/shader.h"
#include "YUME/Renderer/buffer.h"
#include "YUME/Renderer/uniform_buffer.h"
#include "YUME/Renderer/pipeline.h"
#include "YUME/Renderer/texture.h"
#include "YUME/Renderer/sub_texture.h"
#include "YUME/Renderer/texture_importer.h"
#include "YUME/Renderer/descriptor_set.h"
#include "YUME/Renderer/renderer.h"
#include "YUME/Renderer/camera.h"
#include "YUME/Renderer/model.h"
#include "YUME/Renderer/mesh.h"
// --------------------------