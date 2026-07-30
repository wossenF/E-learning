#ifndef URL_ROUTER_H
#define URL_ROUTER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace temene {

using HttpHandler = std::function<std::string(const std::unordered_map<std::string, std::string>& params, const std::string& body)>;

struct RouteNode {
  std::unordered_map<std::string, std::shared_ptr<RouteNode>> staticChildren;
  std::shared_ptr<RouteNode> paramChild{nullptr};
  std::string paramName;
  HttpHandler handler{nullptr};
};

class UrlRouter {
public:
  void addRoute(const std::string& method, const std::string& path, HttpHandler handler);
  std::string handleRequest(const std::string& method, const std::string& path, const std::string& body);

private:
  std::unordered_map<std::string, std::shared_ptr<RouteNode>> rootNodes_;

  std::vector<std::string> splitPath(const std::string& path) const;
};

} // namespace temene

#endif // URL_ROUTER_H
