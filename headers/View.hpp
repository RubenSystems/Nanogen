#pragma once


#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include "BaseView.hpp"


namespace RubenSystems {
	namespace NanoGen {
		class View : public BaseView {
			public:
				View(const std::string & type, bool close, const std::vector<std::shared_ptr<BaseView> > & subviews);

				View addStyle(const std::string & type, const std::string & value);

				View addAttribute(const std::string & type, const std::string & value);

				std::string generate() override;
			
			private:
				std::unordered_map<std::string, std::vector<std::string>> attributes;
				std::vector<std::shared_ptr<BaseView>> subviews;
				std::string type;
				bool close;


		};

		View::View(const std::string & type, bool close, const std::vector<std::shared_ptr<BaseView> > & subviews) : type(type), close(close), subviews(subviews) {}

		View View::addStyle(const std::string & type, const std::string & value) {
			return this->addAttribute("style", type + ":" + value + ";");
		}

		View View::addAttribute(const std::string & type, const std::string & value) {
			attributes[type].push_back(value);
			return (*this);
		}

		std::string View::generate() {
			std::string content = "<";
			content += this->type + " ";
			for (auto & i : attributes) {
				content += i.first + "='";
				for (auto & c : i.second) {
					content += c;
				}
				content += "' ";
			}

			content += (this->close ? "" : "/");
			content += ">";
			if (this->close) {
				for (auto & i : this->subviews) {
					content += i->generate();
				}
				content += "</" + this->type + ">";
			}


			return content;
		}
	}
}

