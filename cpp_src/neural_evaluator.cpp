#include "neural_evaluator.h"
#include <random>

namespace temene {

DenseLayer::DenseLayer(size_t inputDim, size_t outputDim)
  : inputDim_(inputDim), outputDim_(outputDim), weights_(outputDim, std::vector<double>(inputDim)), biases_(outputDim, 0.0) {
  std::mt19937 rng(42);
  std::normal_distribution<double> dist(0.0, std::sqrt(2.0 / inputDim));

  for (size_t i = 0; i < outputDim_; ++i) {
    for (size_t j = 0; j < inputDim_; ++j) {
      weights_[i][j] = dist(rng);
    }
  }
}

std::vector<double> DenseLayer::forward(const std::vector<double>& input) {
  if (input.size() != inputDim_) throw std::invalid_argument("Input dimension mismatch.");
  std::vector<double> output(outputDim_, 0.0);

  for (size_t i = 0; i < outputDim_; ++i) {
    double sum = biases_[i];
    for (size_t j = 0; j < inputDim_; ++j) {
      sum += weights_[i][j] * input[j];
    }
    output[i] = sum;
  }
  return output;
}

void DenseLayer::applyRelu(std::vector<double>& vec) {
  for (double& val : vec) {
    if (val < 0.0) val = 0.0;
  }
}

void DenseLayer::applySoftmax(std::vector<double>& vec) {
  if (vec.empty()) return;
  double maxVal = *std::max_element(vec.begin(), vec.end());
  double sum = 0.0;
  for (double& val : vec) {
    val = std::exp(val - maxVal);
    sum += val;
  }
  for (double& val : vec) {
    val /= sum;
  }
}

void NeuralNetworkEvaluator::addLayer(DenseLayer layer) {
  layers_.push_back(layer);
}

std::vector<double> NeuralNetworkEvaluator::predict(const std::vector<double>& input) {
  std::vector<double> curr = input;
  for (size_t i = 0; i < layers_.size(); ++i) {
    curr = layers_[i].forward(curr);
    if (i + 1 < layers_.size()) {
      layers_[i].applyRelu(curr);
    } else {
      layers_[i].applySoftmax(curr);
    }
  }
  return curr;
}

} // namespace temene
