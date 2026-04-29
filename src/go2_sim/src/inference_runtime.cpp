#include "inference_runtime.hpp"
#include <stdexcept>
#include <iostream>
#include <numeric>
#include <filesystem>
#include <algorithm>
#include <ATen/Parallel.h>

namespace InferenceRuntime
{

namespace
{
bool IsTorchScriptPath(const std::string& model_path)
{
    std::filesystem::path path(model_path);
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == ".pt" || extension == ".pth";
}

torch::Tensor ExtractOutputTensor(const c10::IValue& output)
{
    if (output.isTensor()) {
        return output.toTensor();
    }

    if (output.isTuple()) {
        const auto& elements = output.toTuple()->elements();
        for (const auto& element : elements) {
            if (element.isTensor()) {
                return element.toTensor();
            }
        }
    }

    throw std::runtime_error("Torch model output does not contain a tensor action");
}
} // namespace

// ============================================================================
// TorchModel Implementation
// ============================================================================

TorchModel::TorchModel()
{
    // Set threads before model load
    torch::set_num_threads(1);
}

TorchModel::~TorchModel()
{
}

bool TorchModel::load(const std::string& model_path)
{
    try
    {
        // Load TorchScript model
        torch::NoGradGuard no_grad;
        model_ = torch::jit::load(model_path, torch::kCPU);
        model_.eval();
        model_path_ = model_path;
        loaded_ = true;
        std::cout << LOGGER::INFO << "Successfully loaded Torch model: " << model_path << std::endl;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cout << LOGGER::ERROR << "Failed to load Torch model: " << e.what() << std::endl;
        loaded_ = false;
        return false;
    }
}

std::vector<float> TorchModel::forward(const std::vector<std::vector<float>>& inputs) {
    if (!loaded_) {
        throw std::runtime_error("Model not loaded");
    }

    try {
        if (inputs.empty()) {
            throw std::runtime_error("Torch model input is empty");
        }

        // Convert input vector to Torch tensor (use first input only)
        const auto& input = inputs[0];
        auto input_tensor = torch::tensor(input, torch::kFloat32).reshape({1, static_cast<int64_t>(input.size())});

        // Disable gradient computation before each forward pass
        torch::NoGradGuard no_grad;

        // Ensure single-threaded execution (critical for performance!)
        torch::set_num_threads(1);

        // Execute forward inference
        auto output = ExtractOutputTensor(model_.forward({input_tensor}));

        // Convert output tensor to vector
        return torch_to_vector(output);
    }
    catch (const std::exception& e) {
        std::cout << LOGGER::ERROR << "Torch inference error: " << e.what() << std::endl;
        throw;
    }
}

torch::Tensor TorchModel::vector_to_torch(const std::vector<float>& data, const std::vector<int64_t>& shape) {
    // Use torch::tensor() + reshape() to match test program behavior
    auto tensor = torch::tensor(data, torch::kFloat32).reshape(shape);
    return tensor;
}

std::vector<float> TorchModel::torch_to_vector(const torch::Tensor& tensor) {
    // Ensure tensor is contiguous and on CPU
    auto cpu_tensor = tensor.is_contiguous() ? tensor : tensor.contiguous();
    if (cpu_tensor.device().type() != torch::kCPU) {
        cpu_tensor = cpu_tensor.to(torch::kCPU);
    }

    // Get data pointer and size
    float* data_ptr = cpu_tensor.data_ptr<float>();
    int64_t num_elements = cpu_tensor.numel();

    // Copy data to vector
    return std::vector<float>(data_ptr, data_ptr + num_elements);
}

// ============================================================================
// ModelFactory Implementation
// ============================================================================

std::unique_ptr<Model> ModelFactory::load_model(const std::string& model_path) {
    if (!IsTorchScriptPath(model_path)) {
        throw std::runtime_error("Only TorchScript .pt/.pth models are supported: " + model_path);
    }

    // Create and load model
    auto model = std::make_unique<TorchModel>();
    if (model && model->load(model_path)) {
        return model;
    }
    return nullptr;
}

} // namespace InferenceRuntime
