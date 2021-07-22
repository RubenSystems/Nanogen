#pragma once


#include <memory>
#include <unordered_map>

namespace RubenSystems {
	namespace NanoGen {

		template <typename T>
		std::shared_ptr<T> ng(const T & view){
			return std::make_shared<T>(view);
		}


		std::unordered_map<std::string, std::string> mimeTypes ({
			{"js", "text/javascript"},
			{"png", "image/png"},
			{"jpg", "image/jpeg"},
			{"jpeg", "image/jpeg"},
			{"svg", "image/svg+xml"},
			{"gif", "image/gif"},
			{"css", "text/css"}


		});

	}
}

