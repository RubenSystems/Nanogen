#pragma once


#include <memory>

namespace RubenSystems {
	namespace NanoGen {

		template <typename T>
		std::shared_ptr<T> ng(const T & view) {
			return std::make_shared<T>(view);
		}

	}
}

