#include "asio_standalone.hpp"
#include <string>
#include <functional>

namespace trane
{
    template<typename Proto>
    class Resolver
    {
        static_assert(std::is_same<Proto, asio::ip::tcp>::value || std::is_same<Proto, asio::ip::udp>::value,
                "Trane Resolver is only defined for TCP and UDP");
    public:
        typedef std::function<void(const asio::error_code& ec, typename Proto::resolver::iterator)> Callback;
        Resolver(asio::io_service& ios);
        virtual void resolve(const std::string& host, const std::string& port, Callback callback);

    private:
        typename Proto::resolver m_resolver;
    };

}


template<typename Proto>
trane::Resolver<Proto>::Resolver(asio::io_service& ios)
    : m_resolver{ios}
{ }


template<typename Proto>
void trane::Resolver<Proto>::resolve(const std::string& host, const std::string& port, Callback callback)
{
    m_resolver.async_resolve(host, port, callback);
}
