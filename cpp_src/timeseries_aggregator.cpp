#include "timeseries_aggregator.h"
#include <numeric>

namespace temene {

void TimeSeriesAggregator::addDataPoint(uint64_t timestamp, double value) {
  points_.push_back({timestamp, value});
}

double TimeSeriesAggregator::calculateEMA(double alpha) const {
  if (points_.empty()) return 0.0;
  double ema = points_[0].value;
  for (size_t i = 1; i < points_.size(); ++i) {
    ema = alpha * points_[i].value + (1.0 - alpha) * ema;
  }
  return ema;
}

std::pair<double, double> TimeSeriesAggregator::calculateHoltWintersSmoothing(double alpha, double beta) const {
  if (points_.empty()) return {0.0, 0.0};
  if (points_.size() == 1) return {points_[0].value, 0.0};

  double level = points_[0].value;
  double trend = points_[1].value - points_[0].value;

  for (size_t i = 1; i < points_.size(); ++i) {
    double prevLevel = level;
    level = alpha * points_[i].value + (1.0 - alpha) * (level + trend);
    trend = beta * (level - prevLevel) + (1.0 - beta) * trend;
  }

  return {level, trend};
}

std::vector<MetricPoint> TimeSeriesAggregator::detectAnomalies(double stdDevThreshold) const {
  if (points_.size() < 2) return {};

  double sum = 0.0;
  for (const auto& p : points_) sum += p.value;
  double mean = sum / points_.size();

  double sqSum = 0.0;
  for (const auto& p : points_) sqSum += (p.value - mean) * (p.value - mean);
  double stdDev = std::sqrt(sqSum / points_.size());

  std::vector<MetricPoint> anomalies;
  for (const auto& p : points_) {
    if (std::abs(p.value - mean) > stdDevThreshold * stdDev) {
      anomalies.push_back(p);
    }
  }

  return anomalies;
}

double TimeSeriesAggregator::calculatePercentile(double p) const {
  if (points_.empty()) return 0.0;
  std::vector<double> vals;
  for (const auto& pt : points_) vals.push_back(pt.value);
  std::sort(vals.begin(), vals.end());

  double rank = (p / 100.0) * (vals.size() - 1);
  size_t idx = static_cast<size_t>(rank);
  double frac = rank - idx;

  if (idx + 1 < vals.size()) {
    return vals[idx] + frac * (vals[idx + 1] - vals[idx]);
  }
  return vals[idx];
}

} // namespace temene
