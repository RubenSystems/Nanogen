
#include "headers/Text.hpp"

#include <iostream>

using namespace RubenSystems::NanoGen;

int main(int argc, char const *argv[]) {
	Text x = Text ("HI");

	std::cout << x.generate();
	return 0;
}

// #include <vector>
// #include <unordered_map>
// #include <string>
// #include <memory>
// #include <fstream>


// #include "headers/Text.hpp"
// #include "headers/View.hpp"
// #include "headers/NanoServ.hpp"
// #include "headers/Helpers.hpp"

// using namespace RubenSystems::NanoGen;

// int main(int argc, char const *argv[]){
// 	/* code */


// 	NanoServ x;

// 	auto head = ng(View("head", true, {
// 		ng(View("link", false, {})
// 			.addAttribute("rel", "preconnect")
// 			.addAttribute("href", "https://fonts.googleapis.com")
// 			),
// 		ng(View("link", false, {})
// 			.addAttribute("rel", "stylesheet")
// 			.addAttribute("href", "https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@300;500;700&display=swap")
// 			),
// 		ng(View("Title", true, {ng(Text("RubenSystems"))}))
// 	}));

// 	auto header = ng(View("header", true, {
// 		ng(View("div", true, {
// 			ng(View("p", true, {
// 				ng(Text("RubenSystems")),
// 				ng(View("span", true, {ng(Text("."))}).addStyle("color", "#00b595"))
// 			})
// 				.addStyle("font-size", "2em")
// 				.addStyle("margin", "0")),
// 			ng(View("h1", true, {
// 				ng(Text("Simplifing simplicity"))
// 			})
// 			.addStyle("font-size", "6em")
// 			.addStyle("line-height", "0.8em")
// 			.addStyle("margin", "0px")
// 			)
// 		})
// 		.addStyle("margin", "auto"))
// 	})
// 	.addStyle("display", "flex")
// 	.addStyle("justify-content", "center")
// 	.addStyle("align-items", "center")
// 	.addStyle("height", "75%")
// 	.addStyle("color", "#333")
// 	.addStyle("background", "#efefef"));



// 	x.route("GET", "/rubensystems/$/responds", [&head, &header](std::vector<std::string> pathVars) -> std::shared_ptr<BaseView> {
// 		return ng(View("html", true, {
// 			head,
// 			ng(View("body", true, {
// 				header, 
// 				ng(Text("This is the " + pathVars[0] + " page!"))
// 			})
// 			.addStyle("font-family", "\"Space Grotesk\", sans-serif")
// 			.addStyle("font-weight", "300")
// 			.addStyle("margin", "0"))
// 		}));
// 	});

// 	x.start(8080);

// 	return 0;
// }