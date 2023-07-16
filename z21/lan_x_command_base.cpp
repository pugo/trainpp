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

#include <iostream>
#include <sstream>
#include <iterator>
#include <numeric>

#include <boost/log/trivial.hpp>

#include "lan_x_command.h"


std::string decode_bcd_version(std::vector<uint8_t> data, bool little_endian)
{
    std::vector<std::string> fragments;

    size_t size = data.size();
    for(size_t i = 0; i < size; i++) {
        uint8_t v = little_endian ? data[size - i - 1] : data[i];
        if (v == 0) { continue; }

        uint8_t value = 0;
        value += v & 0x0f;
        value += (v >> 4) * 10;
        fragments.push_back(std::to_string(value));
    }

    std::string result = std::accumulate(fragments.begin() + 1, fragments.end(), fragments[0],
                                         [](const std::string& a, const std::string& b) { return a + '.' + b; });
    return result;
}


void LanX_Command::append_checksum(std::vector<uint8_t>& data)
{
    uint8_t result = 0;
    for (auto c = data.begin(); c < data.end(); c++) {
        result ^= *c;
    }

    data.insert(data.end(), result);
}


