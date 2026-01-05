import argparse

def generate_header(hex_string, output_file, varname):
    hex_string = hex_string.replace(" ", "").replace("\n", "")

    byte_array = [f"0x{hex_string[i:i+2]}" for i in range(0, len(hex_string), 2)]
    byte_array_str = ",\n".join(byte_array)

    header_content = f"""
unsigned char {varname}[] = {{
        {byte_array_str}
}};
unsigned int {varname}_len = {len(byte_array)};
"""

    with open(output_file, 'w') as f:
        f.write(header_content)

    print(f"Key file generated: {output_file}")

def main():
    parser = argparse.ArgumentParser(description="Generate a C header file from a hex string")
    parser.add_argument('hex_string', type=str, help="The hex string to convert into a C header")
    parser.add_argument('-o', '--output', type=str, default="pub.h", help="Output file name (default: pub.h)")
    parser.add_argument('-vn', '--varname', type=str, default="pub_key_der", help="Name for variable")

    args = parser.parse_args()

    generate_header(args.hex_string, args.output, args.varname)

if __name__ == "__main__":
    main()
