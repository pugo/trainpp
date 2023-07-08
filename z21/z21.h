#ifndef TRAINPP_Z21_H
#define TRAINPP_Z21_H

#include <map>

#include <boost/asio.hpp>

class Z21_DataSet;

/**
 * Represents an instance of a Roco Z21.
 */
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

    bool sent_get_hw_info{false};
    bool sent_set_bc_flags{false};
    bool sent_get_system_state{false};
    bool sent_lan_get_code{false};
    bool sent_set_cv{false};

    uint32_t m_serial_number{0};

    uint32_t m_hw_type{0};
    std::string m_fw_version{};

    int16_t m_main_current{0};
    int16_t m_prog_current{0};
    int16_t m_filtered_main_current{0};
    int16_t m_temperature{0};

    uint16_t m_supply_voltage{0};
    uint16_t m_vcc_voltage{0};

    uint8_t m_central_state{0};
    uint8_t m_central_state_ex{0};
    uint8_t m_capabilities{0};

    bool emergency_stop{false};
    bool track_voltage_off{false};
    bool short_cirtcuit{false};
    bool programming_mode{false};
};


#endif //TRAINPP_Z21_H
