
#ifndef TRAINPP_Z21_LAN_X_DATASET_H
#define TRAINPP_Z21_LAN_X_DATASET_H

#include <map>

#include "z21_dataset.h"

enum class LanXCommands
{
    // Client to Z21
    LAN_X_GET_VERSION,
    LAN_X_GET_STATUS,
    LAN_X_SET_TRACK_POWER_OFF,
    LAN_X_SET_TRACK_POWER_ON,
    LAN_X_DCC_READ_REGISTER,
    LAN_X_CV_READ,
    LAN_X_DCC_WRITE_REGISTER,
    LAN_X_CV_WRITE,
    LAN_X_MM_WRITE_BYTE,
    LAN_X_GET_TURNOUT_INFO,
    LAN_X_GET_EXT_ACCESSORY_INFO,
    LAN_X_SET_TURNOUT,
    LAN_X_SET_EXT_ACCESSORY,
    LAN_X_SET_STOP,
    LAN_X_GET_LOCO_INFO,
    LAN_X_SET_LOCO_DRIVE,
    LAN_X_SET_LOCO_FUNCTION,
    LAN_X_SET_LOCO_FUNCTION_GROUP,
    LAN_X_SET_LOCO_BINARY_STATE,
    LAN_X_CV_POM_WRITE_BYTE,
    LAN_X_CV_POM_WRITE_BIT,
    LAN_X_CV_POM_READ_BYTE,
    LAN_X_CV_POM_ACCESSORY_WRITE_BYTE,
    LAN_X_CV_POM_ACCESSORY_WRITE_BIT,
    LAN_X_CV_POM_ACCESSORY_READ_BYTE,
    LAN_X_GET_FIRMWARE_VERSION,

    // Z21 to client
    LAN_X_TURNOUT_INFO,
    LAN_X_EXT_ACCESSORY_INFO,
    LAN_X_BC_TRACK_POWER_OFF,
    LAN_X_BC_TRACK_POWER_ON,
    LAN_X_BC_PROGRAMMING_MODE,
    LAN_X_BC_TRACK_SHORT_CIRCUIT,
    LAN_X_CV_NACK_SC,
    LAN_X_CV_NACK,
    LAN_X_UNKNOWN_COMMAND,
    LAN_X_STATUS_CHANGED,
    LAN_X_GET_VERSION_RESPONSE,
    LAN_X_CV_RESULT,
    LAN_X_BC_STOPPED,
    LAN_X_LOCO_INFO,
    LAN_X_GET_FIRMWARE_VERSION_RESPONSE
};


class LanX_Packet
{
public:
    LanX_Packet(LanXCommands id) : id(id) {}

    virtual std::vector<uint8_t> pack() {
        std::vector<uint8_t> result;
        return result;
    }

    virtual void unpack(std::vector<uint8_t>& data) {}

    const LanXCommands id;

protected:
    void append_checksum(std::vector<uint8_t>& data);
};


// ---- Client to Z21

class LanX_GetLocoInfo : public LanX_Packet
{
public:
    LanX_GetLocoInfo(uint16_t address) :
        LanX_Packet(LanXCommands::LAN_X_GET_LOCO_INFO),
        m_address(address)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
};


class LanX_SetLocoDrive : public LanX_Packet
{
public:
    LanX_SetLocoDrive(uint16_t address, uint8_t speed, bool forward) :
            LanX_Packet(LanXCommands::LAN_X_SET_LOCO_DRIVE),
            m_address(address), m_speed(speed), m_forward(forward)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
    uint8_t m_speed;
    bool m_forward;
};


class LanX_SetLocoFunction : public LanX_Packet
{
public:
    LanX_SetLocoFunction(uint16_t address, uint8_t function) :
            LanX_Packet(LanXCommands::LAN_X_SET_LOCO_FUNCTION),
            m_address(address), m_function(function)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
    uint8_t m_function;
};



class LanX_SetLocoFunctionGroup : public LanX_Packet
{
public:
    enum FunctionGroup {
        GROUP_1 = 0x20,
        GROUP_2 = 0x21,
        GROUP_3 = 0x22,
        GROUP_4 = 0x23,
        GROUP_5 = 0x28,
        GROUP_6 = 0x29,
        GROUP_7 = 0x2a,
        GROUP_8 = 0x2b,
        GROUP_9 = 0x50,
        GROUP_10 = 0x51
    };

    LanX_SetLocoFunctionGroup(uint16_t address, FunctionGroup group, uint8_t functions) :
            LanX_Packet(LanXCommands::LAN_X_SET_LOCO_FUNCTION_GROUP),
            m_address(address), m_group(group), m_functions(functions)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
    uint16_t m_group;
    uint8_t m_functions;
};


// ---- Z21 to client

class LanX_TurnoutInfo : public LanX_Packet
{
public:
    LanX_TurnoutInfo() : LanX_Packet(LanXCommands::LAN_X_TURNOUT_INFO) {}
};

class LanX_ExtAccessoryInfo : public LanX_Packet
{
public:
    LanX_ExtAccessoryInfo() : LanX_Packet(LanXCommands::LAN_X_EXT_ACCESSORY_INFO) {}
};

class LanX_BcTrackPowerOff : public LanX_Packet
{
public:
    LanX_BcTrackPowerOff() : LanX_Packet(LanXCommands::LAN_X_BC_TRACK_POWER_OFF) {}
};

class LanX_BcTrackPowerOn : public LanX_Packet
{
public:
    LanX_BcTrackPowerOn() : LanX_Packet(LanXCommands::LAN_X_BC_TRACK_POWER_ON) {}
};

class LanX_BcProgrammingMode : public LanX_Packet
{
public:
    LanX_BcProgrammingMode() : LanX_Packet(LanXCommands::LAN_X_BC_PROGRAMMING_MODE) {}
};

class LanX_BcTrackShortCircuit : public LanX_Packet
{
public:
    LanX_BcTrackShortCircuit() : LanX_Packet(LanXCommands::LAN_X_BC_TRACK_SHORT_CIRCUIT) {}
};

class LanX_CvNackSc : public LanX_Packet
{
public:
    LanX_CvNackSc() : LanX_Packet(LanXCommands::LAN_X_CV_NACK_SC) {}
};

class LanX_CvNack : public LanX_Packet
{
public:
    LanX_CvNack() : LanX_Packet(LanXCommands::LAN_X_CV_NACK) {}
};

class LanX_UnknownCommand : public LanX_Packet
{
public:
    LanX_UnknownCommand() : LanX_Packet(LanXCommands::LAN_X_UNKNOWN_COMMAND) {}
};

class LanX_StatusChanged : public LanX_Packet
{
public:
    LanX_StatusChanged() : LanX_Packet(LanXCommands::LAN_X_STATUS_CHANGED) {}
};

class LanX_GetVersionResponse : public LanX_Packet
{
public:
    LanX_GetVersionResponse() : LanX_Packet(LanXCommands::LAN_X_GET_VERSION_RESPONSE) {}
};

class LanX_CvResult : public LanX_Packet
{
public:
    LanX_CvResult() : LanX_Packet(LanXCommands::LAN_X_CV_RESULT) {}
};

class LanX_BcStopped : public LanX_Packet
{
public:
    LanX_BcStopped() : LanX_Packet(LanXCommands::LAN_X_BC_STOPPED) {}
};

class LanX_LocoInfo : public LanX_Packet
{
public:
    LanX_LocoInfo() : LanX_Packet(LanXCommands::LAN_X_LOCO_INFO) {}

    virtual void unpack(std::vector<uint8_t>& data);

    uint16_t address{0};
    bool busy{false};
    uint8_t speed_steps{0};
    bool direction_forward{false};
    uint8_t speed{0};

    bool double_traction{false};
    bool smart_search{false};
    bool f0_light{false};
    bool f4{false};
    bool f3{false};
    bool f2{false};
    bool f1{false};
};

class LanX_GetFirmwareVersionResponse : public LanX_Packet
{
public:
    LanX_GetFirmwareVersionResponse() : LanX_Packet(LanXCommands::LAN_X_GET_FIRMWARE_VERSION_RESPONSE) {}
};


class LanX : public Z21_DataSet
{
public:
    LanX();
    LanX(LanX_Packet* command);

    virtual void unpack(std::vector<uint8_t>& data);

    LanX_Packet* command() { return m_command; }

protected:
    virtual std::vector<uint8_t> pack_data();

private:
    bool check_checksum(std::vector<uint8_t>& data);

    std::map<LanXCommands, LanX_Packet*> command_handlers;

    LanX_Packet* m_command{nullptr};
};


#endif // TRAINPP_Z21_LAN_X_DATASET_H

