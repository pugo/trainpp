#ifndef TRAINPP_Z21_H
#define TRAINPP_Z21_H

#include <boost/asio.hpp>

class Z21
{
public:
    Z21();
    ~Z21();

    bool connect();
    void listen();

    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);

private:
    std::vector<uint8_t> recv_buf;

    boost::asio::io_context io_context;
    boost::asio::ip::udp::endpoint receiver_endpoint;
    boost::asio::ip::udp::socket socket;
//    boost::asio::ip::udp::endpoint remote_endpoint;

    bool sent_get_hw_info;
    bool sent_set_bc_flags;

};


#endif //TRAINPP_Z21_H
