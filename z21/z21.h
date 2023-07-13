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

    void request_system_state();
    void get_hardware_info();
    void get_broadcast_flags();
    void set_broadcast_flags();

    void get_loco_info(uint16_t address);

    void get_feature_set();
    void get_loco_control_standard(uint16_t address);

    void set_track_power_on();
    void set_track_power_off();

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
