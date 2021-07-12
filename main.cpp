#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <fstream>


#include "headers/Helpers.hpp"
#include "headers/Text.hpp"
#include "headers/View.hpp"
#include "headers/NanoServ.hpp"

using namespace RubenSystems::NanoGen;

int main(int argc, char const *argv[]){
	/* code */


	NanoServ x;

	auto head = ng(View("head", true, {
		ng(View("link", false, {})
			.addAttribute("rel", "preconnect")
			.addAttribute("href", "https://fonts.googleapis.com")
			),
		ng(View("link", false, {})
			.addAttribute("rel", "stylesheet")
			.addAttribute("href", "https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@300;500;700&display=swap")
			)
	}));

	auto header = ng(View("header", true, {
		ng(View("div", true, {
			ng(View("p", true, {ng(Text("Welcome to"))})
				.addStyle("font-size", "2em")
				.addStyle("margin", "0")),
			ng(View("h1", true, {
				ng(Text("RubenSystems")),
				ng(View("span", true, {ng(Text("."))}).addStyle("color", "#00b595"))
			})
			.addStyle("font-size", "6em")
			.addStyle("margin", "0px")
			)
		})
		.addStyle("max-width", "800px")
		.addStyle("margin", "auto"))
	})
	.addStyle("padding", "100px 0")
	.addStyle("color", "#333")
	.addStyle("background", "#efefef"));



	x.route("GET", "/rubensystems/$/responds", [&head, &header](std::vector<std::string> pathVars) -> std::shared_ptr<BaseView> {
		return ng(View("html", true, {
			head,
			ng(View("body", true, {
				header, 
				ng(Text("This is the " + pathVars[0] + " page!"))
			})
			.addStyle("font-family", "\"Space Grotesk\", sans-serif")
			.addStyle("margin", "0"))
		}));
	});

	x.start(8080);

	return 0;
}