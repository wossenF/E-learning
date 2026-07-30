#include "relational_engine.h"
#include <stdexcept>
#include <sstream>

namespace temene {

RelationalTable::RelationalTable(std::string name, std::vector<Column> schema)
  : name_(std::move(name)), schema_(std::move(schema)) {}

void RelationalTable::insertRow(int64_t rowId, const std::unordered_map<std::string, ColumnValue>& values) {
  if (rows_.find(rowId) != rows_.end()) {
    throw std::runtime_error("Duplicate primary key in table " + name_);
  }
  Row r;
  r.id = rowId;
  r.values = values;
  rows_[rowId] = r;
}

bool RelationalTable::updateRow(int64_t rowId, const std::unordered_map<std::string, ColumnValue>& values) {
  auto it = rows_.find(rowId);
  if (it == rows_.end()) return false;
  for (const auto& [k, v] : values) {
    it->second.values[k] = v;
  }
  return true;
}

bool RelationalTable::deleteRow(int64_t rowId) {
  return rows_.erase(rowId) > 0;
}

Row RelationalTable::getRow(int64_t rowId) const {
  auto it = rows_.find(rowId);
  if (it != rows_.end()) return it->second;
  return Row{-1, {}};
}

std::vector<Row> RelationalTable::scanTable() const {
  std::vector<Row> result;
  for (const auto& [_, r] : rows_) {
    result.push_back(r);
  }
  return result;
}

WriteAheadLogManager::WriteAheadLogManager(std::string logFilePath)
  : logFilePath_(std::move(logFilePath)) {}

uint64_t WriteAheadLogManager::appendRecord(
  uint64_t txId,
  LogRecordType type,
  const std::string& tableName,
  int64_t rowId,
  const std::string& before,
  const std::string& after
) {
  uint64_t lsn = currentLsn_++;
  inMemoryLog_.push_back({lsn, txId, type, tableName, rowId, before, after});
  return lsn;
}

void WriteAheadLogManager::flush() {
  std::ofstream out(logFilePath_, std::ios::app);
  for (const auto& rec : inMemoryLog_) {
    out << rec.lsn << "," << rec.txId << "," << static_cast<int>(rec.type) << ","
        << rec.tableName << "," << rec.rowId << "\n";
  }
  out.close();
}

std::vector<LogRecord> WriteAheadLogManager::readAllRecords() {
  return inMemoryLog_;
}

void WriteAheadLogManager::performAriesRecovery(std::unordered_map<std::string, std::shared_ptr<RelationalTable>>& tables) {
  // Analysis & Redo Pass
  for (const auto& rec : inMemoryLog_) {
    if (rec.type == LogRecordType::INSERT) {
      auto tabIt = tables.find(rec.tableName);
      if (tabIt != tables.end()) {
        tabIt->second->insertRow(rec.rowId, {});
      }
    }
  }
}

TransactionManager::TransactionManager(std::shared_ptr<WriteAheadLogManager> wal)
  : wal_(std::move(wal)) {}

uint64_t TransactionManager::beginTransaction() {
  uint64_t txId = nextTxId_++;
  activeTransactions_[txId] = true;
  wal_->appendRecord(txId, LogRecordType::BEGIN, "", 0, "", "");
  return txId;
}

void TransactionManager::commitTransaction(uint64_t txId) {
  wal_->appendRecord(txId, LogRecordType::COMMIT, "", 0, "", "");
  wal_->flush();
  activeTransactions_.erase(txId);
}

void TransactionManager::abortTransaction(uint64_t txId) {
  wal_->appendRecord(txId, LogRecordType::ABORT, "", 0, "", "");
  wal_->flush();
  activeTransactions_.erase(txId);
}

} // namespace temene
