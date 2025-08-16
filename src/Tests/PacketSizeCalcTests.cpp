#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "Packets/PacketSizeCalc.h"

TEST_CASE("VarInt size calculation for small number") {
    CPacketSizeCalc calc;
    int value = 1;
    calc.OnVarInt(value);
    CHECK(calc.GetFullSize() == 1);
}

TEST_CASE("VarInt size calculation for larger number") {
    CPacketSizeCalc calc;
    int value = 128;
    calc.OnVarInt(value);
    CHECK(calc.GetFullSize() == 2);
}

TEST_CASE("String exceeding maximum size fails") {
    CPacketSizeCalc calc;
    std::string str = "abcdef";
    calc.OnString(str, 5);
    CHECK_FALSE(calc.Success());
}