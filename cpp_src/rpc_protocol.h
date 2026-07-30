#ifndef RPC_PROTOCOL_H
#define RPC_PROTOCOL_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <cstdint>

namespace temene {

struct RpcMessageHeader {
  uint32_t requestId;
  uint8_t messageType; // 0: Request, 1: Response, 2: Error
  uint32_t serviceId;
  uint32_t methodId;
  uint32_t payloadSize;
};

using RpcHandler = std::function<std::vector<uint8_t>(const std::vector<uint8_t>&)>;

class RpcServerDispatcher {
public:
  void registerMethod(uint32_t serviceId, uint32_t methodId, RpcHandler handler);
  std::vector<uint8_t> handleRpcRequest(const std::vector<uint8_t>& requestPacket);

private:
  std::unordered_map<uint64_t, RpcHandler> methodRegistry_;
  uint64_t makeKey(uint32_t sId, uint32_t mId) const {
    return (static_cast<uint64_t>(sId) << 32) | mId;
  }
};

} // namespace temene

#endif // RPC_PROTOCOL_H
