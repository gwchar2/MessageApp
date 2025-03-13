import inspect

# I hate python....I lost motivation making my owner logger class....
# I will survive with print >:(

def format_hex(data: bytes, group_size: int = 4) -> str:
    try:
        hex_str = data.hex()  
        formatted = " ".join(hex_str[i:i + group_size * 2] for i in range(0, len(hex_str), group_size * 2))
    except AttributeError as e:
        print(f"Error: {e}")
        return;
    return formatted.upper()

