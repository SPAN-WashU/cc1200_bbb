#!/bin/bash

echo "Compiling the overlay from .dts to .dtbo"

dtc -O dtb -o CC1200-SPIDEV-00A0.dtbo -b 0 -@ CC1200-SPIDEV-00A0.dts
sudo mv *.dtbo /lib/firmware
