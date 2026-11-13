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
#include <algorithm>
#include <string>
#include <cmath>

//forward declaration of core part.
namespace syzygy {
	namespace standard {
		struct screen_context;
		struct instance_context;

		namespace core {
			namespace win {
				inline screen_context get_screen_information(instance_context& context);
				inline bool create_overlay(instance_context& context);
				inline bool destroy_overlay(instance_context& context);
				inline void present(instance_context& context);
				inline void vsync(instance_context& context);
			}
			namespace mac {
				inline screen_context get_screen_information(instance_context& context);
				inline bool create_overlay(instance_context& context);
				inline bool destroy_overlay(instance_context& context);
				inline void present(instance_context& context);
				inline void vsync(instance_context& context);
			}
			namespace linux_x11 {
				inline screen_context get_screen_information(instance_context& context);
				inline bool create_overlay(instance_context& context);
				inline bool destroy_overlay(instance_context& context);
				inline void present(instance_context& context);
				inline void vsync(instance_context& context);
			}
			namespace linux_wayland {
				inline screen_context get_screen_information(instance_context& context);
				inline bool create_overlay(instance_context& context);
				inline bool destroy_overlay(instance_context& context);
				inline void present(instance_context& context);
				inline void vsync(instance_context& context);
			}
			namespace unknown {
				inline screen_context get_screen_information(instance_context& context);
				inline bool create_overlay(instance_context& context);
				inline bool destroy_overlay(instance_context& context);
				inline void present(instance_context& context);
				inline void vsync(instance_context& context);
			}
		}
	}
}

namespace syzygy {
	namespace standard {
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

		namespace logger {
			inline std::vector<std::string> history;
			inline void send(const std::string& string) {
				history.push_back(string);
			}
		}

		namespace database {
			inline std::vector<std::unique_ptr<instance_context>> instances = {};
			inline std::atomic<bool> initialized{ false };
		}

		namespace backend {
			inline void setup() {
				syzygy::standard::database::instances.clear();
				syzygy::standard::database::initialized = true;
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

			inline screen_context get_screen_information(instance_context& instance) {
				if (instance.device == device_list::win) return syzygy::standard::core::win::get_screen_information(instance);
				else if (instance.device == device_list::mac) return syzygy::standard::core::mac::get_screen_information(instance);
				else if (instance.device == device_list::linux_x11) return syzygy::standard::core::linux_x11::get_screen_information(instance);
				else if (instance.device == device_list::linux_wayland) return syzygy::standard::core::linux_wayland::get_screen_information(instance);
				else return syzygy::standard::core::unknown::get_screen_information(instance);
			}

			inline bool create_overlay(instance_context& instance) {
				if (instance.device == device_list::win) return syzygy::standard::core::win::create_overlay(instance);
				else if (instance.device == device_list::mac) return syzygy::standard::core::mac::create_overlay(instance);
				else if (instance.device == device_list::linux_x11) return syzygy::standard::core::linux_x11::create_overlay(instance);
				else if (instance.device == device_list::linux_wayland) return syzygy::standard::core::linux_wayland::create_overlay(instance);
				else return syzygy::standard::core::unknown::create_overlay(instance);
			}

			inline bool destroy_overlay(instance_context& instance) {
				if (instance.device == device_list::win) return syzygy::standard::core::win::destroy_overlay(instance);
				else if (instance.device == device_list::mac) return syzygy::standard::core::mac::destroy_overlay(instance);
				else if (instance.device == device_list::linux_x11) return syzygy::standard::core::linux_x11::destroy_overlay(instance);
				else if (instance.device == device_list::linux_wayland) return syzygy::standard::core::linux_wayland::destroy_overlay(instance);
				else return syzygy::standard::core::unknown::destroy_overlay(instance);
			}

			inline void present(instance_context& instance) {
				if (instance.device == device_list::win) syzygy::standard::core::win::present(instance);
				else if (instance.device == device_list::mac) syzygy::standard::core::mac::present(instance);
				else if (instance.device == device_list::linux_x11) syzygy::standard::core::linux_x11::present(instance);
				else if (instance.device == device_list::linux_wayland) syzygy::standard::core::linux_wayland::present(instance);
				else syzygy::standard::core::unknown::present(instance);
			}

			inline void vsync(instance_context& instance) {
				if (instance.device == device_list::win) syzygy::standard::core::win::vsync(instance);
				else if (instance.device == device_list::mac) syzygy::standard::core::mac::vsync(instance);
				else if (instance.device == device_list::linux_x11) syzygy::standard::core::linux_x11::vsync(instance);
				else if (instance.device == device_list::linux_wayland) syzygy::standard::core::linux_wayland::vsync(instance);
				else syzygy::standard::core::unknown::vsync(instance);
			}

			inline void internal_drawrect(instance_context& instance, const drawrect_context& rect) {
				int canvas_w = static_cast<int>(instance.screen.field.z - instance.screen.field.x);
				int canvas_h = static_cast<int>(instance.screen.field.w - instance.screen.field.y);
				if (canvas_w <= 0 || canvas_h <= 0) return;

				int start_x = std::clamp(static_cast<int>(rect.field.x), 0, canvas_w);
				int start_y = std::clamp(static_cast<int>(rect.field.y), 0, canvas_h);
				int end_x = std::clamp(static_cast<int>(rect.field.z), 0, canvas_w);
				int end_y = std::clamp(static_cast<int>(rect.field.w), 0, canvas_h);

				for (int y = start_y; y < end_y; ++y) {
					for (int x = start_x; x < end_x; ++x) {
						instance.pixels[y * canvas_w + x] = rect.color;
					}
				}
			}

			inline void accomplish_command_request(instance_context& instance) {
				std::lock_guard<std::mutex> lock(instance.command_mutex);
				for (const auto& cmd : instance.command_list) {
					if (cmd.variant == command_variants::rect) {
						syzygy::standard::backend::internal_drawrect(instance, std::get<drawrect_context>(cmd.context));
					}
				}
			}

			inline void thread_handling(instance_context* instance) {
				if (!instance) return;

				// create_overlay(*instance);

				instance->render_state = render_state_list::running;

				while (instance->render_state == render_state_list::running) {
					//initialization of canvas / command list

					//resize / pixel buffer
					int canvas_w = static_cast<int>(instance->screen.field.z - instance->screen.field.x);
					int canvas_h = static_cast<int>(instance->screen.field.w - instance->screen.field.y);
					size_t required_size = static_cast<size_t>(std::max(0, canvas_w) * std::max(0, canvas_h));

					if (instance->pixels.size() != required_size) {
						instance->pixels.resize(required_size);
					}

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
					syzygy::standard::backend::accomplish_command_request(*instance);
					//present(*instance);
					//vsync(*instance);
				}

				//destruction of window (when closed)
				//destroy_overlay(*instance);
			}
		}

		class engine {
		public:
			static instance_context* create_instance(void(*callback)(instance_context& instance), void* target_window = nullptr, bool use_custom_output = false) {
				if (!syzygy::standard::database::initialized) syzygy::standard::backend::setup();

				auto new_instance = std::make_unique<instance_context>();

				new_instance->render_state = render_state_list::initializing;

				//new_instance->fail_reasons.clear();
				if (use_custom_output) new_instance->device = device_list::unknown;
				else new_instance->device = syzygy::standard::backend::get_device();
				//new_instance->screen = {};

				new_instance->callback = callback;
				new_instance->target_window = target_window;
				new_instance->screen = backend::get_screen_information(*new_instance);

				new_instance->render_thread = std::thread(backend::thread_handling, new_instance.get());

				syzygy::standard::database::instances.push_back(std::move(new_instance));
				return syzygy::standard::database::instances.back().get();
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

				std::erase_if(syzygy::standard::database::instances, [instance](const auto& item) { return item.get() == instance; });
			}

			static void shutdown() {
				while (!syzygy::standard::database::instances.empty()) {
					syzygy::standard::engine::destroy_instance(syzygy::standard::database::instances.back().get());
				}
				syzygy::standard::database::initialized = false;
			}
		};
	}
}

inline syzygy::standard::screen_context syzygy::standard::core::win::get_screen_information(instance_context& context) { return {}; }
inline bool syzygy::standard::core::win::create_overlay(instance_context& context) { return false; }
inline bool syzygy::standard::core::win::destroy_overlay(instance_context& context) { return false; }
inline void syzygy::standard::core::win::present(instance_context& context) {}
inline void syzygy::standard::core::win::vsync(instance_context& context) {}

inline syzygy::standard::screen_context syzygy::standard::core::mac::get_screen_information(instance_context& context) { return {}; }
inline bool syzygy::standard::core::mac::create_overlay(instance_context& context) { return false; }
inline bool syzygy::standard::core::mac::destroy_overlay(instance_context& context) { return false; }
inline void syzygy::standard::core::mac::present(instance_context& context) {}
inline void syzygy::standard::core::mac::vsync(instance_context& context) {}

inline syzygy::standard::screen_context syzygy::standard::core::linux_x11::get_screen_information(instance_context& context) { return {}; }
inline bool syzygy::standard::core::linux_x11::create_overlay(instance_context& context) { return false; }
inline bool syzygy::standard::core::linux_x11::destroy_overlay(instance_context& context) { return false; }
inline void syzygy::standard::core::linux_x11::present(instance_context& context) {}
inline void syzygy::standard::core::linux_x11::vsync(instance_context& context) {}

inline syzygy::standard::screen_context syzygy::standard::core::linux_wayland::get_screen_information(instance_context& context) { return {}; }
inline bool syzygy::standard::core::linux_wayland::create_overlay(instance_context& context) { return false; }
inline bool syzygy::standard::core::linux_wayland::destroy_overlay(instance_context& context) { return false; }
inline void syzygy::standard::core::linux_wayland::present(instance_context& context) {}
inline void syzygy::standard::core::linux_wayland::vsync(instance_context& context) {}

inline syzygy::standard::screen_context syzygy::standard::core::unknown::get_screen_information(instance_context& context) { return {}; }
inline bool syzygy::standard::core::unknown::create_overlay(instance_context& context) { return false; }
inline bool syzygy::standard::core::unknown::destroy_overlay(instance_context& context) { return false; }
inline void syzygy::standard::core::unknown::present(instance_context& context) {}
inline void syzygy::standard::core::unknown::vsync(instance_context& context) {
	float fps = context.screen.refresh_rate > 0.f ? context.screen.refresh_rate : 60.f;
	auto frame_duration = std::chrono::duration< float, std::milli >(1000.f / fps);
	auto next_frame = context.last_frame_time + std::chrono::duration_cast<std::chrono::steady_clock::duration>(frame_duration);

	std::this_thread::sleep_until(next_frame);
	context.last_frame_time = std::chrono::steady_clock::now();
}
