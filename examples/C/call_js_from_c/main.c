// Call JavaScript from C Example

#include "webinix.h"

void my_function_exit(webinix_event_t* e) {

	// Close all opened windows
	webinix_exit();
}

void my_function_count(webinix_event_t* e) {

	// This function gets called every time the user clicks on "my_function_count"

	// Create a buffer to hold the response
	char response[64];

	// Run JavaScript
	if (!webinix_script(e->window, "return GetCount();", 0, response, 64)) {

		if (!webinix_is_shown(e->window))
			printf("Window closed.\n");
		else
			printf("JavaScript Error: %s\n", response);
		return;
	}

	// Get the count
	int count = atoi(response);

	// Increment
	count++;

	// Generate a JavaScript
	char js[64];
	sprintf(js, "SetCount(%d);", count);

	// Run JavaScript (Quick Way)
	webinix_run(e->window, js);
}

int main() {

	// HTML
	const char* my_html = "<!DOCTYPE html>"
	                      "<html>"
	                      "  <head>"
	                      "    <meta charset=\"UTF-8\">"
	                      "    <script src=\"webinix.js\"></script>"
	                      "    <title>Call JavaScript from C Example</title>"
	                      "    <style>"
	                      "       body {"
	                      "            font-family: 'Arial', sans-serif;"
	                      "            color: white;"
	                      "            background: linear-gradient(to right, #507d91, #1c596f, #022737);"
	                      "            text-align: center;"
	                      "            font-size: 18px;"
	                      "        }"
	                      "        button, input {"
	                      "            padding: 10px;"
	                      "            margin: 10px;"
	                      "            border-radius: 3px;"
	                      "            border: 1px solid #ccc;"
	                      "            box-shadow: 0 3px 5px rgba(0,0,0,0.1);"
	                      "            transition: 0.2s;"
	                      "        }"
	                      "        button {"
	                      "            background: #3498db;"
	                      "            color: #fff; "
	                      "            cursor: pointer;"
	                      "            font-size: 16px;"
	                      "        }"
	                      "        h1 { text-shadow: -7px 10px 7px rgb(67 57 57 / 76%); }"
	                      "        button:hover { background: #c9913d; }"
	                      "        input:focus { outline: none; border-color: #3498db; }"
	                      "    </style>"
	                      "  </head>"
	                      "  <body>"
	                      "    <h1>Webinix - Call JavaScript from C</h1>"
	                      "    <br>"
	                      "    <h1 id=\"count\">0</h1>"
	                      "    <br>"
	                      "    <button OnClick=\"my_function_count();\">Manual Count</button>"
	                      "    <br>"
	                      "    <button id=\"MyTest\" OnClick=\"AutoTest();\">Auto Count (Every 10ms)</button>"
	                      "    <br>"
	                      "    <button OnClick=\"my_function_exit();\">Exit</button>"
	                      "    <script>"
	                      "      let count = 0;"
	                      "      function GetCount() {"
	                      "        return count;"
	                      "      }"
	                      "      function SetCount(number) {"
	                      "        document.getElementById('count').innerHTML = number;"
	                      "        count = number;"
	                      "      }"
	                      "      function AutoTest(number) {"
	                      "        setInterval(function(){ my_function_count(); }, 10);"
	                      "      }"
	                      "    </script>"
	                      "  </body>"
	                      "</html>";

	// Create a window
	size_t my_window = webinix_new_window();

	// Bind HTML elements with C functions
	webinix_bind(my_window, "my_function_count", my_function_count);
	webinix_bind(my_window, "my_function_exit", my_function_exit);

	// Show the window
	webinix_show(my_window, my_html); // webinix_show_browser(my_window, my_html, Chrome);

	// Wait until all windows get closed
	webinix_wait();

	// Free all memory resources (Optional)
	webinix_clean();

	return 0;
}

#if defined(_MSC_VER)
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) { return main(); }
#endif
