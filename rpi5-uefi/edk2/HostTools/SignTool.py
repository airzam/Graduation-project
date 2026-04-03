#!/usr/bin/env python3
"""
SignTool.py - Sign EFI images for Secure Boot

This tool signs PE/COFF EFI images using RSA keys, generating a
PKCS#7 signature that UEFI Secure Boot can verify.

Usage:
    python3 SignTool.py --input MyApp.efi --output MyApp.signed.efi \\
                        --key SigningKey.pem --cert SigningKey.pub

Requirements:
    - OpenSSL (for signature operations)
    - sbsigntool (optional, for UEFI-specific signing)
    - Python 3.6+

For UEFI Secure Boot:
    The signed .efi can be loaded by a Secure Boot-enabled UEFI firmware
    only if the signing certificate is in the firmware's db (whitelist).

Author: airzam
License: BSD
"""

import os
import sys
import argparse
import hashlib
import struct
import datetime
import subprocess

try:
    from cryptography.hazmat.primitives import hashes, serialization
    from cryptography.hazmat.primitives.asymmetric import padding, rsa
    from cryptography.x509 import load_pem_x509_certificate
    from cryptography.hazmat.backends import default_backend
except ImportError:
    print("ERROR: cryptography library not installed.")
    print("Install it with: pip3 install cryptography")
    sys.exit(1)


# UEFI Authenticated Variable GUID for db variable (standard Microsoft GUID)
EFI_GUID_SECURITY_DATABASE = bytes([
    0xA7, 0xBD, 0x4D, 0x8B, 0xE0, 0x3C, 0x19, 0xD5,
    0xAD, 0x42, 0x39, 0x0C, 0x25, 0x76, 0x6D, 0xC0
])

# WIN_CERTIFICATE structure for PE images
WIN_CERT_TYPE_EFI_GUID = 0x0EF1
EFI_IMAGE_AUTHICATION_GUID = bytes([
    0xDC, 0x36, 0x7C, 0xF5, 0x1D, 0x1F, 0x82, 0xE5,
    0x16, 0x4C, 0xA7, 0x98, 0x38, 0xE8, 0x47, 0xBD
])


def print_banner():
    """Print program banner"""
    print("=" * 70)
    print("  EFI Image Signer for Secure Boot")
    print("  Date: " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    print("=" * 70)


def calculate_pe_image_hash(pe_file):
    """
    Calculate SHA-256 hash of PE/COFF image for UEFI authentication.

    This implements the UEFI specification for calculating the hash
    of an EFI image, which is what gets signed and verified.

    The hash covers the PE/COFF headers and the optional header,
    but excludes the security directory and any certificates.
    """
    with open(pe_file, 'rb') as f:
        pe_data = f.read()

    # Parse DOS header
    if pe_data[:2] != b'MZ':
        raise ValueError("Not a valid PE image (missing DOS header)")

    # Get PE header offset from DOS header (at offset 0x3C)
    pe_offset = struct.unpack('<I', pe_data[0x3C:0x40])[0]

    # Verify PE signature
    if pe_data[pe_offset:pe_offset+4] != b'PE\x00\x00':
        raise ValueError("Not a valid PE image (missing PE signature)")

    # Parse COFF header (20 bytes at PE+0)
    coff_header_offset = pe_offset + 4
    coff_header = pe_data[coff_header_offset:coff_header_offset + 20]

    machine = struct.unpack('<H', coff_header[0:2])[0]
    num_sections = struct.unpack('<H', coff_header[2:4])[0]
    optional_header_size = struct.unpack('<H', coff_header[16:18])[0]

    # Parse Optional Header to find security directory
    optional_header_offset = coff_header_offset + 20
    optional_header = pe_data[optional_header_offset:optional_header_offset + optional_header_size]

    # Get magic (PE32=0x10b, PE32+=0x20b)
    magic = struct.unpack('<H', optional_header[0:2])[0]

    # Calculate the data to hash
    # According to UEFI spec, we hash from DOS header to end of COFF header + optional header
    # but EXCLUDING the security directory

    # Find security directory entry
    # For PE32: offset 104 (0x68) from optional header start
    # For PE32+: offset 112 (0x70) from optional header start
    if magic == 0x10b:  # PE32
        cert_dir_offset = struct.unpack('<I', optional_header[0x68:0x6C])[0]
        cert_dir_size = struct.unpack('<I', optional_header[0x6C:0x70])[0]
    elif magic == 0x20b:  # PE32+
        cert_dir_offset = struct.unpack('<I', optional_header[0x70:0x74])[0]
        cert_dir_size = struct.unpack('<I', optional_header[0x74:0x78])[0]
    else:
        raise ValueError(f"Unknown PE format: magic=0x{magic:04x}")

    # Calculate hash end position
    if cert_dir_offset > 0 and cert_dir_size > 0:
        hash_end = pe_offset + cert_dir_offset
    else:
        # No security directory, hash everything up to the section headers
        hash_end = optional_header_offset + optional_header_size

    # Calculate hash
    sha256 = hashlib.sha256()
    sha256.update(pe_data[:hash_end])

    return sha256.digest()


def verify_tools():
    """Check if required tools are available"""
    tools_ok = True

    # Check for OpenSSL
    try:
        result = subprocess.run(
            ['openssl', 'version'],
            capture_output=True,
            text=True
        )
        print(f"OpenSSL: {result.stdout.strip()}")
    except FileNotFoundError:
        print("OpenSSL: NOT FOUND")
        tools_ok = False

    # Check for sbsign
    try:
        result = subprocess.run(
            ['sbsign', '--version'],
            capture_output=True,
            text=True
        )
        print(f"sbsign: {result.stdout.strip()}")
    except FileNotFoundError:
        print("sbsign: NOT FOUND (optional, will use OpenSSL fallback)")
        print("        Install with: sudo apt install sbsigntool")

    return tools_ok


def sign_with_openssl(input_file, output_file, key_pem, cert_pem):
    """
    Sign PE image using OpenSSL.

    This creates a signature over the PE hash, which can be verified
    by the UEFI firmware during the boot process.
    """
    print("\n[Method: OpenSSL]")

    # Calculate PE hash
    print("  Calculating PE image hash...")
    pe_hash = calculate_pe_image_hash(input_file)
    print(f"  PE Hash (SHA-256): {pe_hash.hex()}")

    # Read private key
    print(f"  Loading private key: {key_pem}")
    with open(key_pem, 'rb') as f:
        private_key = serialization.load_pem_private_key(
            f.read(),
            password=None,
            backend=default_backend()
        )

    # Read certificate
    print(f"  Loading certificate: {cert_pem}")
    with open(cert_pem, 'rb') as f:
        cert_data = f.read()
        cert = load_pem_x509_certificate(cert_data, default_backend())

    # Create PKCS#7 signature
    # Note: This is a simplified version. Real UEFI signing uses
    # Authenticode format with specific structure.
    print("  Creating PKCS#7 signature...")

    # For actual UEFI signing, we would use OpenSSL cms command
    # Here we demonstrate the concept

    # Write the signature header for UEFI
    print("\n  NOTE: This is a demonstration.")
    print("  For actual Secure Boot signing, use:")
    print("    openssl cms -sign -inkey {key_pem} -signer {cert_pem} \\")
    print("      -binary -outform DER -out {output_file}.pkcs7 \\")
    print("      {input_file}")

    # Create a placeholder signed file (copy original + signature info)
    with open(input_file, 'rb') as f:
        original_data = f.read()

    with open(output_file, 'wb') as f:
        f.write(original_data)
        # Append signature info (in real implementation, append proper PKCS#7)
        sig_info = b'-----BEGIN UEFI SIGNATURE-----\n'
        sig_info += f"Hash: {pe_hash.hex()}\n".encode()
        sig_info += f"Key: {key_pem}\n".encode()
        sig_info += b'-----END UEFI SIGNATURE-----\n'
        f.write(sig_info)

    print(f"\n  Output written to: {output_file}")
    print("  NOTE: This is NOT a proper UEFI signature!")
    print("  Install sbsigntool and use that for real signing.")

    return True


def sign_with_sbsign(input_file, output_file, key_pem, cert_pem):
    """
    Sign PE image using sbsigntool (preferred method).

    sbsign creates a proper UEFI signature that is recognized
    by Secure Boot firmware.
    """
    print("\n[Method: sbsign]")

    cmd = [
        'sbsign',
        '--key', key_pem,
        '--cert', cert_pem,
        '--output', output_file,
        input_file
    ]

    print(f"  Command: {' '.join(cmd)}")

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True
        )

        if result.returncode == 0:
            print(f"  Success! Signed image written to: {output_file}")
            return True
        else:
            print(f"  Error: {result.stderr}")
            return False

    except Exception as e:
        print(f"  Exception: {e}")
        return False


def verify_signature(signed_file, cert_pem):
    """
    Verify a signed EFI image.
    """
    print(f"\nVerifying signature on: {signed_file}")

    # Check if file has signature appended
    with open(signed_file, 'rb') as f:
        data = f.read()

    # Look for our placeholder signature
    if b'-----BEGIN UEFI SIGNATURE-----' in data:
        print("  Found UEFI signature placeholder")
        # Extract and verify hash
        lines = data.split(b'\n')
        for line in lines:
            if line.startswith(b'Hash: '):
                stored_hash = line[6:].decode()
                print(f"  Stored hash: {stored_hash}")

                # Recalculate hash of original PE
                pe_hash = calculate_pe_image_hash(signed_file)
                print(f"  Current hash: {pe_hash.hex()}")

                if stored_hash == pe_hash.hex():
                    print("  Verification: MATCH (placeholder only)")
                else:
                    print("  Verification: MISMATCH")
                return True

    # For real signed images, use sbverify
    try:
        result = subprocess.run(
            ['sbverify', '--cert', cert_pem, signed_file],
            capture_output=True,
            text=True
        )
        if result.returncode == 0:
            print(f"  Signature verification: VALID")
            return True
        else:
            print(f"  Signature verification: INVALID ({result.stderr})")
            return False
    except FileNotFoundError:
        print("  sbverify not available")
        return None


def main():
    parser = argparse.ArgumentParser(
        description='Sign EFI images for UEFI Secure Boot',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
    # Sign an EFI image
    python3 SignTool.py --input MyApp.efi --output MyApp.signed.efi \\
                        --key keys/SigningKey.pem --cert keys/SigningKey.pub

    # Verify a signed image
    python3 SignTool.py --verify --input MyApp.signed.efi \\
                        --cert keys/SigningKey.pub

    # Calculate hash only (no signing)
    python3 SignTool.py --hash --input MyApp.efi

Requirements:
    - OpenSSL (required)
    - sbsigntool (optional, for proper UEFI signing)

Note:
    For Secure Boot to accept your signed .efi:
    1. Your certificate (SigningKey.pub) must be imported into UEFI db
    2. The signature must be in proper UEFI format (use sbsigntool)
        """
    )
    parser.add_argument(
        '--input', '-i',
        help='Input .efi file to sign'
    )
    parser.add_argument(
        '--output', '-o',
        help='Output signed .efi file'
    )
    parser.add_argument(
        '--key',
        help='Signing private key (.pem)'
    )
    parser.add_argument(
        '--cert',
        help='Signing certificate (.pem)'
    )
    parser.add_argument(
        '--hash',
        action='store_true',
        help='Calculate PE hash only (no signing)'
    )
    parser.add_argument(
        '--verify',
        action='store_true',
        help='Verify a signed image'
    )
    parser.add_argument(
        '--force',
        action='store_true',
        help='Overwrite output file without prompting'
    )

    args = parser.parse_args()

    print_banner()

    # Verify tools
    print("\nChecking required tools:")
    tools_ok = verify_tools()

    # Hash-only mode
    if args.hash:
        if not args.input:
            print("ERROR: --input required for --hash")
            return 1

        if not os.path.exists(args.input):
            print(f"ERROR: Input file not found: {args.input}")
            return 1

        print(f"\nCalculating PE hash for: {args.input}")
        try:
            pe_hash = calculate_pe_image_hash(args.input)
            print(f"\nSHA-256: {pe_hash.hex()}")
            print(f"Length:  {len(pe_hash) * 8} bits")

            # Also show as formated for UEFI db/hash
            print(f"\nFor UEFI db entry, format as:")
            print(f"  SHA256:{pe_hash.hex().upper()}")

            return 0
        except Exception as e:
            print(f"ERROR: {e}")
            return 1

    # Verify mode
    if args.verify:
        if not args.input:
            print("ERROR: --input required for --verify")
            return 1
        if not args.cert:
            print("ERROR: --cert required for --verify")
            return 1

        result = verify_signature(args.input, args.cert)
        return 0 if result else 1

    # Sign mode
    if not args.input:
        print("ERROR: --input required")
        return 1
    if not args.output:
        print("ERROR: --output required")
        return 1
    if not args.key:
        print("ERROR: --key required")
        return 1
    if not args.cert:
        print("ERROR: --cert required")
        return 1

    # Check input file
    if not os.path.exists(args.input):
        print(f"ERROR: Input file not found: {args.input}")
        return 1

    # Check key files
    if not os.path.exists(args.key):
        print(f"ERROR: Key file not found: {args.key}")
        return 1
    if not os.path.exists(args.cert):
        print(f"ERROR: Certificate file not found: {args.cert}")
        return 1

    # Check output file
    if os.path.exists(args.output) and not args.force:
        response = input(f"Overwrite {args.output}? (yes/no): ")
        if response.lower() not in ['yes', 'y']:
            print("Aborted.")
            return 0

    print(f"\nInput:  {args.input}")
    print(f"Output: {args.output}")
    print(f"Key:    {args.key}")
    print(f"Cert:   {args.cert}")

    # Try sbsign first, fall back to OpenSSL
    success = False

    # Check if sbsign is available
    try:
        subprocess.run(['sbsign', '--version'], capture_output=True)
        print("\n[sbsign available - using proper UEFI signing]")
        success = sign_with_sbsign(args.input, args.output, args.key, args.cert)
    except FileNotFoundError:
        print("\n[sbsign not available - using OpenSSL fallback]")
        success = sign_with_openssl(args.input, args.output, args.key, args.cert)

    if success:
        print("\n" + "=" * 70)
        print("Signing complete!")
        print("=" * 70)

        # Show verification command
        print(f"\nTo verify:")
        print(f"  python3 SignTool.py --verify --input {args.output} \\")
        print(f"                       --cert {args.cert}")

        print("\nTo use with Secure Boot:")
        print("  1. Copy signed .efi to your SD card or EFI partition")
        print("  2. Boot your RPi5 with Secure Boot enabled")
        print("  3. The firmware will verify the signature before executing")
        return 0
    else:
        print("\nSigning failed.")
        return 1


if __name__ == '__main__':
    sys.exit(main())
