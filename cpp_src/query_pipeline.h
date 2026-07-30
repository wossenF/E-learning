#ifndef QUERY_PIPELINE_H
#define QUERY_PIPELINE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>

namespace temene {

struct PipelineRecord {
  int id;
  std::unordered_map<std::string, std::string> fields;
};

class ExecutionOperator {
public:
  virtual ~ExecutionOperator() = default;
  virtual void init() = 0;
  virtual bool next(PipelineRecord& record) = 0;
  virtual void close() = 0;
};

class TableScanOperator : public ExecutionOperator {
public:
  explicit TableScanOperator(std::vector<PipelineRecord> dataset);
  void init() override;
  bool next(PipelineRecord& record) override;
  void close() override;

private:
  std::vector<PipelineRecord> dataset_;
  size_t index_{0};
};

class FilterOperator : public ExecutionOperator {
public:
  FilterOperator(
    std::shared_ptr<ExecutionOperator> child,
    std::string fieldName,
    std::string expectedValue
  );
  void init() override;
  bool next(PipelineRecord& record) override;
  void close() override;

private:
  std::shared_ptr<ExecutionOperator> child_;
  std::string fieldName_;
  std::string expectedValue_;
};

class ProjectOperator : public ExecutionOperator {
public:
  ProjectOperator(
    std::shared_ptr<ExecutionOperator> child,
    std::vector<std::string> targetFields
  );
  void init() override;
  bool next(PipelineRecord& record) override;
  void close() override;

private:
  std::shared_ptr<ExecutionOperator> child_;
  std::vector<std::string> targetFields_;
};

class HashJoinOperator : public ExecutionOperator {
public:
  HashJoinOperator(
    std::shared_ptr<ExecutionOperator> leftChild,
    std::shared_ptr<ExecutionOperator> rightChild,
    std::string joinField
  );
  void init() override;
  bool next(PipelineRecord& record) override;
  void close() override;

private:
  std::shared_ptr<ExecutionOperator> leftChild_;
  std::shared_ptr<ExecutionOperator> rightChild_;
  std::string joinField_;
  std::unordered_map<std::string, std::vector<PipelineRecord>> hashTable_;
  std::vector<PipelineRecord> joinedResults_;
  size_t resultIdx_{0};
};

} // namespace temene

#endif // QUERY_PIPELINE_H
