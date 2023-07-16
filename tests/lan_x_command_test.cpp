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

#include <vector>
#include <memory>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../z21/lan_x_command.h"


using namespace testing;


class LanX_CommandTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};


TEST_F(LanX_CommandTest, LanGetHWInfo)
{
    LanX_GetFirmwareVersionResponse response;

    std::vector<uint8_t> response_data = {0xf3, 0x0a, 0x01, 0x33};

    response.unpack(response_data);
    ASSERT_EQ(response.fw_version, "1.33");
}
