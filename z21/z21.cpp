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
            case Z21_DataSet::DataSet::LAN_GET_SERIAL_NUMBER:
                m_z21_status.id.serial_number = static_cast<LanGetSerialNumber*>(dataset)->serial_number;
                break;
            case Z21_DataSet::DataSet::LAN_GET_CODE: {
                uint8_t code = static_cast<LanGetCode*>(dataset)->code;
                m_z21_status.id.feature_set = static_cast<Z21FeatureSet>(code);
                BOOST_LOG_TRIVIAL(debug) << " ---> LAN_GET_CODE: " << (int) m_z21_status.id.feature_set;
            } break;
            case Z21_DataSet::DataSet::LAN_GET_HWINFO:
                m_z21_status.id.hw_type = static_cast<LanGetHWInfo*>(dataset)->hw_type;
                m_z21_status.id.fw_version = static_cast<LanGetHWInfo*>(dataset)->fw_version;
                break;
            case Z21_DataSet::DataSet::LAN_X: {
                LanX* lanx = static_cast<LanX*>(dataset);
                LanX_Command* command = lanx->command();
                if (command) {
                    handle_lanx_command(command);
                }
            }   break;
            case Z21_DataSet::DataSet::LAN_GET_BROADCASTFLAGS: {
                LanGetBroadcastFlags* bf = static_cast<LanGetBroadcastFlags*>(dataset);
                BOOST_LOG_TRIVIAL(debug) << " ---> LAN_GET_BROADCASTFLAGS: " << std::hex << (int)bf->flags;
            } break;
            case Z21_DataSet::DataSet::LAN_GET_LOCOMODE: {
                LanGetLocomode* lm = static_cast<LanGetLocomode*>(dataset);
                BOOST_LOG_TRIVIAL(debug) << " ---> LAN_GET_LOCOMODE: " << std::hex << (int)lm->address << " = " <<
                                                                                    (lm->mode == Locomode::DCC ? "DCC" : "MM");
            } break;
            case Z21_DataSet::DataSet::LAN_GET_TURNOUTMODE: {
                LanGetTurnoutmode* lm = static_cast<LanGetTurnoutmode*>(dataset);
                BOOST_LOG_TRIVIAL(debug) << " ---> LAN_GET_TURNOUTMODE: " << std::hex << (int)lm->address << " = " << (int)static_cast<uint8_t>(lm->mode);
            } break;
            case Z21_DataSet::DataSet::LAN_SYSTEMSTATE_DATACHANGED: {
                LanSystemstateDatachanged* ss = static_cast<LanSystemstateDatachanged*>(dataset);
                m_z21_status.track.main_current = ss->main_current;
                m_z21_status.track.prog_current = ss->prog_current;
                m_z21_status.track.filtered_main_current = ss->filtered_main_current;
                m_z21_status.track.supply_voltage = ss->supply_voltage;
                m_z21_status.track.vcc_voltage = ss->vcc_voltage;
                m_z21_status.temperature = ss->temperature;
                m_z21_status.central_state = ss->central_state;
                m_z21_status.central_state_ex = ss->central_state_ex;
                m_z21_status.capabilities = ss->capabilities;

                m_z21_status.mode.emergency_stop = ss->emergency_stop;
                m_z21_status.mode.track_voltage_off = ss->track_voltage_off;
                m_z21_status.mode.short_cirtcuit = ss->short_cirtcuit;
                m_z21_status.mode.programming_mode = ss->programming_mode;

            }   break;
            default:
                break;
        }
    }
}

void Z21::handle_lanx_command(LanX_Command* command)
{
    switch (command->id)
    {
        case LanXCommands::LAN_X_TURNOUT_INFO:
            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_TURNOUT_INFO";
            break;
        case LanXCommands::LAN_X_EXT_ACCESSORY_INFO:
            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_EXT_ACCESSORY_INFO";
            break;
        case LanXCommands::LAN_X_BC_TRACK_POWER_OFF:
            m_z21_status.mode.track_voltage_off = true;
            break;
        case LanXCommands::LAN_X_BC_TRACK_POWER_ON:
            m_z21_status.mode.track_voltage_off = false;
            break;
        case LanXCommands::LAN_X_BC_PROGRAMMING_MODE:
            m_z21_status.mode.programming_mode = true;
            break;
        case LanXCommands::LAN_X_BC_TRACK_SHORT_CIRCUIT:
            m_z21_status.mode.short_cirtcuit = true;
            break;
        case LanXCommands::LAN_X_CV_NACK_SC:
            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_CV_NACK_SC";
            break;
        case LanXCommands::LAN_X_CV_NACK:
            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_CV_NACK";
            break;
        case LanXCommands::LAN_X_UNKNOWN_COMMAND:
            m_z21_status.mode.invalid_request = true;
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
            m_z21_status.mode.emergency_stop = true;
            break;
        case LanXCommands::LAN_X_LOCO_INFO:
            BOOST_LOG_TRIVIAL(debug) << " ### LAN_X_LOCO_INFO";
            break;
        case LanXCommands::LAN_X_GET_FIRMWARE_VERSION_RESPONSE: {
            LanX_GetFirmwareVersionResponse* lanx_command = static_cast<LanX_GetFirmwareVersionResponse*>(command);
            m_z21_status.id.fw_version = lanx_command->fw_version;
        } break;
        default:
            break;
    }
}

void Z21::get_serial_number()
{
    socket.send_to(boost::asio::buffer(LanGetSerialNumber().pack()), receiver_endpoint);
}

void Z21::get_feature_set()
{
    socket.send_to(boost::asio::buffer(LanGetCode().pack()), receiver_endpoint);
}

void Z21::get_hardware_info()
{
    socket.send_to(boost::asio::buffer(LanGetHWInfo().pack()), receiver_endpoint);
}

void Z21::logoff()
{
    socket.send_to(boost::asio::buffer(LanLogoff().pack()), receiver_endpoint);
}

void Z21::xbus_get_version()
{
    LanX_GetVersion lanx_command;
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_get_status()
{
    LanX_GetStatus lanx_command;
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_set_track_power_off()
{
    LanX_SetTrackPowerOff lanx_command;
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_set_track_power_on()
{
    LanX_SetTrackPowerOn lanx_command;
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_dcc_read_register(uint8_t reg)
{
    LanX_DccReadRegister lanx_command(reg);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_cv_read(uint16_t cv)
{
    LanX_CvRead lanx_command(cv);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_dcc_write_register(uint8_t reg, uint8_t value)
{
    LanX_DccWriteRegister lanx_command(reg, value);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_cv_write(uint16_t cv, uint8_t value)
{
    LanX_CvWrite lanx_command(cv, value);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_mm_write_byte(uint8_t reg, uint8_t value)
{
    LanX_MmWriteByte lanx_command(reg, value);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_get_turnout_info(uint16_t address)
{
    LanX_GetTurnoutInfo lanx_command(address);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_get_ext_accessory_info(uint16_t address)
{
    LanX_GetExtAccessoryInfo lanx_command(address);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_set_turnout(uint16_t address, uint8_t value)
{
    LanX_SetTurnout lanx_command(address, value);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_set_ext_accessory(uint16_t address, uint8_t state)
{
    LanX_SetExtAccessory lanx_command(address, state);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_set_stop()
{
    LanX_SetStop lanx_command;
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_get_loco_info(uint16_t address)
{
    LanX_GetLocoInfo lanx_command(address);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_set_loco_drive(uint16_t address, uint8_t speed, bool forward)
{
    LanX_SetLocoDrive lanx_command(address, speed, forward);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_set_loco_function(uint16_t address, uint8_t function)
{
    LanX_SetLocoFunction lanx_command(address, function);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_set_loco_function_group(uint16_t address, LanX_SetLocoFunctionGroup::FunctionGroup group, uint8_t functions)
{
    LanX_SetLocoFunctionGroup lanx_command(address, group, functions);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_set_loco_binary_state(uint16_t address, bool on, uint8_t binary_address)
{
    LanX_SetLocoBinaryState lanx_command(address, on, binary_address);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_cv_pom_write_byte(uint16_t address, uint16_t cv, uint8_t value)
{
    LanX_CvPomWriteByte lanx_command(address, cv, value);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_cv_pom_write_bit(uint16_t address, uint16_t cv, uint8_t bit_position, uint8_t value)
{
    LanX_CvPomWriteBit lanx_command(address, cv, bit_position, value);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_cv_pom_read_byte(uint16_t address, uint16_t cv)
{
    LanX_CvPomReadByte lanx_command(address, cv);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_cv_pom_accessory_write_byte(uint16_t address, PomAccessorySelection selction, uint8_t output, uint16_t cv, uint8_t value)
{
    LanX_CvPomAccessoryWriteByte lanx_command(address, selction, output, cv, value);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_cv_pom_accessory_write_bit(uint16_t address, PomAccessorySelection selction, uint8_t output, uint16_t cv, uint8_t bit_position, uint8_t value)
{
    LanX_CvPomAccessoryWriteBit lanx_command(address, selction, output, cv, bit_position, value);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_cv_pom_accessory_read_byte(uint16_t address, PomAccessorySelection selction, uint8_t output, uint16_t cv)
{
    LanX_CvPomAccessoryReadByte lanx_command(address, selction, output, cv);
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::xbus_get_firmware_version()
{
    LanX_GetFirmwareVersion lanx_command;
    socket.send_to(boost::asio::buffer(LanX(&lanx_command).pack()), receiver_endpoint);
}

void Z21::set_broadcast_flags()
{
    LanSetBroadcastFlags sbf(BroadcastFlags::DRIVING_AND_SWITCHING | BroadcastFlags::Z21_STATUS_CHANGES);
    socket.send_to(boost::asio::buffer(sbf.pack()), receiver_endpoint);
}

void Z21::get_broadcast_flags()
{
    socket.send_to(boost::asio::buffer(LanGetBroadcastFlags().pack()), receiver_endpoint);
}

void Z21::get_loco_mode(uint16_t address)
{
    socket.send_to(boost::asio::buffer(LanGetLocomode(address).pack()), receiver_endpoint);
}

void Z21::set_loco_mode(uint16_t address, Locomode mode)
{
    socket.send_to(boost::asio::buffer(LanSetLocomode(address, mode).pack()), receiver_endpoint);
}

void Z21::get_turnout_mode(uint16_t address)
{
    socket.send_to(boost::asio::buffer(LanGetTurnoutmode(address).pack()), receiver_endpoint);
}

void Z21::set_turnout_mode(uint16_t address, Locomode mode)
{
    socket.send_to(boost::asio::buffer(LanSetTurnoutmode(address, mode).pack()), receiver_endpoint);
}

void Z21::systemstate_get_data()
{
    socket.send_to(boost::asio::buffer(LanSystemstateGetData().pack()), receiver_endpoint);
}


