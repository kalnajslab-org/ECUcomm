#include <Arduino.h>
#define ETL_NO_STL
#define ETL_NO_INITIALIZER_LIST

#include "etl/bit_stream.h"
#include "etl/array.h"

// It's not strictly necessary to create a struct with bitfields,
// but it makes it easier to see the bit layout.
struct BitfieldStruct
{
    uint8_t a : 4;
    uint8_t b : 2;
    uint8_t c : 1;
    uint16_t d : 13;
    int32_t e : 32;
    uint8_t f : 3;
    uint16_t g : 11;
};

void bin_print(uint32_t n, uint8_t w=8)
{
    for (int i = w-1; i >= 0; i--)
    {
        SerialUSB.print((n & (1 << i)) ? "1" : "0");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(3000);


    BitfieldStruct test_write;
    test_write.a = 0b0111;
    test_write.b = 0b10;
    test_write.c = 0b1;
    test_write.d = 0b0000010000000;
    test_write.e = (int32_t)(-105.054931555*1.0e6);
    test_write.f = 0b111;
    test_write.g = 0b10000000001;

    SerialUSB.println("\nWrite values:");
    SerialUSB.print("a: "); bin_print(test_write.a, 4); SerialUSB.println();
    SerialUSB.print("b: "); bin_print(test_write.b, 2); SerialUSB.println();
    SerialUSB.print("c: "); bin_print(test_write.c, 1); SerialUSB.println();
    SerialUSB.print("d: "); bin_print(test_write.d, 13); SerialUSB.println();
    SerialUSB.print("e: "); bin_print(test_write.e, 32); SerialUSB.print(" (" + String(test_write.e/1.0e6, 9) + ")"); SerialUSB.println();
    SerialUSB.print("f: "); bin_print(test_write.f, 3); SerialUSB.println();
    SerialUSB.print("g: "); bin_print(test_write.g, 11); SerialUSB.println();
    SerialUSB.println();

    // Create the write buffer
    etl::array<char, 9U> write_buffer;
    etl::span<char> write_span(write_buffer.data(), write_buffer.size());

    etl::bit_stream_writer bit_stream(write_span, etl::endian::little);
    bit_stream.write_unchecked(test_write.a, 4);
    bit_stream.write_unchecked(test_write.b, 2);
    bit_stream.write_unchecked(test_write.c, 1);
    bit_stream.write_unchecked(test_write.d, 13);
    bit_stream.write_unchecked(test_write.e, 32);
    bit_stream.write_unchecked(test_write.f, 3);
    bit_stream.write_unchecked(test_write.g, 11);

    SerialUSB.println("Storage bits: " + String(bit_stream.size_bits()) + ", dump of the write buffer:");

    for (size_t i = 0; i < write_buffer.size(); ++i)
    {
        bin_print(write_buffer[i]);
        SerialUSB.print(" ");
    }
    SerialUSB.println();
    SerialUSB.println();

    // Copy the write buffer to the read buffer.
    etl::array<char, 9U> read_buffer; // Assume the buffer gets filled with bit stream data.
    read_buffer = write_buffer;

    etl::span<char> read_span(read_buffer.data(), read_buffer.size());
    etl::bit_stream_reader bit_stream_reader(write_span, etl::endian::little);
    BitfieldStruct test_read;
    test_read.a = bit_stream_reader.read_unchecked<uint8_t>(4);
    test_read.b = bit_stream_reader.read_unchecked<uint8_t>(2);
    test_read.c = bit_stream_reader.read_unchecked<uint8_t>(1);
    test_read.d = bit_stream_reader.read_unchecked<uint16_t>(13);
    test_read.e = bit_stream_reader.read_unchecked<int32_t>(32);
    test_read.f = bit_stream_reader.read_unchecked<uint8_t>(3);
    test_read.g = bit_stream_reader.read_unchecked<uint16_t>(11);

    SerialUSB.println("Read values:");
    SerialUSB.print("a: "); bin_print(test_read.a, 4); SerialUSB.println();
    SerialUSB.print("b: "); bin_print(test_read.b, 2); SerialUSB.println();
    SerialUSB.print("c: "); bin_print(test_read.c, 1); SerialUSB.println();
    SerialUSB.print("d: "); bin_print(test_read.d, 13); SerialUSB.println();
    SerialUSB.print("e: "); bin_print(test_read.e, 32); SerialUSB.print(" (" + String(test_read.e/1.0e6, 9) + ")"); SerialUSB.println();
    SerialUSB.print("f: "); bin_print(test_read.f, 3); SerialUSB.println();
    SerialUSB.print("g: "); bin_print(test_read.g, 11); SerialUSB.println();

}

void loop()
{
    delay(1000);
}
