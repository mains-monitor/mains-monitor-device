# Mains Monitor firmware

* Uses Arduino library
* Uses WiFiManager for first configuration of the device

## How to build and upload firmware

1. Copy and rename ``env.example`` to ``.env``
1. Provide the actual values for environment variables, for example

        MD_API_ENDPOINT_HOST=api.example.server.com
        MD_API_ENDPOINT_PATH=/mains-monitor/set-state
1. Open the project in VSCode with Platformio extension installed and build it
1. Connect the device to a laptop or PC via USB, it will appear as a COM port on Windows
1. Configure Platformio to use that COM port
1. Click "Platformio: Upload" and click reset button on the device to wake it up. Note, the device will be active for 20s and then will disconnect from USB. You have to start flashing it within this interval.

## How to dump nvs partition and read its values on PC

1. Read partition table first, to see where is ``nvs`` partition is located

        esptool -p <COM_PORT OR TTY DEV> read_flash 0x8000 0x1000 part_table.bin

2. Decode partition table with ``gen_esp32part.py``, so you know the partition offset and size of ``nvs``

        python gen_esp32part.py part_table.bin

You should get output like the following:

        Parsing binary partition input...
        Verifying table...
        # ESP-IDF Partition Table
        # Name, Type, SubType, Offset, Size, Flags
        nvs,data,nvs,0x9000,24K,
        phy_init,data,phy,0xf000,4K,
        factory,app,factory,0x10000,1M,
        storage,data,spiffs,0x110000,960K,

3. Read ``nvs`` partition data using ``esptool``

        esptool -p <COM_PORT OR TTY DEV> read_flash 0x9000 0x6000 nvs_readout.bin


``0x6000`` is 24K in HEX

4. Decode ``nvs`` binary

        python analyze_nvs.py nvs_readout.bin -s -b 32

## How to erase ``nvs`` to reset the settings

        python parttool.py -p <COM_PORT OR TTY DEV> erase_partition --partition-name nvs
