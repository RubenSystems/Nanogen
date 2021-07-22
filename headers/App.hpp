#pragma once



#include <string>

#include "BaseView.hpp"
#include "View.hpp"


namespace RubenSystems {
	namespace NanoGen {
		class App : public BaseView {
			public:
				App(const View & head, const View & body) : view (View("html", true, {ng(head), ng(body)})) {}

				std::string generate() override {
					return this->view.generate();
				}

			private: 
				View view;
			
		};
	}
}
