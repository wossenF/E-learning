#ifndef NEURAL_EVALUATOR_H
#define NEURAL_EVALUATOR_H

#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>

namespace temene {

class DenseLayer {
public:
  DenseLayer(size_t inputDim, size_t outputDim);

  std::vector<double> forward(const std::vector<double>& input);
  void applyRelu(std::vector<double>& vec);
  void applySoftmax(std::vector<double>& vec);

  size_t inputDimension() const { return inputDim_; }
  size_t outputDimension() const { return outputDim_; }

private:
  size_t inputDim_;
  size_t outputDim_;
  std::vector<std::vector<double>> weights_;
  std::vector<double> biases_;
};

class NeuralNetworkEvaluator {
public:
  void addLayer(DenseLayer layer);
  std::vector<double> predict(const std::vector<double>& input);

private:
  std::vector<DenseLayer> layers_;
};

} // namespace temene

#endif // NEURAL_EVALUATOR_H
