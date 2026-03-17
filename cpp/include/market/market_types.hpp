#pragma once

#include <cstdint>
#include <string>

namespace fq::market {

// TickSource:
// 用于标记一条行情来自哪个上游通道，便于调试和质量分析。
// - WebSocket: 主通道，实时性更高
// - RestFallback: 兜底通道，频率低但更稳
enum class TickSource : uint8_t {
    WebSocket = 1,
    RestFallback = 2,
};

// MarketTick:
// 行情模块对下游暴露的统一数据结构。
// 后续 Risk/OMS/Strategy 只依赖这个结构，不直接关心 Binance 原始 JSON。
struct MarketTick {
    std::string symbol;      // 交易对，如 BTCUSDT
    double price{0.0};       // 最新成交价
    uint64_t exchange_ts{0}; // 交易所事件时间（毫秒），若无则为 0
    uint64_t recv_ts{0};     // 本地接收时间（毫秒）
    TickSource source{TickSource::WebSocket};
};

} // namespace fq::market
