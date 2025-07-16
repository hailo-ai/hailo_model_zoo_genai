/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file app_component.hpp
 * @brief Components to be injected to te server
 **/

#pragma once

#include <cstdint>

#include <oatpp/json/ObjectMapper.hpp>
#include <oatpp/macro/component.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/web/mime/ContentMappers.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>

/**
 *  Class which creates and holds Application components and registers
 * components in oatpp::base::Environment Order of components initialization is
 * from top to bottom
 */
class AppComponent {
  public:
    AppComponent(const std::string& host, uint16_t port) :
        serverConnectionProvider(
            oatpp::network::tcp::server::ConnectionProvider::createShared(
                {host, port, oatpp::network::Address::IP_4}
            )
        ) {}

    /**
   *  Create ConnectionProvider component which listens on the port
   */
    oatpp::Environment::Component<
        std::shared_ptr<oatpp::network::ServerConnectionProvider>>
        serverConnectionProvider;
    /**
   *  Create Router component
   */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)(
        [] { return oatpp::web::server::HttpRouter::createShared(); }()
    );

    /**
   *  Create ConnectionHandler component which uses Router component to route
   * requests
   */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)(
        [] {
            OATPP_COMPONENT(
                std::shared_ptr<oatpp::web::server::HttpRouter>,
                router
            );  // get Router component
            return oatpp::web::server::HttpConnectionHandler::createShared(
                router
            );
        }()
    );

    /**
   *  Create ObjectMapper component to serialize/deserialize DTOs in Controller's
   * API
   */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::mime::ContentMappers>, apiContentMappers)(
        [] {
            auto json = std::make_shared<oatpp::json::ObjectMapper>();
            // json->serializerConfig().json.useBeautifier = true;
            json->serializerConfig().json.includeNullElements = false;
            // json->serializerConfig().json.alwaysIncludeRequired = true;

            auto mappers = std::make_shared<oatpp::web::mime::ContentMappers>();
            mappers->putMapper(json);
            mappers->setDefaultMapper(json);

            return mappers;
        }()
    );
};
