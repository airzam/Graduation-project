# MyGuiFrame 代码详解

## 一、整体架构

MyGuiFrame 是一个完整的 UEFI 图形界面应用程序，代码共 18 个文件。以下按照**程序执行的逻辑顺序**逐步解析。

---

## 二、程序入口

### 2.1 main 函数入口

源码位置：`MyGuiFrame.c` 第 50-126 行

```c
int main(int argc, char **argv)                    // ① argc=参数个数, argv=参数列表
{
    UINT64 flag;                                    // ① flag 用于接收协议初始化结果

    // ② 初始化所有需要的 UEFI 协议，返回值每一位代表一个协议是否初始化成功
    // S_TEXT_INPUT_EX=键盘, GRAPHICS_OUTPUT=图形, PCI_ROOTBRIDGE_IO=PCI桥,
    // PCI_IO=PCI设备, FILE_IO=文件系统, SIMPLE_POINTER=鼠标
    flag = InintGloabalProtocols(
        S_TEXT_INPUT_EX | GRAPHICS_OUTPUT | PCI_ROOTBRIDGE_IO | PCI_IO | FILE_IO | SIMPLE_POINTER
    );

    Print(L"flag=%x\n", flag);                      // ③ 打印协议初始化结果（十六进制）
    WaitKey();                                       // ④ 等待按键（调试用，停下来查看结果）

    SwitchGraphicsMode(TRUE);                        // ⑤ 进入图形模式，设置分辨率为 1024x768
    InSYGraphicsMode();                              // ⑥ 初始化图形系统（分配显存等）
    SetBKG(&(gColorTable[3]));                       // ⑦ 设置背景色（灰色，gColorTable[3]）
    ShowMyGuiPic(400, 100);                         // ⑧ 在坐标(400,100)处显示GUI图片
    InitGUI();                                       // ⑨ 初始化GUI框架（创建窗口、注册事件等）
    HanlderGUI();                                    // ⑩ 进入GUI事件循环（主循环，处理鼠标键盘事件）

    SetMyMode(0x0);                                 // ⑪ 恢复显示模式为 0x0（UEFI 控制台默认模式）
    OutSYGraphicsMode();                             // ⑫ 退出图形系统（释放显存等资源）
    SwitchGraphicsMode(FALSE);                       // ⑬ 切换回文本模式（恢复 UEFI Shell 文本界面）

    return(0);                                       // ⑭ 程序正常退出
}
```

**执行流程图**：
```
启动 → 初始化协议 → 等待按键 → 切换图形模式 → 初始化图形系统
     → 设置背景 → 显示图片 → 初始化GUI → 事件循环 → 退出图形模式 → 返回
```

**UEFI 启动服务 (Boot Services) 常用函数**：
| 函数 | 作用 |
|------|------|
| `gBS->LocateProtocol()` | 查找已加载的协议 |
| `gBS->CreateEvent()` | 创建事件 |
| `gBS->SetTimer()` | 设置定时器 |
| `gBS->WaitForEvent()` | 等待事件 |
| `gBS->CloseEvent()` | 关闭事件 |

---

## 三、第一步：初始化协议 (InintGloabalProtocols)

源码位置：`Common.c`

### 3.1 函数定义

```c
// 功能：根据 flag 参数批量初始化 UEFI 协议
// 参数：flag - 位掩码，指定要初始化的协议（如 GRAPHICS_OUTPUT | SIMPLE_POINTER）
// 返回：ret - 位掩码，返回实际初始化成功的协议
UINT64 InintGloabalProtocols(UINT64 flag)
{
    UINT64 ret = 0;                                 // 初始化返回值为 0

    // 判断 S_TEXT_INPUT_EX（扩展文本输入协议，键盘）是否被请求
    if (flag & S_TEXT_INPUT_EX) {
        // 调用 LocateSimpleTextInputEx() 尝试获取键盘协议
        // 如果成功（返回 EFI_SUCCESS），将 S_TEXT_INPUT_EX 位加入返回值
        if (LocateSimpleTextInputEx() == EFI_SUCCESS)
            ret |= S_TEXT_INPUT_EX;
    }

    // 判断 GRAPHICS_OUTPUT（图形输出协议，GOP）是否被请求
    if (flag & GRAPHICS_OUTPUT) {
        // GOP 是 UEFI 中最重要的图形协议，负责屏幕绘制、分辨率设置等
        if (LocateGraphicsOutput() == EFI_SUCCESS)
            ret |= GRAPHICS_OUTPUT;
    }

    // 判断 PCI_ROOTBRIDGE_IO（PCI 根桥 IO 协议）是否被请求
    if (flag & PCI_ROOTBRIDGE_IO) {
        // PCI 根桥用于访问 PCI 总线和设备配置空间
        if (LocatePCIRootBridgeIO() == EFI_SUCCESS)
            ret |= PCI_ROOTBRIDGE_IO;
    }

    // 判断 PCI_IO（PCI IO 协议）是否被请求
    if (flag & PCI_IO) {
        // PCI IO 用于读写 PCI 设备的 BAR 空间（内存映射 I/O）
        if (LocatePCIIO() == EFI_SUCCESS)
            ret |= PCI_ROOTBRIDGE_IO;  // 注意：这里有个 bug，应该是 ret |= PCI_IO
    }

    // 判断 FILE_IO（简单文件系统协议）是否被请求
    if (flag & FILE_IO) {
        // 用于访问 FAT 文件系统（如 SD 卡的 FAT32 分区）
        if (LocateFileRoot() == EFI_SUCCESS)
            ret |= FILE_IO;
    }

    // 判断 SIMPLE_POINTER（简单指针协议，鼠标）是否被请求
    if (flag & SIMPLE_POINTER) {
        // 鼠标协议，获取相对移动量和按键状态
        if (LocateMouse() == EFI_SUCCESS)
            ret |= SIMPLE_POINTER;
    }

    return ret;  // 返回实际初始化成功的协议掩码
}
```

### 3.2 获取图形输出协议 (GOP)

GOP（Graphics Output Protocol）是 UEFI 中最重要的图形协议，通过它可以在屏幕上绘制像素。

```c
// 功能：在系统中查找 GOP（图形输出协议）
// 返回：EFI_SUCCESS 表示找到，EFI_NOT_FOUND 表示未找到
EFI_STATUS LocateGraphicsOutput(VOID)
{
    EFI_STATUS Status;                               // UEFI 状态码（EFI_SUCCESS=0）

    // GOP 协议的 GUID（全局唯一标识符）
    // GUID 是 UEFI 协议的身份标识，通过它找到系统中的协议实例
    EFI_GUID gEfiGraphicsOutputProtocolGuid =
        {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};

    // 调用 Boot Services 的 LocateProtocol 在已加载的协议中查找 GOP
    // 参数1：要找的协议的 GUID（身份证号）
    // 参数2：注册数据，通常为 NULL
    // 参数3：输出参数，指向找到的协议实例指针的指针（*gGraphicsOutput = 协议地址）
    Status = gBS->LocateProtocol(
        &gEfiGraphicsOutputProtocolGuid,  // 协议 GUID（0x9042a9de...）
        NULL,                             // 注册数据（不需要）
        (VOID**)&gGraphicsOutput          // 输出：协议指针的地址（协议指针存入 gGraphicsOutput）
    );

    return Status;  // 返回查找结果
}
```

**GOP 协议核心数据结构**：

```c
// GOP 协议结构体（简化版）
typedef struct {
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;  // 当前模式信息（分辨率、帧缓冲地址等）

    // 查询可用显示模式
    // ModeNumber：要查询的模式编号（0~MaxMode-1）
    // SizeOfInfo：输出，模式信息结构体的大小
    // Info：输出，指向模式信息结构体的指针
    EFI_STATUS (*QueryMode)(
        IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
        IN UINT32 ModeNumber,
        OUT UINTN *SizeOfInfo,
        OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info
    );

    // 设置显示模式
    // ModeNumber：要设置的模式编号
    EFI_STATUS (*SetMode)(
        IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
        IN UINT32 ModeNumber
    );

    // 块传输（Blt）- 最核心的绘制函数
    // BltBuffer：颜色数据缓冲区
    // Operation：操作类型（填充/复制）
    // SourceX/Y：源坐标
    // DestX/Y：目标坐标
    // Width/Height：区域宽高
    // Delta：每行字节数（0表示Width*sizeof(BLT_PIXEL)）
    EFI_STATUS (*Blt)(
        IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
        IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer,
        IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION Operation,
        IN UINTN SourceX, IN UINTN SourceY,
        IN UINTN DestinationX, IN UINTN DestinationY,
        IN UINTN Width, IN UINTN Height,
        IN UINTN Delta
    );
} EFI_GRAPHICS_OUTPUT_PROTOCOL;
```

### 3.3 获取鼠标协议

```c
// 功能：查找鼠标（简单指针协议）
// 注意：原代码中 GUID 部分被截断，完整 GUID 应为：
// {0x31878c87, 0x0b75, 0x11d5, {0x9a, 0x45, 0x00, 0x10, 0x83, 0x43, 0x04, 0x00}}
EFI_STATUS LocateMouse(VOID)
{
    EFI_STATUS Status;

    // 鼠标协议的 GUID
    EFI_GUID gEfiSimplePointerProtocolGuid =
        {0x31878c87, 0xb75, 0x11d5, {0x86, 0x17, 0x5a, 0x45, 0x27, 0x78, 0xxx, 0xxx}};

    // 通过 gBS->LocateProtocol 在已加载的协议中查找鼠标协议
    // 找到后，协议指针存入全局变量 gMouse
    Status = gBS->LocateProtocol(
        &gEfiSimplePointerProtocolGuid,
        NULL,
        (VOID**)&gMouse
    );

    return Status;
}
```

**鼠标协议核心函数**：

```c
// 获取鼠标当前状态
// State：输出，鼠标状态结构体
typedef struct {
    INT32 RelativeMovementX;   // 相对X移动量（像素，与上次相比）
    INT32 RelativeMovementY;   // 相对Y移动量
    INT32 RelativeMovementZ;   // 滚轮滚动量
    BOOLEAN LeftButton;        // 左键是否按下
    BOOLEAN RightButton;       // 右键是否按下
} EFI_SIMPLE_POINTER_STATE;

EFI_STATUS (*GetState)(
    IN EFI_SIMPLE_POINTER_PROTOCOL *This,
    OUT EFI_SIMPLE_POINTER_STATE *State
);
```

---

## 四、第二步：图形模式切换

### 4.1 SwitchGraphicsMode 函数

```c
// 功能：切换图形/文本模式
// 参数：graphics=TRUE 进入图形模式，FALSE 切回文本模式
VOID SwitchGraphicsMode(BOOLEAN graphics)
{
    if (graphics) {
        // 获取当前模式编号
        // gGraphicsOutput->Mode->Mode：当前 GOP 模式编号（0~MaxMode-1）
        UINT32 currentMode = gGraphicsOutput->Mode->Mode;

        // 遍历所有支持的显示模式
        // gGraphicsOutput->Mode->MaxMode：支持的总模式数
        for (UINT32 mode = 0; mode < gGraphicsOutput->Mode->MaxMode; mode++) {
            EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;  // 模式信息结构体指针
            UINTN size;                                    // 模式信息结构体大小

            // 调用 GOP 的 QueryMode 查询第 mode 号模式的信息
            // info 输出：指向模式信息结构体的指针（由 GOP 内部分配内存）
            gGraphicsOutput->QueryMode(gGraphicsOutput, mode, &size, &info);

            // 检查是否是 1024x768 分辨率
            if (info->HorizontalResolution == 1024 &&  // 水平分辨率（宽）= 1024
                info->VerticalResolution == 768) {    // 垂直分辨率（高）= 768
                // 找到了！设置为 1024x768 模式
                gGraphicsOutput->SetMode(gGraphicsOutput, mode);
                return;  // 设置成功，返回
            }
        }
        // 如果循环结束没找到 1024x768，则保持当前模式不变
    }
}
```

**模式信息结构体**：

```c
typedef struct {
    UINT32 Version;                     // 结构体版本
    UINT32 HorizontalResolution;         // 水平分辨率（像素）
    UINT32 VerticalResolution;           // 垂直分辨率（像素）
    EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;  // 像素格式（像素在内存中的排列方式）
    EFI_PIXEL_BITMASK PixelInformation;  // 像素位掩码（RGBA 各几位）
    UINT32 PixelsPerScanLine;            // 每行像素数（可能大于 HorizontalResolution）
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;
```

### 4.2 GOP 协议核心函数详解

```c
// 1. QueryMode - 查询显示模式信息
// 功能：获取指定模式的详细信息
// 参数：
//   This：指向 GOP 协议实例的指针（通常是 gGraphicsOutput）
//   ModeNumber：要查询的模式编号（从 0 开始）
//   SizeOfInfo：输出，Info 结构体的字节大小
//   Info：输出，指向模式信息结构体的指针（内存由 GOP 分配，调用者不可修改）
EFI_STATUS QueryMode(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN UINT32 ModeNumber,
    OUT UINTN *SizeOfInfo,
    OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info
);

// 2. SetMode - 设置显示模式
// 功能：切换屏幕分辨率
// 注意：切换分辨率会改变帧缓冲区地址，可能导致屏幕闪烁
EFI_STATUS SetMode(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN UINT32 ModeNumber
);
```

---

## 五、第三步：屏幕绘制 (Blt 操作)

### 5.1 颜色结构

```c
// UEFI 图形颜色结构体（BGRA 格式，注意是 Blue-Green-Red-Alpha 顺序）
// 注意：很多系统使用 RGBA，但 UEFI 使用 BGRA
typedef struct {
    UINT8 Blue;       // 蓝色分量，范围 0~255
    UINT8 Green;      // 绿色分量，范围 0~255
    UINT8 Red;       // 红色分量，范围 0~255
    UINT8 Reserved;  // 保留，通常为 0（Alpha 通道）
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

// 定义颜色示例：
// 黑色：{0, 0, 0, 0}
// 白色：{255, 255, 255, 0}
// 红色：{0, 0, 255, 0}
// 蓝色：{255, 0, 0, 0}
// 绿色：{0, 255, 0, 0}
// 灰色：{128, 128, 128, 0}
```

### 5.2 Blt 操作（块传输）详解

Blt（Block Transfer，块传输）是 GOP 协议的核心绘制函数，可以在显存和内存之间复制像素数据。

```c
// Blt - 块传输操作（像素复制/填充）
// 参数详解：
//   This：GOP 协议指针
//   BltBuffer：颜色数据缓冲区
//     - 当 Operation 是 EfiBltVideoFill 时：用作填充色（单个像素重复）
//     - 当 Operation 是 EfiBltBufferToVideo 时：源数据（要绘制到屏幕的像素）
//     - 当 Operation 是 EfiBltVideoToBuffer 时：目标缓冲区（保存屏幕像素）
//   Operation：操作类型
//     - EfiBltVideoFill：用 BltBuffer 的第一个像素填充目标矩形（矩形填充）
//     - EfiBltVideoToBltBuffer：复制屏幕区域到 BltBuffer（截图）
//     - EfiBltBufferToVideo：绘制 BltBuffer 到屏幕（贴图）
//     - EfiBltVideoToVideo：屏幕区域之间复制（区域移动）
//   SourceX/Y：源矩形的左上角坐标（像素单位）
//   DestinationX/Y：目标矩形的左上角坐标（像素单位）
//   Width：矩形宽度（像素）
//   Height：矩形高度（像素）
//   Delta：每行字节数
//     - 通常传 0，表示 Width * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
//     - 如果 BltBuffer 的行宽（pitch）大于实际像素宽，需要指定
EFI_STATUS Blt(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer,       // 颜色数据
    IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION Operation,    // 操作类型
    IN UINTN SourceX, IN UINTN SourceY,                // 源坐标
    IN UINTN DestinationX, IN UINTN DestinationY,      // 目标坐标
    IN UINTN Width, IN UINTN Height,                   // 宽高
    IN UINTN Delta                                     // 行字节数
);
```

**Blt 操作类型**：

| 操作类型 | 源 | 目标 | 用途 |
|---------|-----|------|------|
| `EfiBltVideoFill` | BltBuffer（单像素） | 屏幕 | 用纯色填充矩形（画矩形） |
| `EfiBltBufferToVideo` | BltBuffer | 屏幕 | 绘制图片/贴图 |
| `EfiBltVideoToBltBuffer` | 屏幕 | BltBuffer | 读取屏幕像素（截图） |
| `EfiBltVideoToVideo` | 屏幕 | 屏幕 | 屏幕区域移动（带透明效果） |

### 5.3 设置背景色

```c
// 功能：用指定颜色填充整个屏幕作为背景
// 参数：Color - 指向颜色结构体的指针
VOID SetBKG(EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Color)
{
    // 调用 GOP 的 Blt 函数，用 Color 填充整个屏幕
    gGraphicsOutput->Blt(
        gGraphicsOutput,              // GOP 协议指针
        Color,                        // 颜色数据（这里直接用 Color 作为填充色）
        EfiBltVideoFill,             // 操作：视频填充（用单一颜色填充）
        0, 0,                        // 源坐标（对 EfiBltVideoFill 无意义，可设为 0）
        0, 0,                        // 目标坐标：屏幕左上角 (0, 0)
        1024, 768,                    // 宽度 1024 像素，高度 768 像素（整个屏幕）
        0                             // Delta：0 表示每行宽度 = Width * 4 字节
    );
}
```

### 5.4 显示图片

```c
// 功能：在屏幕指定位置显示预定义的图片
// 参数：
//   x - 图片左上角的 X 坐标
//   y - 图片左上角的 Y 坐标
VOID ShowMyGuiPic(UINT32 x, UINT32 y)
{
    // 从 GUIPIC.c 中获取预定义的图片数据（外部变量声明）
    // MyGuiPic 是一个像素数组，每 4 字节代表一个像素（BGRA 格式）
    extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL MyGuiPic[];

    // 使用 Blt 将图片数据绘制到屏幕
    // BltBuffer = MyGuiPic（图片的像素数据）
    // Operation = EfiBltBufferToVideo（从缓冲区到屏幕，即"贴图"）
    // SourceX/Y = 0, 0（从图片的左上角开始）
    // DestX/Y = x, y（绘制到屏幕的指定位置）
    // Width/Height = MyGuiPicWidth, MyGuiPicHeight（图片的宽高）
    gGraphicsOutput->Blt(
        gGraphicsOutput,                    // GOP 协议
        MyGuiPic,                          // 图片像素数据（源缓冲区）
        EfiBltBufferToVideo,               // 操作：从内存绘制到屏幕
        0, 0,                               // 源偏移：从图片的(0,0)开始
        x, y,                               // 目标位置：屏幕上的(x,y)
        MyGuiPicWidth,                      // 图片宽度（像素）
        MyGuiPicHeight,                     // 图片高度（像素）
        0                                   // Delta：0 = Width * 4
    );
}
```

---

## 六、第四步：GUI 初始化

### 6.1 InitGUI 函数

```c
// 功能：初始化 GUI 框架
// 包括：窗口链表、鼠标、事件处理回调
VOID InitGUI(VOID)
{
    // 1. 初始化窗口管理系统
    //    - 创建窗口链表的数据结构
    //    - 可能涉及分配内存、设置链表头等
    InitWindowList();

    // 2. 创建主窗口
    //    参数：窗口对象指针, x, y, width, height
    //    位置：(100, 100)，大小：600x400
    CreateWindow(&MainWindow, 100, 100, 600, 400);

    // 3. 初始化鼠标光标
    //    - 加载鼠标箭头图像数据
    //    - 初始化鼠标状态（位置=屏幕中央，按键状态=未按下）
    initMouseArrow();

    // 4. 注册事件处理回调
    //    - MOUSE_EVENT：鼠标事件（移动、点击）由 MouseEventHandler 处理
    //    - KEY_EVENT：键盘事件（按键）由 KeyEventHandler 处理
    //    - 这样当硬件中断触发时，UEFI 会调用对应的处理函数
    RegisterEventHandler(MOUSE_EVENT, MouseEventHandler);
    RegisterEventHandler(KEY_EVENT, KeyEventHandler);
}
```

### 6.2 窗口结构体

```c
// 窗口结构体（简化版）
typedef struct {
    UINT32 x;           // 窗口左上角 X 坐标（屏幕坐标系）
    UINT32 y;           // 窗口左上角 Y 坐标
    UINT32 width;       // 窗口宽度（像素）
    UINT32 height;      // 窗口高度（像素）
    CHAR16 title[50];   // 窗口标题（Unicode 16 字符串）
    BOOLEAN visible;    // 窗口是否可见
    BOOLEAN focused;    // 窗口是否获得焦点（接收键盘输入）
    // 可能还有：
    // - 背景颜色
    // - 边框样式
    // - 子控件列表
    // - 窗口过程函数指针
} WINDOW;
```

---

## 七、第五步：GUI 事件循环（核心）

### 7.1 HanlderGUI 主循环

```c
// 功能：GUI 主事件循环
// 原理：轮询检查鼠标和键盘状态，有变化时响应
// 循环直到用户按 ESC 或满足退出条件才退出
VOID HanlderGUI(VOID)
{
    while (1) {                                   // 无限循环，程序主心骨
        // 1. 处理鼠标事件
        //    - 调用 gMouse->GetState() 获取当前鼠标状态
        //    - 与上次状态对比，判断是否移动或点击
        //    - 必要时重绘鼠标光标
        UpdateMouse();

        // 2. 处理键盘事件
        //    - 检查是否有按键按下
        //    - 读取按键信息（Scancode、UnicodeChar）
        //    - 分发到焦点窗口的键盘处理函数
        UpdateKeyboard();

        // 3. 绘制桌面背景
        //    - 可能是纯色或图片
        //    - 在窗口之下，作为最底层
        DrawDesktop();

        // 4. 绘制所有窗口
        //    - 按 Z-order（前后顺序）从底到顶绘制
        //    - 每个窗口包括：背景、标题栏、边框、控件
        DrawAllWindows();

        // 5. 检查退出条件
        //    - 通常是检测到 ESC 按键
        //    - ShouldExit() 返回 TRUE 时退出循环
        if (ShouldExit()) {
            break;                                // 退出主循环
        }

        // 注意：这里没有 sleep，循环全速运行
        // 在真实 UEFI 环境中，应加入 WaitForEvent 等待事件触发
        // 否则会 100% 占用 CPU
    }
}
```

### 7.2 UpdateMouse 鼠标更新

```c
// 功能：更新鼠标状态（移动光标、检测点击）
VOID UpdateMouse(VOID)
{
    EFI_SIMPLE_POINTER_STATE state;              // 鼠标状态结构体

    // 调用鼠标协议的 GetState 获取当前鼠标状态
    // 重要：RelativeMovementX/Y 是相对移动量（与上次相比）
    // 每次调用后，硬件会重置为 0，所以必须每次都调用并累加
    gMouse->GetState(gMouse, &state);

    // 计算新位置：旧位置 + 相对移动量
    // 注意：RelativeMovementX 是有符号整数，可正可负
    UINT32 newX = mouse_state.x + state.RelativeMovementX;
    UINT32 newY = mouse_state.y + state.RelativeMovementY;

    // 边界检查：防止鼠标移出屏幕
    if (newX >= SY_SCREEN_WIDTH)                 // 如果超出右边界
        newX = SY_SCREEN_WIDTH - 1;             //  clamp 到最右边
    if (newY >= SY_SCREEN_HEIGHT)                // 如果超出下边界
        newY = SY_SCREEN_HEIGHT - 1;             //  clamp 到最下边

    // 清除旧鼠标：恢复鼠标当前位置的背景
    // 原理：保存鼠标位置的背景到缓冲区，绘制新鼠标前先用它恢复
    putMouseArrow(mouse_state.x, mouse_state.y);

    // 绘制新鼠标：在新位置显示鼠标箭头
    putMouseArrow(newX, newY);

    // 更新全局鼠标状态
    mouse_state.x = newX;                        // 保存新 X 坐标
    mouse_state.y = newY;                        // 保存新 Y 坐标
    mouse_state.LeftButton = state.LeftButton;   // 保存左键状态
}
```

### 7.3 putMouseArrow 绘制鼠标

```c
// 功能：在指定位置绘制或清除鼠标箭头
// 原理：绘制时保存背景到缓冲区，清除时用缓冲区恢复背景
// 参数：(x, y) 是鼠标箭头左上角在屏幕上的坐标
VOID putMouseArrow(UINT32 x, UINT32 y)
{
    // 获取预定义的鼠标箭头图像（16x16 像素）
    // 这是一个外部定义的常量数组，每像素一个 BGRA 结构
    extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL MouseArrowData[16 * 16];

    // 保存当前位置的背景（用于清除鼠标时恢复）
    // 会在内部将当前区域的像素保存到全局缓冲区
    SaveBackground(x, y);

    // 使用 Blt 将鼠标箭头绘制到屏幕上
    // 注意：鼠标箭头有透明像素，透明位置应跳过或用特定颜色表示
    gGraphicsOutput->Blt(
        gGraphicsOutput,                          // GOP 协议
        MouseArrowData,                          // 鼠标箭头像素数据
        EfiBltBufferToVideo,                     // 从缓冲区绘制到屏幕
        0, 0,                                     // 源偏移：从箭头图像的(0,0)开始
        x, y,                                     // 目标位置：屏幕(x,y)
        MOUSE_WIDTH, MOUSE_HEIGHT,               // 鼠标宽度/高度（16x16）
        0                                         // Delta
    );
}
```

---

## 八、协议汇总表

| 协议名 | GUID 用途 | 关键函数 |
|--------|----------|---------|
| **GraphicsOutput (GOP)** | 图形输出/绘制 | `Blt()`, `SetMode()`, `QueryMode()` |
| **SimplePointer** | 鼠标输入 | `GetState()` |
| **SimpleTextInputEx** | 键盘输入 | `ReadKeyStrokeEx()`, `RegisterKeyNotify()` |
| **SimpleFileSystem** | 文件系统 | `OpenVolume()` |
| **PciIo** | PCI 设备访问 | `Io.Read()`, `Io.Write()` |
| **PciRootBridgeIo** | PCI 根桥访问 | `Mem.Read()`, `Mem.Write()` |
| **HiiFont** | 字体渲染 | `DrawString()` |
| **HiiImage** | 图片渲染 | `DrawImage()` |

---

## 九、全局变量表

| 变量名 | 类型 | 定义位置 | 用途 |
|--------|------|---------|------|
| `gGraphicsOutput` | `EFI_GRAPHICS_OUTPUT_PROTOCOL*` | Common.c | 图形输出协议指针 |
| `gMouse` | `EFI_SIMPLE_POINTER_PROTOCOL*` | Common.c | 鼠标协议指针 |
| `gPCIRootBridgeIO` | `EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL*` | Common.c | PCI根桥协议指针 |
| `gConInEx` | `EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL*` | Keyboard.c | 扩展键盘协议指针 |
| `gColorTable[]` | `EFI_GRAPHICS_OUTPUT_BLT_PIXEL[]` | GUIPIC.c | 预定义颜色数组 |
| `mouse_state` | `MOUSE_STATE` | Mouse.c | 鼠标当前状态（位置、按键） |
| `MyGuiPic[]` | `EFI_GRAPHICS_OUTPUT_BLT_PIXEL[]` | GUIPIC.c | GUI 图片像素数据 |
| `MouseArrowData[]` | `EFI_GRAPHICS_OUTPUT_BLT_PIXEL[]` | Mouse.c | 鼠标箭头像素数据 |

---

## 十、文件依赖关系

```
MyGuiFrame.inf (模块定义文件)
  │
  ├─[Packages] 依赖的包
  │     ├─MdePkg.dec          # UEFI 核心类型和库
  │     └─HelloWorldPkg.dec   # 提供 PLATFORM_VERSION 等 PCD
  │
  ├─[LibraryClasses] 依赖的库
  │     ├─UefiApplicationEntryPoint  # 入口点库（定义 UefiMain）
  │     ├─UefiLib                    # UEFI 工具库（Print 等）
  │     ├─PrintLib                   # 打印库
  │     ├─DebugLib                   # 调试库
  │     ├─BaseMemoryLib              # 内存操作库（memcpy 等）
  │     └─Aarch64MemLib              # ARM64 内存操作（ARM架构专用）
  │
  └─[Sources] 源文件
          └─MyGuiFrame.c  # 主程序入口

MyGuiFrame.c (主程序)
  │
  ├─#include "Common.h"       # 通用定义（协议指针、全局变量声明）
  │     ├─#include <Uefi.h>  # UEFI 基本类型（EFI_STATUS、UINTN 等）
  │     ├─#include <Library/UefiLib.h>      # Print()、WaitKey() 等
  │     ├─#include <Library/UefiBootServicesTableLib.h>  # gST、gBS 全局变量
  │     └─extern 全局变量声明  # gGraphicsOutput、gMouse、gPCIRootBridgeIO
  │
  ├─#include "Graphic.h"      # 图形绘制（SetPixel、DrawLine、DrawRect、Blt）
  │     └─Graphic.c          # 图形原语实现
  │
  ├─#include "Mouse.h"        # 鼠标操作（initMouseArrow、putMouseArrow、UpdateMouse）
  │     └─Mouse.c             # 鼠标实现
  │
  ├─#include "Keyboard.h"     # 键盘操作（GetKey、WaitKey、ReadKeyStrokeEx）
  │     └─Keyboard.c          # 键盘实现
  │
  ├─#include "Window.h"       # 窗口管理（InitGUI、HanlderGUI、CreateWindow、DrawAllWindows）
  │     └─Window.c            # 窗口实现
  │
  ├─#include "Font.h"         # 字体渲染（draw_string、draw_single_char）
  │     └─Font.c             # 字体实现
  │
  ├─#include "GUIPIC.h"       # GUI 图片（gColorTable[]、MyGuiPic[]、ShowMyGuiPic）
  │     └─GUIPIC.c            # 图片和颜色数据
  │
  └─#include "FileRW.h"       # 文件读写
        └─FileRW.c            # 文件系统操作实现
```

---

## 十一、源码位置

- **项目工程路径**：`~/Graduation-project/rpi5-uefi/edk2/MyAppPkg/`
- **参考完整实现**：`~/edk2/RobinPkg/Applications/MyGuiFrame/`
- **编译产物**：`~/Graduation-project/rpi5-uefi/edk2/Build/MyAppPkg/DEBUG_GCC5/AARCH64/MyGuiFrame.efi`
