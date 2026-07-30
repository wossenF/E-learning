#include "event_stream.h"
#include <algorithm>

namespace temene {

void EventStreamEngine::subscribe(const std::string& streamId, EventConsumer consumer) {
  consumers_[streamId].push_back(consumer);
}

void EventStreamEngine::publish(const StreamEvent& event) {
  eventBuffers_[event.streamId].push_back(event);

  auto it = consumers_.find(event.streamId);
  if (it != consumers_.end()) {
    for (const auto& consumer : it->second) {
      consumer(event);
    }
  }
}

void EventStreamEngine::registerTumblingWindow(
  const std::string& streamId,
  uint64_t windowSizeMs,
  std::function<void(const WindowResult&)> callback
) {
  windowConfigs_[streamId] = {WindowType::TUMBLING, windowSizeMs, windowSizeMs};
  windowCallbacks_[streamId] = callback;
}

void EventStreamEngine::processWatermark(uint64_t currentTimestampMs) {
  for (auto& [streamId, events] : eventBuffers_) {
    auto configIt = windowConfigs_.find(streamId);
    auto cbIt = windowCallbacks_.find(streamId);

    if (configIt == windowConfigs_.end() || cbIt == windowCallbacks_.end()) continue;

    const auto& config = configIt->second;
    const auto& callback = cbIt->second;

    std::sort(events.begin(), events.end(), [](const StreamEvent& a, const StreamEvent& b) {
      return a.timestampMs < b.timestampMs;
    });

    if (events.empty()) continue;

    uint64_t wStart = events.front().timestampMs;
    uint64_t wEnd = wStart + config.windowSizeMs;

    while (wEnd <= currentTimestampMs) {
      size_t count = 0;
      double sum = 0.0;

      auto evIt = events.begin();
      while (evIt != events.end()) {
        if (evIt->timestampMs >= wStart && evIt->timestampMs < wEnd) {
          count++;
          try {
            sum += std::stod(evIt->value);
          } catch (...) {}
          evIt = events.erase(evIt);
        } else if (evIt->timestampMs < wStart) {
          evIt = events.erase(evIt);
        } else {
          ++evIt;
        }
      }

      callback({wStart, wEnd, count, sum});
      wStart = wEnd;
      wEnd = wStart + config.windowSizeMs;
    }
  }
}

} // namespace temene
