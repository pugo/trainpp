/* TrainPP
 * Copyright (C) 2023 Anders Piniesjö
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <iostream>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>

#include "z21.h"
#include "z21_dataset.h"
#include "lan_x_command.h"

using boost::asio::ip::udp;


Z21::Z21(const std::string& z21_host, const std::string& z21_port) :
    host(z21_host), port(z21_port),
    socket(io_context)
{
    recv_buf.resize(128);
    command_handlers[Z21_DataSet::LAN_GET_SERIAL_NUMBER] = new LanGetSerialNumber();
    command_handlers[Z21_DataSet::LAN_GET_CODE] = new LanGetCode();
    command_handlers[Z21_DataSet::LAN_GET_HWINFO] = new LanGetHWInfo();
    command_handlers[Z21_DataSet::LAN_GET_BROADCASTFLAGS] = new LanGetBroadcastFlags();
    command_handlers[Z21_DataSet::LAN_GET_LOCOMODE] = new LanGetLocomode();
    command_handlers[Z21_DataSet::LAN_GET_TURNOUTMODE] = new LanGetTurnoutmode();
    command_handlers[Z21_DataSet::LAN_SYSTEMSTATE_DATACHANGED] = new LanSystemstateDatachanged();
    command_handlers[0x40] = new LanX();
}

Z21::~Z21()
{
    listen_thread.join();

    for (auto& item: command_handlers) {
        delete item.second;
    }
}

bool Z21::connect()
{
    try
    {
        BOOST_LOG_TRIVIAL(info) << "Connecting to Z21: host = " << host << ", port = " << port;
        udp::resolver resolver(io_context);
        receiver_endpoint = *resolver.resolve(udp::v4(), host, port).begin();
        socket.open(udp::v4());
    }
    catch(std::exception& e)
    {
        std::cout <<  e.what() << std::endl;
        return true;
        return false;
    }

    return true;
}

void Z21::listen()
{
    listen_thread = std::thread(&Z21::listen_thread_fn, this);
}

void Z21::listen_thread_fn()
{
    BOOST_LOG_TRIVIAL(debug) << "Running Z21 listener thread";
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
//    BOOST_LOG_TRIVIAL(debug) << "- Z21 serial number: " << m_serial_number;
//    BOOST_LOG_TRIVIAL(debug) << "- Z21 HW type: " << m_hw_type;
//    BOOST_LOG_TRIVIAL(debug) << "- Z21 firmware version: " << m_fw_version;
//    BOOST_LOG_TRIVIAL(debug) << "- Main current: " << (int)m_main_current << " mA";
//    BOOST_LOG_TRIVIAL(debug) << "- Prog current: " << (int)m_prog_current << " mA";
//    BOOST_LOG_TRIVIAL(debug) << "- Filtered main current: " << (int)m_filtered_main_current << " mA";
//    BOOST_LOG_TRIVIAL(debug) << "- Temperature: " << (int)m_temperature;
//    BOOST_LOG_TRIVIAL(debug) << "- Supply voltage: " << (int)m_supply_voltage / 1000 << " V";
//    BOOST_LOG_TRIVIAL(debug) << "- VCC voltage: " << (int)m_vcc_voltage / 1000 << " V";
//    BOOST_LOG_TRIVIAL(debug) << "- Emergency stop: " << (emergency_stop ? "yes" : "no");
//    BOOST_LOG_TRIVIAL(debug) << "- Track voltage off: " << (track_voltage_off ? "yes" : "no");
//    BOOST_LOG_TRIVIAL(debug) << "- Short circuit: " << (short_cirtcuit ? "yes" : "no");
//    BOOST_LOG_TRIVIAL(debug) << "- Programming mode: " << (programming_mode ? "yes" : "no");

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

    socket.async_receive_from(
            boost::asio::buffer(recv_buf), receiver_endpoint,
            boost::bind(&Z21::handle_receive, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
}

// Handlers for all received DataSets and commands.
void Z21::handle_dataset(uint16_t size, uint16_t id, std::vector<uint8_t>& data)
{
    BOOST_LOG_TRIVIAL(debug) << "Received for ID " << std::hex << (int)id << ": " << PRINT_HEX(data);
    if (command_handlers.contains(id)) {
        Z21_DataSet* dataset = command_handlers[id];
        dataset->unpack(data);

        switch(id)
        {
            case 0x10:
                m_serial_number = static_cast<LanGetSerialNumber*>(dataset)->serial_number;
                break;
            case 0x18: {
                uint8_t code = static_cast<LanGetCode*>(dataset)->code;
                BOOST_LOG_TRIVIAL(debug) << " ---> LAN_GET_CODE: " << (int) code;
            } break;
            case 0x1a:
                m_hw_type = static_cast<LanGetHWInfo*>(dataset)->hw_type;
                m_fw_version = static_cast<LanGetHWInfo*>(dataset)->fw_version;
                break;
            case 0x40: {
                LanX* lanx = static_cast<LanX*>(dataset);
                LanX_Packet* packet = lanx->command();
                if (packet) {
                    switch (packet->id)
                    {
                        case LanXCommands::LAN_X_TURNOUT_INFO:
                            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_TURNOUT_INFO";
                            break;
                        case LanXCommands::LAN_X_EXT_ACCESSORY_INFO:
                            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_EXT_ACCESSORY_INFO";
                            break;
                        case LanXCommands::LAN_X_BC_TRACK_POWER_OFF:
                            BOOST_LOG_TRIVIAL(debug) << " ### Track power off";
                            break;
                        case LanXCommands::LAN_X_BC_TRACK_POWER_ON:
                            BOOST_LOG_TRIVIAL(debug) << " ### Track power on";
                            break;
                        case LanXCommands::LAN_X_BC_PROGRAMMING_MODE:
                            BOOST_LOG_TRIVIAL(debug) << " ### Programming mode";
                            break;
                        case LanXCommands::LAN_X_BC_TRACK_SHORT_CIRCUIT:
                            BOOST_LOG_TRIVIAL(debug) << " ### Short circuit";
                            break;
                        case LanXCommands::LAN_X_CV_NACK_SC:
                            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_CV_NACK_SC";
                            break;
                        case LanXCommands::LAN_X_CV_NACK:
                            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_CV_NACK";
                            break;
                        case LanXCommands::LAN_X_UNKNOWN_COMMAND:
                            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_UNKNOWN_COMMAND";
                            break;
                        case LanXCommands::LAN_X_STATUS_CHANGED:
                            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_STATUS_CHANGED";
                            break;
                        case LanXCommands::LAN_X_GET_VERSION_RESPONSE:
                            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_GET_VERSION_RESPONSE";
                            break;
                        case LanXCommands::LAN_X_CV_RESULT:
                            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_CV_RESULT";
                            break;
                        case LanXCommands::LAN_X_BC_STOPPED:
                            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_BC_STOPPED";
                            break;
                        case LanXCommands::LAN_X_LOCO_INFO:
                            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_LOCO_INFO";
                            break;
                        case LanXCommands::LAN_X_GET_FIRMWARE_VERSION_RESPONSE:
                            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_GET_FIRMWARE_VERSION_RESPONSE";
                            break;
                        default:
                            break;
                    }
                }
            }   break;
            case 0x60: {
                LanGetLocomode* lm = static_cast<LanGetLocomode*>(dataset);
                BOOST_LOG_TRIVIAL(debug) << " ---> LAN_GET_LOCOMODE: " << std::hex << (int)lm->address << " = " <<
                                                                                    (lm->mode == Locomode::DCC ? "DCC" : "MM");
            } break;
            case 0x70: {
                LanGetTurnoutmode* lm = static_cast<LanGetTurnoutmode*>(dataset);
                BOOST_LOG_TRIVIAL(debug) << " ---> LAN_GET_TURNOUTMODE: " << std::hex << (int)lm->address << " = " << (int)static_cast<uint8_t>(lm->mode);
            } break;
            case 0x84: {
                LanSystemstateDatachanged* ss = static_cast<LanSystemstateDatachanged*>(dataset);
                m_main_current = ss->main_current;
                m_prog_current = ss->prog_current;
                m_filtered_main_current = ss->filtered_main_current;
                m_temperature = ss->temperature;
                m_supply_voltage = ss->supply_voltage;
                m_vcc_voltage = ss->vcc_voltage;
                m_central_state = ss->central_state;
                m_central_state_ex = ss->central_state_ex;
                m_capabilities = ss->capabilities;

                emergency_stop = ss->emergency_stop;
                track_voltage_off = ss->track_voltage_off;
                short_cirtcuit = ss->short_cirtcuit;
                programming_mode = ss->programming_mode;

            }   break;
            default:
                break;
        }
    }
}

