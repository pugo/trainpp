#ifndef TRAINPP_Z21_H
#define TRAINPP_Z21_H

#include <boost/asio.hpp>

class Z21_DataSet;


class Z21
{
public:
    Z21();
    ~Z21();

    bool connect();
    void listen();

    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
    void handle_dataset(uint16_t size, uint16_t id, std::vector<uint8_t>& data);

    uint32_t serial_number() const { return m_serial_number; }

    uint32_t hw_type() const { return m_hw_type; }
    const std::string& fw_version() const { return m_fw_version; }

private:
    std::vector<uint8_t> recv_buf;
    std::map<uint16_t, Z21_DataSet*> command_handlers;


    boost::asio::io_context io_context;
    boost::asio::ip::udp::endpoint receiver_endpoint;
    boost::asio::ip::udp::socket socket;
//    boost::asio::ip::udp::endpoint remote_endpoint;

    bool sent_get_hw_info;
    bool sent_set_bc_flags;

    uint32_t m_serial_number;
    uint32_t m_hw_type;
    std::string m_fw_version;
};


#endif //TRAINPP_Z21_H
