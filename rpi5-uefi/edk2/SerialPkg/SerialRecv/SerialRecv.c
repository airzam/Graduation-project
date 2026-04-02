/** @file
    SerialRecv - Comprehensive serial loopback test

    Copyright (c) 2026, airzam. All rights reserved.<BR>

    THE PROGRAM IS DISTRIBUTED UNDER BSD LICENSE ON AN "AS IS" BASIS.
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>

// Serial I/O Protocol structures
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

// Initialize graphics - try 1024x600
EFI_STATUS InitGraphics()
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
    EFI_STATUS Status;
    UINT32 mode;
    UINTN sizeOfInfo;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;

    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&Gop);
    if (Status != EFI_SUCCESS) {
        Print(L"GOP not found\n");
        return Status;
    }

    Print(L"Current mode: %d x %d\n",
          Gop->Mode->Info->HorizontalResolution,
          Gop->Mode->Info->VerticalResolution);

    // Find 1024x600 mode
    for (mode = 0; mode < Gop->Mode->MaxMode; mode++) {
        Status = Gop->QueryMode(Gop, mode, &sizeOfInfo, &info);
        if (Status == EFI_SUCCESS) {
            if (info->HorizontalResolution == 1024 && info->VerticalResolution == 600) {
                Status = Gop->SetMode(Gop, mode);
                if (Status == EFI_SUCCESS) {
                    mScreenWidth = 1024;
                    mScreenHeight = 600;
                    Print(L"Set to 1024x600\n");
                    return EFI_SUCCESS;
                }
            }
        }
    }

    Print(L"1024x600 not found, keeping current\n");
    mScreenWidth = Gop->Mode->Info->HorizontalResolution;
    mScreenHeight = Gop->Mode->Info->VerticalResolution;
    return EFI_SUCCESS;
}

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

// Test a single serial port
EFI_STATUS TestSerialPort(EFI_SERIAL_IO_PROTOCOL *SerialIo, UINTN index)
{
    EFI_STATUS Status;
    UINT32 Control;
    UINT8 testByte = 'X';
    UINTN BytesWritten = 1;
    UINTN BytesRead = 1;

    Print(L"\n[%d] ", index);
    Print(L"Baud=%d, Bits=%d, Stop=%d, Parity=%d\n",
          SerialIo->Mode->BaudRate, SerialIo->Mode->DataBits,
          SerialIo->Mode->StopBits, SerialIo->Mode->Parity);

    // Configure 115200, 8N1
    Status = SerialIo->SetAttributes(SerialIo, 115200, 0, 1000000, 0, 8, 0);
    if (Status != EFI_SUCCESS) {
        Print(L"    SetAttr failed: %r\n", Status);
    }

    // Reset
    SerialIo->Reset(SerialIo);

    // Check control signals
    Status = SerialIo->GetControl(SerialIo, &Control);
    Print(L"    Control: 0x%X", Control);
    if (Control & 0x10) Print(L" [Input empty]");
    if (Control & 0x20) Print(L" [DSR]");
    if (Control & 0x80) Print(L" [DCD]");
    Print(L"\n");

    // Try to write test byte
    Status = SerialIo->Write(SerialIo, &BytesWritten, &testByte);
    if (Status == EFI_SUCCESS && BytesWritten == 1) {
        Print(L"    Write OK\n");
    } else {
        Print(L"    Write: %r\n", Status);
    }

    // Wait for potential loopback
    gBS->Stall(10000);

    // Check if anything came back
    Status = SerialIo->Read(SerialIo, &BytesRead, &testByte);
    if (Status == EFI_SUCCESS && BytesRead == 1) {
        Print(L"    LOOPBACK DETECTED! Got: '%c'\n", testByte);
        return EFI_SUCCESS;
    }

    Print(L"    No loopback (normal for GPIO)\n");
    return EFI_NOT_READY;
}

EFI_STATUS EFIAPI UefiMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_SERIAL_IO_PROTOCOL *serialPorts[8];
    UINTN portCount;
    UINTN i;
    UINT8 Buffer[256];
    UINTN BytesRead, BytesWritten;
    EFI_INPUT_KEY Key;
    EFI_STATUS Status;

    // Initialize graphics
    InitGraphics();

    Print(L"");
    Print(L"===========================================");
    Print(L"");
    Print(L"  Serial Loopback Test v2");
    Print(L"  Screen: %d x %d", mScreenWidth, mScreenHeight);
    Print(L"");
    Print(L"===========================================");
    Print(L"");

    // Find all serial ports
    portCount = FindAllSerialPorts(serialPorts, 8);

    if (portCount == 0) {
        Print(L"");
        Print(L"ERROR: No Serial I/O ports found!");
        Print(L"");
        Print(L"Press any key to exit...");
        Print(L"");
        gST->ConIn->Reset(gST->ConIn, FALSE);
        while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY) {
            gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, 0);
        }
        return EFI_NOT_READY;
    }

    Print(L"Found %d Serial Port(s)", portCount);

    // Test all ports
    for (i = 0; i < portCount; i++) {
        TestSerialPort(serialPorts[i], i);
    }

    Print(L"");
    Print(L"===========================================");
    Print(L"Instructions:");
    Print(L"1. Connect PC to GPIO (115200,8N1)");
    Print(L"2. Data will auto-send every 5s");
    Print(L"3. Send data from PC to echo back");
    Print(L"4. Press ESC on keyboard to exit");
    Print(L"===========================================");
    Print(L"");

    // Main loop - read from ALL ports
    UINT64 tick = 0;
    while (1) {
        // Check for ESC key
        if (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_SUCCESS) {
            if (Key.ScanCode == SCAN_ESC) {
                break;
            }
        }

        // Check each serial port for data
        for (i = 0; i < portCount; i++) {
            EFI_SERIAL_IO_PROTOCOL *serial = serialPorts[i];
            UINT32 Control;

            // Check if data available
            serial->GetControl(serial, &Control);
            if ((Control & 0x10) == 0) {  // Not empty
                BytesRead = sizeof(Buffer);
                Status = serial->Read(serial, &BytesRead, Buffer);

                if (Status == EFI_SUCCESS && BytesRead > 0) {
                    Buffer[BytesRead] = '\0';
                    Print(L"[%d] RX(%d): %s", i, BytesRead, Buffer);

                    // Echo back
                    BytesWritten = BytesRead;
                    serial->Write(serial, &BytesWritten, Buffer);
                    Print(L"[%d] TX(%d): %s", i, BytesWritten, Buffer);
                }
            }
        }

        gBS->Stall(1000);

        // Auto-send test data every 5 seconds
        tick++;
        if (tick >= 5000) {
            tick = 0;
            for (i = 0; i < portCount; i++) {
                EFI_SERIAL_IO_PROTOCOL *serial = serialPorts[i];
                CHAR8 msg[] = "Hello from RPi5! (Auto test)\r\n";
                UINTN len = sizeof(msg) - 1;
                UINTN sent = len;
                EFI_STATUS st = serial->Write(serial, &sent, msg);
                if (st == EFI_SUCCESS) {
                    Print(L"[%d] AUTO TX: Hello from RPi5!\n", i);
                }
            }
        }
    }

    Print(L"");
    Print(L"Done.");
    Print(L"");
    Print(L"Press any key to exit...");
    Print(L"");
    gST->ConIn->Reset(gST->ConIn, FALSE);
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY) {
        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, 0);
    }

    return EFI_SUCCESS;
}
