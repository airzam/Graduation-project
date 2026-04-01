# MyGuiFrame 代码详解

## 一、整体架构

MyGuiFrame 是一个完整的 UEFI 图形界面应用程序，代码共 18 个文件。以下按照**程序执行的逻辑顺序**逐步解析。

---

## 二、程序入口

### 2.1 main 函数入口

源码位置：`MyGuiFrame.c` 第 50-126 行

```c
int main(int argc, char **argv)
{
    UINT64 flag;
    flag = InintGloabalProtocols(S_TEXT_INPUT_EX | GRAPHICS_OUTPUT | PCI_ROOTBRIDGE_IO | PCI_IO | FILE_IO | SIMPLE_POINTER);
    Print(L"flag=%x\n", flag);
    WaitKey();

    SwitchGraphicsMode(TRUE);      // ① 进入图形模式
    InSYGraphicsMode();            // ② 初始化图形系统
    SetBKG(&(gColorTable[3]));    // ③ 设置背景色
    ShowMyGuiPic(400, 100);       // ④ 显示GUI图片
    InitGUI();                     // ⑤ 初始化GUI框架
    HanlderGUI();                  // ⑥ 进入GUI事件循环（主循环）

    SetMyMode(0x0);               // ⑦ 恢复显示模式
    OutSYGraphicsMode();           // ⑧ 退出图形模式
    SwitchGraphicsMode(FALSE);     // ⑨ 切回文本模式
    return(0);
}
```

**执行流程图**：
```
启动 → 初始化协议 → 切换图形模式 → 显示GUI → 事件循环 → 退出 → 返回
```

---

## 三、第一步：初始化协议 (InintGloabalProtocols)

源码位置：`Common.c`

### 3.1 函数定义

```c
UINT64 InintGloabalProtocols(UINT64 flag)
{
    UINT64 ret = 0;

    if (flag & S_TEXT_INPUT_EX) {
        if (LocateSimpleTextInputEx() == EFI_SUCCESS)
            ret |= S_TEXT_INPUT_EX;
    }
    if (flag & GRAPHICS_OUTPUT) {
        if (LocateGraphicsOutput() == EFI_SUCCESS)
            ret |= GRAPHICS_OUTPUT;
    }
    if (flag & PCI_ROOTBRIDGE_IO) {
        if (LocatePCIRootBridgeIO() == EFI_SUCCESS)
            ret |= PCI_ROOTBRIDGE_IO;
    }
    if (flag & PCI_IO) {
        if (LocatePCIIO() == EFI_SUCCESS)
            ret |= PCI_ROOTBRIDGE_IO;
    }
    if (flag & FILE_IO) {
        if (LocateFileRoot() == EFI_SUCCESS)
            ret |= FILE_IO;
    }
    if (flag & SIMPLE_POINTER) {
        if (LocateMouse() == EFI_SUCCESS)
            ret |= SIMPLE_POINTER;
    }
    return ret;
}
```

### 3.2 获取图形输出协议 (GOP)

```c
EFI_STATUS LocateGraphicsOutput(VOID)
{
    EFI_STATUS Status;
    EFI_GUID gEfiGraphicsOutputProtocolGuid =
        {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};

    Status = gBS->LocateProtocol(
        &gEfiGraphicsOutputProtocolGuid,  // 协议 GUID
        NULL,                               // 注册数据
        (VOID**)&gGraphicsOutput           // 输出：协议指针
    );
    return Status;
}
```

**关键点**：
- `gEfiGraphicsOutputProtocolGuid` 是 GOP 协议的唯一标识符
- `gBS->LocateProtocol()` 是 UEFI 启动服务提供的查找协议函数
- `gGraphicsOutput` 是全局变量，保存 GOP 协议指针

### 3.3 获取鼠标协议

```c
EFI_STATUS LocateMouse(VOID)
{
    EFI_STATUS Status;
    EFI_GUID gEfiSimplePointerProtocolGuid =
        {0x31878c87, 0xb75, 0x11d5, {0x86, 0x17, 0x5a, 0x45, 0x27, 0x78, 0xxx, 0xxx}};

    Status = gBS->LocateProtocol(
        &gEfiSimplePointerProtocolGuid,
        NULL,
        (VOID**)&gMouse
    );
    return Status;
}
```

---

## 四、第二步：图形模式切换

### 4.1 SwitchGraphicsMode 函数

源码位置：`Common.c` 或 `Graphic.c`

```c
VOID SwitchGraphicsMode(BOOLEAN graphics)
{
    if (graphics) {
        // 获取当前模式
        UINT32 currentMode = gGraphicsOutput->Mode->Mode;

        // 遍历所有模式找到 1024x768
        for (UINT32 mode = 0; mode < gGraphicsOutput->Mode->MaxMode; mode++) {
            EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
            UINTN size;
            gGraphicsOutput->QueryMode(gGraphicsOutput, mode, &size, &info);

            if (info->HorizontalResolution == 1024 &&
                info->VerticalResolution == 768) {
                // 设置为 1024x768 模式
                gGraphicsOutput->SetMode(gGraphicsOutput, mode);
                return;
            }
        }
    }
}
```

### 4.2 GOP 协议核心函数

```c
// 查询模式信息
EFI_STATUS QueryMode(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN UINT32 ModeNumber,
    OUT UINTN *SizeOfInfo,
    OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info
);

// 设置模式
EFI_STATUS SetMode(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN UINT32 ModeNumber
);
```

---

## 五、第三步：屏幕绘制 (Blt 操作)

### 5.1 颜色结构

```c
typedef struct {
    UINT8 Blue;      // 0-255
    UINT8 Green;     // 0-255
    UINT8 Red;       // 0-255
    UINT8 Reserved;  // 保留
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;
```

### 5.2 Blt 操作（块传输）

```c
EFI_STATUS Blt(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer,    // 颜色数据缓冲区
    IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION Operation,   // 操作类型
    IN UINTN SourceX, IN UINTN SourceY,              // 源坐标
    IN UINTN DestinationX, IN UINTN DestinationY,    // 目标坐标
    IN UINTN Width, IN UINTN Height,                 // 区域大小
    IN UINTN Delta                                   // 每行字节数
);
```

**操作类型**：
```c
EfiBltVideoFill        // 用颜色填充
EfiBltVideoToBltBuffer // 屏幕→缓冲区
EfiBltBufferToVideo    // 缓冲区→屏幕
EfiBltVideoToVideo     // 屏幕→屏幕
```

### 5.3 设置背景色

```c
VOID SetBKG(EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Color)
{
    // 填充整个屏幕为指定颜色
    gGraphicsOutput->Blt(
        gGraphicsOutput,
        Color,
        EfiBltVideoFill,    // 填充操作
        0, 0,               // 源坐标（忽略）
        0, 0,               // 目标坐标（左上角）
        1024, 768,          // 宽高（整个屏幕）
        0                    // Delta（忽略）
    );
}
```

---

## 六、第四步：显示图片

### 6.1 ShowMyGuiPic 函数

```c
VOID ShowMyGuiPic(UINT32 x, UINT32 y)
{
    // 从 GUIPIC.c 中获取预定义的图片数据
    extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL MyGuiPic[];

    // 使用 Blt 将图片绘制到屏幕指定位置
    gGraphicsOutput->Blt(
        gGraphicsOutput,
        MyGuiPic,                    // 图片数据
        EfiBltBufferToVideo,         // 从缓冲区到屏幕
        0, 0,                        // 源偏移
        x, y,                        // 目标位置
        MyGuiPicWidth,               // 图片宽度
        MyGuiPicHeight,              // 图片高度
        0
    );
}
```

### 6.2 GUIPIC.c 图片数据

```c
// 预定义的 1024x768 桌面背景图片
// 每个像素是一个 EFI_GRAPHICS_OUTPUT_BLT_PIXEL 结构
extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL DesktopImageData[];

// 图片尺寸
#define MyGuiPicWidth  1024
#define MyGuiPicHeight 768
```

---

## 七、第五步：GUI 初始化

### 7.1 InitGUI 函数

```c
VOID InitGUI(VOID)
{
    // 1. 初始化窗口链表
    InitWindowList();

    // 2. 创建主窗口
    CreateWindow(&MainWindow, 100, 100, 600, 400);

    // 3. 初始化鼠标
    initMouseArrow();

    // 4. 注册事件回调
    RegisterEventHandler(MOUSE_EVENT, MouseEventHandler);
    RegisterEventHandler(KEY_EVENT, KeyEventHandler);
}
```

### 7.2 窗口结构

```c
typedef struct {
    UINT32 x, y;           // 窗口位置
    UINT32 width, height;  // 窗口大小
    CHAR16 title[50];       // 窗口标题
    BOOLEAN visible;        // 是否可见
    // ... 其他属性
} WINDOW;
```

---

## 八、第六步：GUI 事件循环（核心）

### 8.1 HanlderGUI 主循环

```c
VOID HanlderGUI(VOID)
{
    while (1) {
        // 1. 处理鼠标事件
        UpdateMouse();

        // 2. 处理键盘事件
        UpdateKeyboard();

        // 3. 绘制桌面
        DrawDesktop();

        // 4. 绘制所有窗口
        DrawAllWindows();

        // 5. 检查退出条件
        if (ShouldExit()) {
            break;  // 退出循环
        }
    }
}
```

### 8.2 UpdateMouse 鼠标更新

```c
VOID UpdateMouse(VOID)
{
    EFI_SIMPLE_POINTER_STATE state;

    // 获取鼠标当前状态
    gMouse->GetState(gMouse, &state);

    // 计算新位置（累加移动量）
    UINT32 newX = mouse_state.x + state.RelativeMovementX;
    UINT32 newY = mouse_state.y + state.RelativeMovementY;

    // 边界检查
    if (newX >= SY_SCREEN_WIDTH) newX = SY_SCREEN_WIDTH - 1;
    if (newY >= SY_SCREEN_HEIGHT) newY = SY_SCREEN_HEIGHT - 1;

    // 清除旧鼠标，绘制新鼠标
    putMouseArrow(mouse_state.x, mouse_state.y);  // 恢复背景
    putMouseArrow(newX, newY);                     // 绘制新位置

    // 更新状态
    mouse_state.x = newX;
    mouse_state.y = newY;
    mouse_state.LeftButton = state.LeftButton;
}
```

### 8.3 putMouseArrow 绘制鼠标

```c
VOID putMouseArrow(UINT32 x, UINT32 y)
{
    // 鼠标箭头是一个预定义的 16x16 像素图像
    extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL MouseArrowData[16 * 16];

    // 保存当前位置的背景（用于清除鼠标）
    SaveBackground(x, y);

    // 绘制鼠标箭头
    gGraphicsOutput->Blt(
        gGraphicsOutput,
        MouseArrowData,
        EfiBltBufferToVideo,
        0, 0,
        x, y,
        MOUSE_WIDTH, MOUSE_HEIGHT,
        0
    );
}
```

---

## 九、协议汇总表

| 协议名 | GUID 用途 | 关键函数 |
|--------|----------|---------|
| GraphicsOutput | 图形输出/绘制 | Blt(), SetMode(), QueryMode() |
| SimplePointer | 鼠标输入 | GetState() |
| SimpleTextInputEx | 键盘输入 | ReadKeyStrokeEx(), RegisterKeyNotify() |
| SimpleFileSystem | 文件系统 | OpenVolume() |
| PciIo | PCI 设备访问 | Io.Read(), Io.Write() |
| HiiFont | 字体渲染 | DrawString() |
| HiiImage | 图片渲染 | DrawImage() |

---

## 十、全局变量表

| 变量名 | 类型 | 定义位置 | 用途 |
|--------|------|---------|------|
| gGraphicsOutput | EFI_GRAPHICS_OUTPUT_PROTOCOL* | Common.c | 图形输出协议指针 |
| gMouse | EFI_SIMPLE_POINTER_PROTOCOL* | Common.c | 鼠标协议指针 |
| gPCIRootBridgeIO | EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL* | Common.c | PCI根桥协议指针 |
| gColorTable[] | EFI_GRAPHICS_OUTPUT_BLT_PIXEL[] | GUIPIC.c | 预定义颜色数组 |
| mouse_state | MOUSE_STATE | Mouse.c | 鼠标当前状态 |
| gConInEx | EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* | Keyboard.c | 扩展键盘协议 |

---

## 十一、关键代码片段

### 11.1 颜色定义 (GUIPIC.c)

```c
// 预定义颜色表
EFI_GRAPHICS_OUTPUT_BLT_PIXEL gColorTable[] = {
    {0, 0, 0, 0},        // [0] 黑色
    {0, 0, 128, 0},      // [1] 蓝色
    {0, 128, 0, 0},       // [2] 绿色
    {128, 128, 128, 0},  // [3] 灰色
    // ... 更多颜色
};
```

### 11.2 键盘事件注册 (Keyboard.c)

```c
EFI_STATUS HotKeySample(IN EFI_KEY_DATA *hotkey)
{
    EFI_STATUS Status;
    EFI_HANDLE hotkeyNotifyHandle;

    // 设置热键触发条件
    hotkey->KeyState.KeyShiftState |= EFI_SHIFT_STATE_VALID;
    hotkey->KeyState.KeyToggleState |= EFI_TOGGLE_STATE_VALID | EFI_KEY_STATE_EXPOSED;

    // 注册热键通知回调
    Status = gConInEx->RegisterKeyNotify(
        gConInEx,
        hotkey,
        HotKeyNotifyFunc,           // 回调函数
        &hotkeyNotifyHandle
    );
    Print(L"RegisterKeyNotify=%r\n", Status);

    // 等待 ESC 按下
    while(key.Key.ScanCode != 0x17) {  // 0x17 = ESC
        UINTN index;
        gBS->WaitForEvent(1, &(gConInEx->WaitForKey), &index);
        Status = gConInEx->ReadKeyStrokeEx(gConInEx, &key);
    }

    // 注销通知
    gConInEx->UnregisterKeyNotify(gConInEx, hotkeyNotifyHandle);
    return EFI_SUCCESS;
}
```

### 11.3 随机图形测试 (RandomBlock)

```c
VOID RandomBlock(UINT32 Width, UINT32 Height,
                EFI_GRAPHICS_OUTPUT_BLT_PIXEL* BltArray,
                UINT32 BltArraySize)
{
    EFI_EVENT myEvent;
    EFI_STATUS Status;
    UINTN index = 0;

    // 创建定时器事件
    Status = gBS->CreateEvent(EVT_TIMER, TPL_CALLBACK, NULL, NULL, &myEvent);
    Status = gBS->SetTimer(myEvent, TimerPeriodic, 2 * 1000 * 1000); // 200ms

    while (1) {
        Status = gBS->WaitForEvent(1, &myEvent, &index);

        // 随机位置画随机颜色的小方块
        UINT32 rand_x = robin_rand() % Width;
        UINT32 rand_y = robin_rand() % Height;
        UINT32 rand_color = robin_rand() % BltArraySize;

        rectblock(rand_x, rand_y, rand_x + 20, rand_y + 20, &BltArray[rand_color]);

        if (repeats++ == 100) break;
    }

    gBS->CloseEvent(myEvent);
}
```

---

## 十二、文件依赖关系

```
MyGuiFrame.inf
  │
  ├─requires: MdePkg.dec, MdeModulePkg.dec, ShellPkg.dec, StdLib.dec
  │
  └─links with:
       ├─LibC, LibStdio, DevShell (StdLib)
       ├─UefiLib, ShellCEntryLib (MdePkg)
       ├─HiiLib (MdeModulePkg)
       └─Aarch64MemLib (HelloWorldPkg)

MyGuiFrame.c
  │
  ├─#include "Common.h"
  │     ├─#include <Uefi.h>
  │     ├─#include <Library/UefiLib.h>
  │     └─extern gGraphicsOutput, gMouse, gPCIRootBridgeIO
  │
  ├─#include "Graphic.h"
  │     └─SetPixel(), DrawLine(), DrawRect(), rectblock(), DrawCircle()
  │
  ├─#include "Mouse.h"
  │     └─initMouseArrow(), putMouseArrow(), UpdateMouse()
  │
  ├─#include "Keyboard.h"
  │     └─GetKey(), WaitKey(), HotKeySample()
  │
  └─#include "Window.h"
        └─InitGUI(), HanlderGUI(), CreateWindow(), DrawAllWindows()
```

---

## 十三、源码位置

- **编译版本**：`~/Graduation-project/rpi5-uefi/edk2/MyAppPkg/MyGuiFrame/`
- **完整版本（RobinPkg）**：`~/edk2/RobinPkg/Applications/MyGuiFrame/`
- **编译产物**：`Build/RobinPkg/DEBUG_GCC5/AARCH64/MyGuiFrame.efi`
