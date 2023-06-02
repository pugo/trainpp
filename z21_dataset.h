
#ifndef TRAINPP_Z21_DATASET_H
#define TRAINPP_Z21_DATASET_H

#include <iostream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/format.hpp>

using boost::adaptors::transformed;

#define PRINT_HEX(v) (boost::algorithm::join(v | transformed([](uint8_t d) { return (boost::format("%02x") % (unsigned)d).str(); }), " "))

constexpr size_t header_size = 4;


class Z21_DataSet
{
public:
    virtual std::vector<uint8_t> pack();

    virtual void unpack(std::vector<uint8_t>& data);

    uint16_t id;

protected:
    virtual std::vector<uint8_t> pack_data()
    {
        std::vector<uint8_t> result;
        return result;
    }
};


enum BroadcastFlags
{
    DRIVING_AND_SWITCHING = 0x00000001,
    RBUS_FEEDBACK_CHANGES = 0x00000002,
    RAILCOM_LOCO_CHANGES = 0x00000004,
    Z21_STATUS_CHANGES = 0x00000100,
};


class LanSetBroadcastFlags : public Z21_DataSet
{
public:

    LanSetBroadcastFlags(uint32_t flags) : m_flags(flags) { id = 0x50; }

private:
    virtual std::vector<uint8_t> pack_data();

    uint32_t m_flags;
};


class LanGetBroadcastFlags : public Z21_DataSet
{
public:
    LanGetBroadcastFlags() { id = 0x51; }

    virtual void unpack(std::vector<uint8_t>& data);

private:
};


class LanGetSerialNumber : public Z21_DataSet
{
public:
    LanGetSerialNumber() { id = 0x10; }

    virtual void unpack(std::vector<uint8_t>& data);

    uint32_t serial_number;
};


class LanGetHWInfo : public Z21_DataSet
{
public:
    LanGetHWInfo() { id = 0x1a; }

    virtual void unpack(std::vector<uint8_t>& data);

    uint32_t hw_type;
    std::string fw_version;
};


class LanSystemstateDatachanged : public Z21_DataSet
{
public:
    LanSystemstateDatachanged() { id = 0x84; }

    virtual void unpack(std::vector<uint8_t>& data);

    int16_t main_current;
    int16_t prog_current;
    int16_t filtered_main_current;
    int16_t temperature;

    uint16_t supply_voltage;
    uint16_t vcc_voltage;

    uint8_t central_state;
    uint8_t central_state_ex;
    uint8_t capabilities;
};

//    def unpack(self, data):
//        self.main_current, self.prog_current, self.filtered_main_current, self.temperature,\
//                    self.supply_voltage, self.vcc_voltage, self.central_state, self.central_state_ex,\
//                    reserved, self.capabilities = struct.unpack('<hhhhHHBBBB', data)
//
//        self.emergency_stop = bool(self.central_state & 0x01)
//        self.track_power_on = not bool(self.central_state & 0x02)
//        self.short_cirtcuit = bool(self.central_state & 0x04)
//        self.programming_mode = bool(self.central_state & 0x20)


#endif //TRAINPP_Z_21_DATA_SET_H
