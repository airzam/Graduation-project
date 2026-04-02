/** @file
    SerialRecv - Serial Port Loopback Test

    Copyright (c) 2026, airzam. All rights reserved.<BR>

    THE PROGRAM IS DISTRIBUTED UNDER BSD LICENSE ON AN "AS IS" BASIS.
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

// Serial loopback test
// PC sends characters, RPi receives and echoes back
// If PC receives echoed characters, serial connection is OK

EFI_STATUS EFIAPI UefiMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_INPUT_KEY Key;
    UINTN Count = 0;

    Print(L"\n");
    Print(L"===========================================\n");
    Print(L"  Serial Loopback Test\n");
    Print(L"  Author: airzam\n");
    Print(L"===========================================\n");
    Print(L"\n");

    Print(L"Instructions:\n");
    Print(L"1. Connect PC serial terminal to RPi5 UART\n");
    Print(L"2. Set baudrate: 115200, 8N1\n");
    Print(L"3. Type characters from PC keyboard\n");
    Print(L"4. You should see echoed characters below\n");
    Print(L"\n");

    Print(L"Press ESC to exit.\n");
    Print(L"\n");

    Print(L"--- Serial Data ---\n");

    // Clear input buffer
    gST->ConIn->Reset(gST->ConIn, FALSE);

    while (1) {
        // Wait for key (from serial or keyboard)
        EFI_STATUS Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

        if (Status == EFI_SUCCESS) {
            Count++;

            // Display on screen
            if (Key.UnicodeChar != 0) {
                Print(L"%c", Key.UnicodeChar);
            } else if (Key.ScanCode == SCAN_ESC) {
                // ESC to exit
                Print(L"\n");
                break;
            } else {
                // Function keys
                Print(L"[%02x]", Key.ScanCode);
            }

            // Echo back to PC via serial
            if (Key.UnicodeChar != 0) {
                CHAR16 Str[2] = { Key.UnicodeChar, L'\0' };
                gST->ConOut->OutputString(gST->ConOut, Str);
            }
        }

        // Small delay to avoid high CPU usage
        gBS->Stall(1000);  // 1ms
    }

    Print(L"--- End ---\n");
    Print(L"\n");
    Print(L"Total characters received: %d\n", Count);
    Print(L"\n");
    Print(L"Press any key to exit...\n");

    // Wait for any key to exit
    gST->ConIn->Reset(gST->ConIn, FALSE);
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY) {
        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, 0);
    }

    Print(L"Goodbye!\n");
    return EFI_SUCCESS;
}
