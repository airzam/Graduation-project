// HelloWorldPkg/Library/Aarch64MemLib/Aarch64MemLib.c
/**
  @file
  AARCH64-specific memcpy implementation for EDK2.

  Copyright (c) 2026 Your Name. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Base.h>
#include <Library/BaseMemoryLib.h>

/**
  Copy Length bytes from Source to Destination.
  
  This function provides a memcpy implementation for AARCH64 architecture
  when LTO optimization is enabled.
  
  @param  Destination Buffer to copy to.
  @param  Source      Buffer to copy from.
  @param  Length      Number of bytes to copy.
  
  @return Destination.
**/
void *
EFIAPI
memcpy (
  OUT void       *Destination,
  IN  const void *Source,
  IN  UINTN      Length
  )
{
  return CopyMem (Destination, Source, Length);
}