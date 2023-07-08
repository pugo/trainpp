#include <vector>
#include <memory>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../z21/z21_dataset.h"


using namespace testing;


class Z21DataSetTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};


TEST_F(Z21DataSetTest, LanGetSerialNumber)
{
    LanGetSerialNumber packet;

    std::vector<uint8_t> request_expected = {0x04, 0x00, 0x10, 0x00};
    std::vector<uint8_t> request = packet.pack();
    ASSERT_EQ(request, request_expected);

    std::vector<uint8_t> response = {0x11, 0x22, 0x33, 0x44};
    packet.unpack(response);
    ASSERT_EQ(packet.serial_number, 0x44332211);
}


TEST_F(Z21DataSetTest, LanGetCode)
{
    LanGetCode packet;

    std::vector<uint8_t> request_expected = {0x04, 0x00, 0x18, 0x00};
    std::vector<uint8_t> request = packet.pack();
    ASSERT_EQ(request, request_expected);

    std::vector<uint8_t> response = {0x42};
    packet.unpack(response);
    ASSERT_EQ(packet.code, 0x42);
}


