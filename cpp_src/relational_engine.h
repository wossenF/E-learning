#ifndef RELATIONAL_ENGINE_H
#define RELATIONAL_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <memory>
#include <cstdint>
#include <fstream>
#include <iostream>

namespace temene {

enum class DataType { INT, DOUBLE, VARCHAR, BOOLEAN };

struct Column {
  std::string name;
  DataType type;
  bool isPrimaryKey{false};
  bool isNullable{true};
};

using ColumnValue = std::variant<std::monostate, int64_t, double, std::string, bool>;

struct Row {
  int64_t id;
  std::unordered_map<std::string, ColumnValue> values;
};

enum class LogRecordType { INSERT, UPDATE, DELETE, BEGIN, COMMIT, ABORT };

struct LogRecord {
  uint64_t lsn;
  uint64_t txId;
  LogRecordType type;
  std::string tableName;
  int64_t rowId;
  std::string beforeState;
  std::string afterState;
};

class RelationalTable {
public:
  RelationalTable(std::string name, std::vector<Column> schema);

  void insertRow(int64_t rowId, const std::unordered_map<std::string, ColumnValue>& values);
  bool updateRow(int64_t rowId, const std::unordered_map<std::string, ColumnValue>& values);
  bool deleteRow(int64_t rowId);

  Row getRow(int64_t rowId) const;
  std::vector<Row> scanTable() const;

  const std::string& getName() const { return name_; }
  const std::vector<Column>& getSchema() const { return schema_; }

private:
  std::string name_;
  std::vector<Column> schema_;
  std::unordered_map<int64_t, Row> rows_;
};

class WriteAheadLogManager {
public:
  explicit WriteAheadLogManager(std::string logFilePath);

  uint64_t appendRecord(uint64_t txId, LogRecordType type, const std::string& tableName, int64_t rowId, const std::string& before, const std::string& after);
  void flush();

  std::vector<LogRecord> readAllRecords();
  void performAriesRecovery(std::unordered_map<std::string, std::shared_ptr<RelationalTable>>& tables);

private:
  std::string logFilePath_;
  uint64_t currentLsn_{1};
  std::vector<LogRecord> inMemoryLog_;
};

class TransactionManager {
public:
  explicit TransactionManager(std::shared_ptr<WriteAheadLogManager> wal);

  uint64_t beginTransaction();
  void commitTransaction(uint64_t txId);
  void abortTransaction(uint64_t txId);

private:
  std::shared_ptr<WriteAheadLogManager> wal_;
  uint64_t nextTxId_{100};
  std::unordered_map<uint64_t, bool> activeTransactions_;
};

} // namespace temene

#endif // RELATIONAL_ENGINE_H
