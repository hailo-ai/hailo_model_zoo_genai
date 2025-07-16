/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file main.cpp
 * @brief Hailo Ollama server main
 **/

#include <csignal>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

#include <hailo/genai/llm/llm.hpp>
#include <oatpp/macro/component.hpp>
#include <oatpp/network/Server.hpp>

#include "app_component.hpp"
#include "config/runtime_config.hpp"
#include "controller/controller.hpp"
#include "generation_context/deconfigure.hpp"
#include "generation_context/generation_context.hpp"
#include "model/blob_resource.hpp"
#include "model/simple_store.hpp"
#include "utils/path.hpp"
#include "utils/split.hpp"

namespace {
volatile std::sig_atomic_t global_signal_status;
std::atomic<bool> global_shutdown_requested_flag = false;
// must guarantee that we can safely use global_shutdown_requested_flag in signal handler
static_assert(std::atomic<bool>::is_always_lock_free);
}  // namespace

// signal handler - can't do anything but use our safe global flags
void signal_handler(int signal) {
    global_signal_status = signal;
    global_shutdown_requested_flag = true;
}

namespace fs = std::filesystem;
using json = nlohmann::ordered_json;

fs::path
find_dir(const fs::path& user_dir, const std::string& additional_dirs) {
    std::vector<fs::path> options = {
        user_dir,
    };

    for (const auto& path : SplitRange(additional_dirs, ":")) {
        options.push_back(fs::path(path));
    }

    for (const auto& option : options) {
        auto dir_path = option / HAILO_DIR_NAME;
        const auto is_dir = fs::is_directory(fs::status(dir_path));
        if (is_dir) {
            return dir_path;
        }
    }
    throw std::runtime_error("hailo-ollama directory not found");
}

fs::path find_config_dir() {
    return find_dir(config_home(), system_config_home());
}

fs::path find_data_dir() {
    return find_dir(data_home(), system_data_home());
}

void run() {
    const auto config_file_path = find_config_dir() / HAILO_CONFIG_NAME;
    std::ifstream config_stream(config_file_path);
    const auto config_json = json::parse(config_stream);
    const auto config = config_json.template get<RuntimeConfig>();

    /* Register Components in scope of run() method */
    AppComponent components(config.server.host, config.server.port);

    /* Get router component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    auto generation_context = std::make_shared<SyncGenerationContext>();
    Deconfigure deconfigure_loop(generation_context);
    std::thread deconfigure_thread(
        &Deconfigure::deconfigure_loop,
        &deconfigure_loop
    );

    /* Create MyController and add all of its endpoints to router */
    const auto model_directory = find_data_dir() / HAILO_MODELS;
    auto model_store = std::make_shared<SimpleModelStore>(
        model_directory / HAILO_MODEL_MANIFEST
    );
    const auto blob_directory = model_directory / HAILO_BLOB_DIR_NAME;
    (void)fs::create_directory(blob_directory);
    auto resource_provider = std::make_shared<BlobResourceProvider>(
        blob_directory,
        config.library.host,
        config.library.port
    );
    router->addController(
        std::make_shared<MyController>(
            generation_context,
            model_store,
            resource_provider
        )
    );

    /* Get connection handler component */
    OATPP_COMPONENT(
        std::shared_ptr<oatpp::network::ConnectionHandler>,
        connectionHandler
    );

    /* Get connection provider component */
    OATPP_COMPONENT(
        std::shared_ptr<oatpp::network::ServerConnectionProvider>,
        connectionProvider
    );

    /* Create server which takes provided TCP connections and passes them to HTTP
   * connection handler */
    oatpp::network::Server server(connectionProvider, connectionHandler);

    /* Print info about server port */
    OATPP_LOGi(
        "MyApp",
        "Server running on port {}",
        connectionProvider->getProperty("port").toString()
    );

    /* Run server */
    (void)std::signal(SIGINT, signal_handler);
    (void)std::signal(SIGTERM, signal_handler);
    std::thread server_thread([&server] { server.run(); });
    while (!global_shutdown_requested_flag) {
        // wait for signal. We want sigwait here but it's platform-specific
        std::this_thread::sleep_for(
            std::chrono::milliseconds(config.main_poll_time_ms)
        );
    }
    OATPP_LOGi(
        "MyApp",
        "Stop signal received, please wait for server shutdown"
    );
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
}

/**
 *  main
 */
int main(int argc, const char* argv[]) {
    oatpp::Environment::init();

    run();

    /* Print how much objects were created during app running, and what have
   * left-probably leaked */
    /* Disable object counting for release builds using '-D
   * OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
    std::cout << "\nEnvironment:\n";
    std::cout << "objectsCount = " << oatpp::Environment::getObjectsCount()
              << "\n";
    std::cout << "objectsCreated = " << oatpp::Environment::getObjectsCreated()
              << "\n\n";

    oatpp::Environment::destroy();

    return 0;
}
