
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
}


bool LanX::check_checksum(std::vector<uint8_t>& data)
{
    uint8_t result = 0;
    for (auto c = data.begin(); c < data.end() - 1; c++) {
        result ^= *c;
    }
    return result == *(data.end() - 1);
}