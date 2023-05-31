#include <iostream>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/array.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>

#include "z21.h"


using boost::asio::ip::udp;
using boost::adaptors::transformed;

#define PRINT_HEX(v) (boost::algorithm::join(v | transformed([](uint8_t d) { return (boost::format("%02x") % (unsigned)d).str(); }), " "))


Z21::Z21() :
    socket(io_context),
    sent_get_hw_info{false},
    sent_set_bc_flags{false}
{
    recv_buf.resize(128);
}


Z21::~Z21()
{
}


bool Z21::connect()
{
    try
    {
        udp::resolver resolver(io_context);
        receiver_endpoint = *resolver.resolve(udp::v4(), "192.168.0.111", "21105").begin();
        socket.open(udp::v4());
    }
    catch(std::exception& e)
    {
        std::cout <<  e.what() << std::endl;
        return false;
    }

    return true;
}


void Z21::listen()
{
    try
    {
        boost::array<char, 4> send_buf  = {{ 0x04, 0x00, 0x10, 0x00 }};
        socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);

        socket.async_receive_from(
                boost::asio::buffer(recv_buf), receiver_endpoint,
                boost::bind(&Z21::handle_receive, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

   io_context.run();
}

void Z21::handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (!error || error == boost::asio::error::message_size)
    {
        BOOST_LOG_TRIVIAL(debug) << "Received some: " << bytes_transferred;

        std::vector<uint8_t> version(recv_buf.begin(), recv_buf.begin() + recv_buf[0]);
        BOOST_LOG_TRIVIAL(debug) << "Received: " << PRINT_HEX(version);

        if (!sent_get_hw_info) {
            boost::array<char, 4> send_buf  = {{ 0x04, 0x00, 0x1a, 0x00 }};
            socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);
            sent_get_hw_info = true;
        }
        else if (!sent_set_bc_flags) {
            boost::array<char, 8> send_buf  = {{ 0x08, 0x00, 0x50, 0x00, 0x01, 0x01, 0x00, 0x00 }};
            socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);
            sent_set_bc_flags = true;
        }

        socket.async_receive_from(
                boost::asio::buffer(recv_buf), receiver_endpoint,
                boost::bind(&Z21::handle_receive, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    }
}
