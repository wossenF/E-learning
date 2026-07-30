#include "query_pipeline.h"

namespace temene {

TableScanOperator::TableScanOperator(std::vector<PipelineRecord> dataset)
  : dataset_(std::move(dataset)) {}

void TableScanOperator::init() { index_ = 0; }

bool TableScanOperator::next(PipelineRecord& record) {
  if (index_ >= dataset_.size()) return false;
  record = dataset_[index_++];
  return true;
}

void TableScanOperator::close() { index_ = dataset_.size(); }

FilterOperator::FilterOperator(
  std::shared_ptr<ExecutionOperator> child,
  std::string fieldName,
  std::string expectedValue
) : child_(std::move(child)), fieldName_(std::move(fieldName)), expectedValue_(std::move(expectedValue)) {}

void FilterOperator::init() { child_->init(); }

bool FilterOperator::next(PipelineRecord& record) {
  while (child_->next(record)) {
    auto it = record.fields.find(fieldName_);
    if (it != record.fields.end() && it->second == expectedValue_) {
      return true;
    }
  }
  return false;
}

void FilterOperator::close() { child_->close(); }

ProjectOperator::ProjectOperator(
  std::shared_ptr<ExecutionOperator> child,
  std::vector<std::string> targetFields
) : child_(std::move(child)), targetFields_(std::move(targetFields)) {}

void ProjectOperator::init() { child_->init(); }

bool ProjectOperator::next(PipelineRecord& record) {
  PipelineRecord raw;
  if (!child_->next(raw)) return false;

  record.id = raw.id;
  record.fields.clear();

  for (const auto& field : targetFields_) {
    auto it = raw.fields.find(field);
    if (it != raw.fields.end()) {
      record.fields[field] = it->second;
    }
  }
  return true;
}

void ProjectOperator::close() { child_->close(); }

HashJoinOperator::HashJoinOperator(
  std::shared_ptr<ExecutionOperator> leftChild,
  std::shared_ptr<ExecutionOperator> rightChild,
  std::string joinField
) : leftChild_(std::move(leftChild)), rightChild_(std::move(rightChild)), joinField_(std::move(joinField)) {}

void HashJoinOperator::init() {
  leftChild_->init();
  rightChild_->init();
  hashTable_.clear();
  joinedResults_.clear();
  resultIdx_ = 0;

  // Build phase (Right child into hash table)
  PipelineRecord rRec;
  while (rightChild_->next(rRec)) {
    auto it = rRec.fields.find(joinField_);
    if (it != rRec.fields.end()) {
      hashTable_[it->second].push_back(rRec);
    }
  }

  // Probe phase (Left child against hash table)
  PipelineRecord lRec;
  while (leftChild_->next(lRec)) {
    auto it = lRec.fields.find(joinField_);
    if (it != lRec.fields.end()) {
      auto hashIt = hashTable_.find(it->second);
      if (hashIt != hashTable_.end()) {
        for (const auto& rMatch : hashIt->second) {
          PipelineRecord combined;
          combined.id = lRec.id;
          combined.fields = lRec.fields;
          for (const auto& [k, v] : rMatch.fields) {
            combined.fields[k] = v;
          }
          joinedResults_.push_back(combined);
        }
      }
    }
  }
}

bool HashJoinOperator::next(PipelineRecord& record) {
  if (resultIdx_ >= joinedResults_.size()) return false;
  record = joinedResults_[resultIdx_++];
  return true;
}

void HashJoinOperator::close() {
  leftChild_->close();
  rightChild_->close();
  joinedResults_.clear();
  hashTable_.clear();
}

} // namespace temene
