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
#include <sstream>
#include <iterator>
#include <numeric>

#include <boost/log/trivial.hpp>

#include "z21_dataset.h"


std::vector<uint8_t> Z21_DataSet::pack()
{
    std::vector<uint8_t> result;
    std::vector<uint8_t> data = pack_data();
    uint16_t size = header_size + data.size();

    // Pack size
    result.insert(result.end(), size & 0xff);
    result.insert(result.end(), (size >> 8) & 0xff);

    // Pack ID
    result.insert(result.end(), m_id & 0xff);
    result.insert(result.end(), (m_id >> 8) & 0xff);

    // Add data
    result.insert(result.end(), data.begin(), data.end());

    return result;
}

// LAN_GET_SERIAL_NUMBER (0x10)
void LanGetSerialNumber::unpack(std::vector<uint8_t> &data)
{
    BOOST_LOG_TRIVIAL(debug) << "LanGetSerialNumber::unpack(): " << PRINT_HEX(data);
    if (data.size() == 4) {
        serial_number = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
    }
}

// LAN_GET_CODE (0x18)
void LanGetCode::unpack(std::vector<uint8_t> &data)
{
    BOOST_LOG_TRIVIAL(debug) << "LanGetCode::unpack(): " << PRINT_HEX(data);
    if (data.size() == 1) {
        code = data[0];
    }
}

// LAN_GET_HWINFO (0x1a)
void LanGetHWInfo::unpack(std::vector<uint8_t> &data)
{
    BOOST_LOG_TRIVIAL(debug) << "LanGetHWInfo::unpack(): " << PRINT_HEX(data);
    if (data.size() == 8) {
        hw_type = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);

        bool got_value = false;
        std::vector<std::string> result;

        for(size_t i = 4; i > 0; i--) {
            uint8_t v = data[4 + i - 1];
            if (! got_value && v == 0) { continue; }

            uint8_t value = 0;
            value += v & 0x0f;
            value += (v >> 4) * 10;
            result.push_back(std::to_string(value));
        }

        fw_version = std::accumulate(result.begin() + 1, result.end(), result[0],
                                        [](const std::string& a, const std::string& b) { return a + '.' + b; });
    }
}

// LAN_SET_BROADCASTFLAGS (0x50)
std::vector<uint8_t> LanSetBroadcastFlags::pack_data()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), m_flags & 0xff);
    result.insert(result.end(), (m_flags >> 8) & 0xff);
    result.insert(result.end(), (m_flags >> 16) & 0xff);
    result.insert(result.end(), (m_flags >> 24) & 0xff);
    BOOST_LOG_TRIVIAL(debug) << "LanSetBroadcastFlags::pack_data(): " << PRINT_HEX(result);
    return result;
}

// LAN_GET_BROADCASTFLAGS (0x51)
void LanGetBroadcastFlags::unpack(std::vector<uint8_t> &data)
{
    BOOST_LOG_TRIVIAL(debug) << "LanGetBroadcastFlags::unpack(): " << PRINT_HEX(data);
}

// LAN_GET_LOCOMODE (0x60)
std::vector<uint8_t> LanGetLocomode::pack_data()
{
    std::vector<uint8_t> result;
    // Loco address should be big endian according to spec.
    result.insert(result.end(), (address >> 8) & 0xff);
    result.insert(result.end(), address & 0xff);
    return result;
}

void LanGetLocomode::unpack(std::vector<uint8_t> &data)
{
    address = (data[0] << 8) + data[1];
    mode = static_cast<Locomode>(data[2]);
}

// LAN_SET_LOCOMODE (0x61)
std::vector<uint8_t> LanSetLocomode::pack_data()
{
    std::vector<uint8_t> result;
    // Loco address should be big endian according to spec.
    result.insert(result.end(), (m_address >> 8) & 0xff);
    result.insert(result.end(), m_address & 0xff);
    result.insert(result.end(), static_cast<uint8_t>(m_mode));
    BOOST_LOG_TRIVIAL(debug) << "LanSetLocomode::pack_data(): " << PRINT_HEX(result);
    return result;
}

// LAN_SYSTEMSTATE_DATACHANGED (0x84)
void LanSystemstateDatachanged::unpack(std::vector<uint8_t> &data)
{
    if (data.size() == 16) {
        main_current = static_cast<int16_t>((data[1] << 8) + data[0]);
        prog_current = static_cast<int16_t>((data[3] << 8) + data[2]);
        filtered_main_current = static_cast<int16_t>((data[5] << 8) + data[4]);
        temperature = static_cast<int16_t>((data[7] << 8) + data[6]);

        supply_voltage = (data[9] << 8) + data[8];
        vcc_voltage = (data[11] << 8) + data[10];

        central_state = data[12];
        central_state_ex = data[13];
        capabilities = data[15];

        emergency_stop = central_state & 0x01;
        track_voltage_off = central_state & 0x02;
        short_cirtcuit = central_state & 0x04;
        programming_mode = central_state & 0x20;
    }
}
