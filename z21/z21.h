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

#ifndef TRAINPP_Z21_H
#define TRAINPP_Z21_H

#include <map>

#include <boost/asio.hpp>
#include <string>

#include "z21_dataset.h"
#include "lan_x_command.h"

class Z21_DataSet;


enum Z21FeatureSet
{
    UNKNOWN = 0xff,
    Z21_NO_LOCK = 0x00,
    Z21_START_LOCKED = 0x01,
    Z21_START_UNLOCKED = 0x02,
};


struct Z21Id
{
    uint32_t serial_number{0};
    uint32_t hw_type{0};
    std::string fw_version{};
    Z21FeatureSet feature_set{Z21FeatureSet::UNKNOWN};
};

struct TrackStatus
{
    int16_t main_current{0};
    int16_t prog_current{0};
    int16_t filtered_main_current{0};
    uint16_t supply_voltage{0};
    uint16_t vcc_voltage{0};
};

struct Mode
{
    bool emergency_stop{false};
    bool track_voltage_off{false};
    bool short_cirtcuit{false};
    bool programming_mode{false};
};

struct Z21Status
{
    Z21Id id;
    TrackStatus track;

    int16_t temperature{0};

    uint8_t central_state{0};
    uint8_t central_state_ex{0};
    uint8_t capabilities{0};

    Mode  mode;
};


/**
 * Represents an instance of a Roco Z21.
 */
class Z21
{
public:
    Z21(const std::string& z21_host, const std::string& z21_port);
    ~Z21();

    bool connect();
    void listen();

    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
    void handle_dataset(uint16_t size, uint16_t id, std::vector<uint8_t>& data);

    Z21Status& z21_status() { return m_z21_status; }

    void get_serial_number();                                           // LAN_GET_SERIAL_NUMBER
    void get_feature_set();                                             // LAN_GET_CODE
    void get_hardware_info();                                           // LAN_GET_HWINFO
    void logoff();                                                      // LAN_LOGOFF

    void xbus_get_version();                                            // LAN_X_GET_VERSION
    void xbus_get_status();                                             // LAN_X_GET_STATUS
    void xbus_set_track_power_off();                                    // LAN_X_SET_TRACK_POWER_OFF
    void xbus_set_track_power_on();                                     // LAN_X_SET_TRACK_POWER_ON

    void xbus_dcc_read_register(uint8_t reg);                           // LAN_X_DCC_READ_REGISTER
    void xbus_cv_read(uint16_t cv);                                     // LAN_X_CV_READ
    void xbus_dcc_write_register(uint8_t reg, uint8_t value);           // LAN_X_DCC_WRITE_REGISTER
    void xbus_cv_write(uint16_t cv, uint8_t value);                     // LAN_X_CV_WRITE
    void xbus_mm_write_byte(uint8_t reg, uint8_t value);                // LAN_X_MM_WRITE_BYTE
    void xbus_get_turnout_info(uint16_t address);                       // LAN_X_GET_TURNOUT_INFO
    void xbus_get_ext_accessory_info(uint16_t address);                 // LAN_X_GET_EXT_ACCESSORY_INFO
    void xbus_set_turnout(uint16_t address, uint8_t value);             // LAN_X_SET_TURNOUT
    void xbus_set_ext_accessory(uint16_t address, uint8_t state);       // LAN_X_SET_EXT_ACCESSORY
    void xbus_set_stop();     // LAN_X_SET_STOP

    void xbus_get_loco_info(uint16_t address);                                                                                  // LAN_X_GET_LOCO_INFO
    void xbus_set_loco_drive(uint16_t address, uint8_t speed, bool forward);                                                    // LAN_X_SET_LOCO_DRIVE
    void xbus_set_loco_function(uint16_t address, uint8_t function);                                                            // LAN_X_SET_LOCO_FUNCTION
    void xbus_set_loco_function_group(uint16_t address, LanX_SetLocoFunctionGroup::FunctionGroup group, uint8_t functions);     // LAN_X_SET_LOCO_FUNCTION_GROUP
    void xbus_set_loco_binary_state(uint16_t address, bool on, uint8_t binary_address);                                         // LAN_X_SET_LOCO_BINARY_STATE

    void xbus_cv_pom_write_byte(uint16_t address, uint16_t cv, uint8_t value);                                                  // LAN_X_CV_POM_WRITE_BYTE
    void xbus_cv_pom_write_bit(uint16_t address, uint16_t cv, uint8_t bit_position, uint8_t value);                             // LAN_X_CV_POM_WRITE_BIT
    void xbus_cv_pom_read_byte(uint16_t address, uint16_t cv);                                                                  // LAN_X_CV_POM_READ_BYTE
    void xbus_cv_pom_accessory_write_byte(uint16_t address, PomAccessorySelection selction, uint8_t output, uint16_t cv, uint8_t value);                                // LAN_X_CV_POM_ACCESSORY_WRITE_BYTE
    void xbus_cv_pom_accessory_write_bit(uint16_t address, PomAccessorySelection selction, uint8_t output, uint16_t cv, uint8_t bit_position, uint8_t value);           // LAN_X_CV_POM_ACCESSORY_WRITE_BIT
    void xbus_cv_pom_accessory_read_byte(uint16_t address, PomAccessorySelection selction, uint8_t output, uint16_t cv);        // LAN_X_CV_POM_ACCESSORY_READ_BYTE

    void xbus_get_firmware_version();                                   // LAN_X_GET_FIRMWARE_VERSION

    void set_broadcast_flags();                         // LAN_SET_BROADCASTFLAGS
    void get_broadcast_flags();                         // LAN_GET_BROADCASTFLAGS

    void get_loco_mode(uint16_t address);                         // LAN_GET_LOCOMODE
    void set_loco_mode(uint16_t address, Locomode mode);                         // LAN_SET_LOCOMODE

    void get_turnout_mode(uint16_t address);                         // LAN_GET_LOCOMODE
    void set_turnout_mode(uint16_t address, Locomode mode);                         // LAN_SET_LOCOMODE

    void systemstate_get_data();                        // LAN_SYSTEMSTATE_GETDATA




private:
    void listen_thread_fn();

    const std::string host;
    const std::string port;

    std::thread listen_thread;
    std::vector<uint8_t> recv_buf;
    std::map<uint16_t, Z21_DataSet*> command_handlers;

    boost::asio::io_context io_context;
    boost::asio::ip::udp::endpoint receiver_endpoint;
    boost::asio::ip::udp::socket socket;

    Z21Status m_z21_status;
};


#endif //TRAINPP_Z21_H
