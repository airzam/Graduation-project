## @file
#  My Application Package DSC
#
#  Copyright (c) 2026, airzam. All rights reserved.<BR>
#

[Defines]
  PLATFORM_NAME                  = MyAppPkg
  PLATFORM_GUID                  = 5a8f2e1b-3c4d-5e6f-7a8b-9c0d1e2f3a4b
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/MyAppPkg
  SUPPORTED_ARCHITECTURES       = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE

[LibraryClasses]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BasePcdLibNull|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  Aarch64MemLib|HelloWorldPkg/Library/Aarch64MemLib/Aarch64MemLib.inf
  StackCheckLib|MdePkg/Library/StackCheckLib/StackCheckLib.inf
  StackCheckFailureHookLib|MdePkg/Library/StackCheckFailureHookLibNull/StackCheckFailureHookLibNull.inf

[Components]
  MyAppPkg/MyGuiFrame/MyGuiFrame.inf

[BuildOptions]
  GCC:*_*_AARCH64_CC_FLAGS = -fno-builtin -fno-stack-protector
