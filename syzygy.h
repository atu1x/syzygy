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

//forward declaration of core part.
namespace syzygy {
	struct instance_context;

	namespace core {
		namespace win { //windows is the highest priority. firstly we implement this part.
			extern void create_overlay(instance_context& context);
			extern void destroy_overlay(instance_context& context);
			extern void present(instance_context& context);
		}
		namespace mac {
			extern void create_overlay(instance_context& context);
			extern void destroy_overlay(instance_context& context);
			extern void present(instance_context& context);
		}
		namespace linux_x11 {
			extern void create_overlay(instance_context& context);
			extern void destroy_overlay(instance_context& context);
			extern void present(instance_context& context);
		
		}
		namespace linux_wayland {
			extern void create_overlay(instance_context& context);
			extern void destroy_overlay(instance_context& context);
			extern void present(instance_context& context);
		}
		namespace unknown {
			extern void create_overlay(instance_context& context);
			extern void destroy_overlay(instance_context& context);
			extern void present(instance_context& context);
		}
	}
}

namespace syzygy {
	struct syzvec2 { float x = 0.f, y = 0.f; };
	struct syzvec3 { float x = 0.f, y = 0.f, z = 0.f; };
	struct syzvec4 { float x = 0.f, y = 0.f, z = 0.f, w = 0.f; };

	struct instance_context {};

	namespace database {
		inline std::vector<std::unique_ptr<instance_context>> instances = {};
		inline std::atomic<bool> initialized{ false };
	}

	namespace backend {
		inline void setup() {
			syzygy::database::instances.clear();
			syzygy::database::initialized = true;
		}

		inline void transfer_create_overlay(instance_context& context) {
			//get device from instance context
			//like

			//if (context.device == device_list::win) syzygy::core::win::create_overlay(context);
			//else if (context.device == device_list::mac) syzygy::core::mac::create_overlay(context);
			//else if (context.device == device_list::linux_x11) syzygy::core::linux_x11::create_overlay(context);
			//else if (context.device == device_list::linux_wayland) syzygy::core::linux_wayland::create_overlay(context);
			//else if (context.device == device_list::unknown) syzygy::core::unknown::create_overlay(context); //user has to write the adaptation code.
		}
	}

	class engine {
	public:
		static instance_context* create_instance() {
			if (!syzygy::database::initialized) syzygy::backend::setup();

			auto new_instance = std::make_unique<instance_context>();

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

inline void syzygy::core::win::create_overlay(instance_context& context) {}
inline void syzygy::core::win::destroy_overlay(instance_context& context) {}
inline void syzygy::core::win::present(instance_context& context) {}

inline void syzygy::core::mac::create_overlay(instance_context& context) {}
inline void syzygy::core::mac::destroy_overlay(instance_context& context) {}
inline void syzygy::core::mac::present(instance_context& context) {}

inline void syzygy::core::linux_x11::create_overlay(instance_context& context) {}
inline void syzygy::core::linux_x11::destroy_overlay(instance_context& context) {}
inline void syzygy::core::linux_x11::present(instance_context& context) {}

inline void syzygy::core::linux_wayland::create_overlay(instance_context& context) {}
inline void syzygy::core::linux_wayland::destroy_overlay(instance_context& context) {}
inline void syzygy::core::linux_wayland::present(instance_context& context) {}

inline void syzygy::core::unknown::create_overlay(instance_context& context) {}
inline void syzygy::core::unknown::destroy_overlay(instance_context& context) {}
inline void syzygy::core::unknown::present(instance_context& context) {}
