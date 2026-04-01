[Defines]
    PLATFORM_NAME = HelloWorldPkg
    PLATFORM_GUID = 05c9cf14-e0f4-4b9a-8f03-4fa13cdf6694
    PLATFORM_VERSION = 0.1
    DSC_SPECIFICATION = 0x00010005
    SUPPORTED_ARCHITECTURES = IA32|X64|MIPS64EL|AARCH64|ARM
    BUILD_TARGETS = DEBUG|RELEASE

[LibraryClasses]
    UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
    UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
    PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
    MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
    DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
    BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
    BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
    UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
    DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
    UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
    RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
    DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
    StackCheckLib|MdePkg/Library/StackCheckLib/StackCheckLib.inf
    StackCheckFailureHookLib|MdePkg/Library/StackCheckFailureHookLibNull/StackCheckFailureHookLibNull.inf

    # 添加Aarch64MemLib（所有架构通用）
    Aarch64MemLib|HelloWorldPkg/Library/Aarch64MemLib/Aarch64MemLib.inf

[LibraryClasses.AARCH64]

[Components]
    HelloWorldPkg/HelloWorld.inf

[BuildOptions]
    # 为AARCH64添加特定编译选项
    GCC:*_*_AARCH64_CC_FLAGS = -fno-builtin