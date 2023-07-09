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

#ifndef TRAINPP_Z21_DATASET_H
#define TRAINPP_Z21_DATASET_H

#include <iostream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/format.hpp>

#include "lan_x_packet.h"

using boost::adaptors::transformed;

#define PRINT_HEX(v) (boost::algorithm::join(v | transformed([](uint8_t d) { return (boost::format("%02x") % (unsigned)d).str(); }), " "))

constexpr size_t header_size = 4;


/**
 * Base class for all Z21 DataSets (commands).
 */
class Z21_DataSet
{
public:
    enum DataSet {
        LAN_GET_SERIAL_NUMBER = 0x10,
        LAN_GET_CODE = 0x18,
        LAN_GET_HWINFO = 0x1a,
        LAN_X = 0x40,
        LAN_SET_BROADCASTFLAGS = 0x50,
        LAN_GET_BROADCASTFLAGS = 0x51,
        LAN_GET_LOCOMODE = 0x60,
        LAN_SET_LOCOMODE = 0x61,
        LAN_GET_TURNOUTMODE = 0x70,
        LAN_SET_TURNOUTMODE = 0x71,
        LAN_SYSTEMSTATE_DATACHANGED = 0x84,
        LAN_SYSTEMSTATE_GETDATA = 0x85
    };

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


// LAN_GET_SERIAL_NUMBER (0x10)
class LanGetSerialNumber : public Z21_DataSet
{
public:
    LanGetSerialNumber() { m_id = LAN_GET_SERIAL_NUMBER; }

    virtual void unpack(std::vector<uint8_t>& data);

    uint32_t serial_number;
};


// LAN_GET_CODE (0x18)
class LanGetCode : public Z21_DataSet
{
public:
    LanGetCode() { m_id = LAN_GET_CODE; }

    virtual void unpack(std::vector<uint8_t>& data);

    uint8_t code{0};
};


// LAN_GET_HWINFO (0x1a)
class LanGetHWInfo : public Z21_DataSet
{
public:
    LanGetHWInfo() { m_id = LAN_GET_HWINFO; }

    virtual void unpack(std::vector<uint8_t>& data);

    uint32_t hw_type;
    std::string fw_version;
};


// LAN_X (0x40)
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


// LAN_SET_BROADCASTFLAGS (0x50)
class LanSetBroadcastFlags : public Z21_DataSet
{
public:
    LanSetBroadcastFlags(uint32_t flags) : m_flags(flags) { m_id = LAN_SET_BROADCASTFLAGS; }

private:
    virtual std::vector<uint8_t> pack_data();

    uint32_t m_flags;
};


// LAN_GET_BROADCASTFLAGS (0x51)
class LanGetBroadcastFlags : public Z21_DataSet
{
public:
    LanGetBroadcastFlags() { m_id = LAN_GET_BROADCASTFLAGS; }

    virtual void unpack(std::vector<uint8_t>& data);

private:
};


enum class Locomode : uint8_t {
    UNKNOWN = 255,
    DCC = 0,
    MM = 1
};


// LAN_GET_LOCOMODE (0x60)
class LanGetLocomode : public Z21_DataSet
{
public:
    LanGetLocomode(uint16_t address=0) : address(address)
    { m_id = LAN_GET_LOCOMODE; }

    virtual void unpack(std::vector<uint8_t>& data);

    Locomode mode{Locomode::UNKNOWN};
    uint16_t address;

protected:
    virtual std::vector<uint8_t> pack_data();
};


// LAN_SET_LOCOMODE (0x61)
class LanSetLocomode : public Z21_DataSet
{
public:
    LanSetLocomode(uint16_t address, Locomode mode) :
            m_address(address), m_mode(mode)
    { m_id = LAN_SET_LOCOMODE; }

private:
    virtual std::vector<uint8_t> pack_data();

    uint16_t m_address;
    Locomode m_mode;
};


// LAN_GET_TURNOUTMODE (0x70)
class LanGetTurnoutmode : public LanGetLocomode
{
public:
    LanGetTurnoutmode(uint16_t address=0) : LanGetLocomode(address)
    { m_id = LAN_GET_TURNOUTMODE; }
};


// LAN_SET_TURNOUTMODE (0x71)
class LanSetTurnoutmode : public LanSetLocomode
{
public:
    LanSetTurnoutmode(uint16_t address, Locomode mode) : LanSetLocomode(address, mode)
    { m_id = LAN_SET_TURNOUTMODE; }
};


// LAN_SYSTEMSTATE_DATACHANGED (0x84)
class LanSystemstateDatachanged : public Z21_DataSet
{
public:
    LanSystemstateDatachanged() { m_id = LAN_SYSTEMSTATE_DATACHANGED; }

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


// LAN_SYSTEMSTATE_GETDATA (0x85)
class LanSystemstateGetData : public Z21_DataSet
{
public:
    LanSystemstateGetData() { m_id = LAN_SYSTEMSTATE_GETDATA; }
};


#endif //TRAINPP_Z_21_DATA_SET_H
