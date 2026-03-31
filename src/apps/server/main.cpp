/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file main.cpp
 * @brief Hailo Ollama server main
 **/

#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

#include <hailo/genai/llm/llm.hpp>
#include <oatpp/macro/component.hpp>
#include <oatpp/network/Server.hpp>

#include "app_component.hpp"
#include "config/static_config.hpp"
#include "controller/controller.hpp"
#include "generation_context/deconfigure.hpp"
#include "generation_context/generation_context.hpp"
#include "model/blob_resource.hpp"
#include "model/simple_store.hpp"
#include "utils/path.hpp"
#include "utils/split.hpp"

namespace
{
volatile std::sig_atomic_t global_signal_status;
std::atomic<bool> global_shutdown_requested_flag = false;
// must guarantee that we can safely use global_shutdown_requested_flag in signal handler
static_assert(std::atomic<bool>::is_always_lock_free);
} // namespace

// signal handler - can't do anything but use our safe global flags
void signal_handler(int signal)
{
    global_signal_status = signal;
    global_shutdown_requested_flag = true;
}

namespace fs = std::filesystem;
using json = nlohmann::ordered_json;
using namespace hailo_ollama;

fs::path find_dir(const fs::path &user_dir, const std::string &additional_dirs, const fs::path &subdir = "")
{
    std::vector<fs::path> options = {
        user_dir,
    };

    for (const auto &path : SplitRange(additional_dirs, PATH_DELIMITER)) {
        options.push_back(fs::path(path));
    }

    for (const auto &option : options) {
        auto dir_path = option / HAILO_DIR_NAME / subdir;
        const auto is_dir = fs::is_directory(fs::status(dir_path));
        if (is_dir) {
            return option / HAILO_DIR_NAME;
        }
    }
    throw std::runtime_error("hailo-ollama directory not found");
}

// Find data dir that contains manifests (installed data, not user blobs)
fs::path find_manifest_data_dir()
{
    return find_dir(data_home(), system_data_home(), fs::path(HAILO_MODELS) / HAILO_MODEL_MANIFEST);
}

// Parse OLLAMA_HOST environment variable in the format "host:port" or just "host"
// Returns the parsed host and port, using defaults if not specified
config::ConnectionDetails parse_ollama_host(const char *ollama_host_env, const config::ConnectionDetails &defaults)
{
    if (!ollama_host_env || *ollama_host_env == '\0') {
        OATPP_LOGw("MyApp", "OLLAMA_HOST is unset or empty, using defaults");
        return defaults;
    }

    config::ConnectionDetails result = defaults;
    std::string input(ollama_host_env);

    // Reject raw IPv6 (must be bracketed)
    if ((input.find(':') != std::string_view::npos) && (input.front() != '[') && (input.find(':') != input.rfind(':'))) {
        OATPP_LOGw("MyApp", "Invalid OLLAMA_HOST: '{}' contains unbracketed IPv6, using defaults", input);
        return defaults;
    }

    std::string host;
    std::string port;

    // IPv6 case: [addr] or [addr]:port
    if (input.front() == '[') {
        auto close_bracket_index = input.find(']');
        if (close_bracket_index == std::string_view::npos) {
            OATPP_LOGw("MyApp", "Invalid OLLAMA_HOST: '{}' has '[' without matching ']', using defaults", input);
            return defaults;
        }

        host = input.substr(1, close_bracket_index - 1);

        if (close_bracket_index + 1 < input.size()) {
            if (input[close_bracket_index + 1] != ':') {
                OATPP_LOGw("MyApp", "Invalid OLLAMA_HOST: '{}' has invalid characters after ']', using defaults", input);
                return defaults;
            }
            port = input.substr(close_bracket_index + 2);
        }
    } else {
        // IPv4 / hostname case: host:port
        auto colon_index = input.find(':');
        if (colon_index != std::string_view::npos) {
            host = input.substr(0, colon_index);
            port = input.substr(colon_index + 1);
        } else {
            host = input;
        }
    }

    // Host must not be empty
    if (host.empty()) {
        OATPP_LOGw("MyApp", "Invalid OLLAMA_HOST: '{}' has empty host, using defaults", input);
        return defaults;
    }

    result.host = host;

    // Parse port if present
    if (!port.empty()) {
        if (!std::all_of(port.begin(), port.end(), [](unsigned char c) { return std::isdigit(c); })) {
            OATPP_LOGw("MyApp", "Invalid OLLAMA_HOST: '{}' has non-numeric port '{}', using defaults", input, port);
            return defaults;
        }

        try {
            unsigned long value = std::stoul(port);
            if (value < config::MIN_PORT_NUMBER || value > config::MAX_PORT_NUMBER) {
                OATPP_LOGw("MyApp", "Invalid OLLAMA_HOST: '{}' has out-of-range port {}, using defaults", input, value);
                return defaults;
            }
            result.port = static_cast<uint16_t>(value);
        } catch (const std::exception& e) {
            OATPP_LOGw("MyApp", "Invalid OLLAMA_HOST: '{}' port parsing failed ({}), using defaults", input, e.what());
            return defaults;
        }
    }

    return result;
}

config::ConnectionDetails get_server_connection_details()
{
    // Override server host/port from OLLAMA_HOST environment variable if set
    if (const char *ollama_host_env = std::getenv(config::OLLAMA_HOST_ENV_VAR.c_str())) {
        return parse_ollama_host(ollama_host_env, config::DEFAULT_SERVER_OLLAMA_CONNECTION);
    } else {
        return config::DEFAULT_SERVER_OLLAMA_CONNECTION;
    }
}
config::ConnectionDetails get_library_connection_details()
{
    // Override library host/port from HAILO_OLLAMA_LIBRARY_HOST environment variable if set (internal use only)
    if (const char *library_host_env = std::getenv(config::HAILO_OLLAMA_LIBRARY_HOST_ENV_VAR.c_str())) {
        return parse_ollama_host(library_host_env, config::DEFAULT_LIBRARY_OLLAMA_CONNECTION);
    } else {
        return config::DEFAULT_LIBRARY_OLLAMA_CONNECTION;
    }
}

void run()
{
    try {
        auto server_connection = get_server_connection_details();
        auto library_connection = get_library_connection_details();
        OATPP_LOGi("MyApp", "Using 'OLLAMA_HOST' server connection: {}:{}", server_connection.host, server_connection.port);

        /* Register Components in scope of run() method */
        AppComponent components(server_connection.host, server_connection.port);

        /* Get router component */
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

        // Read VDevice group_id from environment variable
        std::optional<std::string> vdevice_group_id;
        if (const char* group_id_env = std::getenv(config::HAILO_OLLAMA_VDEVICE_GROUP_ID_ENV_VAR.c_str())) {
            vdevice_group_id = group_id_env;
            OATPP_LOGi("MyApp", "Using VDevice group_id: {}", *vdevice_group_id);
        }

        auto generation_context = std::make_shared<SyncGenerationContext>(std::move(vdevice_group_id));
        Deconfigure deconfigure_loop(generation_context);
        std::thread deconfigure_thread(&Deconfigure::deconfigure_loop, &deconfigure_loop);

        /* Create MyController and add all of its endpoints to router */
        // Manifests are read-only (installed in system dirs like /usr/local/share)
        fs::path manifest_directory;
        try {
            manifest_directory = find_manifest_data_dir() / HAILO_MODELS / HAILO_MODEL_MANIFEST;
        } catch (const std::exception &e) {
            OATPP_LOGe("MyApp", "Failed to find manifest directory: {}", e.what());
            throw;
        }
        
        auto model_store = std::make_shared<SimpleModelStore>(manifest_directory);

        // Blobs are user-writable (stored in user's home directory)
        const auto user_blob_directory = data_home() / HAILO_DIR_NAME / HAILO_MODELS / HAILO_BLOB_DIR_NAME;
        fs::create_directories(user_blob_directory);
        auto resource_provider =
            std::make_shared<BlobResourceProvider>(user_blob_directory, library_connection.host, library_connection.port);
        router->addController(std::make_shared<MyController>(generation_context, model_store, resource_provider));

        /* Get connection handler component */
        OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

        /* Get connection provider component */
        OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

        /* Create server which takes provided TCP connections and passes them to HTTP
         * connection handler */
        oatpp::network::Server server(connectionProvider, connectionHandler);

        /* Print info about server port */
        OATPP_LOGi("MyApp", "Server running on port {}", connectionProvider->getProperty("port").toString());


        /* Run server */
        (void)std::signal(SIGINT, signal_handler);
        (void)std::signal(SIGTERM, signal_handler);
        std::thread server_thread([&server] { server.run(); });
        while (!global_shutdown_requested_flag) {
            // wait for signal. We want sigwait here but it's platform-specific
            std::this_thread::sleep_for(std::chrono::milliseconds(config::DEFAULT_MAIN_POLL_TIME_MS));
        }
        OATPP_LOGi("MyApp", "Stop signal received, please wait for server shutdown");
        // reinstall the default handler - allow user to kill immediately with extra CTRL+C
        (void)std::signal(SIGINT, SIG_DFL);

        /* First, stop the ServerConnectionProvider so we don't accept any new connections */
        connectionProvider->stop();

        /* Now, check if server is still running and stop it if needed */
        if (server.getStatus() == oatpp::network::Server::STATUS_RUNNING) {
            server.stop();
        }

        /* Finally, stop the ConnectionHandler and wait until all running connections are closed */
        connectionHandler->stop();

        // Stop the deconfigure thread
        generation_context->lock()->stop();

        /* Before returning, check if the server-thread has already stopped or if we need to wait for the server to stop */
        if (server_thread.joinable()) {
            /* We need to wait until the thread is done */
            server_thread.join();
        }
        if (deconfigure_thread.joinable()) {
            deconfigure_thread.join();
        }
    } catch (const std::exception &e) {
        OATPP_LOGe("MyApp", "Exception in run(): {}", e.what());
        throw;
    }
}

/**
 *  main
 */
int main(int argc, const char *argv[])
{
    oatpp::Environment::init();

    try {
        run();
    } catch (const std::exception &e) {
        OATPP_LOGe("MyApp", "Fatal error: {}", e.what());
        oatpp::Environment::destroy();
        return 1;
    }

    /* Print how much objects were created during app running, and what have
     * left-probably leaked */
    /* Disable object counting for release builds using '-D
     * OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
    std::cout << "\nEnvironment:\n";
    std::cout << "objectsCount = " << oatpp::Environment::getObjectsCount() << "\n";
    std::cout << "objectsCreated = " << oatpp::Environment::getObjectsCreated() << "\n\n";

    oatpp::Environment::destroy();

    return 0;
}
