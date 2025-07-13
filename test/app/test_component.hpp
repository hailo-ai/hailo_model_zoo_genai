/**
 * Copyright (c) 2019-2025 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file test_component.hpp
 * @brief Components for the test server
 **/

#ifndef TestComponent_htpp
#define TestComponent_htpp

#include <oatpp/json/ObjectMapper.hpp>
#include <oatpp/macro/component.hpp>
#include <oatpp/network/virtual_/Interface.hpp>
#include <oatpp/network/virtual_/client/ConnectionProvider.hpp>
#include <oatpp/network/virtual_/server/ConnectionProvider.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>

/**
 * Test Components config
 */
class TestComponent {
  public:
    /**
   * Create oatpp virtual network interface for test networking
   */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, virtualInterface)(
        [] {
            return oatpp::network::virtual_::Interface::obtainShared(
                "virtualhost"
            );
        }()
    );

    /**
   * Create server ConnectionProvider of oatpp virtual connections for test
   */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)(
        [] {
            OATPP_COMPONENT(
                std::shared_ptr<oatpp::network::virtual_::Interface>,
                _interface
            );
            return oatpp::network::virtual_::server::ConnectionProvider::
                createShared(_interface);
        }()
    );

    /**
   * Create client ConnectionProvider of oatpp virtual connections for test
   */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, clientConnectionProvider)(
        [] {
            OATPP_COMPONENT(
                std::shared_ptr<oatpp::network::virtual_::Interface>,
                _interface
            );
            return oatpp::network::virtual_::client::ConnectionProvider::
                createShared(_interface);
        }()
    );

    /**
   *  Create Router component
   */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)(
        [] { return oatpp::web::server::HttpRouter::createShared(); }()
    );

    /**
   *  Create ConnectionHandler component which uses Router component to route requests
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
   *  Create ObjectMapper component to serialize/deserialize DTOs in Controller's API
   */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::mime::ContentMappers>, apiContentMappers)(
        [] {
            auto json = std::make_shared<oatpp::json::ObjectMapper>();
            json->serializerConfig().json.includeNullElements = false;

            auto mappers = std::make_shared<oatpp::web::mime::ContentMappers>();
            mappers->putMapper(json);

            return mappers;
        }()
    );
};

#endif  // TestComponent_htpp
