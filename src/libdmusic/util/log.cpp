#include "log.h"

// 分类名必须带 org.deepin. 前缀，与 desktop 名(deepin-music)及 DConfig appId 对应，
// 调试模式(deepin-debug-config)通过 DConfig log_rules 按该前缀动态过滤日志
Q_LOGGING_CATEGORY(dmMusic, "org.deepin.music")
