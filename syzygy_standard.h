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
#include <fstream>
#include <unordered_map>

//forward declaration of core part.
namespace syzygy {
	namespace standard {
		struct screen_context;
		struct instance_context;
		struct drawtext_context;
		struct text_bitmap_context;

		namespace core {
			namespace win {
				inline screen_context get_screen_information(instance_context& context);
				inline bool create_overlay(instance_context& context);
				inline bool destroy_overlay(instance_context& context);
				inline void present(instance_context& context);
				inline void vsync(instance_context& context);
				inline text_bitmap_context rasterize_text(instance_context& context, const drawtext_context& argument);
			}
			namespace mac {
				inline screen_context get_screen_information(instance_context& context);
				inline bool create_overlay(instance_context& context);
				inline bool destroy_overlay(instance_context& context);
				inline void present(instance_context& context);
				inline void vsync(instance_context& context);
				inline text_bitmap_context rasterize_text(instance_context& context, const drawtext_context& argument);
			}
			namespace linux_x11 {
				inline screen_context get_screen_information(instance_context& context);
				inline bool create_overlay(instance_context& context);
				inline bool destroy_overlay(instance_context& context);
				inline void present(instance_context& context);
				inline void vsync(instance_context& context);
				inline text_bitmap_context rasterize_text(instance_context& context, const drawtext_context& argument);
			}
			namespace linux_wayland {
				inline screen_context get_screen_information(instance_context& context);
				inline bool create_overlay(instance_context& context);
				inline bool destroy_overlay(instance_context& context);
				inline void present(instance_context& context);
				inline void vsync(instance_context& context);
				inline text_bitmap_context rasterize_text(instance_context& context, const drawtext_context& argument);
			}
			namespace unknown {
				inline screen_context get_screen_information(instance_context& context);
				inline bool create_overlay(instance_context& context);
				inline bool destroy_overlay(instance_context& context);
				inline void present(instance_context& context);
				inline void vsync(instance_context& context);
				inline text_bitmap_context rasterize_text(instance_context& context, const drawtext_context& argument);
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
		enum class fail_reason_list { unsupported_device, invalid_target_window, failed_obtainment_of_screen_info, failed_overlay_creation, failed_destruction_of_overlay, invalid_callback };
		enum class device_list { win, mac, linux_x11, linux_wayland, unknown };
		struct screen_context { syzvec4 resolution; syzvec4 field; float refresh_rate = 60.f; };

		//commands
		struct drawrect_context { syzvec4 field; uint32_t color; float opacity; float rounding; };
		struct drawtext_context { syzvec2 position; std::string string; uint32_t color; float opacity; float textsize; };

		//fonts (commands)
		struct pushfont_context { const std::vector<unsigned char>* font = nullptr; };
		struct popfont_context {};

		//bitmap
		struct text_bitmap_context { int width = 0; int height = 0; std::vector<uint8_t> mask = {}; };
		
		enum class command_variants { rect, text, pushfont, popfont };
		struct command_context {
			command_variants variant = {};
			std::variant<drawrect_context, drawtext_context, pushfont_context, popfont_context> context = {};
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

			//fonts
			std::vector<const std::vector<unsigned char>*> font_stack = {};
			std::unordered_map<std::string, text_bitmap_context> text_cache = {};

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
			inline std::mutex log_mutex = {};

			inline void send(const std::string& message) {
				std::lock_guard<std::mutex> lock(log_mutex);
				history.push_back(message);
			}

			inline void report_fail(instance_context& instance, fail_reason_list reason, const std::string& message) {
				instance.fail_reasons.push_back(reason);
				instance.render_state = render_state_list::failed;
				send(message);
			}
		}

		namespace database {
			inline std::vector<std::unique_ptr<instance_context>> instances = {};
			inline std::unordered_map<std::string, std::vector<unsigned char>> fonts = {};
			inline std::mutex font_mutex = {};
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

			inline text_bitmap_context rasterize_text(instance_context& instance, const drawtext_context& argument) {
				if (instance.device == device_list::win) return syzygy::standard::core::win::rasterize_text(instance, argument);
				else if (instance.device == device_list::mac) return syzygy::standard::core::mac::rasterize_text(instance, argument);
				else if (instance.device == device_list::linux_x11) return syzygy::standard::core::linux_x11::rasterize_text(instance, argument);
				else if (instance.device == device_list::linux_wayland) return syzygy::standard::core::linux_wayland::rasterize_text(instance, argument);
				else return syzygy::standard::core::unknown::rasterize_text(instance, argument);
			}

			//literally it is in charge of opacity
			inline uint32_t accomplish_opacity(uint32_t bg, uint32_t fg, float opacity) {
				uint32_t alpha = static_cast<uint32_t>(((fg >> 24) & 0xFF) * std::clamp(opacity, 0.f, 1.f));
				if (alpha == 0) return bg;
				if (alpha == 255) return fg | 0xFF000000u;

				uint32_t inv_alpha = 256 - alpha;
				uint32_t fg_full = fg | 0xFF000000u;
				uint32_t rb = ((((fg_full & 0x00FF00FFu) * alpha) + ((bg & 0x00FF00FFu) * inv_alpha)) >> 8) & 0x00FF00FFu;
				uint32_t ag = (((((fg_full >> 8) & 0x00FF00FFu) * alpha) + (((bg >> 8) & 0x00FF00FFu) * inv_alpha))) & 0xFF00FF00u;
				return rb | ag;
			}

			inline float get_corner_alpha(int x, int y, const syzvec4& field, float r) {
				if (r <= 0.f) return 1.f;
				float px = x + 0.5f;
				float py = y + 0.5f;
				float dx = std::max(field.x + r - px, std::max(0.f, px - (field.z - r)));
				float dy = std::max(field.y + r - py, std::max(0.f, py - (field.w - r)));
				if (dx <= 0.f || dy <= 0.f) return 1.f;

				float dist_sq = dx * dx + dy * dy;
				if (dist_sq >= (r + 0.5f) * (r + 0.5f)) return 0.f;
				if (dist_sq <= (r - 0.5f) * (r - 0.5f)) return 1.f;
				return r + 0.5f - std::sqrt(dist_sq);
			}

			inline void internal_drawrect(instance_context& instance, const drawrect_context& argument) {
				int canvas_w = static_cast<int>(instance.screen.field.z - instance.screen.field.x);
				int canvas_h = static_cast<int>(instance.screen.field.w - instance.screen.field.y);
				if (canvas_w <= 0 || canvas_h <= 0 || argument.opacity <= 0.f) return;

				int start_x = std::clamp(static_cast<int>(argument.field.x), 0, canvas_w);
				int start_y = std::clamp(static_cast<int>(argument.field.y), 0, canvas_h);
				int end_x = std::clamp(static_cast<int>(argument.field.z), 0, canvas_w);
				int end_y = std::clamp(static_cast<int>(argument.field.w), 0, canvas_h);
				if (start_x >= end_x || start_y >= end_y) return; //guard

				float max_r = std::min(argument.field.z - argument.field.x, argument.field.w - argument.field.y) * 0.5f;
				float r = std::clamp(argument.rounding, 0.f, max_r);
				uint32_t alpha = static_cast<uint32_t>(((argument.color >> 24) & 0xFF) * std::clamp(argument.opacity, 0.f, 1.f));

				if (alpha == 255 && r <= 0.f) { //untransparent
					for (int y = start_y; y < end_y; ++y) {
						std::fill(instance.pixels.data() + (y * canvas_w + start_x), instance.pixels.data() + (y * canvas_w + end_x), argument.color | 0xFF000000u);
					}
					return;
				}

				for (int y = start_y; y < end_y; ++y) {
					for (int x = start_x; x < end_x; ++x) {
						float corner_a = get_corner_alpha(x, y, argument.field, r);
						if (corner_a <= 0.f) continue;

						uint32_t& pixel = instance.pixels[y * canvas_w + x];
						pixel = accomplish_opacity(pixel, argument.color, argument.opacity * corner_a);
					}
				}
			}

			inline void internal_drawtext(instance_context& instance, const drawtext_context& argument) {
				int canvas_w = static_cast<int>(instance.screen.field.z - instance.screen.field.x);
				int canvas_h = static_cast<int>(instance.screen.field.w - instance.screen.field.y);
				if (canvas_w <= 0 || canvas_h <= 0 || argument.opacity <= 0.f || argument.string.empty()) return;

				const void* font_pointer = instance.font_stack.empty() ? nullptr : instance.font_stack.back();
				std::string cache_key = std::to_string(reinterpret_cast<uintptr_t>(font_pointer)) + "_" + std::to_string(static_cast<int>(argument.textsize)) + "_" + argument.string;

				auto it = instance.text_cache.find(cache_key);
				if (it == instance.text_cache.end()) {
					if (instance.text_cache.size() > 1024) instance.text_cache.clear();
					it = instance.text_cache.emplace(cache_key, rasterize_text(instance, argument)).first;
				}

				const text_bitmap_context& bitmap = it->second;
				if (bitmap.width <= 0 || bitmap.height <= 0 || bitmap.mask.empty()) return;

				int base_x = static_cast<int>(argument.position.x);
				int base_y = static_cast<int>(argument.position.y);

				int start_x = std::clamp(base_x, 0, canvas_w);
				int start_y = std::clamp(base_y, 0, canvas_h);
				int end_x = std::clamp(base_x + bitmap.width, 0, canvas_w);
				int end_y = std::clamp(base_y + bitmap.height, 0, canvas_h);
				if (start_x >= end_x || start_y >= end_y) return;

				for (int y = start_y; y < end_y; ++y) {
					for (int x = start_x; x < end_x; ++x) {
						uint8_t mask_alpha = bitmap.mask[(y - base_y) * bitmap.width + (x - base_x)];
						if (mask_alpha == 0) continue;

						float pixel_opacity = argument.opacity * (static_cast<float>(mask_alpha) / 255.f);
						uint32_t& pixel = instance.pixels[y * canvas_w + x];
						pixel = accomplish_opacity(pixel, argument.color, pixel_opacity);
					}
				}
			}

			inline void accomplish_command_request(instance_context& instance) {
				std::vector<command_context> local_list;
				{
					std::lock_guard<std::mutex> lock(instance.command_mutex);
					std::swap(instance.command_list, local_list);
				}

				for (const auto& cmd : local_list) {
					if (cmd.variant == command_variants::rect) {
						syzygy::standard::backend::internal_drawrect(instance, std::get<drawrect_context>(cmd.context));
						continue;
					}
					if (cmd.variant == command_variants::text) {
						syzygy::standard::backend::internal_drawtext(instance, std::get<drawtext_context>(cmd.context));
						continue;
					}
					if (cmd.variant == command_variants::pushfont) {
						instance.font_stack.push_back(std::get<pushfont_context>(cmd.context).font);
						continue;
					}
					if (cmd.variant == command_variants::popfont) {
						if (!instance.font_stack.empty()) instance.font_stack.pop_back();
						continue;
					}
				}
			}

			inline void thread_handling(instance_context* instance) {
				if (!instance) {
					syzygy::standard::logger::send("Thread handling failed: instance is null.");
					return;
				}

				if (!instance->callback) {
					syzygy::standard::logger::report_fail(*instance, fail_reason_list::invalid_callback, "Invalid callback.");
					return;
				}

				if (!create_overlay(*instance)) {
					logger::report_fail(*instance, fail_reason_list::failed_overlay_creation, "Failed to create overlay window.");
					return;
				}

				render_state_list expected = render_state_list::initializing;
				if (!instance->render_state.compare_exchange_strong(expected, render_state_list::running)) {
					if (expected != render_state_list::stopping) {
						logger::report_fail(*instance, fail_reason_list::failed_overlay_creation, "Expected status couldn't match. (initializing - running)");
					}
					destroy_overlay(*instance);
					return;
				}

				while (instance->render_state == render_state_list::running) {
					instance->screen = syzygy::standard::backend::get_screen_information(*instance);
					if (instance->screen.field.z - instance->screen.field.x <= 0.f || instance->screen.field.w - instance->screen.field.y <= 0.f) {
						if (instance->target_window) {
							logger::report_fail(*instance, fail_reason_list::invalid_target_window, "Invalid or closed target window.");
						}
						else {
							logger::report_fail(*instance, fail_reason_list::failed_obtainment_of_screen_info, "Failed to get screen information.");
						}
						break;
					}

					//resize / pixel buffer, initialization of canvas / command list
					int canvas_w = static_cast<int>(instance->screen.field.z - instance->screen.field.x);
					int canvas_h = static_cast<int>(instance->screen.field.w - instance->screen.field.y);
					size_t required_size = static_cast<size_t>(std::max(0, canvas_w)) * static_cast<size_t>(std::max(0, canvas_h));

					if (instance->pixels.size() != required_size) {
						instance->pixels.resize(required_size);
					}

					std::fill(instance->pixels.begin(), instance->pixels.end(), instance->transparent_color);

					//check callback
					instance->font_stack.clear(); //guard if the user forgot to send popfont

					if (instance->callback) {
						instance->callback(*instance);
					}

					//literally function name explains. no explanation needed.
					syzygy::standard::backend::accomplish_command_request(*instance);
					present(*instance);
					vsync(*instance);
				}

				//destruction of window (when closed)
				if (!destroy_overlay(*instance)) {
					logger::report_fail(*instance, fail_reason_list::failed_destruction_of_overlay, "Failed to destroy overlay window.");
				}
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

				//writing pointer instead of ptr is my habit, dont care about it.
				syzygy::standard::database::instances.push_back(std::move(new_instance));
				auto* instance_pointer = syzygy::standard::database::instances.back().get();
				instance_pointer->render_thread = std::thread(backend::thread_handling, instance_pointer);
				return instance_pointer;
			}

			static void drawrect(instance_context& instance, syzvec4 field, uint32_t color, float opacity = 1.f, float rounding = 0.f) {
				std::lock_guard<std::mutex> lock(instance.command_mutex);
				instance.command_list.push_back({ command_variants::rect, drawrect_context{ field, color, opacity, rounding } });
			}

			static void drawtext(instance_context& instance, const std::string& string, syzvec2 position, uint32_t color, float opacity = 1.f, float textsize = 18.f) {
				std::lock_guard<std::mutex> lock(instance.command_mutex);
				instance.command_list.push_back({ command_variants::text, drawtext_context{ position, string, color, opacity, textsize } });
			}

			static const std::vector<unsigned char>* serialize_font(const std::string& filepath) {
				std::lock_guard<std::mutex> lock(syzygy::standard::database::font_mutex);
				auto it = syzygy::standard::database::fonts.find(filepath);
				if (it != syzygy::standard::database::fonts.end()) {
					return &it->second;
				}

				std::ifstream file(filepath, std::ios::binary | std::ios::ate);
				if (!file.is_open()) return nullptr;

				std::streamsize size = file.tellg();
				if (size <= 0) return nullptr; //guard
				file.seekg(0, std::ios::beg);

				std::vector<unsigned char> buffer(size);
				if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
					auto& stored = syzygy::standard::database::fonts[filepath];
					stored = std::move(buffer);
					return &stored;
				}
				return nullptr;
			}

			static void pushfont(instance_context& instance, const std::vector<unsigned char>* font) {
				std::lock_guard<std::mutex> lock(instance.command_mutex);
				instance.command_list.push_back({ command_variants::pushfont, pushfont_context{ font } });
			}

			static void pushfont(instance_context& instance, const std::vector<unsigned char>& font) {
				std::lock_guard<std::mutex> lock(instance.command_mutex);
				instance.command_list.push_back({ command_variants::pushfont, pushfont_context{ &font } });
			}

			static void popfont(instance_context& instance) {
				std::lock_guard<std::mutex> lock(instance.command_mutex);
				instance.command_list.push_back({ command_variants::popfont, popfont_context{} });
			}

			static void destroy_instance(instance_context* instance) {
				if (!instance) return;

				instance->render_state = render_state_list::stopping;
				if (instance->render_thread.joinable() && instance->render_thread.get_id() != std::this_thread::get_id()) {
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

#if defined(_WIN32) || defined(_WIN64)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

inline syzygy::standard::screen_context syzygy::standard::core::win::get_screen_information(instance_context& context) {
	screen_context info = context.screen;
#if defined(_WIN32) || defined(_WIN64)
	if (info.resolution.z <= 0.f) {
		DEVMODEW dm = {};
		dm.dmSize = sizeof(dm);
		if (EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dm)) {
			info.resolution = { 0.f, 0.f, static_cast<float>(dm.dmPelsWidth), static_cast<float>(dm.dmPelsHeight) };
			info.refresh_rate = static_cast<float>(dm.dmDisplayFrequency);
		}
	}

	if (context.target_window) {
		RECT rect = {};
		GetWindowRect(static_cast<HWND>(context.target_window), &rect);
		info.field = { static_cast<float>(rect.left), static_cast<float>(rect.top), static_cast<float>(rect.right), static_cast<float>(rect.bottom) };
	}
	else {
		info.field = info.resolution;
	}
#endif
	return info;
}

inline bool syzygy::standard::core::win::create_overlay(instance_context& context) {
#if defined(_WIN32) || defined(_WIN64)
	WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW, DefWindowProcW, 0, 0, GetModuleHandleW(nullptr), nullptr, nullptr, nullptr, nullptr, L"syzygy_overlay", nullptr };
	RegisterClassExW(&wc);

	int x = static_cast<int>(context.screen.field.x);
	int y = static_cast<int>(context.screen.field.y);
	int w = static_cast<int>(context.screen.field.z - context.screen.field.x);
	int h = static_cast<int>(context.screen.field.w - context.screen.field.y);

	HWND hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, wc.lpszClassName, L"syzygy", WS_POPUP, x, y, w, h, nullptr, nullptr, wc.hInstance, nullptr);
	if (!hwnd) return false;

	SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
	MARGINS margins = { -1, -1, -1, -1 };
	DwmExtendFrameIntoClientArea(hwnd, &margins);

	ShowWindow(hwnd, SW_SHOWNOACTIVATE);
	context.overlay_window = hwnd;
	return true;
#else
	return false;
#endif
}

inline bool syzygy::standard::core::win::destroy_overlay(instance_context& context) {
#if defined(_WIN32) || defined(_WIN64)
	if (!context.overlay_window) return false;
	DestroyWindow(static_cast<HWND>(context.overlay_window));
	context.overlay_window = nullptr;
	return true;
#else
	return false;
#endif
}

inline void syzygy::standard::core::win::present(instance_context& context) {
#if defined(_WIN32) || defined(_WIN64)
	HWND hwnd = static_cast<HWND>(context.overlay_window);
	if (!hwnd || context.pixels.empty()) return;

	MSG msg = {};
	while (PeekMessageW(&msg, hwnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	int x = static_cast<int>(context.screen.field.x);
	int y = static_cast<int>(context.screen.field.y);
	int w = static_cast<int>(context.screen.field.z - context.screen.field.x);
	int h = static_cast<int>(context.screen.field.w - context.screen.field.y);
	if (context.target_window) {
		SetWindowPos(hwnd, HWND_TOPMOST, x, y, w, h, SWP_NOACTIVATE);
	}

	HDC hdc = GetDC(hwnd);
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = -h; //reversed because of up-side down prevention
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	StretchDIBits(hdc, 0, 0, w, h, 0, 0, w, h, context.pixels.data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
	ReleaseDC(hwnd, hdc);
#endif
}

inline void syzygy::standard::core::win::vsync(instance_context& context) {
#if defined(_WIN32) || defined(_WIN64)
	if (FAILED(DwmFlush())) {
		syzygy::standard::core::unknown::vsync(context);
	}
#endif
}

inline syzygy::standard::text_bitmap_context syzygy::standard::core::win::rasterize_text(instance_context& context, const drawtext_context& argument) {
	text_bitmap_context result = {};
#if defined(_WIN32) || defined(_WIN64)
	if (argument.string.empty() || argument.textsize <= 0.f) return result;

	std::wstring font_name = L"Arial";
	const auto* font_data = context.font_stack.empty() ? nullptr : context.font_stack.back();

	if (font_data && font_data->size() >= 12) {
		static std::unordered_map<const void*, std::wstring> registered_fonts;
		if (auto it = registered_fonts.find(font_data); it != registered_fonts.end()) {
			font_name = it->second;
		}
		else {
			DWORD count = 0;
			const uint8_t* raw = font_data->data();
			AddFontMemResourceEx(const_cast<uint8_t*>(raw), static_cast<DWORD>(font_data->size()), nullptr, &count);

			uint16_t tables = _byteswap_ushort(*reinterpret_cast<const uint16_t*>(raw + 4));
			for (uint16_t i = 0; i < tables && 12u + i * 16u + 16u <= font_data->size(); ++i) {
				const uint8_t* entry = raw + 12 + i * 16;
				if (std::string_view(reinterpret_cast<const char*>(entry), 4) != "name") continue;

				uint32_t offset = _byteswap_ulong(*reinterpret_cast<const uint32_t*>(entry + 8));
				uint16_t records = _byteswap_ushort(*reinterpret_cast<const uint16_t*>(raw + offset + 2));
				uint16_t storage = _byteswap_ushort(*reinterpret_cast<const uint16_t*>(raw + offset + 4));

				for (uint16_t j = 0; j < records && offset + 6u + j * 12u + 12u <= font_data->size(); ++j) {
					const uint8_t* rec = raw + offset + 6 + j * 12;
					if (_byteswap_ushort(*reinterpret_cast<const uint16_t*>(rec)) != 3 || _byteswap_ushort(*reinterpret_cast<const uint16_t*>(rec + 6)) != 1) continue;

					uint16_t len = _byteswap_ushort(*reinterpret_cast<const uint16_t*>(rec + 8));
					uint16_t pos = _byteswap_ushort(*reinterpret_cast<const uint16_t*>(rec + 10));
					font_name.clear();
					for (uint16_t k = 0; k + 1 < len; k += 2) {
						font_name.push_back(static_cast<wchar_t>(_byteswap_ushort(*reinterpret_cast<const uint16_t*>(raw + offset + storage + pos + k))));
					}
					break;
				}
				break;
			}
			registered_fonts[font_data] = font_name;
		}
	}

	int wlen = MultiByteToWideChar(CP_UTF8, 0, argument.string.c_str(), -1, nullptr, 0);
	if (wlen <= 1) return result;
	std::wstring wstr(wlen - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, argument.string.c_str(), -1, wstr.data(), wlen);

	HDC hdc = CreateCompatibleDC(nullptr);
	HFONT hfont = CreateFontW(-static_cast<int>(argument.textsize), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, font_name.c_str());
	HGDIOBJ old_font = SelectObject(hdc, hfont);

	SIZE size = {};
	GetTextExtentPoint32W(hdc, wstr.c_str(), static_cast<int>(wstr.size()), &size);
	if (size.cx > 0 && size.cy > 0) {
		BITMAPINFO bmi = { { sizeof(BITMAPINFOHEADER), size.cx, -size.cy, 1, 32, BI_RGB } };
		uint32_t* dib = nullptr;
		if (HBITMAP hbm = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&dib), nullptr, 0)) {
			HGDIOBJ old_bm = SelectObject(hdc, hbm);
			SetBkColor(hdc, 0x000000);
			SetTextColor(hdc, 0xFFFFFF);
			TextOutW(hdc, 0, 0, wstr.c_str(), static_cast<int>(wstr.size()));

			result = { size.cx, size.cy, std::vector<uint8_t>(static_cast<size_t>(size.cx) * size.cy) };
			for (size_t i = 0; i < result.mask.size(); ++i) result.mask[i] = static_cast<uint8_t>(dib[i] & 0xFF);

			SelectObject(hdc, old_bm);
			DeleteObject(hbm);
		}
	}
	SelectObject(hdc, old_font);
	DeleteObject(hfont);
	DeleteDC(hdc);
#endif
	return result;
}

inline syzygy::standard::screen_context syzygy::standard::core::mac::get_screen_information(instance_context& context) { return {}; }
inline bool syzygy::standard::core::mac::create_overlay(instance_context& context) { return false; }
inline bool syzygy::standard::core::mac::destroy_overlay(instance_context& context) { return false; }
inline void syzygy::standard::core::mac::present(instance_context& context) {}
inline void syzygy::standard::core::mac::vsync(instance_context& context) {}
inline syzygy::standard::text_bitmap_context syzygy::standard::core::mac::rasterize_text(instance_context& context, const drawtext_context& argument) { return {}; }

inline syzygy::standard::screen_context syzygy::standard::core::linux_x11::get_screen_information(instance_context& context) { return {}; }
inline bool syzygy::standard::core::linux_x11::create_overlay(instance_context& context) { return false; }
inline bool syzygy::standard::core::linux_x11::destroy_overlay(instance_context& context) { return false; }
inline void syzygy::standard::core::linux_x11::present(instance_context& context) {}
inline void syzygy::standard::core::linux_x11::vsync(instance_context& context) {}
inline syzygy::standard::text_bitmap_context syzygy::standard::core::linux_x11::rasterize_text(instance_context& context, const drawtext_context& argument) { return {}; }

inline syzygy::standard::screen_context syzygy::standard::core::linux_wayland::get_screen_information(instance_context& context) { return {}; }
inline bool syzygy::standard::core::linux_wayland::create_overlay(instance_context& context) { return false; }
inline bool syzygy::standard::core::linux_wayland::destroy_overlay(instance_context& context) { return false; }
inline void syzygy::standard::core::linux_wayland::present(instance_context& context) {}
inline void syzygy::standard::core::linux_wayland::vsync(instance_context& context) {}
inline syzygy::standard::text_bitmap_context syzygy::standard::core::linux_wayland::rasterize_text(instance_context& context, const drawtext_context& argument) { return {}; }

inline syzygy::standard::screen_context syzygy::standard::core::unknown::get_screen_information(instance_context& context) { return {}; }
inline bool syzygy::standard::core::unknown::create_overlay(instance_context& context) { return false; }
inline bool syzygy::standard::core::unknown::destroy_overlay(instance_context& context) { return false; }
inline void syzygy::standard::core::unknown::present(instance_context& context) {}
inline void syzygy::standard::core::unknown::vsync(instance_context& context) {
	float fps = context.screen.refresh_rate > 0.f ? context.screen.refresh_rate : 60.f;
	auto frame_duration = std::chrono::duration<float, std::milli>(1000.f / fps);
	auto next_frame = context.last_frame_time + std::chrono::duration_cast<std::chrono::steady_clock::duration>(frame_duration);

	std::this_thread::sleep_until(next_frame);
	context.last_frame_time = std::chrono::steady_clock::now();
}
inline syzygy::standard::text_bitmap_context syzygy::standard::core::unknown::rasterize_text(instance_context& context, const drawtext_context& argument) { return {}; }
