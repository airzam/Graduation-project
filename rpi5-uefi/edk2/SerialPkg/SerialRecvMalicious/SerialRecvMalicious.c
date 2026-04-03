/** @file
    SerialRecvMalicious - Malicious version demonstration (HARMLESS)

    This is a SIMULATED attack for demonstration purposes only.
    It shows what a real malware could do if Secure Boot is not enabled.

    Copyright (c) 2026, airzam. All rights reserved.<BR>

    THE PROGRAM IS DISTRIBUTED UNDER BSD LICENSE ON AN "AS IS" BASIS.
    FOR EDUCATIONAL DEMONSTRATION ONLY - NO ACTUAL MALICIOUS FUNCTIONALITY.
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>

// Serial I/O Protocol structures (same as SerialRecv.c)
typedef struct _EFI_SERIAL_IO_PROTOCOL EFI_SERIAL_IO_PROTOCOL;

typedef struct {
  UINT32    ControlMask;
  UINT32    Timeout;
  UINT64    BaudRate;
  UINT32    ReceiveFifoDepth;
  UINT32    DataBits;
  UINT32    Parity;
  UINT32    StopBits;
} EFI_SERIAL_IO_MODE;

typedef
EFI_STATUS
(EFIAPI *EFI_SERIAL_RESET)(
  IN EFI_SERIAL_IO_PROTOCOL *This
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SERIAL_SET_ATTRIBUTES)(
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN UINT64 BaudRate,
  IN UINT32 ReceiveFifoDepth,
  IN UINT32 Timeout,
  IN UINT8 Parity,
  IN UINT8 DataBits,
  IN UINT8 StopBits
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SERIAL_SET_CONTROL_BITS)(
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN UINT32 Control
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SERIAL_GET_CONTROL_BITS)(
  IN EFI_SERIAL_IO_PROTOCOL *This,
  OUT UINT32 *Control
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SERIAL_WRITE)(
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN OUT UINTN *BufferSize,
  IN VOID *Buffer
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SERIAL_READ)(
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN OUT UINTN *BufferSize,
  OUT VOID *Buffer
  );

struct _EFI_SERIAL_IO_PROTOCOL {
  UINT32                         Revision;
  EFI_SERIAL_RESET               Reset;
  EFI_SERIAL_SET_ATTRIBUTES      SetAttributes;
  EFI_SERIAL_SET_CONTROL_BITS    SetControl;
  EFI_SERIAL_GET_CONTROL_BITS    GetControl;
  EFI_SERIAL_WRITE               Write;
  EFI_SERIAL_READ                Read;
  EFI_SERIAL_IO_MODE             *Mode;
};

// Serial I/O Protocol GUID - hardcoded
EFI_GUID mSerialIoGuid = { 0xBB25CF6F, 0xF1D4, 0x11D2, { 0x9A, 0x0C, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0xFD }};

// Screen dimensions
UINTN mScreenWidth = 1024;
UINTN mScreenHeight = 600;

// Find all handles with Serial I/O protocol

// Find all handles with Serial I/O protocol
UINTN FindAllSerialPorts(EFI_SERIAL_IO_PROTOCOL **ports, UINTN maxPorts)
{
    EFI_STATUS Status;
    UINTN handleCount = 0;
    EFI_HANDLE *handles = NULL;
    UINTN i;
    UINTN portCount = 0;

    Status = gBS->LocateHandleBuffer(
        AllHandles,
        NULL,
        NULL,
        &handleCount,
        &handles
    );

    if (Status != EFI_SUCCESS || handleCount == 0) {
        return 0;
    }

    for (i = 0; i < handleCount && portCount < maxPorts; i++) {
        EFI_SERIAL_IO_PROTOCOL *serial = NULL;

        Status = gBS->HandleProtocol(
            handles[i],
            &mSerialIoGuid,
            (VOID**)&serial
        );

        if (Status == EFI_SUCCESS && serial != NULL) {
            ports[portCount++] = serial;
        }
    }

    gBS->FreePool(handles);
    return portCount;
}

// Send fake sensor data - SIMULATED, NOT REAL
VOID SendFakeData(EFI_SERIAL_IO_PROTOCOL *SerialIo)
{
    // These are FAKE values for demonstration only
    CHAR8 *fakeReadings[] = {
        "FAKE_SENSOR: Temp=9999.9C, Humidity=99%, Pressure=0hPa\r\n",
        "FAKE_SENSOR: CO2=9999ppm, PM2.5=9999ug/m3\r\n",
        "FAKE_SENSOR: GPS Lat=99.9999, Lon=999.9999, Alt=99999m\r\n",
        "FAKE_SENSOR: Battery=99.9V, Current=-9999A, Power=99999W\r\n"
    };

    UINTN i = 0;
    for (i = 0; i < 4; i++) {
        UINTN len = AsciiStrLen(fakeReadings[i]);
        UINTN sent = len;
        SerialIo->Write(SerialIo, &sent, fakeReadings[i]);
        gBS->Stall(500000);  // 0.5s delay between readings
    }
}

// Display compromise warning on screen
VOID DisplayWarning(IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
    EFI_STATUS Status;
    UINTN i;

    // Fill screen with red warning
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&Gop);
    if (Status == EFI_SUCCESS) {
        UINTN BufferSize = Gop->Mode->Info->HorizontalResolution *
                          Gop->Mode->Info->VerticalResolution * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
        EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer = NULL;

        Status = gBS->AllocatePool(EfiBootServicesData, BufferSize, (VOID**)&BltBuffer);
        if (Status == EFI_SUCCESS) {
            // Red background (BGRA format)
            for (i = 0; i < Gop->Mode->Info->HorizontalResolution * Gop->Mode->Info->VerticalResolution; i++) {
                BltBuffer[i].Blue = 0;
                BltBuffer[i].Green = 0;
                BltBuffer[i].Red = 255;
                BltBuffer[i].Reserved = 0;
            }
            Gop->Blt(Gop, BltBuffer, EfiBltVideoFill, 0, 0, 0, 0,
                    Gop->Mode->Info->HorizontalResolution,
                    Gop->Mode->Info->VerticalResolution, 0);
            gBS->FreePool(BltBuffer);
        }
    }

    Print(L"\n");
    Print(L"================================================================================\n");
    Print(L"\n");
    Print(L"  !!!  ATTENTION: SYSTEM COMPROMISED!  !!!\n");
    Print(L"\n");
    Print(L"================================================================================\n");
    Print(L"\n");
    Print(L"  This EFI application has been REPLACED by a malicious version!\n");
    Print(L"\n");
    Print(L"  What happened?\n");
    Print(L"  ---------------");
    Print(L"  1. Attacker modified SerialRecv.efi on GitHub/SD card\n");
    Print(L"  2. You unknowingly ran this TAMPERED version\n");
    Print(L"  3. Attacker now controls your RPi5!\n");
    Print(L"\n");
    Print(L"  What could this malware do (if it were real)?\n");
    Print(L"  ----------------------------------------------");
    Print(L"  - Send FAKE sensor data to your PC (already happening below)\n");
    Print(L"  - Exfiltrate sensitive data via serial port\n");
    Print(L"  - Establish a backdoor for remote access\n");
    Print(L"  - Modify system configuration or brick your device\n");
    Print(L"\n");
    Print(L"  How to prevent this?\n");
    Print(L"  ----------------------");
    Print(L"  - Enable UEFI Secure Boot (SECURE_BOOT_ENABLE = TRUE)\n");
    Print(L"  - Sign your .efi files with a private key\n");
    Print(L"  - Import your public key into UEFI db (whitelist)\n");
    Print(L"  - Unsigned/ tampered .efi will be BLOCKED at boot time\n");
    Print(L"\n");
    Print(L"================================================================================\n");
    Print(L"\n");

    // Wait for user to read
    gBS->Stall(3000000);  // 3 seconds
}

// Log fake compromise evidence - SIMULATED, NOT REAL
VOID SimulateLogCompromise(IN EFI_SYSTEM_TABLE *SystemTable)
{
    Print(L"\n");
    Print(L"[MALICIOUS] Simulating attack artifacts...\n");
    Print(L"\n");
    Print(L"  [FAKE] /var/log/malware.log (written by attacker):\n");
    Print(L"  --------\n");
    Print(L"  [2026-04-03 10:30:15] Malware loaded successfully\n");
    Print(L"  [2026-04-03 10:30:16] Serial port access: GRANTED\n");
    Print(L"  [2026-04-03 10:30:17] Sending fake telemetry data...\n");
    Print(L"  [2026-04-03 10:30:18] Data exfiltration: PENDING\n");
    Print(L"  [2026-04-03 10:30:19] Backdoor listening on UART...\n");
    Print(L"\n");
    Print(L"  [FAKE] /etc/passwd.modified:\n");
    Print(L"  --------\n");
    Print(L"  root:x:0:0:root:/root:/bin/bash\n");
    Print(L"  attacker:x:1000:1000:attacker:/home/attacker:/bin/sh\n");
    Print(L"\n");
    Print(L"  (These are FAKE logs for demonstration only!)\n");
    Print(L"\n");
}

EFI_STATUS EFIAPI UefiMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_SERIAL_IO_PROTOCOL *serialPorts[8];
    UINTN portCount;
    UINTN i;
    EFI_INPUT_KEY Key;
    UINT64 tick = 0;
    UINT8 fakeCounter = 0;

    // Display the scary warning
    DisplayWarning(SystemTable);

    Print(L"\n");
    Print(L"===========================================\n");
    Print(L"  MALICIOUS Demo v1.0 (HARMLESS)\n");
    Print(L"===========================================\n");
    Print(L"\n");

    // Find all serial ports
    portCount = FindAllSerialPorts(serialPorts, 8);

    if (portCount == 0) {
        Print(L"[!] No serial ports found (that's OK for demo)\n");
    } else {
        Print(L"[+] Found %d serial port(s)\n", portCount);
        Print(L"[+] Starting fake data transmission...\n");
        Print(L"\n");
    }

    // Main "attack" simulation loop
    while (1) {
        // Check for ESC key (to exit demo)
        if (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key) == EFI_SUCCESS) {
            if (Key.ScanCode == SCAN_ESC) {
                break;
            }
        }

        // Every 3 seconds, send fake data through all ports
        tick++;
        if (tick >= 3000) {
            tick = 0;
            fakeCounter++;

            if (portCount > 0) {
                for (i = 0; i < portCount; i++) {
                    EFI_SERIAL_IO_PROTOCOL *serial = serialPorts[i];

                    // Configure port
                    serial->SetAttributes(serial, 115200, 0, 1000000, 0, 8, 0);
                    serial->Reset(serial);

                    // Send fake telemetry header (fixed string)
                    CHAR8 header[] = "\r\n=== FAKE DATA ===\r\n";
                    UINTN len = AsciiStrLen(header);
                    UINTN sent = len;
                    serial->Write(serial, &sent, header);

                    // Send fake sensor readings
                    SendFakeData(serial);

                    Print(L"[MALICIOUS] Sent fake data batch #%d via port %d\n", fakeCounter, i);
                }
            } else {
                // No serial port - just show fake output on screen
                Print(L"[MALICIOUS] Fake data batch #%d (no serial port)\n", fakeCounter);
                Print(L"  FAKE_SENSOR: Temp=99.9C, Humidity=50%%\n");
            }
        }

        gBS->Stall(1000);
    }

    // Show simulated log
    SimulateLogCompromise(SystemTable);

    Print(L"\n");
    Print(L"===========================================\n");
    Print(L"  DEMO COMPLETE\n");
    Print(L"===========================================\n");
    Print(L"\n");
    Print(L"  Remember: This was a SIMULATION!\n");
    Print(L"  Enable Secure Boot to protect yourself.\n");
    Print(L"\n");
    Print(L"  Press any key to exit...\n");
    Print(L"\n");

    // Wait for key before exit
    SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
    while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key) == EFI_NOT_READY) {
        gBS->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, 0);
    }

    return EFI_SUCCESS;
}
