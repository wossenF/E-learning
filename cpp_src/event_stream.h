#ifndef EVENT_STREAM_H
#define EVENT_STREAM_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <queue>
#include <iostream>

namespace temene {

struct StreamEvent {
  std::string streamId;
  uint64_t timestampMs;
  std::string key;
  std::string value;
};

using EventConsumer = std::function<void(const StreamEvent&)>;

enum class WindowType { TUMBLING, SLIDING };

struct WindowConfig {
  WindowType type{WindowType::TUMBLING};
  uint64_t windowSizeMs{5000};
  uint64_t slideIntervalMs{1000};
};

struct WindowResult {
  uint64_t windowStartMs;
  uint64_t windowEndMs;
  size_t eventCount;
  double aggregatedSum;
};

class EventStreamEngine {
public:
  void subscribe(const std::string& streamId, EventConsumer consumer);
  void publish(const StreamEvent& event);

  void registerTumblingWindow(const std::string& streamId, uint64_t windowSizeMs, std::function<void(const WindowResult&)> callback);

  void processWatermark(uint64_t currentTimestampMs);

private:
  std::unordered_map<std::string, std::vector<EventConsumer>> consumers_;
  std::unordered_map<std::string, std::vector<StreamEvent>> eventBuffers_;
  std::unordered_map<std::string, WindowConfig> windowConfigs_;
  std::unordered_map<std::string, std::function<void(const WindowResult&)>> windowCallbacks_;
};

} // namespace temene

#endif // EVENT_STREAM_H
