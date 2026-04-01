/** @file
    MyGuiFrame - A simple UEFI GUI application

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
  Print(L"\n");
  Print(L"===========================================\n");
  Print(L"  MyGuiFrame UEFI Application\n");
  Print(L"  Author: airzam\n");
  Print(L"===========================================\n");
  Print(L"\n");
  Print(L"Hello from UEFI!\n");
  Print(L"\n");

  // Wait for key press using gST->ConIn->ReadKeyStroke
  Print(L"Press any key to exit...\n");
  gST->ConIn->Reset(gST->ConIn, FALSE);
  EFI_INPUT_KEY Key;
  while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY) {
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, 0);
  }

  Print(L"Goodbye!\n");
  return EFI_SUCCESS;
}
