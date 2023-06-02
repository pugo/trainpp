#include <iostream>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>


#include "z21.h"
#include "z21_dataset.h"


using boost::asio::ip::udp;



Z21::Z21() :
    socket(io_context),
    sent_get_hw_info{false},
    sent_set_bc_flags{false},
    m_serial_number{0},
    m_hw_type{0},
    m_fw_version{""}
{
    recv_buf.resize(128);

    command_handlers[0x10] = new LanGetSerialNumber();
    command_handlers[0x1a] = new LanGetHWInfo();
    command_handlers[0x51] = new LanGetBroadcastFlags();
    command_handlers[0x84] = new LanSystemstateDatachanged();
}


Z21::~Z21()
{}


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
        socket.send_to(boost::asio::buffer(LanGetSerialNumber().pack()), receiver_endpoint);

        socket.async_receive_from(boost::asio::buffer(recv_buf), receiver_endpoint,
                boost::bind(&Z21::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

   io_context.run();
}


void Z21::handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    BOOST_LOG_TRIVIAL(debug) << "Z21 serial number: " << m_serial_number;
    BOOST_LOG_TRIVIAL(debug) << "Z21 HW type: " << m_hw_type;
    BOOST_LOG_TRIVIAL(debug) << "Z21 firmware version: " << m_fw_version;

    if (!error || error == boost::asio::error::message_size) {
        size_t pos = 0;
        while (bytes_transferred - pos >= 4) {
            uint16_t size = recv_buf[pos] | (recv_buf[pos + 1] << 8);
            uint16_t id = recv_buf[pos + 2] | (recv_buf[pos + 3] << 8);

            const auto dataset_start = recv_buf.begin() + pos;
            const auto data_start = dataset_start + header_size;
            std::vector<uint8_t> data(data_start, data_start + size - header_size);

            handle_dataset(size, id, data);
            pos += size;
        }
    }

    if (!sent_get_hw_info) {
        socket.send_to(boost::asio::buffer(LanGetHWInfo().pack()), receiver_endpoint);
        sent_get_hw_info = true;
    }
    else if (!sent_set_bc_flags) {
        LanSetBroadcastFlags sbf(BroadcastFlags::DRIVING_AND_SWITCHING | BroadcastFlags::Z21_STATUS_CHANGES);
        socket.send_to(boost::asio::buffer(sbf.pack()), receiver_endpoint);
        socket.send_to(boost::asio::buffer(LanGetBroadcastFlags().pack()), receiver_endpoint);
        sent_set_bc_flags = true;
    }

    socket.async_receive_from(
            boost::asio::buffer(recv_buf), receiver_endpoint,
            boost::bind(&Z21::handle_receive, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
}


void Z21::handle_dataset(uint16_t size, uint16_t id, std::vector<uint8_t>& data)
{
    BOOST_LOG_TRIVIAL(debug) << "Received for ID " << std::hex << (int)id << ": " << PRINT_HEX(data);
    if (command_handlers.contains(id)) {
        Z21_DataSet* ds = command_handlers[id];
        ds->unpack(data);

        switch(id)
        {
            case 0x10:
                m_serial_number = static_cast<LanGetSerialNumber*>(ds)->serial_number;
                break;
            case 0x1a:
                m_hw_type = static_cast<LanGetHWInfo*>(ds)->hw_type;
                m_fw_version = static_cast<LanGetHWInfo*>(ds)->fw_version;
                break;
            case 0x84:
            {
                LanSystemstateDatachanged* ss = static_cast<LanSystemstateDatachanged*>(ds);
                BOOST_LOG_TRIVIAL(debug) << "-- Main current: " << (int)ss->main_current << " mA";
                BOOST_LOG_TRIVIAL(debug) << "-- Prog current: " << (int)ss->prog_current << " mA";
                BOOST_LOG_TRIVIAL(debug) << "-- Filtered main current: " << (int)ss->filtered_main_current << " mA";
                BOOST_LOG_TRIVIAL(debug) << "-- Temperature: " << (int)ss->temperature;
                BOOST_LOG_TRIVIAL(debug) << "-- Supply voltage: " << (int)ss->supply_voltage / 1000 << " V";
                BOOST_LOG_TRIVIAL(debug) << "-- VCC voltage: " << (int)ss->vcc_voltage / 1000 << " V";
                BOOST_LOG_TRIVIAL(debug) << "-- Central state: " << (int)ss->central_state;
                BOOST_LOG_TRIVIAL(debug) << "-- Central state Ex: " << (int)ss->central_state_ex;
                BOOST_LOG_TRIVIAL(debug) << "-- Capabilities: " << (int)ss->capabilities;
            }

            default:
                break;
        }
    }
}

