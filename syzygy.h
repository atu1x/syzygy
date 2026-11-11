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

//forward declarations of core functions. (because of readability/accessibility)
namespace syzygy {
	namespace core {
		namespace win {
			//typeshit
			extern void setup();
			extern void destroy();
			extern void present();
		}
		namespace mac {

		}
		namespace linux_x11 {

		}
		namespace linux_wayland {

		}
		namespace unknown { //for users
			//write yourself :clown:
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
		inline void present() {
			//get device? idk :sob:
			//send it to dagestan for 2 or 3 years and forget:
			//if (instance.device == device_list::win) syzygy::core::win::present();
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

//main core (implementation)
inline void syzygy::core::win::setup() {

}
//typeshit
