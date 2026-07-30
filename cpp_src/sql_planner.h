#ifndef SQL_PLANNER_H
#define SQL_PLANNER_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace temene {

enum class SqlPlanNodeType {
  LOGICAL_SCAN,
  LOGICAL_FILTER,
  LOGICAL_PROJECT,
  LOGICAL_JOIN,
  LOGICAL_SORT
};

class SqlPlanNode {
public:
  virtual ~SqlPlanNode() = default;
  virtual SqlPlanNodeType getType() const = 0;
  virtual std::string toString() const = 0;
};

class LogicalScanNode : public SqlPlanNode {
public:
  explicit LogicalScanNode(std::string tableName) : tableName_(std::move(tableName)) {}
  SqlPlanNodeType getType() const override { return SqlPlanNodeType::LOGICAL_SCAN; }
  std::string toString() const override { return "Scan(" + tableName_ + ")"; }
  std::string getTableName() const { return tableName_; }
private:
  std::string tableName_;
};

class LogicalFilterNode : public SqlPlanNode {
public:
  LogicalFilterNode(std::shared_ptr<SqlPlanNode> child, std::string predicate)
    : child_(std::move(child)), predicate_(std::move(predicate)) {}
  SqlPlanNodeType getType() const override { return SqlPlanNodeType::LOGICAL_FILTER; }
  std::string toString() const override { return "Filter(" + predicate_ + ") -> " + child_->toString(); }
private:
  std::shared_ptr<SqlPlanNode> child_;
  std::string predicate_;
};

class SqlPlanner {
public:
  std::shared_ptr<SqlPlanNode> createPlan(const std::string& sqlQuery);
  std::shared_ptr<SqlPlanNode> optimizePlan(std::shared_ptr<SqlPlanNode> root);
};

} // namespace temene

#endif // SQL_PLANNER_H
