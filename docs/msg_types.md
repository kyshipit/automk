# AutoMLOS IPC消息类型定义规范

## 概述

本文档定义了AutoMLOS系统中使用的IPC消息类型规范，包括消息主题命名规则、数据结构定义、安全要求和最佳实践。

## 安全合规性要求

### 类型安全要求
- **枚举定义**：所有主题定义必须使用枚举类型而非字符串常量
- **参数验证**：主题构建函数必须包含参数验证和边界检查
- **长度限制**：主题字符串长度不得超过64字符

### 编译时检查
- **静态断言**：使用静态断言确保主题长度限制
- **边界检查**：枚举值必须连续且包含COUNT成员用于边界检查
- **命名空间验证**：保留命名空间必须进行运行时验证

### 错误处理机制
- **明确错误码**：主题验证失败必须返回明确的错误代码
- **安全返回值**：无效参数必须返回NULL而非空字符串
- **边界检查**：所有主题操作必须包含安全边界检查

## 消息主题命名规范

### 分层命名结构
采用三级分层命名：`domain/component/action`

- **一级域(domain)**：功能域，标识消息所属的功能模块
- **二级域(component)**：具体组件，标识消息来源的具体组件  
- **动作类型(action)**：标识消息的具体行为

### 命名规则
1. **小写字母**：全部使用小写字母
2. **斜杠分隔**：层级间使用斜杠(`/`)分隔
3. **语义明确**：名称应直接反映业务语义
4. **避免缩写**：除非是行业标准缩写
5. **类型安全**：优先使用枚举类型而非字符串常量

## 消息数据结构规范

### 通用消息头
```c
/**
 * @brief IPC消息通用头部结构
 * 
 * 所有IPC消息都必须包含此头部，用于消息路由和处理。
 * 结构体设计为64字节对齐，确保缓存友好性。
 */
typedef struct {
    msg_topic_t topic;           ///< 消息主题枚举值
    uint32_t source_pid;         ///< 发布者进程ID
    uint64_t timestamp;          ///< 时间戳（微秒精度）
    uint32_t sequence_number;    ///< 序列号（用于消息追踪）
    uint8_t priority;           ///< 优先级（0-255，0为最高）
    uint32_t data_size;         ///< 数据部分大小（字节）
    uint16_t checksum;          ///< 消息完整性校验和
    uint8_t version;            ///< 消息版本号
    uint8_t flags;              ///< 消息标志位
    uint8_t reserved[10];       ///< 保留字段（未来扩展）
} ipc_message_header_t;

// 消息标志位定义
#define IPC_FLAG_RELIABLE    (1 << 0)  ///< 可靠传输标志
#define IPC_FLAG_BROADCAST   (1 << 1)  ///< 广播消息标志
#define IPC_FLAG_URGENT      (1 << 2)  ///< 紧急消息标志
```

### 具体消息类型数据结构

#### 摄像头帧消息
```c
/**
 * @brief 摄像头帧消息数据结构
 * 
 * 用于传输摄像头采集的图像帧数据。
 * 图像数据跟随在结构体后，通过data_size字段指示大小。
 */
typedef struct {
    uint32_t frame_id;          ///< 帧序列号
    uint32_t width;             ///< 图像宽度（像素）
    uint32_t height;            ///< 图像高度（像素）
    uint32_t format;            ///< 图像格式（如RGB、YUV等）
    uint64_t capture_timestamp; ///< 采集时间戳
    uint32_t exposure_time;     ///< 曝光时间（微秒）
    uint16_t gain;              ///< 增益值
    uint8_t quality;           ///< 图像质量（0-100）
    uint8_t reserved[3];       ///< 保留字段
} camera_frame_header_t;

// 图像格式枚举定义
typedef enum {
    IMAGE_FORMAT_RGB888 = 0,
    IMAGE_FORMAT_YUV420,
    IMAGE_FORMAT_GRAY8,
    IMAGE_FORMAT_RAW_BAYER,
    IMAGE_FORMAT_COUNT
} image_format_t;
```

#### AI检测结果消息
```c
/**
 * @brief AI目标检测结果数据结构
 * 
 * 用于传输AI模型推理的检测结果。
 * 支持多目标检测，每个目标包含边界框和置信度信息。
 */
typedef struct {
    uint32_t object_id;         ///< 目标唯一标识
    float confidence;           ///< 检测置信度（0.0-1.0）
    float x, y;                 ///< 边界框中心坐标
    float width, height;        ///< 边界框尺寸
    uint32_t class_id;          ///< 类别ID
    uint64_t detection_time;    ///< 检测时间戳
    uint16_t track_id;          ///< 跟踪ID（如适用）
    uint8_t class_name[32];     ///< 类别名称字符串
} detection_result_t;

/**
 * @brief AI检测结果消息头
 * 
 * 包含多个检测结果的完整消息。
 */
typedef struct {
    uint32_t frame_id;          ///< 对应的帧ID
    uint32_t result_count;      ///< 检测结果数量
    uint64_t inference_time;    ///< 推理耗时（微秒）
    float overall_confidence;   ///< 整体置信度
    uint8_t model_version[16];  ///< 模型版本标识
} ai_detection_header_t;
```

#### 车辆控制命令消息
```c
/**
 * @brief 车辆控制命令数据结构
 * 
 * 用于传输车辆执行器控制命令，确保实时性和安全性。
 */
typedef struct {
    float brake_pressure;       ///< 刹车压力（0.0-1.0）
    float steering_angle;       ///< 转向角度（弧度）
    float throttle_position;    ///< 油门位置（0.0-1.0）
    int8_t gear_position;       ///< 档位位置（-1:倒车,0:空档,1-6:前进档）
    uint32_t command_id;        ///< 命令序列号
    uint64_t command_time;      ///< 命令生成时间
    uint16_t safety_code;       ///< 安全校验码
    uint8_t emergency_level;    ///< 紧急级别（0-3）
    uint8_t reserved[3];        ///< 保留字段
} vehicle_command_t;
```

## 消息优先级和QoS策略

### 优先级定义
| 优先级 | 数值范围 | 适用消息类型 | 处理策略 |
|--------|----------|--------------|----------|
| 最高紧急 | 0-31 | 安全关键消息、紧急停止 | 立即处理，无缓冲 |
| 高优先级 | 32-63 | 控制命令、传感器数据 | 小缓冲，高优先级队列 |
| 普通优先级 | 64-191 | 业务消息、状态更新 | 默认队列，平衡处理 |
| 低优先级 | 192-255 | 调试信息、统计数据 | 大缓冲，可丢弃 |

### QoS服务质量策略
- **可靠传输**：关键消息设置RELIABLE标志，确保送达
- **时效性**：实时消息设置URGENT标志，优先处理
- **广播消息**：系统状态消息使用广播，减少重复发送
- **背压控制**：监控队列深度，防止消息堆积

## 消息处理最佳实践

### 消息发布
```c
// 示例：发布摄像头帧消息
int publish_camera_frame(const camera_frame_header_t* frame, const uint8_t* image_data) {
    // 参数安全检查
    if (frame == NULL || image_data == NULL) {
        return ERR_INVALID_PARAM;
    }
    
    // 计算消息总大小
    size_t total_size = sizeof(ipc_message_header_t) + 
                       sizeof(camera_frame_header_t) + 
                       frame->width * frame->height * 3; // RGB888
    
    // 分配消息内存
    uint8_t* message = malloc(total_size);
    if (message == NULL) {
        return ERR_MEMORY_ALLOC;
    }
    
    // 填充消息头
    ipc_message_header_t* header = (ipc_message_header_t*)message;
    header->topic = MSG_TOPIC_VISION_CAMERA_FRAME_READY;
    header->source_pid = get_current_pid();
    header->timestamp = get_system_time();
    header->priority = IPC_PRIORITY_HIGH;
    header->data_size = total_size - sizeof(ipc_message_header_t);
    
    // 发布消息
    int result = sys_ipc_publish(header);
    free(message);
    return result;
}
```

### 消息订阅和处理
```c
// 示例：订阅和处理AI检测结果
void ai_detection_handler(const ipc_message_header_t* header) {
    // 消息验证
    if (header == NULL || header->data_size < sizeof(ai_detection_header_t)) {
        log_error("Invalid AI detection message");
        return;
    }
    
    // 解析消息内容
    const ai_detection_header_t* detection_header = 
        (const ai_detection_header_t*)(header + 1);
    
    // 处理检测结果
    process_detection_results(detection_header);
    
    // 记录处理日志
    log_info("Processed AI detection for frame %u", detection_header->frame_id);
}
```

## 错误处理和重试机制

### 错误代码定义
```c
typedef enum {
    IPC_SUCCESS = 0,
    IPC_ERR_INVALID_TOPIC = -1,
    IPC_ERR_MESSAGE_TOO_LARGE = -2,
    IPC_ERR_QUEUE_FULL = -3,
    IPC_ERR_TIMEOUT = -4,
    IPC_ERR_PERMISSION_DENIED = -5,
    IPC_ERR_SYSTEM = -6
} ipc_error_t;
```

### 重试策略
- **立即重试**：临时错误（如队列满）
- **指数退避**：系统繁忙错误
- **不重试**：权限错误、无效参数

## 性能优化建议

### 消息大小优化
- **小消息**：<1KB的消息使用纯IPC传输
- **大消息**：>1KB的消息使用共享内存+小消息通知
- **批量处理**：高频小消息考虑批量发送

### 内存管理
- **预分配池**：高频消息类型使用内存池
- **零拷贝**：大数据使用引用计数避免拷贝
- **缓存友好**：结构体对齐到缓存行大小

## 调试和监控

### 消息追踪
```c
// 启用消息追踪
#define IPC_ENABLE_TRACING 1

#if IPC_ENABLE_TRACING
#define IPC_TRACE(msg) log_debug("IPC Trace: %s", msg)
#else
#define IPC_TRACE(msg)
#endif
```

### 性能监控指标
- **消息延迟**：发布到接收的时间差
- **吞吐量**：单位时间内处理的消息数量
- **队列深度**：各优先级队列的当前长度
- **错误率**：消息处理失败的比例

## 版本兼容性

### 向后兼容策略
- **扩展字段**：在结构体末尾添加新字段
- **版本号**：消息头包含版本号字段
- **默认值**：新字段提供合理的默认值
- **废弃机制**：标记废弃消息类型，提供迁移期

## 附录

### 消息类型速查表
| 一级域 | 二级域 | 消息类型 | 优先级 | 频率 | 数据大小 |
|--------|--------|----------|--------|------|----------|
| vision | camera | frame_ready | 高 | 高频 | 可变 |
| vehicle | control | brake_command | 最高 | 中频 | 固定 |
| ai | detection | obstacle | 高 | 高频 | 可变 |
| system | monitor | heartbeat | 中 | 低频 | 固定 |
| safety | fault | memory_access | 最高 | 低频 | 固定 |

### 相关文档链接
- [架构设计文档](architecture.md) - 系统整体架构
- [API参考文档](api_reference.md) - 系统API接口说明
- [开发指南](development_guide.md) - 应用开发指南

---

*本文档最后更新：2024年1月*
*维护团队：架构团队*