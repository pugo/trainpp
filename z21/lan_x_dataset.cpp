
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
                    BOOST_LOG_TRIVIAL(debug) << "!! LAN_X_BC_TRACK_POWER_OFF";
                    break;
                case 0x01:
                    m_command = command_handlers[LanXCommands::LAN_X_BC_TRACK_POWER_ON];
                    BOOST_LOG_TRIVIAL(debug) << "!! LAN_X_BC_TRACK_POWER_ON";
                    break;
                case 0x02:
                    m_command = command_handlers[LanXCommands::LAN_X_BC_PROGRAMMING_MODE];
                    BOOST_LOG_TRIVIAL(debug) << "!! LAN_X_BC_PROGRAMMING_MODE";
                    break;
                case 0x08:
                    m_command = command_handlers[LanXCommands::LAN_X_BC_TRACK_SHORT_CIRCUIT];
                    BOOST_LOG_TRIVIAL(debug) << "!! LAN_X_BC_TRACK_SHORT_CIRCUIT";
                    break;
                case 0x12:
                    m_command = command_handlers[LanXCommands::LAN_X_CV_NACK_SC];
                    BOOST_LOG_TRIVIAL(debug) << "!! LAN_X_CV_NACK_SC";
                    break;
                case 0x13:
                    m_command = command_handlers[LanXCommands::LAN_X_CV_NACK];
                    BOOST_LOG_TRIVIAL(debug) << "!! LAN_X_CV_NACK";
                    break;
                case 0x82:
                    m_command = command_handlers[LanXCommands::LAN_X_UNKNOWN_COMMAND];
                    BOOST_LOG_TRIVIAL(debug) << "!! LAN_X_UNKNOWN_COMMAND";
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

// ==========================================================================
//     LANN_X commands
// ==========================================================================

// ==== Client to Z21 ====

// LAN_X_CV_READ
std::vector<uint8_t> LanX_CvRead::pack()
{
    uint16_t cv_address = m_cv - 1;

    std::vector<uint8_t> result;
    result.insert(result.end(), 0x23);
    result.insert(result.end(), 0x11);
    result.insert(result.end(), cv_address >> 8);
    result.insert(result.end(), cv_address & 0xff);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_CvRead::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_CV_WRITE
std::vector<uint8_t> LanX_CvWrite::pack()
{
    uint16_t cv_address = m_cv - 1;

    std::vector<uint8_t> result;
    result.insert(result.end(), 0x24);
    result.insert(result.end(), 0x12);
    result.insert(result.end(), cv_address >> 8);
    result.insert(result.end(), cv_address & 0xff);
    result.insert(result.end(), m_value);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_CvWrite::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_CV_POM_WRITE_BYTE
std::vector<uint8_t> LanX_CvPomWriteByte::pack()
{
    uint16_t cv_address = m_cv - 1;

    std::vector<uint8_t> result;
    result.insert(result.end(), 0xe6);
    result.insert(result.end(), 0x30);
    result.insert(result.end(), (m_address >> 8) & 0x3f);
    result.insert(result.end(), m_address & 0xff);
    result.insert(result.end(), 0xec + ((cv_address >> 8) & 0x03));
    result.insert(result.end(), cv_address & 0xff);
    result.insert(result.end(), m_value);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_CvPomWriteByte::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_CV_POM_WRITE_BIT
std::vector<uint8_t> LanX_CvPomWriteBit::pack()
{
    std::vector<uint8_t> result;
    uint16_t cv_address = m_cv - 1;
    result.insert(result.end(), 0xe6);
    result.insert(result.end(), 0x30);  // Same values as LAN_X_CV_POM_WRITE_BYTE above, but with other option (0xe8).
    result.insert(result.end(), (m_address >> 8) & 0x3f);
    result.insert(result.end(), m_address & 0xff);
    result.insert(result.end(), 0xe8 + ((cv_address >> 8) & 0x03));
    result.insert(result.end(), cv_address & 0xff);
    result.insert(result.end(), (m_value ? 0x08 : 0) + (m_bit_position & 0x07));
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_CvPomWriteBit::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_CV_POM_READ_BYTE
std::vector<uint8_t> LanX_CvPomReadByte::pack()
{
    uint16_t cv_address = m_cv - 1;

    std::vector<uint8_t> result;
    result.insert(result.end(), 0xe6);
    result.insert(result.end(), 0x30);  // Same values as LAN_X_CV_POM_WRITE_BYTE above, but with other option (0xe4).
    result.insert(result.end(), (m_address >> 8) & 0x3f);
    result.insert(result.end(), m_address & 0xff);
    result.insert(result.end(), 0xe4 + ((cv_address >> 8) & 0x03));
    result.insert(result.end(), cv_address & 0xff);
    result.insert(result.end(), 0x00);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_CvPomReadByte::pack(): " << PRINT_HEX(result);
    return result;
}

// Accessory
// LAN_X_CV_POM_ACCESSORY_WRITE_BYTE
std::vector<uint8_t> LanX_CvPomAccessoryWriteByte::pack()
{
    uint16_t cv_address = m_cv - 1;

    std::vector<uint8_t> result;
    result.insert(result.end(), 0xe6);
    result.insert(result.end(), 0x31);
    result.insert(result.end(), (m_address & 0x1ff) >> 4);
    result.insert(result.end(), ((m_address & 0x0f) << 4) + m_selection + (m_output & 0x07));
    result.insert(result.end(), 0xec + ((cv_address >> 8) & 0x03));
    result.insert(result.end(), cv_address & 0xff);
    result.insert(result.end(), m_value);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_CvPomAccessoryWriteByte::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_CV_POM_ACCESSORY_WRITE_BIT
std::vector<uint8_t> LanX_CvPomAccessoryWriteBit::pack()
{
    std::vector<uint8_t> result;
    uint16_t cv_address = m_cv - 1;
    result.insert(result.end(), 0xe6);
    result.insert(result.end(), 0x31);  // Same values as LAN_X_CV_POM_ACCESSORY_WRITE_BYTE above, but with other option (0xe8).
    result.insert(result.end(), (m_address & 0x1ff) >> 4);
    result.insert(result.end(), ((m_address & 0x0f) << 4) + m_selection + (m_output & 0x07));
    result.insert(result.end(), 0xe8 + ((cv_address >> 8) & 0x03));
    result.insert(result.end(), cv_address & 0xff);
    result.insert(result.end(), (m_value ? 0x08 : 0) + (m_bit_position & 0x07));
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_CvPomAccessoryWriteBit::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_CV_POM_ACCESSORY_READ_BYTE
std::vector<uint8_t> LanX_CvPomAccessoryReadByte::pack()
{
    uint16_t cv_address = m_cv - 1;

    std::vector<uint8_t> result;
    result.insert(result.end(), 0xe6);
    result.insert(result.end(), 0x31);  // Same values as LAN_X_CV_POM_ACCESSORY_WRITE_BYTE above, but with other option (0xe4).
    result.insert(result.end(), (m_address & 0x1ff) >> 4);
    result.insert(result.end(), ((m_address & 0x0f) << 4) + m_selection + (m_output & 0x07));
    result.insert(result.end(), 0xe4 + ((cv_address >> 8) & 0x03));
    result.insert(result.end(), cv_address & 0xff);
    result.insert(result.end(), 0x00);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_CvPomAccessoryReadByte::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_MM_WRITE_BYTE
std::vector<uint8_t> LanX_MmWriteByte::pack()
{
    std::vector<uint8_t> result;
    if (m_register <= 78) {
        result.insert(result.end(), 0x24);
        result.insert(result.end(), 0xff);
        result.insert(result.end(), 0x00);
        result.insert(result.end(), m_register);
        result.insert(result.end(), m_value);
        append_checksum(result);
        // TODO: better error result than bad package.
    }
    BOOST_LOG_TRIVIAL(debug) << "LanX_DccWriteRegister::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_DCC_READ_REGISTER
std::vector<uint8_t> LanX_DccReadRegister::pack()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), 0x22);
    result.insert(result.end(), 0x11);
    result.insert(result.end(), m_register);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_DccReadRegister::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_DCC_WRITE_REGISTER
std::vector<uint8_t> LanX_DccWriteRegister::pack()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), 0x23);
    result.insert(result.end(), 0x12);
    result.insert(result.end(), m_register);
    result.insert(result.end(), m_value);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_DccWriteRegister::pack(): " << PRINT_HEX(result);
    return result;
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

    if (m_group == GROUP_1 || m_group == GROUP_2 || m_group == GROUP_3 || m_group == GROUP_4 || m_group == GROUP_5 ||
        m_group == GROUP_6 || m_group == GROUP_7 || m_group == GROUP_8 || m_group == GROUP_9 || m_group == GROUP_10)
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

// LAN_X_SET_LOCO_BINARY_STATE
std::vector<uint8_t> LanX_SetLocoBinaryState::pack()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), 0xe5);
    result.insert(result.end(), 0x5f);
    result.insert(result.end(), (m_address >> 8) & 0x3f);
    result.insert(result.end(), m_address & 0xff);

    result.insert(result.end(), (m_on ? 0x80 : 0) + (m_address & 0x7f));    // Low order is 7 bits
    result.insert(result.end(), (m_address >> 7) & 0xff);
    append_checksum(result);

    BOOST_LOG_TRIVIAL(debug) << "LanX_SetLocoBinaryState::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_GET_TURNOUT_INFO
std::vector<uint8_t> LanX_GetTurnoutInfo::pack()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), 0x43);
    result.insert(result.end(), m_address >> 8);
    result.insert(result.end(), m_address & 0xff);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_GetTurnoutInfo::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_SET_TURNOUT
// TODO: Better value handling of switch settings.
std::vector<uint8_t> LanX_SetTurnout::pack()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), 0x53);
    result.insert(result.end(), m_address >> 8);
    result.insert(result.end(), m_address & 0xff);
    result.insert(result.end(), m_value);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_SetTurnout::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_SET_EXT_ACCESSORY
std::vector<uint8_t> LanX_SetExtAccessory::pack()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), 0x54);
    result.insert(result.end(), m_address >> 8);
    result.insert(result.end(), m_address & 0xff);
    result.insert(result.end(), m_state);
    result.insert(result.end(), 0x00);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_SetExtAccessory::pack(): " << PRINT_HEX(result);
    return result;
}

// LAN_X_GET_EXT_ACCESSORY_INFO
std::vector<uint8_t> LanX_GetExtAccessoryInfo::pack()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), 0x44);
    result.insert(result.end(), m_address >> 8);
    result.insert(result.end(), m_address & 0xff);
    result.insert(result.end(), 0x00);
    append_checksum(result);
    BOOST_LOG_TRIVIAL(debug) << "LanX_GetExtAccessoryInfo::pack(): " << PRINT_HEX(result);
    return result;
}

// ==== Z21 to client ====

// LAN_X_GET_TURNOUT_INFO
void LanX_TurnoutInfo::unpack(std::vector<uint8_t>& data)
{
    address = (data[1] << 8) + data[2];

    switch (data[3] & 0x03)
    {
        case TurnoutStatus::NOT_SWITCHED:
            status = TurnoutStatus::NOT_SWITCHED;
            break;
        case TurnoutStatus::SWITCHED_P0:
            status = TurnoutStatus::SWITCHED_P0;
            break;
        case TurnoutStatus::SWITCHED_P1:
            status = TurnoutStatus::SWITCHED_P1;
            break;
        default:
            status = TurnoutStatus::UNKNOWN;
    }

    BOOST_LOG_TRIVIAL(debug) << "LanX_TurnoutInfo::unpack(): " << (int)address << ": status = " << (int)status;
}


// LAN_X_EXT_ACCESSORY_INFO
void LanX_ExtAccessoryInfo::unpack(std::vector<uint8_t>& data)
{
    address = (data[1] << 8) + data[2];
    state = data[3];
    data_valid = data[4] == 0x00;

    BOOST_LOG_TRIVIAL(debug) << "LanX_ExtAccessoryInfo::unpack(): " << (int)address << ": status = " << (int)state;
}


inline void read_function_values(std::vector<bool>& functions, uint8_t index_start, uint8_t data)
{
    functions[index_start] = data & 0x01;
    functions[index_start + 1] = data & 0x02;
    functions[index_start + 2] = data & 0x04;
    functions[index_start + 3] = data & 0x08;
    functions[index_start + 4] = data & 0x10;
    functions[index_start + 5] = data & 0x20;
    functions[index_start + 6] = data & 0x40;
    functions[index_start + 7] = data & 0x80;
}

// LAN_X_LOCO_INFO
void LanX_LocoInfo::unpack(std::vector<uint8_t>& data)
{
    address = ((data[1] & 0x3f) << 8) + data[2];

    // db2
    busy = data[3] & 0x8;
    uint8_t s = data[3] & 0x07;
    switch (s) {
        case SpeedSteps::DCC_14:
            speed_steps = SpeedSteps::DCC_14;
            break;
        case SpeedSteps::DCC_28:
            speed_steps = SpeedSteps::DCC_28;
            break;
        case SpeedSteps::DCC_128:
            speed_steps = SpeedSteps::DCC_128;
            break;
        default:
            speed_steps = SpeedSteps::UNKNOWN;
    }

    // db3
    direction_forward = data[4] & 0x80;
    speed = data[4] & 0x7f;

    // db4
    double_traction = data[5] & 0x40;
    smart_search = data[5] & 0x20;
    functions[0] = data[5] & 0x10;
    functions[4] = data[5] & 0x08;  // Order of functions is irregular in spec (see 4.4)
    functions[3] = data[5] & 0x04;
    functions[2] = data[5] & 0x02;
    functions[1] = data[5] & 0x01;

    read_function_values(functions, 5, data[6]);
    read_function_values(functions, 13, data[7]);
    read_function_values(functions, 21, data[8]);

    if (data.size() > 10) {
        functions[29] = data[9] & 0x01;
        functions[30] = data[9] & 0x02;
        functions[31] = data[9] & 0x04;
    }

    BOOST_LOG_TRIVIAL(debug) << "LanX_LocoInfo::unpack(): " << (int)address << ": direction = " << direction_forward
                             << ", speed = " << (int)speed << ", light = " << functions[0];
}


// LAN_X_CV_RESULT
void LanX_CvResult::unpack(std::vector<uint8_t>& data)
{
    cv = ((data[2] << 8) + data[3]) + 1;
    value = data[4];

    BOOST_LOG_TRIVIAL(debug) << "!!! LanX_CvResult::unpack(): " << (int)cv << " = " << (int)value;
}

