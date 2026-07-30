#include "rpc_protocol.h"
#include <cstring>
#include <stdexcept>

namespace temene {

void RpcServerDispatcher::registerMethod(uint32_t serviceId, uint32_t methodId, RpcHandler handler) {
  methodRegistry_[makeKey(serviceId, methodId)] = handler;
}

std::vector<uint8_t> RpcServerDispatcher::handleRpcRequest(const std::vector<uint8_t>& requestPacket) {
  if (requestPacket.size() < sizeof(RpcMessageHeader)) {
    throw std::runtime_error("RPC packet too small for header.");
  }

  RpcMessageHeader header;
  std::memcpy(&header, requestPacket.data(), sizeof(RpcMessageHeader));

  if (requestPacket.size() < sizeof(RpcMessageHeader) + header.payloadSize) {
    throw std::runtime_error("RPC packet truncated.");
  }

  std::vector<uint8_t> payload(
    requestPacket.begin() + sizeof(RpcMessageHeader),
    requestPacket.begin() + sizeof(RpcMessageHeader) + header.payloadSize
  );

  auto it = methodRegistry_.find(makeKey(header.serviceId, header.methodId));
  std::vector<uint8_t> responsePayload;

  RpcMessageHeader respHeader;
  respHeader.requestId = header.requestId;

  if (it != methodRegistry_.end()) {
    responsePayload = it->second(payload);
    respHeader.messageType = 1; // Response
  } else {
    respHeader.messageType = 2; // Error
  }

  respHeader.serviceId = header.serviceId;
  respHeader.methodId = header.methodId;
  respHeader.payloadSize = static_cast<uint32_t>(responsePayload.size());

  std::vector<uint8_t> responsePacket(sizeof(RpcMessageHeader) + responsePayload.size());
  std::memcpy(responsePacket.data(), &respHeader, sizeof(RpcMessageHeader));
  if (!responsePayload.empty()) {
    std::memcpy(responsePacket.data() + sizeof(RpcMessageHeader), responsePayload.data(), responsePayload.size());
  }

  return responsePacket;
}

} // namespace temene
