#ifndef TIMESERIES_AGGREGATOR_H
#define TIMESERIES_AGGREGATOR_H

#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace temene {

struct MetricPoint {
  uint64_t timestamp;
  double value;
};

class TimeSeriesAggregator {
public:
  void addDataPoint(uint64_t timestamp, double value);

  double calculateEMA(double alpha = 0.2) const;
  std::pair<double, double> calculateHoltWintersSmoothing(double alpha = 0.2, double beta = 0.1) const;

  std::vector<MetricPoint> detectAnomalies(double stdDevThreshold = 2.5) const;
  double calculatePercentile(double p = 95.0) const;

  size_t dataPointCount() const { return points_.size(); }

private:
  std::vector<MetricPoint> points_;
};

} // namespace temene

#endif // TIMESERIES_AGGREGATOR_H
