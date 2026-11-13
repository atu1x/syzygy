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
#include <cstdlib>
#include <variant>
#include <mutex>
#include <cstdint>
#include <thread>
#include <chrono>

namespace syzygy {
	struct syzvec2 { float x = 0.f, y = 0.f; };
	struct syzvec3 { float x = 0.f, y = 0.f, z = 0.f; };
	struct syzvec4 { float x = 0.f, y = 0.f, z = 0.f, w = 0.f; };

	//base
	enum class render_state_list { uninitialized, initializing, running, stopping, failed };
	enum class fail_reason_list {};
	enum class device_list { win, mac, linux_x11, linux_wayland, unknown };
	struct screen_context { syzvec4 resolution; syzvec4 field; float refresh_rate = 60.f; };

	//commands
	struct drawrect_context { syzvec4 field; uint32_t color; float opacity; float rounding; };

	enum class command_variants { rect };
	struct command_context { 
		command_variants variant = {};
		std::variant<drawrect_context> context = {};
	};

	//instance structure def

	struct instance_context {
		std::atomic<render_state_list> render_state{ render_state_list::uninitialized };
		std::vector<fail_reason_list> fail_reasons = {};
		device_list device = device_list::unknown;
		screen_context screen = {};

		//commands
		std::vector<command_context> command_list;
		std::mutex command_mutex = {};

		// cpu rendering canvas
		std::vector<uint32_t> pixels = {};
		uint32_t transparent_color = 0x00000000;

		void* overlay_window = nullptr; // win->HWND, linux->Window (cast)
		void* target_window = nullptr;

		std::thread render_thread = {};

		void(*callback)(instance_context& instance) = nullptr;

		std::chrono::steady_clock::time_point last_frame_time = std::chrono::steady_clock::now();
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

		//temporarily empty
		inline screen_context get_screen_information(instance_context& instance) {
			if (instance.device == device_list::win);
			else if (instance.device == device_list::mac);
			else if (instance.device == device_list::linux_x11);
			else if (instance.device == device_list::linux_wayland);
			else if (instance.device == device_list::unknown);
			return {};
		}

		inline void thread_handling(instance_context* instance) {
			if (!instance) return;

			// create_overlay(*instance);

			instance->render_state = render_state_list::running;

			while (instance->render_state == render_state_list::running) {
				//initialization of canvas / command list
				std::fill(instance->pixels.begin(), instance->pixels.end(), instance->transparent_color);
				{
					std::lock_guard<std::mutex> lock(instance->command_mutex);
					instance->command_list.clear();
				}

				//check callback
				if (instance->callback) {
					instance->callback(*instance);
				}
				
				//literally function name explains. no explanation needed.
				// handle_commands(*instance);
				// present(*instance);
				// vsync(*instance);
			}

			// destruction of window (when closed)
			// destroy_overlay(*instance);
		}
	}

	class engine {
	public:
		static instance_context* create_instance(void(*callback)(instance_context& instance), void* target_window = nullptr, bool use_custom_output = false) {
			if (!syzygy::database::initialized) syzygy::backend::setup();

			auto new_instance = std::make_unique<instance_context>();

			new_instance->render_state = render_state_list::initializing;
			
			//new_instance->fail_reasons.clear();
			if (use_custom_output) new_instance->device = device_list::unknown;
			else new_instance->device = syzygy::backend::get_device();
			//new_instance->screen = {};

			new_instance->callback = callback;
			new_instance->target_window = target_window;
			new_instance->screen = backend::get_screen_information(*new_instance);

			new_instance->render_thread = std::thread(backend::thread_handling, new_instance.get());

			syzygy::database::instances.push_back(std::move(new_instance));
			return syzygy::database::instances.back().get();
		}

		static void drawrect(instance_context& instance, syzvec4 field, uint32_t color, float opacity = 1.f, float rounding = 0.f) {
			std::lock_guard<std::mutex> lock(instance.command_mutex);
			instance.command_list.push_back({ command_variants::rect, drawrect_context{ field, color, opacity, rounding } });
		}

		static void destroy_instance(instance_context* instance) {
			if (!instance) return;

			instance->render_state = render_state_list::stopping;
			if (instance->render_thread.joinable()) {
				instance->render_thread.join();
			}

			std::erase_if(syzygy::database::instances, [instance](const auto& item) { return item.get() == instance; });
		}

		static void shutdown() {
			while (!syzygy::database::instances.empty()) {
				syzygy::engine::destroy_instance(syzygy::database::instances.back().get());
			}
			syzygy::database::initialized = false;
		}
	};
}
