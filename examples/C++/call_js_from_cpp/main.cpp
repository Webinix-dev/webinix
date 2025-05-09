// Call JavaScript from C++ Example

// Include the Webinix header
#include "webinix.hpp"

// Include C++ STD
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

void my_function_exit(webinix::window::event* e) {

	// Close all opened windows
	webinix::exit();
}

void my_function_count(webinix::window::event* e) {

	// Create a buffer to hold the response
	char response[64];

	// This is another way to create a buffer:
	//  std::string buffer;
	//  buffer.reserve(64);
	//  my_window.script(..., ..., &buffer[0], 64);

	// Run JavaScript
	if (!e->get_window().script("return GetCount();", 0, response, 64)) {

		if (!e->get_window().is_shown())
			std::cout << "Window closed." << std::endl;
		else
			std::cout << "JavaScript Error: " << response << std::endl;
		return;
	}

	// Get the count
	int count = std::stoi(response);

	// Increment
	count++;

	// Generate a JavaScript
	std::stringstream js;
	js << "SetCount(" << count << ");";

	// Run JavaScript (Quick Way)
	e->get_window().run(js.str());
}

int main() {

	// HTML
	const std::string my_html = R"V0G0N(
    <html>
      <head>
        <meta charset="UTF-8">
        <script src="/webinix.js"></script>
        <title>Call JavaScript from C++ Example</title>
        <style>
          body {
            background: linear-gradient(to left, #36265a, #654da9);
            color: AliceBlue;
            font: 16px sans-serif;
            text-align: center;
            margin-top: 30px;
          }
          button {
            margin: 5px 0 10px;
          }
        </style>
      </head>
      <body>
        <h1>Webinix - Call JavaScript from C++</h1>
        <br>
        <h1 id="count">0</h1>
        <br>
        <button OnClick="my_function_count();">Manual Count</button>
        <br>
        <button OnClick="AutoTest();">Auto Count (Every 10ms)</button>
        <br>
        <button id="Exit">Exit</button>
        <script>
          let count = 0;
          function GetCount() {
            return count;
          }
          function SetCount(number) {
            document.getElementById('count').innerHTML = number;
            count = number;
          }
          function AutoTest(number) {
            setInterval(function() {
              my_function_count();
            }, 10);
          }
        </script>
      </body>
    </html>
  )V0G0N";

	// Create a window
	webinix::window my_window;

	// Bind HTML elements with C++ functions
	my_window.bind("my_function_count", my_function_count);
	my_window.bind("Exit", my_function_exit);

	// Show the window
	my_window.show(my_html); // my_window.show_browser(my_html, Chrome);

	// Wait until all windows get closed
	webinix::wait();

	return 0;
}

#if defined(_MSC_VER)
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) { main(); }
#endif
