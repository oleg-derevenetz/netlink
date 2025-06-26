#include <charconv>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string_view>
#include <system_error>

#include <netlink/msg.h>
#include <netlink/socket.h>

int main( int argc, char ** argv )
{
    if ( argc != 3 ) {
        std::cerr << "Syntax: client port json\n";
        return -1;
    }

    uint32_t port;

    {
        const char * const portStr = argv[1];
        const char * const portStrEnd = portStr + strlen( portStr );

        if ( const auto [ptr, ec] = std::from_chars( portStr, portStrEnd, port ); ptr != portStrEnd || ec != std::errc() ) {
            std::cerr << "Invalid port number: " << portStr;
            return -1;
        }
    }

    const std::unique_ptr<nl_sock, void ( * )( nl_sock * )> sock{ nl_socket_alloc(), nl_socket_free };
    if ( !sock ) {
        std::cerr << "nl_socket_alloc() failed\n";
        return -1;
    }

    if ( const int res = nl_socket_modify_cb(
             sock.get(), NL_CB_VALID, NL_CB_CUSTOM,
             []( nl_msg * msg, void * ) -> int {
                 const nlmsghdr * hdr = nlmsg_hdr( msg );

                 const int dataLen = nlmsg_datalen( hdr );
                 if ( dataLen < 0 ) {
                     std::cerr << "Invalid response\n";
                     return NL_OK;
                 }

                 std::cout << std::string_view{ static_cast<const char *>( nlmsg_data( hdr ) ), static_cast<std::size_t>( dataLen ) } << "\n";

                 return NL_OK;
             },
             nullptr );
         res < 0 ) {
        std::cerr << "nl_socket_modify_cb() failed: " << nl_geterror( res ) << "\n";
        return -1;
    }

    nl_socket_disable_seq_check( sock.get() );

    if ( const int res = nl_connect( sock.get(), NETLINK_GENERIC ); res < 0 ) {
        std::cerr << "nl_connect() failed: " << nl_geterror( res ) << "\n";
        return -1;
    }

    nl_socket_set_peer_port( sock.get(), port );

    if ( const int res = nl_send_simple( sock.get(), 0, 0, argv[2], strlen( argv[2] ) ); res < 0 ) {
        std::cerr << "nl_send_simple() failed: " << nl_geterror( res ) << "\n";
        return -1;
    }

    if ( const int res = nl_recvmsgs_default( sock.get() ); res < 0 ) {
        std::cerr << "nl_recvmsgs_default() failed: " << nl_geterror( res ) << "\n";
        return -1;
    }

    return 0;
}
