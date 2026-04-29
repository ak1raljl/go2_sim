/*
 * Copyright (c) 2024-2025 Ziqi Fan
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INFERENCE_RUNTIME_HPP
#define INFERENCE_RUNTIME_HPP

#include <vector>
#include <string>
#include <memory>
#include <torch/script.h>
#include "logger.hpp"

namespace InferenceRuntime
{
/**
 * @brief Model interface base class
 *
 * Defines the TorchScript model loading and inference interface.
 */
class Model
{
public:
    virtual ~Model() = default;

    /**
     * @brief Load model file
     * @param model_path Model file path
     * @return Returns true if loading succeeds, false if it fails
     */
    virtual bool load(const std::string& model_path) = 0;

    /**
     * @brief Check if model is loaded
     * @return Returns true if loaded, false otherwise
     */
    virtual bool is_loaded() const = 0;

    /**
     * @brief Forward inference (single input, supports initializer list)
     * @param inputs Vector of input data vectors (usually single element)
     * @return Inference result vector
     */
    virtual std::vector<float> forward(const std::vector<std::vector<float>>& inputs) = 0;

    /**
     * @brief Get model type string
     * @return Model type ("torch")
     */
    virtual std::string get_model_type() const = 0;
};

/**
 * @brief Torch model implementation class
 *
 * Model inference implementation based on PyTorch TorchScript
 */
class TorchModel : public Model
{
private:
    bool loaded_ = false;               ///< Whether model is loaded
    std::string model_path_;            ///< Model file path

    torch::jit::script::Module model_; ///< TorchScript model object

public:
    TorchModel();
    ~TorchModel();

    bool load(const std::string& model_path) override;
    bool is_loaded() const override { return loaded_; }
    std::vector<float> forward(const std::vector<std::vector<float>>& inputs) override;
    std::string get_model_type() const override { return "torch"; }

private:
    /**
     * @brief Convert vector data to Torch tensor
     * @param data Input data vector
     * @param shape Tensor shape
     * @return Torch tensor
     */
    torch::Tensor vector_to_torch(const std::vector<float>& data, const std::vector<int64_t>& shape);

    /**
     * @brief Convert Torch tensor to vector data
     * @param tensor Input tensor
     * @return Data vector
     */
    std::vector<float> torch_to_vector(const torch::Tensor& tensor);
};


/**
 * @brief Model factory class
 *
 * Responsible for creating and loading TorchScript models.
 */
class ModelFactory
{
public:
    /**
     * @brief Load model file
     * @param model_path Model file path
     * @param type Model type (default: auto-detect)
     * @return Successfully loaded model smart pointer, returns nullptr on failure
     */
    static std::unique_ptr<Model> load_model(const std::string& model_path);
};

} // namespace InferenceRuntime

#endif // INFERENCE_RUNTIME_HPP
