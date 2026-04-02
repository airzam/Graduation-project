/** @file
    SerialRecv - Serial loopback test for UEFI

    Copyright (c) 2026, airzam. All rights reserved.<BR>

    THE PROGRAM IS DISTRIBUTED UNDER BSD LICENSE ON AN "AS IS" BASIS.
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS EFIAPI UefiMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_INPUT_KEY Key;
    UINTN Count = 0;

    Print(L"===========================================\n");
    Print(L"  Serial Loopback Test\n");
    Print(L"===========================================\n");

    // Clear input buffer
    gST->ConIn->Reset(gST->ConIn, FALSE);

    while (1) {
        // Wait for key from serial
        EFI_STATUS Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

        if (Status == EFI_SUCCESS) {
            Count++;

            // Display on screen
            if (Key.UnicodeChar != 0) {
                Print(L"%c", Key.UnicodeChar);
            } else if (Key.ScanCode == SCAN_ESC) {
                break;
            }

            // Echo back to PC via serial
            if (Key.UnicodeChar != 0) {
                CHAR16 Str[2] = { Key.UnicodeChar, L'\0' };
                gST->ConOut->OutputString(gST->ConOut, Str);
            }
        }

        gBS->Stall(1000);  // 1ms delay
    }

    Print(L"\nTotal: %d\n", Count);
    return EFI_SUCCESS;
}
