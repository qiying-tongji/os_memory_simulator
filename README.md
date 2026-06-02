# 操作系统内存管理模拟器

C++ 程序，包含**控制台版**和 **EasyX 图形界面版**，适合操作系统课程设计。

## 项目结构

```
os_memory_simulator/
├── 源码/                       # 源代码目录
│   ├── main.cpp                # 控制台入口
│   ├── main_gui.cpp            # EasyX 图形界面入口
│   ├── dynamic_partition.h/.cpp # 模块一：动态分区算法
│   ├── paging.h/.cpp           # 模块二：请求分页算法
│   ├── gui_helpers.h           # EasyX 公共绘图工具
│   ├── gui_dynamic.h/.cpp      # 动态分区可视化界面
│   └── gui_paging.h/.cpp       # 请求分页可视化界面
├── MemorySimulatorGui.sln      # VS 解决方案（图形版）
├── MemorySimulatorGui.vcxproj  # VS 项目文件
├── MemorySimulatorGui.exe      # 图形界面可执行文件（静态链接）
├── EasyXw.lib                 # EasyX 静态库
├── README.md                   # 项目说明文档
└── 操作系统内存管理模拟器项目设计文档.docx  # 项目设计文档
```

---

## 控制台版（无需 EasyX）

```bash
g++ -std=c++17 -o memory_simulator.exe main.cpp dynamic_partition.cpp paging.cpp
memory_simulator.exe
```

---

## EasyX 图形界面版

### 直接运行（推荐）

项目已提供预编译的可执行文件 `MemorySimulatorGui.exe`，采用静态链接方式编译，**无需安装 EasyX** 即可直接运行。

**运行要求：**
- Windows 操作系统
- Visual C++ 2022 Redistributable（通常 Windows 已自带）

### 开发编译

如果需要修改代码并重新编译，则需要安装 EasyX。

**安装 EasyX：**
1. 打开 [https://easyx.cn](https://easyx.cn) 下载安装包
2. 运行安装程序，选择你的 Visual Studio 版本（推荐 VS2019 / VS2022）
3. 安装完成后会自动配置 `graphics.h`

**编译方式：**
- 双击打开 `MemorySimulatorGui.sln`
- 确认已安装 EasyX
- 选择 `x64` 平台，按 **F5** 运行

**手动添加文件：**
将 `main_gui.cpp`、`gui_*.cpp`、`dynamic_partition.cpp`、`paging.cpp` 加入 VS 空项目，字符集设为 **Unicode**，编译运行。

---

## 图形界面功能

### 模块一：动态分区

| 功能      | 说明                            |
|-----------|---------------------------------|
| 彩色内存条 | 各进程不同颜色，空闲区灰色       |
| 分区表    | 实时显示已分配 / 空闲分区        |
| 自动演示  | 逐步播放 A/B/C/D/E/F 申请释放动画 |
| 单步执行  | 手动点击逐步操作                 |
| 高亮动画 | 当前操作的分区黄色边框闪烁 |

**演示序列：**
- 申请 A(130KB)、B(60KB)、C(100KB)
- 释放 A
- 申请 D(50KB)
- 释放 B
- 申请 E(300KB)、F(40KB)

### 模块二：请求分页

| 功能       | 说明                            |
|------------|--------------------------------|
| 页框可视化  | 4 个物理页框，显示当前页号       |
| 逻辑页地图  | 32 页网格，命中绿色 / 缺页红色   |
| 自动播放    | 320 条指令逐步动画演示           |
| 缺页闪烁    | 缺页时红色高亮，命中时绿色       |
| 实时统计    | 访问次数、缺页次数、缺页率       |
| 进度条      | 显示模拟执行进度                |

---

## 操作说明

- **鼠标点击**按钮操作
- **ESC** 返回上级菜单
- 分页模块支持 **暂停/继续**、**加速** 控制动画速度

---

## 算法说明

### 动态分区
- 首次适应 (First Fit)
- 最佳适应 (Best Fit)
- 释放后自动合并相邻空闲区

### 请求分页
- FIFO 先进先出置换
- LRU 最近最少使用置换
- 320 条指令按局部性规律生成
