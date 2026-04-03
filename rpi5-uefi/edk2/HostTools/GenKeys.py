#!/usr/bin/env python3
"""
GenKeys.py - Generate RSA key pairs for EFI Secure Boot

This script generates the cryptographic keys needed for UEFI Secure Boot:
- Platform Key (PK): Highest authority, used to sign KEK
- Key Exchange Key (KEK): Used to sign db/dbx updates
- Signing Key: Used to sign EFI binaries (.efi files)

Usage:
    python3 GenKeys.py [--key-dir ./keys] [--key-size 2048]

Generated files:
    - PlatformKey.pem      (PK private key - KEEP SECRET!)
    - PlatformKey.pub      (PK public key)
    - KEKKey.pem           (KEK private key - KEEP SECRET!)
    - KEKKey.pub           (KEK public key)
    - SigningKey.pem       (Signing private key - KEEP SECRET!)
    - SigningKey.pub       (Signing public key, import to UEFI db)

Requirements:
    pip3 install cryptography

WARNING: Keep all private keys (.pem) SECRET! If someone steals your private
         key, they can sign malicious EFI binaries that will run on your system.

Author: airzam
License: BSD
"""

import os
import sys
import argparse
import datetime

try:
    from cryptography.hazmat.primitives.asymmetric import rsa
    from cryptography.hazmat.primitives import serialization
    from cryptography.hazmat.backends import default_backend
except ImportError:
    print("ERROR: cryptography library not installed.")
    print("Install it with: pip3 install cryptography")
    sys.exit(1)


def generate_rsa_key(key_size=2048):
    """Generate RSA key pair"""
    print(f"    Generating RSA-{key_size} key pair...")
    private_key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=key_size,
        backend=default_backend()
    )
    return private_key


def save_private_key(private_key, filepath):
    """Save private key in PEM format"""
    pem = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.TraditionalOpenSSL,
        encryption_algorithm=serialization.NoEncryption()
    )
    with open(filepath, 'wb') as f:
        f.write(pem)
    os.chmod(filepath, 0o600)  # Restrict permissions


def save_public_key(private_key, filepath):
    """Save public key in PEM format"""
    public_key = private_key.public_key()
    pem = public_key.public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo
    )
    with open(filepath, 'wb') as f:
        f.write(pem)


def save_public_key_der(private_key, filepath):
    """Save public key in DER format (for UEFI db import)"""
    public_key = private_key.public_key()
    der = public_key.public_bytes(
        encoding=serialization.Encoding.DER,
        format=serialization.PublicFormat.SubjectPublicKeyInfo
    )
    with open(filepath, 'wb') as f:
        f.write(der)


def print_banner():
    """Print program banner"""
    print("=" * 70)
    print("  EFI Secure Boot Key Generator")
    print("  Generated for: RPi5 UEFI Secure Boot")
    print("  Date: " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    print("=" * 70)


def print_warning():
    """Print security warning"""
    print("""
    ╔══════════════════════════════════════════════════════════════════════╗
    ║                          ! WARNING !                                 ║
    ║══════════════════════════════════════════════════════════════════════║
    ║                                                                      ║
    ║  Your private keys are stored in .pem files.                        ║
    ║                                                                      ║
    ║  - NEVER share your private keys                                    ║
    ║  - NEVER commit them to git                                         ║
    ║  - Backup them to a SECURE location (encrypted USB, password manager)║
    ║                                                                      ║
    ║  If someone gets your signing key, they can sign malware that will   ║
    ║  run on your system with Secure Boot enabled!                       ║
    ║                                                                      ║
    ╚══════════════════════════════════════════════════════════════════════╝
    """)


def main():
    parser = argparse.ArgumentParser(
        description='Generate RSA keys for EFI Secure Boot',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
    python3 GenKeys.py                          # Generate keys in ./keys/
    python3 GenKeys.py --key-dir ~/secure-keys   # Custom directory
    python3 GenKeys.py --key-size 4096           # Stronger 4096-bit keys

Next steps after generation:
    1. Import SigningKey.pub (or SigningKey.der) into UEFI db
    2. Use SigningKey.pem to sign your .efi files with SignTool.py
    3. Store backup copies of all keys in a SECURE location
        """
    )
    parser.add_argument(
        '--key-dir',
        default='./keys',
        help='Directory to store keys (default: ./keys)'
    )
    parser.add_argument(
        '--key-size',
        type=int,
        default=2048,
        choices=[2048, 4096],
        help='RSA key size (default: 2048, recommended: 4096)'
    )
    parser.add_argument(
        '--force',
        action='store_true',
        help='Overwrite existing keys without prompting'
    )

    args = parser.parse_args()

    print_banner()

    # Create key directory
    key_dir = os.path.abspath(args.key_dir)
    os.makedirs(key_dir, exist_ok=True)
    print(f"\nKey directory: {key_dir}")

    # Check for existing keys
    existing_keys = []
    for name in ['PlatformKey', 'KEKKey', 'SigningKey']:
        pem_file = os.path.join(key_dir, f'{name}.pem')
        if os.path.exists(pem_file):
            existing_keys.append(name)

    if existing_keys and not args.force:
        print(f"\nWARNING: Found existing keys: {', '.join(existing_keys)}")
        response = input("Overwrite? (yes/no): ")
        if response.lower() not in ['yes', 'y']:
            print("Aborted.")
            sys.exit(0)

    print(f"\nKey size: RSA-{args.key_size}")

    # Generate Platform Key (PK)
    print("\n" + "-" * 70)
    print("[1/3] Generating Platform Key (PK)...")
    print("-" * 70)
    print("    PK is the highest authority in Secure Boot.")
    print("    It can sign KEK and enable/disable Secure Boot.")
    pk_private = generate_rsa_key(args.key_size)
    save_private_key(pk_private, os.path.join(key_dir, 'PlatformKey.pem'))
    save_public_key(pk_private, os.path.join(key_dir, 'PlatformKey.pub'))
    save_public_key_der(pk_private, os.path.join(key_dir, 'PlatformKey.der'))
    print("    Generated: PlatformKey.pem (PRIVATE - keep secret!)")
    print("    Generated: PlatformKey.pub")
    print("    Generated: PlatformKey.der (for UEFI import)")

    # Generate KEK
    print("\n" + "-" * 70)
    print("[2/3] Generating Key Exchange Key (KEK)...")
    print("-" * 70)
    print("    KEK can update the db and dbx signature databases.")
    kek_private = generate_rsa_key(args.key_size)
    save_private_key(kek_private, os.path.join(key_dir, 'KEKKey.pem'))
    save_public_key(kek_private, os.path.join(key_dir, 'KEKKey.pub'))
    save_public_key_der(kek_private, os.path.join(key_dir, 'KEKKey.der'))
    print("    Generated: KEKKey.pem (PRIVATE - keep secret!)")
    print("    Generated: KEKKey.pub")
    print("    Generated: KEKKey.der (for UEFI import)")

    # Generate Signing Key
    print("\n" + "-" * 70)
    print("[3/3] Generating Signing Key...")
    print("-" * 70)
    print("    This key signs your .efi applications.")
    print("    The public key goes into db (whitelist).")
    sign_private = generate_rsa_key(args.key_size)
    save_private_key(sign_private, os.path.join(key_dir, 'SigningKey.pem'))
    save_public_key(sign_private, os.path.join(key_dir, 'SigningKey.pub'))
    save_public_key_der(sign_private, os.path.join(key_dir, 'SigningKey.der'))
    print("    Generated: SigningKey.pem (PRIVATE - keep secret!)")
    print("    Generated: SigningKey.pub")
    print("    Generated: SigningKey.der (for UEFI db import)")

    # Summary
    print("\n" + "=" * 70)
    print("Key generation complete!")
    print("=" * 70)

    print("\nGenerated files:")
    print("-" * 70)
    for name in ['PlatformKey', 'KEKKey', 'SigningKey']:
        for ext in ['.pem', '.pub', '.der']:
            fpath = os.path.join(key_dir, name + ext)
            if os.path.exists(fpath):
                size = os.path.getsize(fpath)
                is_private = ext == '.pem'
                marker = " [PRIVATE]" if is_private else ""
                print(f"  {name}{ext:8} {size:6} bytes{marker}")

    print_warning()

    print("Next steps:")
    print("-" * 70)
    print("  1. Boot your RPi5 and enter UEFI Setup (press ESC during boot)")
    print("  2. Go to 'Secure Boot Configuration'")
    print("  3. Set 'Secure Boot Enable' = TRUE")
    print("  4. In 'PK/KEK/db Options', import the .der public keys:")
    print("     - KEKKey.der  → Key Exchange Key (KEK)")
    print("     - SigningKey.der → Signature Database (db)")
    print("  5. Save and exit")
    print("\n  To sign your .efi files:")
    print(f"     python3 SignTool.py --input MyApp.efi --output MyApp.signed.efi \\")
    print(f"                       --key {key_dir}/SigningKey.pem \\")
    print(f"                       --cert {key_dir}/SigningKey.pub")
    print()

    return 0


if __name__ == '__main__':
    sys.exit(main())
