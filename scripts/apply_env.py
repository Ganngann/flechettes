import os

try:
    Import("env")
except ImportError:
    pass

def parse_mac(mac_str):
    # Converts AA:BB:CC:DD:EE:FF to {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
    if not mac_str:
        return None

    # Remove separators
    clean_mac = mac_str.replace(":", "").replace("-", "").replace(" ", "")

    if len(clean_mac) != 12:
        print(f"Warning: Invalid MAC address format: {mac_str}")
        return None

    bytes_list = []
    for i in range(0, 12, 2):
        byte_val = "0x" + clean_mac[i:i+2]
        bytes_list.append(byte_val)

    return "{" + ", ".join(bytes_list) + "}"

# Define path to .env file (root of repo)
# env.subst("$PROJECT_DIR") gives projects/bar, so we go up two levels
project_dir = env.subst("$PROJECT_DIR")
root_dir = os.path.abspath(os.path.join(project_dir, "..", ".."))
env_file = os.path.join(root_dir, ".env")

config = {}

# Load .env file
if os.path.isfile(env_file):
    print(f"Loading configuration from {env_file}")
    with open(env_file, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if "=" in line:
                key, value = line.split("=", 1)
                config[key.strip()] = value.strip()
else:
    print(f"No .env file found at {env_file}. Using defaults.")

# Determine current project
# Check the last component of the path to be safe
path_parts = os.path.normpath(project_dir).split(os.sep)
if "bar" in path_parts:
    is_bar = True
    is_flechettes = False
elif "flechettes" in path_parts:
    is_bar = False
    is_flechettes = True
else:
    # Fallback or shared lib testing?
    is_bar = False
    is_flechettes = False

# Apply configurations
if is_bar:
    print("Configuring environment for BAR project")

    # Ports
    port = config.get("BAR_UPLOAD_PORT")
    if port:
        print(f"Setting Upload/Monitor Port to {port}")
        env.Replace(UPLOAD_PORT=port)
        env.Replace(MONITOR_PORT=port)

    # Peer MAC (The Bar talks to the Flechettes machine)
    target_mac = config.get("FLECHETTES_MAC_ADDRESS")
    if target_mac:
        c_mac = parse_mac(target_mac)
        if c_mac:
            print(f"Setting Peer MAC Address to {target_mac}")
            env.Append(CPPDEFINES=[("PEER_MAC_ADDRESS", c_mac)])

elif is_flechettes:
    print("Configuring environment for FLECHETTES project")

    # Ports
    port = config.get("FLECHETTES_UPLOAD_PORT")
    if port:
        print(f"Setting Upload/Monitor Port to {port}")
        env.Replace(UPLOAD_PORT=port)
        env.Replace(MONITOR_PORT=port)

    # Peer MAC (The Flechettes machine talks to the Bar)
    target_mac = config.get("BAR_MAC_ADDRESS")
    if target_mac:
        c_mac = parse_mac(target_mac)
        if c_mac:
            print(f"Setting Peer MAC Address to {target_mac}")
            env.Append(CPPDEFINES=[("PEER_MAC_ADDRESS", c_mac)])
