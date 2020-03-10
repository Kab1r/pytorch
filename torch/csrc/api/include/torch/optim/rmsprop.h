#pragma once

#include <torch/arg.h>
#include <torch/nn/module.h>
#include <torch/optim/optimizer.h>
#include <torch/optim/serialize.h>
#include <torch/serialize/archive.h>
#include <torch/types.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace torch {
namespace serialize {
class OutputArchive;
class InputArchive;
} // namespace serialize
} // namespace torch

namespace torch {
namespace optim {

struct TORCH_API RMSpropOptions : public OptimizerCloneableOptions<RMSpropOptions> {
    RMSpropOptions(double lr = 1e-2);
    TORCH_ARG(double, lr) = 1e-2;
    TORCH_ARG(double, alpha) = 0.99;
    TORCH_ARG(double, eps) = 1e-8;
    TORCH_ARG(double, weight_decay) = 0;
    TORCH_ARG(double, momentum) = 0;
    TORCH_ARG(bool, centered) = false;

public:
    void serialize(torch::serialize::InputArchive& archive) override;
    void serialize(torch::serialize::OutputArchive& archive) const override;
    TORCH_API friend bool operator==(const RMSpropOptions& lhs, const RMSpropOptions& rhs);
    ~RMSpropOptions() = default;
};

struct TORCH_API RMSpropParamState : public OptimizerCloneableParamState<RMSpropParamState> {
    TORCH_ARG(int64_t, step) = 0;
    TORCH_ARG(torch::Tensor, square_avg);
    TORCH_ARG(torch::Tensor, momentum_buffer) = {};
    TORCH_ARG(torch::Tensor, grad_avg) = {};

public:
    void serialize(torch::serialize::InputArchive& archive) override;
    void serialize(torch::serialize::OutputArchive& archive) const override;
    TORCH_API friend bool operator==(const RMSpropParamState& lhs, const RMSpropParamState& rhs);
    ~RMSpropParamState() = default;
};

class TORCH_API RMSprop : public Optimizer {
public:
    explicit RMSprop(std::vector<OptimizerParamGroup> param_groups,
                     RMSpropOptions defaults) : Optimizer(std::move(param_groups), std::make_unique<RMSpropOptions>(defaults)) {
        TORCH_CHECK(defaults.lr() >= 0, "Invalid learning rate: ", defaults.lr());
        TORCH_CHECK(defaults.eps() >= 0, "Invalid epsilon value: ", defaults.eps());
        TORCH_CHECK(defaults.momentum() >= 0, "Invalid momentum value: ", defaults.momentum());
        TORCH_CHECK(defaults.weight_decay() >= 0, "Invalid weight_decay value: ", defaults.weight_decay());
        TORCH_CHECK(defaults.alpha() >= 0, "Invalid alpha value: ", defaults.alpha());
    }

    explicit RMSprop(std::vector<Tensor> params,
                     RMSpropOptions defaults) : RMSprop({std::move(OptimizerParamGroup(params))}, defaults) {}

    void step() override;

    /// Returns the number of parameters referenced by the optimizer.
    size_t size() const noexcept override;

    /// Adds the given vector of parameters to the optimizer's parameter list.
    void add_parameters(const std::vector<Tensor>& parameters) override;

    /// Provides a const reference to the parameters this optimizer holds.
    const std::vector<Tensor>& parameters() const noexcept override;

    /// Provides a reference to the parameters this optimizer holds.
    std::vector<Tensor>& parameters() noexcept override;

    void save(serialize::OutputArchive& archive) const override;
    void load(serialize::InputArchive& archive) override;

private:
    template <typename Self, typename Archive>
    static void serialize(Self& self, Archive& archive) {
        _TORCH_OPTIM_SERIALIZE_WITH_TEMPLATE_ARG(RMSprop);
    }
};
} // namespace optim
} // namespace torch
