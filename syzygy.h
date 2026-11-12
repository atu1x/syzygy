/*
 * ┏━┓╻ ╻╺━┓╻ ╻┏━╸╻ ╻
 * ┗━┓┗┳┛┏━┛┗┳┛┃╺┓┗┳┛
 * ┗━┛ ╹ ┗━╸ ╹ ┗━┛ ╹
 * syzygy - A C++ 2D rendering library with cross-platform support.
 * Copyright (C) 2026 - atu1x
 */
#pragma once
#include <vector>
#include <atomic>
#include <memory>

namespace syzygy {
	struct syzvec2 { float x = 0.f, y = 0.f; };
	struct syzvec3 { float x = 0.f, y = 0.f, z = 0.f; };
	struct syzvec4 { float x = 0.f, y = 0.f, z = 0.f, w = 0.f; };

	enum class render_state_list { uninitialized, initializing, running, stopping, failed };
	enum class fail_reason_list {};
	enum class device_list { win, mac, linux_x11, linux_wayland, unknown };
	struct screen_context { syzvec4 resolution; syzvec4 field; float refresh_hz	= 60.f; };

	struct instance_context {
		render_state_list render_state = render_state_list::uninitialized;
		std::vector<fail_reason_list> fail_reasons = {};
		device_list device = device_list::unknown;
		screen_context screen = {};
	};

	namespace database {
		inline std::vector<std::unique_ptr<instance_context>> instances = {};
		inline std::atomic<bool> initialized{ false };
	}

	namespace backend {
		inline void setup() {
			syzygy::database::instances.clear();
			syzygy::database::initialized = true;
		}

		inline device_list get_device() {
#if defined(_WIN32) || defined(_WIN64)
			return device_list::win;
#elif defined(__APPLE__) && defined(__MACH__)
			return device_list::mac;
#elif defined(__linux__)
			if (std::getenv("WAYLAND_DISPLAY") != nullptr) {
				return device_list::linux_wayland;
			}
			return device_list::linux_x11;
#else
			return device_list::unknown;
#endif
		}
	}

	class engine {
	public:
		static instance_context* create_instance() {
			if (!syzygy::database::initialized) syzygy::backend::setup();

			auto new_instance = std::make_unique<instance_context>();

			new_instance->render_state = render_state_list::initializing;
			//new_instance->fail_reasons.clear();
			new_instance->device = syzygy::backend::get_device();
			//new_instance->screen = {};

			syzygy::database::instances.push_back(std::move(new_instance));
			return syzygy::database::instances.back().get();
		}

		static void destroy_instance(instance_context* instance) {
			std::erase_if(database::instances, [instance](const auto& item) { return item.get() == instance; });
		}

		static void shutdown() {
			syzygy::database::instances.clear();
			syzygy::database::initialized = false;
		}
	};
}
