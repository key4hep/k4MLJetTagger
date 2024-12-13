// From: https://github.com/HEP-FCC/FCCAnalyses/tree/b9b84221837da8868158f5592b48a9af69f0f6e3/addons/ONNXRuntime 
#include "ONNXRuntime.h"

#include "onnxruntime_cxx_api.h"
// #include "experimental_onnxruntime_cxx_api.h"

#include <fstream>
#include <iostream>

#include <numeric>
#include <algorithm>

ONNXRuntime::ONNXRuntime(const std::string& model_path, const std::vector<std::string>& input_names)
    : env_(new Ort::Env(OrtLoggingLevel::ORT_LOGGING_LEVEL_WARNING, "onnx_runtime")), 
      allocator(), 
      input_names_(input_names) { 
  
  std::cout << "Loading ONNX model from " << model_path << std::endl;
  if (model_path.empty())
    throw std::runtime_error("Path to ONNX model cannot be empty!");
  Ort::SessionOptions options;
  options.SetIntraOpNumThreads(1);
  std::string model{model_path};  // fixes a poor Ort experimental API
  session_ = std::make_unique<Ort::Session>(*env_, model.c_str(), options);

  std::cout << "Loaded ONNX model" << std::endl;

  // Get input names and shapes
  input_node_strings_.clear();
  input_node_dims_.clear();

  for (size_t i = 0; i < session_->GetInputCount(); ++i) {
    std::cout << "Loading input " << i << std::endl;
    // get input names
    const auto input_name = session_->GetInputNameAllocated(i, allocator);
    //input_node_strings_.emplace_back(input_name);
    input_node_strings_.emplace_back(input_name.get());

    std::cout << "Loaded input name" << std::endl;

    // get input shapes
    std::string input_name_str(input_name.get()); // Convert to std::string
    std::cout << "Getting input type info" << std::endl;
    //auto type_info = session_->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo();
    std::cout << "Got input type info" << std::endl;
    //auto shape = type_info.GetShape();
    //std::cout << "Run type_info.GetShape()" << std::endl;
    try {
        auto type_info = session_->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo();
        std::cout << "Input " << i << " Element Type: " << type_info.GetElementType() << std::endl;
        //std::cout << "Is Tensor: " << type_info.IsTensor() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception caught while inspecting type info: " << e.what() << std::endl;
    }
    // input_node_dims_[input_name_str] = type_info.GetShape();
    std::cout << "Loaded input shape" << std::endl;

  }

  std::cout << "Loaded input names and shapes" << std::endl;

  // Get output names and shapes
  output_node_strings_.clear();
  output_node_dims_.clear();
  for (size_t i = 0; i < session_->GetOutputCount(); ++i) {
    // Get output names
    const auto output_name = session_->GetInputNameAllocated(i, allocator);
    output_node_strings_.emplace_back(output_name.get());

    // Get output shapes
    auto type_info = session_->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo();
    auto shape = type_info.GetShape();
    shape[0] = -1;  // Set batch size to dynamic (-1)
    std::string output_name_str(output_name.get()); // Convert to std::string
    output_node_dims_[output_name_str] = shape;
  }

  std::cout << "Loaded output names and shapes" << std::endl;

}

ONNXRuntime::~ONNXRuntime() {}

template <typename T>
ONNXRuntime::Tensor<T> ONNXRuntime::run(Tensor<T>& input,
                                        const Tensor<long>& input_shapes,
                                        unsigned long long batch_size) const {
  std::vector<Ort::Value> tensors_in;
  for (const auto& name : input_node_strings_) {
    auto input_pos = variablePos(name);
    auto value = input.begin() + input_pos;
    std::vector<int64_t> input_dims;
    if (input_shapes.empty()) {
      input_dims = input_node_dims_.at(name);
      input_dims[0] = batch_size;
    } else {
      input_dims = input_shapes[input_pos];
    }
    // rely on the given input_shapes to set the batch size
    if (input_dims[0] != static_cast<long>(batch_size)){
      throw std::runtime_error("The first element of `input_shapes` (" + std::to_string(input_dims[0]) +
                                ") does not match the given `batch_size` (" + std::to_string(batch_size) + ")");
    }
    auto expected_len = std::accumulate(input_dims.begin(), input_dims.end(), 1, std::multiplies<int64_t>());
    if (expected_len != (int64_t)value->size())
      throw std::runtime_error("Input array '" + name + "' has a wrong size of " + std::to_string(value->size()) +
                               ", expected " + std::to_string(expected_len));


    const OrtMemoryInfo* fInfo;
    fInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    auto input_tensor = Ort::Value::CreateTensor<float>(
      fInfo,
      value->data(),
      value->size(),
      input_dims.data(),
      input_dims.size()
    );
    if (!input_tensor.IsTensor())
      throw std::runtime_error("Failed to create an input tensor for variable '" + name + "'.");
    tensors_in.emplace_back(std::move(input_tensor));
  }

  // convert to char* 
  std::vector<const char*> input_node_names;
  for (const auto& name_i : input_node_strings_) {
      input_node_names.push_back(name_i.c_str());
  }

  std::vector<const char*> output_node_names;
  for (const auto& name_j : output_node_strings_) {
      output_node_names.push_back(name_j.c_str());
  }

  // run the inference
auto output_tensors = session_->Run(Ort::RunOptions{nullptr},
                                    input_node_names.data(),
                                    tensors_in.data(),
                                    tensors_in.size(),
                                    output_node_names.data(),
                                    output_node_names.size());
  // convert output tensor to values
  Tensor<T> outputs;
  size_t i = 0;
  for (auto& output_tensor : output_tensors) {
    if (!output_tensor.IsTensor())
      throw std::runtime_error("(at least) inference output " + std::to_string(i) + " is not a tensor.");
    // get output shape
    auto tensor_info = output_tensor.GetTensorTypeAndShapeInfo();
    auto length = tensor_info.GetElementCount();

    auto floatarr = output_tensor.GetTensorData<float>();
    outputs.emplace_back(floatarr, floatarr + length);
    ++i;
  }
  if (outputs.size() != session_->GetOutputCount())
    throw std::runtime_error("Number of outputs differ from the expected one: got " + std::to_string(outputs.size()) +
                             ", expected " + std::to_string(session_->GetOutputCount()));

  return outputs;
}

size_t ONNXRuntime::variablePos(const std::string& name) const {
  auto iter = std::find(input_names_.begin(), input_names_.end(), name);
  if (iter == input_names_.end())
    throw std::runtime_error("Input variable '" + name + " is not provided");
  return iter - input_names_.begin();
}

template ONNXRuntime::Tensor<float> ONNXRuntime::run(Tensor<float>&, const Tensor<long>&, unsigned long long) const;
