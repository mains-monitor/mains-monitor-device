Import("env", "projenv")

env.Execute("$PYTHONEXE -m pip install python-dotenv qrcode")

# Install missed package
try:
    import os
    from dotenv import load_dotenv
    load_dotenv()
    host = os.environ['MD_API_ENDPOINT_HOST']
    http_path = os.environ['MD_API_ENDPOINT_PATH']
    device_id = os.environ['MD_DEVICE_ID']
    smartconfig_key = os.environ['MD_SMARTCONFIG_KEY']
    os.system(f"qr --ascii electrostatcfg:///?key={smartconfig_key}")
    with open('src/config.hpp', 'w') as f:
        f.write('#pragma once\n')
        f.write('\n')
        f.write(f'#define MD_API_ENDPOINT_HOST "{host}"\n')
        f.write(f'#define MD_API_ENDPOINT_PATH "{http_path}"\n')
        f.write(f'#define MD_DEVICE_ID "{device_id}"\n')
        f.write(f'#define MD_SMARTCONFIG_KEY "{smartconfig_key}"\n')
        f.write('\n')
except ImportError:
    raise RuntimeError('Unable to set custom CXX flags')