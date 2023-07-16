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
    bool invalid_request{false};
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

    /**
     * Connect to X21.
     * @return true on success
     */
    bool connect();

    /**
     * Start listening for Z21 datasets (in separate thread).
     */
    void listen();


    /**
     * Get a struct with all current Z21 information.
     * @return struct with all current Z21 information
     */
    Z21Status& z21_status() { return m_z21_status; }


    // =========================================================================================
    //   Z21 low level API
    // =========================================================================================

    /**
     * Request Z21 Serial number (LAN_GET_SERIAL_NUMBER).
     */
    void get_serial_number();

    /**
     * Request Z21 feature set (LAN_GET_CODE).
     */
    void get_feature_set();

    /**
     * Request Z21 hardware info (LAN_GET_HWINFO).
     */
    void get_hardware_info();

    /**
     * Logoff from Z21 (LAN_LOGOFF).
     */
    void logoff();

    /**
     * Request XBus: version (LAN_X_GET_VERSION).
     */
    void xbus_get_version();

    /**
     * Request XBus: status (LAN_X_GET_STATUS).
     */
    void xbus_get_status();

    /**
     * Request XBus: set track power off (LAN_X_SET_TRACK_POWER_OFF).
     */
    void xbus_set_track_power_off();

    /**
     * Request XBus: set track power on (LAN_X_SET_TRACK_POWER_ON).
     */
    void xbus_set_track_power_on();

    /**
     * Request XBus: read DCC register (LAN_X_DCC_READ_REGISTER).
     * @param reg register to read
     */
    void xbus_dcc_read_register(uint8_t reg);

    /**
     * Request XBus: read CV value (LAN_X_CV_READ).
     * @param cv CV to read
     */
    void xbus_cv_read(uint16_t cv);

    /**
     * Request XBus: write DCC register (LAN_X_DCC_WRITE_REGISTER).
     * @param reg register to write
     * @param value value to write
     */
    void xbus_dcc_write_register(uint8_t reg, uint8_t value);

    /**
     * Request XBus: write CV (LAN_X_CV_WRITE).
     * @param cv CV to write
     * @param value value to write
     */
    void xbus_cv_write(uint16_t cv, uint8_t value);

    /**
     * Request XBus: write MM byte (LAN_X_MM_WRITE_BYTE).
     * @param reg register to write
     * @param value value to write
     */
    void xbus_mm_write_byte(uint8_t reg, uint8_t value);

    /**
     * Request XBus: get turnout info (LAN_X_GET_TURNOUT_INFO).
     * @param address turnout address
     */
    void xbus_get_turnout_info(uint16_t address);

    /**
     * Request XBus: get ext accessory info (LAN_X_GET_EXT_ACCESSORY_INFO).
     * @param address accessory address
     */
    void xbus_get_ext_accessory_info(uint16_t address);

    /**
     * Request XBus: set turnout value (LAN_X_SET_TURNOUT).
     * @param address turnout address
     * @param value value to set
     */
    void xbus_set_turnout(uint16_t address, uint8_t value);

    /**
     * Request XBus: set ext accessory (LAN_X_SET_EXT_ACCESSORY).
     * @param address accessory address
     * @param state accessory state
     */
    void xbus_set_ext_accessory(uint16_t address, uint8_t state);

    /**
     * Reqyest XBus: set stop (LAN_X_SET_STOP).
     */
    void xbus_set_stop();

    /**
     * Request XBus: get loco info (LAN_X_GET_LOCO_INFO).
     * @param address loco address
     */
    void xbus_get_loco_info(uint16_t address);

    /**
     * Request XBus: set loco drive (LAN_X_SET_LOCO_DRIVE).
     * @param address loco address
     * @param speed loco speed
     * @param forward loco direction
     */
    void xbus_set_loco_drive(uint16_t address, uint8_t speed, bool forward);

    /**
     * Request XBus: set loco function (LAN_X_SET_LOCO_FUNCTION).
     * @param address loco address
     * @param function loco function
     */
    void xbus_set_loco_function(uint16_t address, uint8_t function);

    /**
     * Request XBus: set loco function group (LAN_X_SET_LOCO_FUNCTION_GROUP).
     * @param address loco address
     * @param group loco group
     * @param functions loco functions
     */
    void xbus_set_loco_function_group(uint16_t address, LanX_SetLocoFunctionGroup::FunctionGroup group, uint8_t functions);

    /**
     * Request XBus: set loco binary state (LAN_X_SET_LOCO_BINARY_STATE).
     * @param address loco address
     * @param on `true` if bit should be set to on, otherwise `false`
     * @param binary_address address to set
     */
    void xbus_set_loco_binary_state(uint16_t address, bool on, uint8_t binary_address);

    /**
     * Request XBus: program on main, write byte (LAN_X_CV_POM_WRITE_BYTE).
     * @param address loco address to program
     * @param cv CV to set
     * @param value value to write
     */
    void xbus_cv_pom_write_byte(uint16_t address, uint16_t cv, uint8_t value);

    /**
     * Request XBus: program on main, write bit (LAN_X_CV_POM_WRITE_BIT).
     * @param address loco address to program
     * @param cv CV to set
     * @param bit_position bit position to write
     * @param value value to write
     */
    void xbus_cv_pom_write_bit(uint16_t address, uint16_t cv, uint8_t bit_position, uint8_t value);

    /**
     * Request XBus: program on main, read byte (LAN_X_CV_POM_READ_BYTE)
     * @param address loco address to read
     * @param cv CV to read
     */
    void xbus_cv_pom_read_byte(uint16_t address, uint16_t cv);

    /**
     * Request XBus: program on main, write accessory byte (LAN_X_CV_POM_ACCESSORY_WRITE_BYTE)
     * @param address accessory address
     * @param selction accessory selection
     * @param output accessory output
     * @param cv CV to write
     * @param value value to write
     */
    void xbus_cv_pom_accessory_write_byte(uint16_t address, PomAccessorySelection selction, uint8_t output, uint16_t cv, uint8_t value);
    /**
     * Request XBus: program on main, write accessory bit (LAN_X_CV_POM_ACCESSORY_WRITE_BIT)
     * @param address accessory address
     * @param selction accessory selection
     * @param output accessory output
     * @param cv CV to write
     * @param bit_position bit position to write
     * @param value value to write
     */
    void xbus_cv_pom_accessory_write_bit(uint16_t address, PomAccessorySelection selction, uint8_t output, uint16_t cv, uint8_t bit_position, uint8_t value);

    /**
     * Request XBus: program on main, read accessory byte (LAN_X_CV_POM_ACCESSORY_READ_BYTE)
     * @param address accessory address
     * @param selction accessory selection
     * @param output accessory output
     * @param cv
     */
    void xbus_cv_pom_accessory_read_byte(uint16_t address, PomAccessorySelection selction, uint8_t output, uint16_t cv);

    /**
     * Request XBus: get firmware version (LAN_X_GET_FIRMWARE_VERSION).
     */
    void xbus_get_firmware_version();

    /**
     * Set Z21 broadcast flags (LAN_SET_BROADCASTFLAGS).
     */
    void set_broadcast_flags();

    /**
     * Get Z21 broadcast flags (LAN_GET_BROADCASTFLAGS).
     */
    void get_broadcast_flags();

    /**
     * Request lock mode (LAN_GET_LOCOMODE).
     * @param address loco address
     */
    void get_loco_mode(uint16_t address);

    /**
     * Request to set loco mode (LAN_SET_LOCOMODE).
     * @param address loco address
     * @param mode loco mode
     */
    void set_loco_mode(uint16_t address, Locomode mode);

    /**
     * Request to get turnout mode (LAN_GET_TURNOUTMODE).
     * @param address turnout address
     */
    void get_turnout_mode(uint16_t address);

    /** Request to set turnout mode (LAN_SET_TURNOUTMODE).
     * @param address turnout address
     * @param mode turnout mode
     */
    void set_turnout_mode(uint16_t address, Locomode mode);

    /**
     * Request system state (LAN_SYSTEMSTATE_GETDATA).
     */
    void systemstate_get_data();

private:
    /**
     * Listening thread function.
     */
    void listen_thread_fn();

    /**
     * Handle received data (from listening thread).
     * @param error possible error code
     * @param bytes_transferred number of bytes received
     */
    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);

    /**
     * Handle received dataset (from listening thread).
     * @param size size of dataset
     * @param id ID of dataset
     * @param data dataset data
     */
    void handle_dataset(uint16_t size, uint16_t id, std::vector<uint8_t>& data);

    /**
     * Handle received XBus command (from listening thread).
     * @param command XBus command to handle
     */
    void handle_lanx_command(LanX_Command* command);

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
