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

#ifndef TRAINPP_Z21_LAN_X_PACKET_H
#define TRAINPP_Z21_LAN_X_PACKET_H

#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/format.hpp>

using boost::adaptors::transformed;

#define PRINT_HEX(v) (boost::algorithm::join(v | transformed([](uint8_t d) { return (boost::format("%02x") % (unsigned)d).str(); }), " "))

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

std::string decode_bcd_version(std::vector<uint8_t> data, bool little_endian);


/**
 * Base class for all LanX packets embedded in Z21 DataSets (commands).
 */
class LanX_Command
{
public:
    LanX_Command(LanXCommands id) : id(id) {}

    virtual std::vector<uint8_t> pack() {
        std::vector<uint8_t> result;
        return result;
    }

    virtual void unpack(std::vector<uint8_t>& data) {}

    const LanXCommands id;

protected:
    void append_checksum(std::vector<uint8_t>& data);
};

#endif // TRAINPP_Z21_LAN_X_PACKET_H

