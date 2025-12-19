# AutoMLOS 系统架构设计文档

## 概述

AutoMLOS 是一个面向汽车电子系统的实时微内核操作系统，专为自动驾驶和高级驾驶辅助系统(ADAS)设计。系统采用模块化架构，确保功能安全(ASIL-D)和实时性能。
- **安全第一**：符合ISO 26262功能安全标准
- **实时性能**：微秒级中断延迟，确定性调度
- **模块化设计**：服务隔离，单一故障不扩散
- **可扩展性**：支持动态服务加载和卸载

## 系统架构总览

```mermaid
graph TB
    subgraph "硬件层"
        HW[硬件平台]
    end
    
    subgraph "内核层"
        KERNEL[微内核<br/>kernel/core/]
        IPC[IPC子系统<br/>kernel/core/ipc.c]
        SCHED[调度器<br/>kernel/core/scheduler.c]
    end
    
    subgraph "用户态服务层"
        subgraph "安全管控服务"
            LOGGER[日志服务<br/>services/logger/]
            MONITOR[系统监控<br/>services/system_monitor/]
            SAFETY[安全框架<br/>lib/safety/]
            DIAG[诊断服务<br/>services/diagnostic_server/]
        end
        
        subgraph "实时业务服务"
            AI[AI推理服务<br/>services/ai_server/]
            DM[设备管理器<br/>services/device_manager/]
            FS[文件系统<br/>services/filesystem/]
            APP[控制应用<br/>apps/vehicle_ctrl/]
        end
        
        subgraph "用户态驱动"
            CAM[摄像头驱动<br/>udrivers/camera/]
            CAN[CAN驱动<br/>udrivers/can/]
            POWER[电源驱动<br/>udrivers/power/]
        end
    end
    
    HW --> KERNEL
    KERNEL --> IPC
    IPC --> LOGGER
    IPC --> MONITOR
    IPC --> SAFETY
    IPC --> DIAG
    IPC --> AI
    IPC --> DM
    IPC --> FS
    IPC --> APP
    DM --> CAM
    DM --> CAN
    DM --> POWER
```
## 性能指标

| 指标 | 目标值 | 当前状态 | 备注 |
|------|--------|----------|------|
| 中断延迟 | <10µs | 待测试 | 微内核设计目标 |
| 上下文切换 | <5µs | 待测试 | 进程间切换 |
| IPC消息延迟 | <20µs | 待测试 | 内核路由延迟 |
| 内存使用 | <2MB | 待测试 | 核心服务内存 |
| 启动时间 | <100ms | 待测试 | 冷启动到就绪 |


### 功能安全 (ISO 26262)
- **ASIL-D兼容**：最高安全完整性等级
- **故障检测**：运行时错误检测和报告
- **安全状态**：故障时进入安全状态
- **诊断覆盖**：完整的诊断和监控机制

### 信息安全
- **内存保护**：进程隔离，防止越界访问
- **通信安全**：消息完整性检查，防止篡改
- **访问控制**：基于角色的服务访问权限

## 核心特性
**1.  汽车级功能安全 (ASIL-D Ready)**
- 架构级隔离：每个服务/驱动为独立进程，单一故障不会扩散。

- 时间确定性：微内核极小（<10µs中断延迟），调度器支持时间分区。

- 软件各模块解耦，日志只记录不分析，四级日志系统：

    - L0 (紧急)：内核及安全关键事故，同步存储于安全内存。

    - L1 (错误)：系统级错误，高可靠性存储。

    - L2 (信息)：业务流水线日志，循环存储。

    - L3 (调试)：开发调试信息，可配置丢弃。

**2.  深度集成的实时TinyML**
- 专用服务：ai_server 管理模型生命周期与推理任务调度。

- 内存优化：lib/ai 包含张量内存池、算子融合，峰值内存<512KB。

- 车规适配：支持-40°C~125°C温度范围的精度补偿。

**3.  符合AUTOSAR标准的诊断栈**
- 完整UDS：lib/protocol/uds 实现ISO-14229，diagnostic_server 处理会话安全。

- DTC管理：完整的诊断故障码生命周期管理与冻结帧存储。

- DoIP支持：通过network服务支持基于以太网的诊断。

**4.  高性能纯IPC通信架构**
- 零共享内存（默认）：所有通信通过IPC消息传递，确保数据流清晰、可审计。

- 大数据的优化路径：对摄像头帧等大数据，采用“共享内存+IPC小消息通知”的混合模式，由device_manager管理。

- 背压控制：IPC通道内置流控，防止快速生产者淹没慢速消费者。


## 核心工作机制

### IPC消息机制
autoMLOS采用基于**主题（Topic）的IPC消息机制**，而非传统的事件机制，确保通信的高效性和确定性。
- **消息机制**：基于主题的发布-订阅模式，内核提供统一路由，传统机制通常需要复杂的回调注册和分发逻辑
- **设计选择**：采用消息机制实现更好的解耦和性能
- **集中路由**：所有消息通过内核路由表统一分发
- **策略路由**：支持广播、单播、组播等多种路由策略
- **扩展友好**：支持动态主题注册和发现
- **深度拷贝**：消息数据在内核层深度拷贝，确保隔离安全
- **零共享内存**：默认使用纯IPC消息传递
- **大数据优化**：摄像头帧等大数据采用"共享内存+IPC小消息通知"混合模式
- **背压控制**：IPC通道内置流控机制
- **消息处理流程**
    ```
    发布者 → sys_ipc_publish() → 内核路由表 → 深度拷贝 → 并行投递 → 订阅者
    ``` 
- **分层命名**：主题名称直接反映业务语义，`domain/component/action`格式（如`vision/camera/frame_ready`）
- **通信机制：IPC消息机制**
    详细的消息类型定义请参考：[消息类型定义规范](msg_types.md)

### 系统设计完整序列图

- 系统上电 → 硬件看门狗启动 → BSP时钟初始化 → 内核启动 → 用户态服务启动
- 微内核启动 → Logger → Monitor → DeviceManager → UDrivers → 业务服务

```mermaid
 sequenceDiagram
    participant HW as 硬件层
    box AliceBlue 用户态驱动进程
    participant DRV_CAM as Camera驱动
    participant DRV_CAN as CAN驱动
    participant DRV_PWR as 电源驱动
    end
    box Honeydew 实时业务服务
    participant DM as 设备管理器
    participant AI as AI推理服务
    participant FS as 文件系统服务
    participant APP as 控制应用
    end
    box Cornsilk  安全管控服务
    participant LOG as 日志服务
    participant MON as 系统监控
    participant SAFE as 安全框架
    participant DIAG as 诊断服务
    participant PWRM as 电源管理服务
    end

    Note over HW,PWRM: 阶段 0：系统启动与初始化 (自上而下，严格顺序)
    Note over MON,DIAG: 核心服务启动顺序: 日志 -> 监控 -> 设备管理 -> 驱动 -> 业务服务
    LOG->>LOG: 1. 初始化，注册IPC端口
    MON->>LOG: 2. 启动，记录日志并注册健康监控
    DM->>LOG: 3. 启动，注册设备接口
    DM->>DRV_CAM: 4. 启动驱动进程 (通过IPC)
    DM->>DRV_CAN: 4. 启动驱动进程
    DRV_CAM->>LOG: 5. 驱动启动日志
    AI->>LOG: 6. 业务服务启动
    DIAG->>LOG: 6. 业务服务启动

    Note over HW,PWRM: 阶段 1：正常业务流 - 感知与决策 (绿色路径)
    HW->>DRV_CAM: 1.1 硬件中断 (图像帧)
    DRV_CAM->>DM: 1.2 IPC: 发送原始数据
    DM->>LOG: 1.3 记录L2日志 (数据接收)
    DM->>DM: 1.4 解析并发布“Frame_Ready”消息
    DM->>AI: 1.5 IPC: 消息送达 (内核路由)
    AI->>FS: 1.6 IPC: 加载模型文件 (可选)
    AI->>AI: 1.7 调用lib/ai进行推理
    AI->>LOG: 1.8 记录L2日志 (推理结果)
    AI->>APP: 1.9 IPC: 发布“Obstacle_Detected”消息

    Note over HW,PWRM: 阶段 2：正常业务流 - 控制与执行
    APP->>APP: 2.1 计算控制指令
    APP->>DM: 2.2 IPC: 调用“刹车”设备接口
    DM->>LOG: 2.3 记录L2日志 (控制指令)
    DM->>DRV_CAN: 2.4 IPC: 发送CAN控制帧 (至执行器)
    DRV_CAN->>HW: 2.5 写入寄存器，输出信号

    Note over HW,PWRM: 阶段 3：伴随的安全监控流 (橙色路径，与阶段1/2并行)
    par 并行心跳
        MON->>DRV_CAM: 3.1 心跳请求
        DRV_CAM->>MON: 心跳响应
        MON->>AI: 3.2 心跳请求
        AI->>MON: 心跳响应
        MON->>APP: 3.3 心跳请求
        APP->>MON: 心跳响应
    end
    MON->>LOG: 3.4 记录心跳状态 (L3调试日志)

    Note over HW,PWRM: 阶段 4：异常处理流 - AI服务故障
    AI-->>AI: 4.1 模拟：推理超时/崩溃
    AI->>SAFE: 4.2 IPC: 上报严重错误 (ERR_TIMEOUT)
    SAFE->>LOG: 4.3 IPC: 紧急记录L1错误日志
    SAFE->>MON: 4.4 IPC: 通知“AI服务异常”
    MON->>AI: 4.5 IPC: 发送“优雅终止”信号
    MON->>DM: 4.6 IPC: 暂停向AI服务发送数据
    MON->>MON: 4.7 启动新的AI服务实例
    MON->>DM: 4.8 IPC: 恢复数据流至新实例
    MON->>LOG: 4.9 记录L1恢复日志

    Note over HW,PWRM: 阶段 5：诊断系统介入 - 故障记录与外部查询
    SAFE->>DIAG: 5.1 IPC: 转发故障为“诊断故障”
    DIAG->>DIAG: 5.2 设置对应DTC状态为Active
    DIAG->>LOG: 5.3 记录L2诊断日志 (DTC激活)
    Note over DIAG,DIAG: 外部诊断仪连接...
    DIAG->>DIAG: 5.4 接收UDS请求 (e.g., 0x1902读DTC)
    DIAG->>DM: 5.5 IPC: 调用API，读取相关传感器状态
    DM->>DIAG: 5.6 IPC: 返回数据
    DIAG->>DIAG: 5.7 组织UDS响应帧
    DIAG->>DM: 5.8 IPC: 请求发送响应
    DM->>DRV_CAN: 5.9 IPC: 转发至CAN驱动发送

    Note over HW,PWRM: 阶段 6：电源管理示例 - 响应休眠请求
    PWRM->>PWRM: 6.1 根据策略决定进入休眠
    PWRM->>APP: 6.2 IPC: 通知“系统即将休眠”
    APP->>APP: 6.3 保存状态，准备休眠
    APP->>PWRM: 6.4 IPC: 确认准备就绪
    PWRM->>MON: 6.5 IPC: 请求暂停非核心服务监控
    PWRM->>DRV_PWR: 6.6 IPC: 发送休眠指令
    DRV_PWR->>HW: 6.7 配置硬件进入低功耗状态
```
这张图通过 6个阶段 和 3个并行泳道，系统地串联了所有模块：

1. 严格的分层与启动顺序（阶段0）

    - 直观展示了 “基础设施先行” 的原则：日志服务和系统监控必须先于所有业务服务启动，这是构建可观测和可恢复系统的前提。

2. 业务流与安全流解耦且并行（阶段1-3）

    - 绿色路径（阶段1-2） 是主线业务，代表车辆功能（感知-决策-控制）。

    - 橙色路径（阶段3） 是安全监控，代表系统的“免疫系统”。它与业务流异步并行，通过定期心跳进行检查，确保不干扰主功能的实时性。

3. 完整的故障生命周期处理（阶段4-5）

    - 从 故障发生 (AI服务超时) -> 内部上报与决策 (安全框架) -> 自动恢复 (系统监控重启服务) -> 符合标准的记录 (诊断服务生成DTC) -> 外部可访问 (诊断仪查询)，形成了一个符合汽车功能安全（ISO 26262）要求的闭环。

4. 横切关注点的体现（电源管理，阶段6）

    - 展示了电源管理服务如何作为一个横跨业务与安全域的协调者，它需要通知应用保存状态、调整监控策略，并最终命令驱动执行硬件操作。


### 核心工作总览序列图 (感知-决策-控制主链)
```mermaid
sequenceDiagram
    participant H as 硬件<br>(Camera Sensor)
    participant D as Camera驱动<br>(用户态进程)
    participant DM as 设备管理器<br>(Device Manager)
    participant AI as AI推理服务<br>(AI Server)
    participant APP as 车辆控制应用<br>(Vehicle Control)
    participant MON as 系统监控<br>(System Monitor)
    participant LOG as 日志服务<br>(Logger)
    participant S as 安全框架<br>(Safety Framework)

    Note over H,APP: 主线：实时业务数据流 (绿色路径)
    H->>D: 硬件中断 (帧数据就绪)
    D->>DM: IPC: 发送原始帧数据
    DM->>DM: 1. 调用lib/protocol基础解析<br>2. 发布主题消息(“vision/frame_ready”)
    DM->>AI: IPC: 消息送达 (内核路由)
    AI->>AI: 调用 lib/ai 执行推理计算
    AI->>APP: IPC: 发布消息(“detection/obstacle”)
    APP->>APP: 规划控制指令 (计算刹车力度)
    APP->>DM: IPC: 调用设备API (“紧急刹车”)
    DM->>D: IPC: 转发控制命令 (至执行器驱动，图略)

    Note over D,S: 辅线：伴随的安全监控与日志流 (橙色路径)
    D->>LOG: IPC: 记录L2日志(“帧已捕获”)
    DM->>LOG: IPC: 记录L2日志(“消息已发布”)
    AI->>LOG: IPC: 记录L2日志(“推理完成，置信度XX”)
    
    par 并行监控
        MON->>D: 周期性心跳请求 (IPC)
        D->>MON: 心跳响应
        MON->>AI: 周期性心跳请求 (IPC)
        AI->>MON: 心跳响应
        MON->>APP: 周期性心跳请求 (IPC)
        APP->>MON: 心跳响应
    end

    Note over H,APP: 异常情况模拟：AI服务处理超时
    AI-->>AI: 模拟：推理计算超时 (故障)
    AI->>S: IPC: 上报错误 (ERR_TIMEOUT)
    S->>LOG: IPC: 记录L1错误日志(“AI服务超时”)
    S->>MON: IPC: 通知服务异常
    MON->>AI: IPC: 发送“优雅重启”请求
    MON->>DM: IPC: 暂停向AI服务发送数据
    MON->>MON: 启动新的AI服务实例
    MON->>DM: IPC: 恢复数据流至新实例
    MON->>LOG: IPC: 记录L1信息日志(“AI服务已恢复”)
```
- 垂直时间线：展示了调用发生的精确顺序。

- 绿色路径：是不可中断的实时业务主线，要求低延迟。

- 橙色路径：是异步并行的安全监控与日志流，它们伴随主线发生，但不应阻塞主线。其中系统监控的心跳检查是独立、周期性的。

- 异常处理：展示了当AI服务发生超时错误时，安全框架和系统监控如何介入并完成故障恢复，体现了故障隔离与自愈能力。


### 诊断与错误处理序列图
```mermaid
sequenceDiagram
    participant D as CAN驱动
    participant DM as 设备管理器
    participant DIAG as 诊断服务
    participant SF as 安全框架
    participant MON as 系统监控
    participant LOG as 日志服务

    Note over D,LOG: 阶段一：故障检测与首次上报
    D->>D: 检测到与ECU_0x123通信超时
    D->>LOG: IPC: 记录L1错误日志(“CAN通信超时”)
    D->>SF: IPC: 上报硬件错误(ERR_HW_CAN_TIMEOUT)

    Note over SF,DIAG: 阶段二：安全框架评估与分发
    SF->>SF: 评估：非致命，可恢复，需记录
    SF->>LOG: IPC: 命令持久化该错误日志(L1)
    SF->>MON: IPC: 通知“CAN驱动状态异常”
    SF->>DIAG: IPC: 报告“诊断事故”(DemAccident)

    Note over DIAG,DIAG: 阶段三：诊断服务处理
    DIAG->>DIAG: 1. 根据消息匹配诊断协议<br>2. 设置DTC状态为“Active”<br>3. 存储冻结帧(故障快照)
    DIAG->>LOG: IPC: 记录L2信息日志(“DTC 0xU0123已激活”)

    Note over MON,DM: 阶段四：系统监控执行恢复 (与阶段三并行)
    MON->>D: IPC: 发送“复位通道”指令
    D->>D: 执行硬件复位序列
    D->>MON: IPC: 响应“复位完成”
    MON->>LOG: IPC: 记录L2信息日志(“CAN通道已恢复”)

    Note over DIAG,DM: 阶段五：外部诊断工具介入 (故障发生后)
    DIAG->>DIAG: 接收外部诊断仪UDS请求(0x1902读DTC)
    DIAG->>DM: IPC: 调用API，读取相关传感器状态
    DM->>DIAG: IPC: 返回传感器数据
    DIAG->>DIAG: 组织UDS响应帧(包含DTC信息)
    DIAG->>D: (通过DM) IPC: 发送响应帧至CAN驱动
```
- 流程闭环：完整展示了从故障发生 -> 内部记录与评估 -> 可能的自动恢复 -> 外部可诊断的完整生命周期。

- 角色清晰：

    - 安全框架是决策中枢，决定消息严重性和路由方向。

    - 诊断服务是协议专家，负责将内部消息转化为标准的DTC和诊断响应。

    - 系统监控是执行者，负责实施具体的恢复动作。

    - 日志服务是记录员，提供全流程审计追踪。

- 符合标准：此流程严格遵循AUTOSAR中诊断消息管理(DEM)和诊断通信(DCM)的逻辑。



### 消息路由器工作流程
```mermaid
sequenceDiagram
    participant P as 发布者进程
    participant K_IPC as kernel/core/ipc.c
    participant K_Table as 内核路由表
    participant S1 as 订阅者1
    participant S2 as 订阅者2

    Note over P,S2: 步骤1：订阅（建立路由）
    S1->>K_IPC: sys_ipc_subscribe(“CAMERA_FRAME”, port_1)
    S2->>K_IPC: sys_ipc_subscribe(“CAMERA_FRAME”, port_2)
    K_IPC->>K_Table: 记录主题与端口映射
    
    Note over P,S2: 步骤2：发布（一对多路由）
    P->>K_IPC: sys_ipc_publish(“CAMERA_FRAME”, data)
    K_IPC->>K_Table: 查找订阅者列表
    K_IPC->>K_IPC: 深度拷贝消息数据（隔离）
    
    par 并行投递
        K_IPC->>S1: 投递至 port_1
        K_IPC->>S2: 投递至 port_2
    end
```


### 车辆故障诊断（UDS 诊断服务）序列图
```mermaid
sequenceDiagram
    participant T as 外部诊断仪
    participant A as apps/诊断应用
    participant S as services/诊断服务
    participant L as lib/UDS协议库
    participant U as udrivers/CAN驱动
    participant K as kernel/
    participant E as 其他ECU

    Note over T,E: 场景：读取发动机故障码(DTC)
    
    T->>A: 1. 物理连接或网络连接
    A->>S: 2. IPC: 启动诊断会话(0x10 01)
    activate S
    
    S->>L: 3. 构造UDS请求帧(22 F1 90)
    activate L
    L-->>S: 4. 返回完整UDS报文
    deactivate L
    
    S->>U: 5. IPC: 发送CAN帧(ID:0x7E0, Data:[02 22 F1 90])
    activate U
    U->>K: 6. 系统调用: 写入CAN总线
    K-->>U: 7. 发送确认
    U-->>S: 8. 发送完成
    deactivate U
    
    par 等待响应
        U->>K: 9. 监听CAN总线(中断/轮询)
        K-->>U: 10. 收到CAN帧
        U->>S: 11. IPC: 转发CAN帧(ID:0x7E8, Data:[06 62 F1 90 12 34])
    and 超时监控
        S->>S: 12. 启动P2超时计时器
    end
    
    S->>L: 13. 解析UDS响应(提取故障码:0x1234)
    activate L
    L-->>S: 14. 返回解析结果
    deactivate L
    
    S->>A: 15. IPC: 返回诊断结果
    A->>T: 16. 显示故障码: P1234
    deactivate S
```


### AI服务处理中发生内存访问错误序列图
```mermaid
sequenceDiagram
    participant A as AI服务
    participant SF as 安全框架
    participant LOG as 日志服务
    participant MON as 系统监控
    participant DIAG as 诊断服务
    participant DM as 设备管理器

    Note over A,DM: 阶段1：故障发生与捕获
    A->>A: 发生非法的内存访问
    A->>SF: 立即上报错误(ERROR_MEMORY_FAULT, pid, addr)
    A->>LOG: 记录L1错误日志(“AI服务内存故障”)
    
    Note over SF,DIAG: 阶段2：安全框架决策与分发
    SF->>SF: 评估: 非致命，可恢复
    SF->>LOG: 命令: 持久化该错误日志
    SF->>DIAG: 通知: 生成一个诊断消息(DEM)
    SF->>MON: 通知: AI服务状态异常
    
    Note over MON,DM: 阶段3：监控与恢复执行
    MON->>A: 发送“优雅重启”请求(IPC)
    A->>A: 清理推理状态，保存上下文
    A->>MON: 确认，准备退出
    MON->>DM: 通知: AI服务即将重启，暂停数据流
    MON->>MON: 启动新的AI服务实例
    MON->>DM: 通知: 恢复数据流至新实例
    
    Note over DIAG,DIAG: 阶段4：诊断记录
    DIAG->>DIAG: 根据消息设置DTC状态(0xP1234)
    DIAG->>DIAG: 保存故障发生时的冻结帧
    DIAG->>LOG: 记录诊断操作日志
```

### 日志服务

autoMLOS采用四级日志分类系统，每个级别都与资源分配、处理策略和功能安全(ASIL)强关联，确保系统在可靠性、性能和安全性之间取得最佳平衡。

| 级别 | 名称 | 产生源 | 核心目的 | 处理策略（可靠性/性能权衡） |
|------|------|--------|----------|----------------------------|
| **L0** | 紧急/安全 (EMERGENCY/SAFETY) | 安全框架（错误上报+恢复策略）、关键驱动、看门狗 | 功能安全，记录导致或即将导致系统失效的事故 | **最高可靠**：同步IPC、无缓冲、必须确认、永久存储、禁止覆盖 |
| **L1** | 错误/内核 (ERROR/KERNEL) | 内核、服务管理、资源耗竭 | 系统健康，记录影响功能但未造成安全风险的事故 | **高可靠**：缓冲队列小、高优先级发送、必须存储 |
| **L2** | 信息/流水线 (INFO/PIPELINE) | 诊断服务、AI推理 | 业务审计与追溯，记录正常但重要的业务流水 | **平衡模式**：使用默认缓冲队列、异步确认、循环存储（保留最近24小时） |
| **L3** | 调试/分析 (DEBUG/TRACE) | 任何模块，用于开发调试 | 问题定位与性能分析 | **性能优先**：大缓冲、可丢弃、可开关、可存储于内存文件系统 |

**各级别详细说明**

**L0 - 紧急/安全级别**
- **功能安全要求**：符合ASIL-D标准，确保关键安全事故100%记录
- **存储策略**：写入安全内存区域，禁止覆盖，永久保存
- **传输机制**：同步IPC通信，无缓冲，发送后必须收到确认
- **典型应用**：看门狗超时、安全框架错误上报、关键驱动故障

**L1 - 错误/内核级别**  
- **系统健康监控**：记录系统级错误但未达到安全风险级别
- **存储策略**：高可靠性存储，确保错误信息不丢失
- **传输机制**：小缓冲队列，高优先级发送，必须存储到持久化介质
- **典型应用**：内核异常、服务管理错误、资源耗尽警告

**L2 - 信息/流水线级别**
- **业务审计**：记录正常业务流水，用于事后分析和追溯
- **存储策略**：循环存储策略，保留最近24小时数据
- **传输机制**：默认缓冲队列，异步确认机制
- **典型应用**：诊断服务操作、AI推理结果、设备状态变化

**L3 - 调试/分析级别**
- **开发调试**：用于开发和问题定位，生产环境可选择性关闭
- **存储策略**：可配置存储位置，支持内存文件系统
- **传输机制**：大缓冲队列，可丢弃策略，性能优先
- **典型应用**：性能分析、调试跟踪、详细运行状态

**分级处理策略的优势**

1. **资源优化**：不同级别的日志采用不同的资源分配策略，避免低优先级日志占用关键资源
2. **性能保证**：L0和L1级别的高可靠性要求不会影响L2和L3级别的性能
3. **安全合规**：严格的分级处理满足汽车功能安全标准要求
4. **灵活配置**：可根据实际需求调整各级别的存储和传输策略

### 日志数据流向
假设 services/diagnostic_server 需要记录一条 “已解析UDS 0x22请求” 的日志，整个流程严格遵循生产者-消费者模型，并隔离了解析与记录。
```mermaid
sequenceDiagram
    participant A as srv/diag_service<br>(业务服务)
    participant L as lib/logging<br>(日志客户端库)
    participant K as Kernel IPC
    participant S as srv/logger<br>(日志服务)
    
    Note over A,S: 第一步：业务侧产生并格式化日志（记录但不解析）
    A->>A: 1. 业务逻辑完成UDS解析<br>（获得结构化数据）
    A->>A: 2. 调用 lib/logging API<br>（如 log_info()）
    A->>A: 3. 在进程内格式化日志：<br>“时间戳 [INFO] UDS 0x22 Req: PID=0xF123”
    
    Note over A,S: 第二步：库层可靠发送（仅负责传输）
    A->>L: 4. 调用 lib/logging 发送函数
    L->>L: 5. 可选的进程内缓冲（防阻塞）
    L->>K: 6. 通过IPC发送<br>（仅传递格式化后的完整字符串/消息块）
    
    Note over A,S: 第三步：服务层存储与策略管理（不解析内容）
    K->>S: 7. IPC传递消息
    S->>S: 8. 接收并追加到<br>内存缓冲队列
    S->>S: 9. 应用策略：<br> - 过滤（根据级别）<br> - 分类（根据来源标签）
    S->>S: 10. 异步写入存储<br>（文件/网络/内存）
```
此设计的精髓在于 “只记录不解析” 原则，它通过控制权转移来实现：

- 日志的“内容”（What）：由生产者（如 diag_service）在调用 lib/logging 之前就完全决定和格式化。内容是业务语义（“解析了UDS 0x22”）。

- 日志的“处理方式”（How）：由消费者（srv/logger）统一决定。它看到的是一个带有元数据（如级别、标签）的不透明字符串块，它只负责将其可靠存储或转发，不会也不能去解析字符串内的业务含义（如识别“0x22”是UDS服务）。

最终带来的系统级优势：

1. 安全与可靠：srv/logger 服务极其稳定，因为它与任何业务逻辑变更无关。即使 UDS 协议栈升级，日志服务也无需改动。

2. 高效：最耗时的格式化工作分散在各个业务进程，IPC只传递最终结果，系统总吞吐量高。

3. 清晰与可维护：开发者明确知道：业务逻辑相关的日志内容，在业务服务中生成；日志的存储和输出策略，在 logger 服务中配置。


## 目录结构概览
```
autoMLOS/
├── kernel/                # 微内核核心（仅机制，<10K行代码）
│   ├── core/              # 四大核心机制
│   │   ├── scheduler.c    # 确定性实时调度器
│   │   ├── ipc.c          # IPC原语与内置消息路由
│   │   ├── memory.c       # 虚拟与物理内存管理
│   │   ├── interrupt.c    # 中断接管与用户态通知框架
│   │   └── syscall.c      # 系统调用入口
│   └── arch/              # CPU架构抽象
├── udrivers/              # 用户态设备驱动（独立进程）
│   ├── can/               # CAN总线驱动
│   ├── camera/            # 摄像头驱动
│   ├── spi/, i2c/, uart/  # 标准总线驱动
│   ├── watchdog/          # 硬件看门狗驱动
│   └── power/             # 电源管理驱动
├── lib/                   # 无状态功能库（可独立测试）
│   ├── libsyscall/        # 系统调用封装库
│   ├── logging/           # 日志客户端库（支持L0-L3四级）
│   ├── protocol/          # 协议栈：CAN, UDS, SOME/IP
│   ├── ai/                # TinyML推理引擎库
│   ├── diagnostics/       # 诊断协议库（DTC, DEM）
│   └── safety/            # 安全框架（错误上报+恢复策略模板）
├── services/              # 用户态系统服务（策略与资源管理）
│   ├── device_manager/    # 核心：设备统一抽象与消息路由枢纽
│   ├── diagnostic_server/ # UDS/DoIP诊断服务
│   ├── ai_server/         # AI推理任务服务
│   ├── logger/            # 日志收集、存储、转发服务
│   ├── system_monitor/    # 系统健康监控与恢复执行
│   ├── power_manager/     # 电源策略服务
│   ├── filesystem/        # 文件系统服务（用户态，非内核）
│   └── network/           # 网络协议栈服务
├── apps/                  # 应用程序
│   ├── vehicle_ctrl/      # 车辆控制应用
│   └── hmi/               # 人机交互应用
├── platform/              # 硬件平台适配
│   ├── bsp/               # 板级支持包
│   └── config/            # 系统配置
├── tests/                 # 测试框架
├── docs/                  # 文档
├── tools/                 # 开发工具
└── scripts/               # 构建脚本
```
- 清晰分层：严格区分内核机制、用户态驱动、系统服务和应用程序，职责单一。
- 独立模块：每个功能模块（如日志、诊断、AI）均为独立库和服务，便于测试和复用。
- 可扩展性：新增设备或服务只需添加对应目录，符合开闭原则。
- 易维护性：目录结构直观，便于开发者快速定位代码位置

### 软件配置
- **编译工具链**：GCC for ARM，支持C11标准
- **构建系统**：GNU Make，支持模块化编译
- **调试工具**：GDB，支持远程调试和核心转储

## 相关文档
- [IPC消息类型定义规范](msg_types.md) - 详细的消息类型定义
- [API参考文档](api_reference.md) - 系统API接口说明
- [开发指南](development_guide.md) - 应用开发指南

---

