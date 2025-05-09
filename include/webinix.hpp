/*
  Webinix Library
  https://webinix.me
  https://github.com/webinix-dev/webinix
  Copyright (c) 2020-2025 Hassan Draga.
  Licensed under MIT License.
  All rights reserved.
  Canada.
*/

#ifndef _WEBUI_HPP
#define _WEBUI_HPP

// C++ STD
#include <array>
#include <string>
#include <string_view>

// Webinix C Header
extern "C" {
    #include "webinix.h"
}

namespace webinix {

    enum : int {
        DISCONNECTED = 0, // 0. Window disconnection event
        CONNECTED = 1,    // 1. Window connection event
        MOUSE_CLICK = 2,  // 2. Mouse click event
        NAVIGATION = 3,   // 3. Window navigation event
        CALLBACKS = 4    // 4. Function call event
    };

    class window {
        private:
        size_t webinix_window{webinix_new_window()};

        public:
        // Event Struct
        class event : public webinix_event_t {
            // Window object constructor that
            // initializes the reference, This
            // is to avoid creating copies.
            event(webinix::window& window_obj, webinix_event_t c_e) : webinix_event_t(c_e) {
                reinterpret_cast<webinix_event_t*>(this)->window = window_obj.webinix_window;
            }

            public:
            class handler {

                public:
                using callback_t = void (*)(event*);

                private:
                static inline std::array<callback_t, 512> callback_list{};

                // List of window objects: webinix::window
                static inline std::array<webinix::window*, 512> window_list{};

                public:
                handler() = delete;
                handler(const handler&) = delete;
                handler(handler&&) = delete;
                handler& operator=(const handler&) = delete;
                handler& operator=(handler&&) = delete;
                ~handler() = delete;

                static void add(size_t id, webinix::window* win, callback_t func) {
                    // Save window object
                    window_list[id] = win;
                    // Save callback
                    callback_list[id] = func;
                }

                static void handle(webinix_event_t* c_e) {
                    // Get the binded unique ID
                    const size_t id = c_e->bind_id;
                    if (id > 0) {
                        // Create a new event struct
                        event e(*window_list[id], *c_e);
                        // Call the user callback
                        if (callback_list[id] != nullptr)
                            callback_list[id](&e);
                    }
                }

                static webinix::window& get_window(const size_t index) { return *window_list[index]; }
            };

            // ------ Event methods `e->xxx()` ------

            // Get how many arguments there are in an event.
            size_t get_count() {
                return webinix_get_count(this);
            }

            // Get an argument as integer at a specific index.
            long long int get_int(size_t index = 0) {
                return webinix_get_int_at(this, index);
            }

            // Get an argument as string at a specific index.
            double get_float(size_t index = 0) {
                return webinix_get_float_at(this, index);
            }

            // Get the size in bytes of an argument at a specific index.
            size_t get_size(size_t index = 0) {
                return webinix_get_size_at(this, index);
            }

            // Get an argument as string at a specific index.
            std::string get_string(size_t index = 0) {
                return std::string{webinix_get_string_at(this, index)};
            }

            // Get an argument as string_view at a specific index.
            std::string_view get_string_view(size_t index = 0) {
                return std::string_view{webinix_get_string_at(this, index)};
            }

            // Get an argument as boolean at a specific index.
            bool get_bool(size_t index = 0) {
                return webinix_get_bool_at(this, index);
            }

            // Run JavaScript without waiting for the response. Single client.
            bool script_client(const std::string_view script, unsigned int timeout,
                            char* buffer, size_t buffer_length) {
                return webinix_script_client(this, script.data(), timeout, buffer, buffer_length);
            }

            // Run JavaScript without waiting for the response. Single client.
            void run_client(const std::string_view script) {
                webinix_run_client(this, script.data());
            }

            // Return the response to JavaScript as integer.
            void return_int(long long int n) {
                webinix_return_int(this, n);
            }

            // Return the response to JavaScript as integer.
            void return_float(double f) {
                webinix_return_float(this, f);
            }

            // Safely send raw data to the UI. Single client.
            void send_raw_client(const std::string_view function, const void* raw, size_t size) {
                webinix_send_raw_client(this, function.data(), raw, size);
            }

            // Return the response to JavaScript as string.
            void return_string(const std::string_view s) {
                webinix_return_string(this, s.data());
            }

            // Return the response to JavaScript as boolean.
            void return_bool(bool b) {
                webinix_return_bool(this, b);
            }

            // Close a specific client.
            void close_client() {
                webinix_close_client(this);
            }

            // Navigate to a specific URL. Single client.
            void navigate_client(const std::string_view url) {
                webinix_navigate_client(this, url.data());
            }

            // Same as `show()` but for a specific single client
            bool show_client(const std::string_view s) {
                return webinix_show_client(this, s.data());
            }

            // Get user data that is set using `set_context()`
            void* get_context() {
                return webinix_get_context(this);
            }

            // Extras

            // Get current window object pointer
            webinix::window& get_window() {
                return event::handler::get_window(window);
            }

            // Get event type
            size_t get_type() const {
                return event_type;
            }

            // Get event element name
            std::string_view get_element() const {
                return std::string_view{element};
            }

            // Get event number
            size_t get_number() const {
                return event_number;
            }
        };

        // ------ Window methods `window.xxx()` ------

        // Bind a specific html element click event with a function. Empty element means all events.
        void bind(const std::string_view element, event::handler::callback_t func) {
            // Get unique ID
            const size_t id = webinix_bind(webinix_window, element.data(), event::handler::handle);
            event::handler::add(id, this, func);
        }

        // Show a window using a embedded HTML, or a file. If the window is already opened
        // then it will be refreshed.
        bool show(const std::string_view content) const {
            return webinix_show(webinix_window, content.data());
        }

        // Same as show(). But with a specific web browser.
        bool show_browser(const std::string_view content, unsigned int browser) const {
            return webinix_show_browser(webinix_window, content.data(), browser);
        }

        // Set the window in Kiosk mode (Full screen)
        void set_kiosk(bool status) const {
            webinix_set_kiosk(webinix_window, status);
        }

        // Add user-defined command line parameters
        void set_custom_parameters(char *params) const {
            webinix_set_custom_parameters(webinix_window, params);
        }

        // Set the window with high-contrast support. Useful when you want to build a better high-contrast theme with CSS.
        void set_high_contrast(bool status) const {
            webinix_set_high_contrast(webinix_window, status);
        }

        // Close a specific window only. The window object will still exist.
        void close() const {
            webinix_close(webinix_window);
        }

        // Close a specific window and free all memory resources.
        void destroy() const {
            webinix_destroy(webinix_window);
        }

        // Check a specific window if it's still running
        bool is_shown() const {
            return webinix_is_shown(webinix_window);
        }

        // Set the default embedded HTML favicon
        void set_icon(const std::string_view icon, const std::string_view icon_type) const {
            webinix_set_icon(webinix_window, icon.data(), icon_type.data());
        }

        // Safely send raw data to the UI
        void send_raw(const std::string_view function, const void* raw, size_t size) const {
            webinix_send_raw(webinix_window, function.data(), raw, size);
        }

        // Run the window in hidden mode
        void set_hide(bool status) const {
            webinix_set_hide(webinix_window, status);
        }

        // Set window size
        void set_size(unsigned int width, unsigned int height) const {
            webinix_set_size(webinix_window, width, height);
        }

        // Set window minimum size (currently windows only.)
        void set_minimum_size(unsigned int width, unsigned int height) const {
        	webinix_set_minimum_size(webinix_window, width, height);
        }

        // Get the network port of a running window. This can be useful to determine the HTTP link of `webinix.js`
        size_t get_port() const {
            return webinix_get_port(webinix_window);
        }

        // Set a custom web-server network port to be used by Webinix. This can be useful to determine the HTTP
        // link of `webinix.js` in case you are trying to use Webinix with an external web-server like NGNIX
        bool set_port(size_t port) const {
            return webinix_set_port(webinix_window, port);
        }

        // Set window position
        void set_position(unsigned int x, unsigned int y) const {
            webinix_set_position(webinix_window, x, y);
        }

        // Delete a specific window web-browser local folder profile.
        void delete_profile() const {
            webinix_delete_profile(webinix_window);
        }

        // Get the ID of the parent process (The web browser may create another process for the window).
        size_t get_parent_process_id() const {
            return webinix_get_parent_process_id(webinix_window);
        }

        // Get the ID of the last child process spawned by the browser.
        size_t get_child_process_id() const {
            return webinix_get_child_process_id(webinix_window);
        }

        size_t get_best_browser() const {
            return webinix_get_best_browser(webinix_window);
        }

        // Set the web-server root folder path for this specific window.
        bool set_root_folder(const std::string_view path) const {
            return webinix_set_root_folder(webinix_window, path.data());
        }

        // Set a custom handler to serve files. This custom handler should return full HTTP header and body.
        // Resets previous handler set with `set_file_handler_window`.
        void set_file_handler(const void* (*handler)(const char* filename, int* length)) const {
            webinix_set_file_handler(webinix_window, handler);
        }

        // Set a custom handler to serve files. This custom handler should return full HTTP header and body.
        // Resets previous handler set with `set_file_handler`
        void set_file_handler_window(const void* (*handler)(size_t window, const char* filename, int* length)) const {
            webinix_set_file_handler_window(webinix_window, handler);
        }

        // Set the web browser profile to use. An empty `name` and `path` means the default user profile. Need
        // to be called before `webinix_show()`.
        void set_profile(const std::string_view name = {""}, const std::string_view path = {""}) const {
            webinix_set_profile(webinix_window, name.data(), path.data());
        }

        // Set the web browser proxy to use. Need to be called before `webinix_show()`.
        void set_proxy(const std::string_view proxy_server = {""}) const {
            webinix_set_proxy(webinix_window, proxy_server.data());
        }

        // Get the full current URL
        std::string_view get_url() const {
            return std::string_view{webinix_get_url(webinix_window)};
        }

        // Navigate to a specific URL.
        void navigate(const std::string_view url) const {
            webinix_navigate(webinix_window, url.data());
        }

        // Control if UI events coming from this window should be processed one at a time in a
        // single blocking thread `True`, or process every event in a new non-blocking thread `False`.
        void set_event_blocking(bool status) const {
            webinix_set_event_blocking(webinix_window, status);
        }

        // Show a WebView window using embedded HTML, or a file. If the window is already open, it will be refreshed.
        bool show_wv(const std::string_view content) const {
            return webinix_show_wv(webinix_window, content.data());
        }

        // Allow a specific window address to be accessible from a public network.
        void set_public(bool status) const {
            webinix_set_public(webinix_window, status);
        }

        // Enable or disable the window frame.
        void set_frameless(bool status) const {
            webinix_set_frameless(webinix_window, status);
        }

        // Enable or disable window background transparency.
        // When enabled, the window background will be transparent (if supported).
        void set_transparent(bool status) const {
            webinix_set_transparent(webinix_window, status);
        }

        // Enable or disable window resizing by user.
        void set_resizable(bool status) const {
            webinix_set_resizable(webinix_window, status);
        }

        // Set window position to the center of the screen.
        void set_center() const {
            webinix_set_center(webinix_window);
        }

        // Same as `webinix_show()`. But start only the web server and return the URL. No window will be shown.
        std::string_view start_server(const std::string_view content) const {
            return std::string_view{webinix_start_server(webinix_window, content.data())};
        }

        // Quickly run a JavaScript (no response waiting).
        void run(const std::string_view script) const {
            webinix_run(webinix_window, script.data());
        }

        // Run a JavaScript, and get the response back (Make sure your local buffer can hold the response).
        bool script(const std::string_view script, unsigned int timeout,
                    char* buffer, size_t buffer_length) const {
            return webinix_script(webinix_window, script.data(), timeout, buffer, buffer_length);
        }

        // Chose between Deno and Nodejs runtime for .js and .ts files.
        void set_runtime(unsigned int runtime) const {
            webinix_set_runtime(webinix_window, runtime);
        }

        // Use this API after using `bind()` to add any user data to it that can be
        // read later using `get_context()`.
        void set_context(const std::string_view element, void* context) const {
            webinix_set_context(webinix_window, element.data(), context);
        }

        // Gets Win32 window `HWND`. More reliable with WebView than web browser 
        // window, as browser PIDs may change on launch.
        void* win32_get_hwnd() const {
            return webinix_win32_get_hwnd(webinix_window);
        }
    };

    // ------ Namespace members `webinix::xxx()` ------

    // Wait until all opened windows get closed.
    inline void wait() {
        webinix_wait();
    }

    // Close all opened windows. wait() will break.
    inline void exit() {
        webinix_exit();
    }

    // Get an available usable free network port.
    inline size_t get_free_port() {
        return webinix_get_free_port();
    }

    // Open an URL in the native default web browser.
    inline void open_url(const std::string_view url) {
        webinix_open_url(url.data());
    }

    // Set the web-server root folder path for all windows.
    inline bool set_default_root_folder(const std::string_view path) {
        return webinix_set_default_root_folder(path.data());
    }

    // Get OS high contrast preference.
    inline bool is_high_contrast() {
        return webinix_is_high_contrast();
    }

    // Check if a web browser is installed.
    inline bool browser_exist(unsigned int browser) {
        return webinix_browser_exist(browser);
    }

    // Set the maximum time in seconds to wait for browser to start
    inline void set_timeout(unsigned int second) {
        webinix_set_timeout(second);
    }

    // Get the HTTP mime type of a file.
    inline std::string get_mime_type(const std::string_view file) {
        return std::string{webinix_get_mime_type(file.data())};
    }

    // Base64 encoding. Use this to safely send text based data to the UI. If it fails it will return NULL.
    inline std::string encode(const std::string_view str) {
        return std::string{webinix_encode(str.data())};
    }

    // Base64 decoding. Use this to safely decode received Base64 text from the UI. If it fails it will return NULL.
    inline std::string decode(const std::string_view str) {
        return std::string{webinix_decode(str.data())};
    }

    // Set the SSL/TLS certificate and the private key content, both in PEM format.
    // This works only with `webinix-2-secure` library. If set empty Webinix will generate a self-signed certificate.
    inline bool set_tls_certificate(const std::string_view certificate_pem,
                                    const std::string_view private_key_pem) {
        return webinix_set_tls_certificate(certificate_pem.data(), private_key_pem.data());
    }

    // Safely free a buffer allocated by Webinix, for example when using webinix_encode().
    inline void free(void* ptr) {
        webinix_free(ptr);
    }

    // Safely free a buffer allocated by Webinix, for example when using webinix_encode().
    inline void* malloc(size_t size) {
        return webinix_malloc(size);
    }

    // Free all memory resources. Should be called only at the end.
    inline void clean() {
        webinix_clean();
    }

    // Delete all local web-browser profiles folder. It should called at the end.
    inline void delete_all_profiles() {
        webinix_delete_all_profiles();
    }

    // Get a free window number that can be used with `webinix_new_window_id()`.
    inline size_t get_new_window_id() {
        return webinix_get_new_window_id();
    }

    // Control the Webinix behaviour. Should be called at the beginning.
    inline void set_config(webinix_config option, bool status) {
        webinix_set_config(option, status);
    }

    // Check if the app is still running.
    inline bool is_app_running() {
        return webinix_interface_is_app_running();
    }

    // Copy raw data.
    inline void memcpy(void* dest, const void* src, size_t count) {
        webinix_memcpy(dest, const_cast<void*>(src), count);
    }

} // namespace webinix

#endif /* _WEBUI_HPP */
