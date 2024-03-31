#!/bin/bash

echo "Creating file..."
dd if=/dev/zero of=A.dat bs=1 count=$((4*1024*1024))
echo -ne "\x01" | dd of=A.dat bs=1 seek=0 count=1 conv=notrunc
echo -ne "\x01" | dd of=A.dat bs=1 seek=10000 count=1 conv=notrunc
echo -ne "\x01" >> A.dat
