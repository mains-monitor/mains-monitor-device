Import("env", "projenv")

env.Execute("$PYTHONEXE -m pip install python-dotenv")

# Install missed package
try:
    import os
    from dotenv import load_dotenv
    load_dotenv()
    host = os.environ['MD_API_ENDPOINT_HOST']
    http_path = os.environ['MD_API_ENDPOINT_PATH']
    projenv.Append(CPPDEFINES=[
        ("API_ENDPOINT_HOST", f'\\"{host}\\"'),
        ("API_ENDPOINT_PATH", f'\\"{http_path}\\"')
    ])
except ImportError:
    raise RuntimeError('Unable to set custom CXX flags')