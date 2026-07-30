#include "url_router.h"

namespace temene {

std::vector<std::string> UrlRouter::splitPath(const std::string& path) const {
  std::vector<std::string> parts;
  size_t start = 0;
  while (start < path.size()) {
    if (path[start] == '/') {
      start++;
      continue;
    }
    size_t end = path.find('/', start);
    if (end == std::string::npos) end = path.size();
    parts.push_back(path.substr(start, end - start));
    start = end + 1;
  }
  return parts;
}

void UrlRouter::addRoute(const std::string& method, const std::string& path, HttpHandler handler) {
  auto rootIt = rootNodes_.find(method);
  if (rootIt == rootNodes_.end()) {
    rootNodes_[method] = std::make_shared<RouteNode>();
  }
  auto curr = rootNodes_[method];

  auto parts = splitPath(path);
  for (const auto& part : parts) {
    if (!part.empty() && part[0] == ':') {
      if (!curr->paramChild) {
        curr->paramChild = std::make_shared<RouteNode>();
        curr->paramName = part.substr(1);
      }
      curr = curr->paramChild;
    } else {
      if (curr->staticChildren.find(part) == curr->staticChildren.end()) {
        curr->staticChildren[part] = std::make_shared<RouteNode>();
      }
      curr = curr->staticChildren[part];
    }
  }
  curr->handler = handler;
}

std::string UrlRouter::handleRequest(const std::string& method, const std::string& path, const std::string& body) {
  auto rootIt = rootNodes_.find(method);
  if (rootIt == rootNodes_.end()) return "405 Method Not Allowed";

  auto curr = rootIt->second;
  auto parts = splitPath(path);
  std::unordered_map<std::string, std::string> params;

  for (const auto& part : parts) {
    auto staticIt = curr->staticChildren.find(part);
    if (staticIt != curr->staticChildren.end()) {
      curr = staticIt->second;
    } else if (curr->paramChild) {
      params[curr->paramName] = part;
      curr = curr->paramChild;
    } else {
      return "404 Not Found";
    }
  }

  if (curr->handler) {
    return curr->handler(params, body);
  }
  return "404 Not Found";
}

} // namespace temene
