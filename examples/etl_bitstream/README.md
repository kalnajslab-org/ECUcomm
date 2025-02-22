# ETLCPP Testing

See the [Embedded Template Library](https://github.com/ETLCPP/etl.git).

Output from `etl_bitstream.cpp`:

```
Write values:
a: 0111
b: 10
c: 1
d: 0000010000000
e: 11111001101111001111110100101101 (-105.054931641)
f: 111
g: 10000000001

Storage bits: 66, dump of the write buffer:
01111010 00001000 00001111 10011011 11001111 11010010 11011111 00000000 01000000 

Read values:
a: 0111
b: 10
c: 1
d: 0000010000000
e: 11111001101111001111110100101101 (-105.054931641)
f: 111
g: 10000000001
```

The choice of endianess seems to determine the bit order _within the bytes_. 
This is not the convential meaning for enidianess, which is strictly used for 
byte order and addressing; i.e the byte order for multibyte types. However,
if the same value is used, serialization/deserialization works for both of
them. Will need to experiment to see how this works if trying to decode
a bit packed stream without using ETLCPP.

N.B. The docs do state that the _endian_ parameter specifies
_"Values may be streamed in either msb or lsb format"_.


You can see the effect in the dump of the write buffer,when 
_etl::endian::big_ was changed to _etl::endian::little_:

```
Write values:
a: 0111
b: 10
c: 1
d: 0000010000000
e: 11111001101111001111110100101101 (-105.054931641)
f: 111
g: 10000000001

Storage bits: 66, dump of the write buffer:
11100110 00000010 00001011 01001011 11110011 11011001 11111111 00000000 01000000 

Read values:
a: 0111
b: 10
c: 1
d: 0000010000000
e: 11111001101111001111110100101101 (-105.054931641)
f: 111
g: 10000000001
```