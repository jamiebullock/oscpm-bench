/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "matchers.h"

#include <lo/lo.h>
#include <lo/lo_lowlevel.h>

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>

namespace oscpm_compare
{

namespace
{
    int countHit(const char*, const char*, lo_arg**, int, lo_message, void* userData) noexcept
    {
        ++*static_cast<std::size_t*>(userData);
        return 0;
    }

    struct Server
    {
        lo_server server = nullptr;
        void* data = nullptr;
        std::size_t size = 0;
        std::size_t count = 0;

        explicit Server(const char* message)
            : server(lo_server_new(nullptr, nullptr))
        {
            if (!server)
            {
                return;
            }
            for (const std::string& address : spaceAddresses())
            {
                lo_server_add_method(server, address.c_str(), nullptr, countHit, &count);
            }
            const lo_message empty = lo_message_new();
            data = lo_message_serialise(empty, message, nullptr, &size);
            lo_message_free(empty);
        }

        ~Server()
        {
            std::free(data);
            if (server)
            {
                lo_server_free(server);
            }
        }

        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
    };

    struct LibloSpace
    {
        static constexpr const char* kName = "liblo";

        struct Prepared
        {
            std::shared_ptr<Server> server;

            std::size_t operator()() const noexcept
            {
                server->count = 0;
                lo_server_dispatch_data(server->server, server->data, server->size);
                return server->count;
            }
        };

        static std::optional<Prepared> prepare(const SpaceCase& spaceCase)
        {
            if (spaceCase.repeated)
            {
                return std::nullopt;
            }
            auto server = std::make_shared<Server>(spaceCase.message);
            if (!server->server || !server->data)
            {
                return std::nullopt;
            }
            return Prepared { server };
        }
    };
}

SpaceRegistration registerLibloSpace(const std::vector<SpaceCase>& cases)
{
    return registerDispatcher<LibloSpace>(cases);
}

}
