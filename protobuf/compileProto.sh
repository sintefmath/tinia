#!/bin/bash
#
# Usage: bash

echo "Compiling protobuf"
protoc --cpp_out=./ TiniaProtoBuf.proto

echo "Moving files to correct places"
mv TiniaProtoBuf.pb.h ../include/tinia/protobuf/

#mv TiniaProtoBuf.pb.cc ../src/protobuf/

# Change include path for pb.h
sed -i 's/TiniaProtoBuf.pb.h/..\/include\/tinia\/protobuf\/TiniaProtoBuf.pb.h/g' TiniaProtoBuf.pb.cc
mv TiniaProtoBuf.pb.cc ../src/qtcontroller/

