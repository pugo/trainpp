
#include <iostream>
#include <sstream>
#include <iterator>
#include <numeric>

#include <boost/log/trivial.hpp>

#include "lan_x_dataset.h"


LanX::LanX()
{
    m_id = 0x40;

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


LanX::LanX(LanX_Packet* command) :
    m_command(command)
{
    m_id = 0x40;
}


void LanX::unpack(std::vector<uint8_t>& data)
{
    BOOST_LOG_TRIVIAL(debug) << "LanX::unpack(): " << PRINT_HEX(data);

    if (!check_checksum(data)) {
        BOOST_LOG_TRIVIAL(error) << "Bad LAN_X checksum";
        return;
    }

    uint8_t x_header = data[0];
    m_command = nullptr;

    switch(x_header)
    {
        case 0x43:
            m_command = command_handlers[LanXCommands::LAN_X_TURNOUT_INFO];
            break;
        case 0x44:
            m_command = command_handlers[LanXCommands::LAN_X_EXT_ACCESSORY_INFO];
            break;
        case 0x61:
        {
            uint8_t db0 = data[1];
            switch(db0)
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


void LanX_Packet::append_checksum(std::vector<uint8_t>& data)
{
    uint8_t result = 0;
    for (auto c = data.begin(); c < data.end(); c++) {
        result ^= *c;
    }

    data.insert(data.end(), result);
}


// LAN_X_GET_LOCO_INFO
std::vector<uint8_t> LanX_GetLocoInfo::pack()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), 0xe3);
    result.insert(result.end(), 0xf0);
    result.insert(result.end(), (m_address >> 8) & 0x3f);
    result.insert(result.end(), m_address & 0xff);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_GetLocoInfo::pack(): " << PRINT_HEX(result);
    return result;
}


void LanX_LocoInfo::unpack(std::vector<uint8_t>& data)
{
    address = ((data[1] & 0x3f) << 8) + data[2];
    busy = data[3] & 0x8;
    speed_steps = data[3] & 0x07;
    direction_forward = data[4] & 0x80;
    speed = data[4] & 0x7f;

    double_traction = data[5] & 0x40;
    smart_search = data[5] & 0x20;
    f0_light = data[5] & 0x10;
    f4 = data[5] & 0x08;
    f3 = data[5] & 0x04;
    f2 = data[5] & 0x02;
    f1 = data[5] & 0x01;

    BOOST_LOG_TRIVIAL(debug) << "LanX_LocoInfo::unpack(): " << (int)address << ": direction = " << direction_forward
                             << ", speed = " << (int)speed << ", light = " << f0_light;
}

// LAN_X_SET_LOCO_DRIVE
std::vector<uint8_t> LanX_SetLocoDrive::pack()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), 0xe4);
    result.insert(result.end(), 0x12);
    result.insert(result.end(), (m_address >> 8) & 0x3f);
    result.insert(result.end(), m_address & 0xff);
    result.insert(result.end(), m_speed + (m_forward ? 0x80 : 0));
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_SetLocoDrive::pack(): " << PRINT_HEX(result);
    return result;
}


// LAN_X_SET_LOCO_FUNCTION
std::vector<uint8_t> LanX_SetLocoFunction::pack()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), 0xe4);
    result.insert(result.end(), 0xf8);
    result.insert(result.end(), (m_address >> 8) & 0x3f);
    result.insert(result.end(), m_address & 0xff);
    result.insert(result.end(), m_function);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_SetLocoFunction::pack(): " << PRINT_HEX(result);
    return result;
}


// LAN_X_SET_LOCO_FUNCTION_GROUP
std::vector<uint8_t> LanX_SetLocoFunctionGroup::pack()
{
    std::vector<uint8_t> result;

    if (m_group == 0x20 || m_group == 0x21 || m_group == 0x22 || m_group == 0x23 || m_group == 0x28 ||
        m_group == 0x29 || m_group == 0x2a || m_group == 0x2b || m_group == 0x50 || m_group == 0x51)
    {
        result.insert(result.end(), 0xe4);
        result.insert(result.end(), m_group);
        result.insert(result.end(), (m_address >> 8) & 0x3f);
        result.insert(result.end(), m_address & 0xff);
        result.insert(result.end(), m_functions);
        append_checksum(result);
    }

    BOOST_LOG_TRIVIAL(debug) << "LanX_SetLocoFunction::pack(): " << PRINT_HEX(result);
    return result;
}


