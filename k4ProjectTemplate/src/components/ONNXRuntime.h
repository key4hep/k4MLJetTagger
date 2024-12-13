#ifndef ONNXRuntime_ONNXRuntime_h
#define ONNXRuntime_ONNXRuntime_h

// From: https://github.com/HEP-FCC/FCCAnalyses/tree/b9b84221837da8868158f5592b48a9af69f0f6e3/addons/ONNXRuntime  
// AI generated documentation

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "onnxruntime_cxx_api.h"

namespace Ort {
  class Env; ///< Wrapper class for the ONNX Runtime environment.
  class Session; ///< Wrapper class for ONNX Runtime session handling.
}  // namespace Ort

/**
 * @class ONNXRuntime
 * @brief A wrapper class for managing ONNX model inference using ONNX Runtime.
 * 
 * This class initializes an ONNX Runtime session, manages input/output tensors,
 * and provides an interface for running inference on input data. The implementation
 * supports flexible tensor shapes and data types.
 */
class ONNXRuntime {
public:
  /**
   * @brief Constructor to initialize the ONNXRuntime environment and session.
   * 
   * @param model_path Path to the ONNX model file.
   * @param input_names List of input variable names to bind during inference.
   */
  explicit ONNXRuntime(const std::string& model_path = "", const std::vector<std::string>& input_names = {});

  /**
   * @brief Destructor to clean up the ONNXRuntime environment and session.
   */
  virtual ~ONNXRuntime();

  /**
   * @brief Type alias for a 2D tensor.
   * 
   * This template defines a tensor as a 2D vector of the specified data type.
   * 
   * @tparam T Data type of the tensor elements.
   */
  template <typename T>
  using Tensor = std::vector<std::vector<T>>;

  // Deleted copy constructor and assignment operator
  ONNXRuntime(const ONNXRuntime&) = delete; ///< Prevents copying of ONNXRuntime instances.
  ONNXRuntime& operator=(const ONNXRuntime&) = delete; ///< Prevents assignment of ONNXRuntime instances.

  /**
   * @brief Retrieves the list of input variable names for the model.
   * 
   * @return A constant reference to the vector of input names.
   */
  const std::vector<std::string>& inputNames() const { return input_names_; }

  /**
   * @brief Runs inference on the provided input tensor and returns the output tensor.
   * 
   * @tparam T Data type of the tensor elements.
   * @param input_tensor Input tensor containing the data for inference.
   * @param input_shape Optional tensor specifying the input shape dimensions.
   * @param batch_size Batch size for inference (default is 1).
   * @return A tensor containing the inference results.
   */
  template <typename T>
  Tensor<T> run(Tensor<T>& input_tensor, const Tensor<long>& input_shape = {}, unsigned long long batch_size = 1ull) const;

private:
  /**
   * @brief Retrieves the position of a variable in the input names list.
   * 
   * @param var_name Name of the variable.
   * @return The position of the variable in the input names list.
   */
  size_t variablePos(const std::string& var_name) const;

  std::unique_ptr<Ort::Env> env_; ///< Pointer to the ONNX Runtime environment object.
  std::unique_ptr<Ort::Session> session_; ///< Pointer to the ONNX Runtime session object.
  Ort::AllocatorWithDefaultOptions allocator; ///< Allocator for ONNX Runtime tensors.

  std::vector<std::string> input_node_strings_; ///< List of input node names.
  std::vector<std::string> output_node_strings_; ///< List of output node names.
  std::vector<std::string> input_names_; ///< List of model input names.
  std::map<std::string, std::vector<int64_t>> input_node_dims_; ///< Dimensions of input nodes.
  std::map<std::string, std::vector<int64_t>> output_node_dims_; ///< Dimensions of output nodes.
};

#endif
