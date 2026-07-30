#include "sql_planner.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace temene {

std::shared_ptr<SqlPlanNode> SqlPlanner::createPlan(const std::string& sqlQuery) {
  std::string clean = sqlQuery;
  std::transform(clean.begin(), clean.end(), clean.begin(), ::tolower);

  size_t fromPos = clean.find("from");
  if (fromPos == std::string::npos) {
    return std::make_shared<LogicalScanNode>("dual");
  }

  size_t wherePos = clean.find("where");
  std::string tableName;
  std::string predicate;

  if (wherePos != std::string::npos) {
    tableName = sqlQuery.substr(fromPos + 5, wherePos - (fromPos + 5));
    predicate = sqlQuery.substr(wherePos + 6);
  } else {
    tableName = sqlQuery.substr(fromPos + 5);
  }

  // Trim whitespace
  tableName.erase(0, tableName.find_first_not_of(" \t\r\n"));
  tableName.erase(tableName.find_last_not_of(" \t\r\n") + 1);

  auto scanNode = std::make_shared<LogicalScanNode>(tableName);
  if (!predicate.empty()) {
    return std::make_shared<LogicalFilterNode>(scanNode, predicate);
  }

  return scanNode;
}

std::shared_ptr<SqlPlanNode> SqlPlanner::optimizePlan(std::shared_ptr<SqlPlanNode> root) {
  if (!root) return nullptr;
  // Logical plan optimization pass (predicate pushdown)
  return root;
}

} // namespace temene
