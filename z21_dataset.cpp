
#include <iostream>
#include <sstream>
#include <iterator>
#include <numeric>

#include <boost/log/trivial.hpp>

#include "z21_dataset.h"


std::vector<uint8_t> Z21_DataSet::pack()
{
    std::vector<uint8_t> result;
    std::vector<uint8_t> data = pack_data();
    uint16_t size = header_size + data.size();

    result.insert(result.end(), size & 0xff);
    result.insert(result.end(), (size >> 8) & 0xff);

    result.insert(result.end(), id & 0xff);
    result.insert(result.end(), (id >> 8) & 0xff);

    result.insert(result.end(), data.begin(), data.end());

    return result;
}


void Z21_DataSet::unpack(std::vector<uint8_t>& data)
{
}


std::vector<uint8_t> LanSetBroadcastFlags::pack_data()
{
    std::vector<uint8_t> result;
    result.insert(result.end(), m_flags & 0xff);
    result.insert(result.end(), (m_flags >> 8) & 0xff);
    result.insert(result.end(), (m_flags >> 16) & 0xff);
    result.insert(result.end(), (m_flags >> 24) & 0xff);
    BOOST_LOG_TRIVIAL(debug) << "LanSetBroadcastFlags::pack_data(): " << PRINT_HEX(result);
    return result;
}


void LanGetBroadcastFlags::unpack(std::vector<uint8_t> &data)
{
    BOOST_LOG_TRIVIAL(debug) << "LanGetBroadcastFlags::unpack(): " << PRINT_HEX(data);
}


void LanGetSerialNumber::unpack(std::vector<uint8_t> &data)
{
    BOOST_LOG_TRIVIAL(debug) << "LanGetSerialNumber::unpack(): " << PRINT_HEX(data);
    if (data.size() == 4) {
        serial_number = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
    }
}


void LanGetHWInfo::unpack(std::vector<uint8_t> &data)
{
    BOOST_LOG_TRIVIAL(debug) << "LanGetHWInfo::unpack(): " << PRINT_HEX(data);
    if (data.size() == 8) {
        hw_type = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);

        bool got_value = false;
        std::vector<std::string> result;

        for(size_t i = 4; i > 0; i--) {
            uint8_t v = data[4 + i - 1];
            if (! got_value && v == 0) { continue; }

            uint8_t value = 0;
            value += v & 0x0f;
            value += (v >> 4) * 10;
            result.push_back(std::to_string(value));
        }

        fw_version = std::accumulate(result.begin() + 1, result.end(), result[0],
                                        [](std::string x, std::string y) { return x + '.' + y; });
    }
}


void LanSystemstateDatachanged::unpack(std::vector<uint8_t> &data)
{
    BOOST_LOG_TRIVIAL(debug) << "LanSystemstateDatachanged::unpack(): " << PRINT_HEX(data);
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
    }
}
