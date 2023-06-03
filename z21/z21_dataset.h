
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
    virtual void unpack(std::vector<uint8_t>& data) {}

    uint16_t id() { return m_id; }

protected:
    uint16_t m_id;

    virtual std::vector<uint8_t> pack_data()
    {
        std::vector<uint8_t> result;
        return result;
    }
};


enum BroadcastFlags
{
    DRIVING_AND_SWITCHING = 0x1,
    RBUS_FEEDBACK_CHANGES = 0x2,
    RAILCOM_LOCO_CHANGES = 0x4,
    Z21_STATUS_CHANGES = 0x100,
};


class LanSetBroadcastFlags : public Z21_DataSet
{
public:

    LanSetBroadcastFlags(uint32_t flags) : m_flags(flags) { m_id = 0x50; }

private:
    virtual std::vector<uint8_t> pack_data();

    uint32_t m_flags;
};


class LanGetSerialNumber : public Z21_DataSet
{
public:
    LanGetSerialNumber() { m_id = 0x10; }

    virtual void unpack(std::vector<uint8_t>& data);

    uint32_t serial_number;
};


class LanGetHWInfo : public Z21_DataSet
{
public:
    LanGetHWInfo() { m_id = 0x1a; }

    virtual void unpack(std::vector<uint8_t>& data);

    uint32_t hw_type;
    std::string fw_version;
};


class LanGetCode : public Z21_DataSet
{
public:
    LanGetCode() { m_id = 0x18; }

    virtual void unpack(std::vector<uint8_t>& data);

    uint8_t code{0};
};


class LanGetBroadcastFlags : public Z21_DataSet
{
public:
    LanGetBroadcastFlags() { m_id = 0x51; }

    virtual void unpack(std::vector<uint8_t>& data);

private:
};


class LanSystemstateDatachanged : public Z21_DataSet
{
public:
    LanSystemstateDatachanged() { m_id = 0x84; }

    virtual void unpack(std::vector<uint8_t>& data);

    int16_t main_current{0};
    int16_t prog_current{0};
    int16_t filtered_main_current{0};
    int16_t temperature{0};

    uint16_t supply_voltage{0};
    uint16_t vcc_voltage{0};

    uint8_t central_state{0};
    uint8_t central_state_ex{0};
    uint8_t capabilities{0};

    bool emergency_stop{false};
    bool track_voltage_off{false};
    bool short_cirtcuit{false};
    bool programming_mode{false};
};


class LanSystemstateGetData : public Z21_DataSet
{
public:
    LanSystemstateGetData() { m_id = 0x85; }
};


#endif //TRAINPP_Z_21_DATA_SET_H
