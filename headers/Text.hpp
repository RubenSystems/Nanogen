#pragma once



#include <string>

#include "BaseView.hpp"


namespace RubenSystems {
	namespace NanoGen {
		class Text : public BaseView {
			public:
				Text(const std::string & value) : value(value) {}

				std::string generate() override {
					return value;
				}

			private: 
				std::string value;
			
		};
	}
}

