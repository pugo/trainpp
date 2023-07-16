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
#include "lan_x_command.h"


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
    if (data.size() == 4) {
        serial_number = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
    }
}

// LAN_GET_CODE (0x18)
void LanGetCode::unpack(std::vector<uint8_t> &data)
{
    if (data.size() == 1) {
        code = data[0];
    }
}

// LAN_GET_HWINFO (0x1a)
void LanGetHWInfo::unpack(std::vector<uint8_t> &data)
{
    if (data.size() == 8) {
        hw_type = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
        fw_version = decode_bcd_version(std::vector<uint8_t>(data.begin() + 4, data.begin() + 8), true);
    }
}

// =========================
//       LAN_X (0x40)
// =========================
LanX::LanX()
{
    m_id = LAN_X;

    command_handlers[LanXCommands::LAN_X_TURNOUT_INFO] = new LanX_TurnoutInfo();
    command_handlers[LanXCommands::LAN_X_EXT_ACCESSORY_INFO] = new LanX_ExtAccessoryInfo();
    command_handlers[LanXCommands::LAN_X_BC_TRACK_POWER_OFF] = new LanX_BcTrackPowerOff();
    command_handlers[LanXCommands::LAN_X_BC_TRACK_POWER_ON] = new LanX_BcTrackPowerOn();
    command_handlers[LanXCommands::LAN_X_BC_PROGRAMMING_MODE] = new LanX_BcProgrammingMode();
    command_handlers[LanXCommands::LAN_X_BC_TRACK_SHORT_CIRCUIT] = new LanX_BcTrackShortCircuit();
    command_handlers[LanXCommands::LAN_X_CV_NACK_SC] = new LanX_CvNackSc();
    command_handlers[LanXCommands::LAN_X_CV_NACK] = new LanX_CvNack();
    command_handlers[LanXCommands::LAN_X_UNKNOWN_COMMAND] = new LanX_UnknownCommand();
    command_handlers[LanXCommands::LAN_X_STATUS_CHANGED] = new LanX_StatusChanged();
    command_handlers[LanXCommands::LAN_X_GET_VERSION_RESPONSE] = new LanX_GetVersionResponse();
    command_handlers[LanXCommands::LAN_X_CV_RESULT] = new LanX_CvResult();
    command_handlers[LanXCommands::LAN_X_BC_STOPPED] = new LanX_BcStopped();
    command_handlers[LanXCommands::LAN_X_LOCO_INFO] = new LanX_LocoInfo();
    command_handlers[LanXCommands::LAN_X_GET_FIRMWARE_VERSION_RESPONSE] = new LanX_GetFirmwareVersionResponse();
}

LanX::LanX(LanX_Command* command) :
        m_command(command)
{
    m_id = LAN_X;
}

LanX::~LanX()
{
    for (auto& item: command_handlers) {
        delete item.second;
    }
}

void LanX::unpack(std::vector<uint8_t>& data)
{
    if (!check_checksum(data)) {
        BOOST_LOG_TRIVIAL(error) << "Bad LAN_X checksum";
        return;
    }

    m_command = nullptr;
    switch(data[0])
    {
        case 0x43:
            m_command = command_handlers[LanXCommands::LAN_X_TURNOUT_INFO];
            break;
        case 0x44:
            m_command = command_handlers[LanXCommands::LAN_X_EXT_ACCESSORY_INFO];
            break;
        case 0x61:
        {
            switch(data[1])
            {
                case 0x00:
                    m_command = command_handlers[LanXCommands::LAN_X_BC_TRACK_POWER_OFF];
                    break;
                case 0x01:
                    m_command = command_handlers[LanXCommands::LAN_X_BC_TRACK_POWER_ON];
                    break;
                case 0x02:
                    m_command = command_handlers[LanXCommands::LAN_X_BC_PROGRAMMING_MODE];
                    break;
                case 0x08:
                    m_command = command_handlers[LanXCommands::LAN_X_BC_TRACK_SHORT_CIRCUIT];
                    break;
                case 0x12:
                    m_command = command_handlers[LanXCommands::LAN_X_CV_NACK_SC];
                    break;
                case 0x13:
                    m_command = command_handlers[LanXCommands::LAN_X_CV_NACK];
                    break;
                case 0x82:
                    m_command = command_handlers[LanXCommands::LAN_X_UNKNOWN_COMMAND];
                    break;
            }
        }
            break;
        case 0x62:
            m_command = command_handlers[LanXCommands::LAN_X_STATUS_CHANGED];
            break;
        case 0x63:
            m_command = command_handlers[LanXCommands::LAN_X_GET_VERSION_RESPONSE];
            break;
        case 0x64:
            m_command = command_handlers[LanXCommands::LAN_X_CV_RESULT];
            break;
        case 0x81:
            m_command = command_handlers[LanXCommands::LAN_X_BC_STOPPED];
            break;
        case 0xef:
            m_command = command_handlers[LanXCommands::LAN_X_LOCO_INFO];
            break;
        case 0xf3:
            m_command = command_handlers[LanXCommands::LAN_X_GET_FIRMWARE_VERSION_RESPONSE];
            break;
    }

    if (m_command) {
        m_command->unpack(data);
    }
}

std::vector<uint8_t> LanX::pack_data()
{
    if (m_command) {
        return m_command->pack();
    }

    std::vector<uint8_t> result;
    return result;
}

bool LanX::check_checksum(std::vector<uint8_t>& data)
{
    uint8_t result = 0;
    for (auto c = data.begin(); c < data.end() - 1; c++) {
        result ^= *c;
    }
    return result == *(data.end() - 1);
}


// LAN_SET_BROADCASTFLAGS (0x50)
std::vector<uint8_t> LanSetBroadcastFlags::pack_data()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), m_flags & 0xff);
    result.insert(result.end(), (m_flags >> 8) & 0xff);
    result.insert(result.end(), (m_flags >> 16) & 0xff);
    result.insert(result.end(), (m_flags >> 24) & 0xff);
    return result;
}

// LAN_GET_BROADCASTFLAGS (0x51)
void LanGetBroadcastFlags::unpack(std::vector<uint8_t> &data)
{
    flags = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
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
