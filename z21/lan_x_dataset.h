
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
    LAN_X_GET_FIRMWARE_VERSION,                  // missing

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


// ==========================================================================
//      Client to Z21
// ==========================================================================

// LAN_X_GET_VERSION
class LanX_GetVersion: public LanX_Packet
{
public:
    LanX_GetVersion() : LanX_Packet(LanXCommands::LAN_X_GET_VERSION) {}
    virtual std::vector<uint8_t> pack();
};

// LAN_X_GET_STATUS
class LanX_GetStatus: public LanX_Packet
{
public:
    LanX_GetStatus() : LanX_Packet(LanXCommands::LAN_X_GET_STATUS) {}
    virtual std::vector<uint8_t> pack();
};


// LAN_X_SET_TRACK_POWER_OFF
class LanX_SetTrackPowerOff: public LanX_Packet
{
public:
    LanX_SetTrackPowerOff() : LanX_Packet(LanXCommands::LAN_X_SET_TRACK_POWER_OFF) {}
    virtual std::vector<uint8_t> pack();
};


// LAN_X_SET_TRACK_POWER_ON
class LanX_SetTrackPowerOn: public LanX_Packet
{
public:
    LanX_SetTrackPowerOn() : LanX_Packet(LanXCommands::LAN_X_SET_TRACK_POWER_ON) {}
    virtual std::vector<uint8_t> pack();
};


// LAN_X_DCC_READ_REGISTER
class LanX_DccReadRegister: public LanX_Packet
{
public:
    LanX_DccReadRegister(uint8_t reg) :
            LanX_Packet(LanXCommands::LAN_X_DCC_READ_REGISTER),
            m_register(reg)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint8_t m_register;
};


// LAN_X_CV_READ
class LanX_CvRead : public LanX_Packet
{
public:
    LanX_CvRead(uint16_t cv) :
            LanX_Packet(LanXCommands::LAN_X_CV_READ),
            m_cv(cv)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_cv;
};


// LAN_X_DCC_WRITE_REGISTER
class LanX_DccWriteRegister: public LanX_Packet
{
public:
    LanX_DccWriteRegister(uint8_t reg, uint8_t value) :
            LanX_Packet(LanXCommands::LAN_X_DCC_WRITE_REGISTER),
            m_register(reg), m_value(value)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint8_t m_register;
    uint8_t m_value;
};


// LAN_X_CV_WRITE
class LanX_CvWrite : public LanX_Packet
{
public:
    LanX_CvWrite(uint16_t cv, uint8_t value) :
            LanX_Packet(LanXCommands::LAN_X_CV_WRITE),
            m_cv(cv), m_value(value)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_cv;
    uint8_t m_value;
};


// LAN_X_MM_WRITE_BYTE
class LanX_MmWriteByte: public LanX_Packet
{
public:
    LanX_MmWriteByte(uint8_t reg, uint8_t value) :
            LanX_Packet(LanXCommands::LAN_X_MM_WRITE_BYTE),
            m_register(reg), m_value(value)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint8_t m_register;
    uint8_t m_value;
};


// LAN_X_GET_TURNOUT_INFO
class LanX_GetTurnoutInfo : public LanX_Packet
{
public:
    LanX_GetTurnoutInfo(uint16_t address) :
            LanX_Packet(LanXCommands::LAN_X_GET_TURNOUT_INFO),
            m_address(address)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
};


// LAN_X_GET_EXT_ACCESSORY_INFO
class LanX_GetExtAccessoryInfo : public LanX_Packet
{
public:
    LanX_GetExtAccessoryInfo(uint16_t address) :
            LanX_Packet(LanXCommands::LAN_X_GET_EXT_ACCESSORY_INFO),
            m_address(address)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
};


// LAN_X_SET_TURNOUT
class LanX_SetTurnout : public LanX_Packet
{
public:
    LanX_SetTurnout(uint16_t address, uint8_t value) :
            LanX_Packet(LanXCommands::LAN_X_SET_TURNOUT),
            m_address(address), m_value(value)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
    uint8_t m_value;
};


// LAN_X_SET_EXT_ACCESSORY
class LanX_SetExtAccessory : public LanX_Packet
{
public:
    LanX_SetExtAccessory(uint16_t address, uint8_t state) :
            LanX_Packet(LanXCommands::LAN_X_SET_EXT_ACCESSORY),
            m_address(address), m_state(state)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
    uint8_t m_state;
};


// LAN_X_SET_STOP
class LanX_SetStop: public LanX_Packet
{
public:
    LanX_SetStop() : LanX_Packet(LanXCommands::LAN_X_SET_STOP) {}
    virtual std::vector<uint8_t> pack();
};


// LAN_X_GET_LOCO_INFO
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


// LAN_X_SET_LOCO_DRIVE
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


// LAN_X_SET_LOCO_FUNCTION
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


// LAN_X_SET_LOCO_FUNCTION_GROUP
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


// LAN_X_SET_LOCO_BINARY_STATE
class LanX_SetLocoBinaryState : public LanX_Packet
{
public:
    LanX_SetLocoBinaryState(uint16_t address, bool on, uint8_t binary_address) :
            LanX_Packet(LanXCommands::LAN_X_SET_LOCO_BINARY_STATE),
            m_address(address), m_on(on), m_binary_address(binary_address)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
    bool m_on;
    uint8_t m_binary_address;
};


// LAN_X_CV_POM_WRITE_BYTE
class LanX_CvPomWriteByte : public LanX_Packet
{
public:
    LanX_CvPomWriteByte(uint16_t address, uint16_t cv, uint8_t value) :
            LanX_Packet(LanXCommands::LAN_X_CV_POM_WRITE_BYTE),
            m_address(address), m_cv(cv), m_value(value)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
    uint16_t m_cv;
    uint8_t m_value;
};


// LAN_X_CV_POM_WRITE_BIT
class LanX_CvPomWriteBit : public LanX_Packet
{
public:
    LanX_CvPomWriteBit(uint16_t address, uint16_t cv, uint8_t bit_position, uint8_t value) :
            LanX_Packet(LanXCommands::LAN_X_CV_POM_WRITE_BIT),
            m_address(address), m_cv(cv), m_bit_position(bit_position), m_value(value)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
    uint16_t m_cv;
    uint8_t m_bit_position;
    uint8_t m_value;
};


// LAN_X_CV_POM_READ_BYTE
class LanX_CvPomReadByte : public LanX_Packet
{
public:
    LanX_CvPomReadByte(uint16_t address, uint16_t cv) :
            LanX_Packet(LanXCommands::LAN_X_CV_POM_READ_BYTE),
            m_address(address), m_cv(cv)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
    uint16_t m_cv;
};


enum PomAccessorySelection {
    WHOLE_DECODER = 0x00,
    SPECIFIC_OUTPUT = 0x08
};


// LAN_X_CV_POM_ACCESSORY_WRITE_BYTE
class LanX_CvPomAccessoryWriteByte : public LanX_Packet
{
public:
    LanX_CvPomAccessoryWriteByte(uint16_t address, PomAccessorySelection selction, uint8_t output, uint16_t cv, uint8_t value) :
            LanX_Packet(LanXCommands::LAN_X_CV_POM_ACCESSORY_WRITE_BYTE),
            m_address(address), m_selection(selction), m_output(output), m_cv(cv), m_value(value)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
    PomAccessorySelection m_selection;
    uint8_t m_output;
    uint16_t m_cv;
    uint8_t m_value;
};


// LAN_X_CV_POM_ACCESSORY_WRITE_BIT
class LanX_CvPomAccessoryWriteBit : public LanX_Packet
{
public:
    LanX_CvPomAccessoryWriteBit(uint16_t address, PomAccessorySelection selction, uint8_t output, uint16_t cv, uint8_t bit_position, uint8_t value) :
            LanX_Packet(LanXCommands::LAN_X_CV_POM_ACCESSORY_WRITE_BIT),
            m_address(address), m_selection(selction), m_output(output), m_cv(cv), m_bit_position(bit_position), m_value(value)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
    PomAccessorySelection m_selection;
    uint8_t m_output;
    uint16_t m_cv;
    uint8_t m_bit_position;
    uint8_t m_value;
};


// LAN_X_CV_POM_ACCESSORY_READ_BYTE
class LanX_CvPomAccessoryReadByte : public LanX_Packet
{
public:
    LanX_CvPomAccessoryReadByte(uint16_t address, PomAccessorySelection selction, uint8_t output, uint16_t cv) :
            LanX_Packet(LanXCommands::LAN_X_CV_POM_ACCESSORY_READ_BYTE),
            m_address(address), m_selection(selction), m_output(output), m_cv(cv)
    {}

    virtual std::vector<uint8_t> pack();

protected:
    uint16_t m_address;
    PomAccessorySelection m_selection;
    uint8_t m_output;
    uint16_t m_cv;
};


// LAN_X_GET_FIRMWARE_VERSION
class LanX_GetFirmwareVersion: public LanX_Packet
{
public:
    LanX_GetFirmwareVersion() : LanX_Packet(LanXCommands::LAN_X_GET_FIRMWARE_VERSION) {}
    virtual std::vector<uint8_t> pack();
};



// ==========================================================================
//      Z21 to client
// ==========================================================================

class LanX_TurnoutInfo : public LanX_Packet
{
public:
    enum TurnoutStatus {
        UNKNOWN = 255,
        NOT_SWITCHED = 0,
        SWITCHED_P0 = 1,
        SWITCHED_P1 = 2,
    };

    LanX_TurnoutInfo() : LanX_Packet(LanXCommands::LAN_X_TURNOUT_INFO) {}

    virtual void unpack(std::vector<uint8_t>& data);

    uint16_t address{0};
    TurnoutStatus status;
};


class LanX_ExtAccessoryInfo : public LanX_Packet
{
public:
    LanX_ExtAccessoryInfo() : LanX_Packet(LanXCommands::LAN_X_EXT_ACCESSORY_INFO) {}

    virtual void unpack(std::vector<uint8_t>& data);

    uint16_t address{0};
    uint8_t state{0};
    bool data_valid{false};
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

    virtual void unpack(std::vector<uint8_t>& data);

    uint16_t cv{0};
    uint8_t value{0};
};


class LanX_BcStopped : public LanX_Packet
{
public:
    LanX_BcStopped() : LanX_Packet(LanXCommands::LAN_X_BC_STOPPED) {}
};


class LanX_LocoInfo : public LanX_Packet
{
public:
    enum SpeedSteps {
        UNKNOWN = 255,
        DCC_14 = 0,
        DCC_28 = 2,
        DCC_128 = 4,
    };

    LanX_LocoInfo() : LanX_Packet(LanXCommands::LAN_X_LOCO_INFO)
    {
        functions.resize(32);
    }

    virtual void unpack(std::vector<uint8_t>& data);

    uint16_t address{0};
    bool busy{false};
    SpeedSteps speed_steps{SpeedSteps::UNKNOWN};
    bool direction_forward{false};
    uint8_t speed{0};

    bool double_traction{false};
    bool smart_search{false};
    std::vector<bool> functions;
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

