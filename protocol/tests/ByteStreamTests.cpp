/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ByteStreamTests.cpp
*/

#include <string>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "TestSupport.hpp"

using namespace rtype::protocol;

void testRoundTrip()
{
    std::string bytes;
    ByteWriter writer(bytes);

    writer.writeU8(0xAB).writeU16(0x1234).writeU32(0xDEADBEEF);
    writer.writeU64(0x0102030405060708ULL).writeI16(-2);

    ByteReader reader(bytes);

    CHECK(reader.readU8() == 0xAB);
    CHECK(reader.readU16() == 0x1234);
    CHECK(reader.readU32() == 0xDEADBEEF);
    CHECK(reader.readU64() == 0x0102030405060708ULL);
    CHECK(reader.readI16() == -2);
    CHECK(reader.finished());
}

void testBigEndianLayout()
{
    std::string bytes;

    ByteWriter(bytes).writeU32(0x01020304);
    CHECK(bytes == std::string("\x01\x02\x03\x04", 4));
}

void testReadPastTheEndFailsForGood()
{
    const std::string bytes("\x01\x02", 2);
    ByteReader reader(bytes);

    CHECK(reader.readU32() == 0);
    CHECK(!reader.ok());
    CHECK(reader.readU8() == 0);
    CHECK(!reader.finished());
}

void testReadBytes()
{
    const std::string bytes = "abcdef";
    ByteReader reader(bytes);

    CHECK(reader.readBytes(4) == "abcd");
    CHECK(reader.remaining() == 2);
    CHECK(reader.readBytes(3).empty());
    CHECK(!reader.ok());
}

void testUnconsumedBytesAreNotFinished()
{
    const std::string bytes("\x01\x02", 2);
    ByteReader reader(bytes);

    CHECK(reader.readU8() == 1);
    CHECK(reader.ok());
    CHECK(!reader.finished());
}

int main()
{
    testRoundTrip();
    testBigEndianLayout();
    testReadPastTheEndFailsForGood();
    testReadBytes();
    testUnconsumedBytesAreNotFinished();
    return TEST_RESULT();
}
