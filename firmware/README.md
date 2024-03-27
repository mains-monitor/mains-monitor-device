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