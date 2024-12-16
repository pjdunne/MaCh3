#include <vector>
#include <functional>
#include <cmath>

/// @brief Base class for applying simple transformations to parameters
class FunctionalParameterTransform{
  public:

    /// @brief constructor
    FunctionalParameterTransform();
    /// @brief destructor
    virtual ~FunctionalParameterTransform()=default;

    /// @brief Apply transformation to all parameters
    /// @param pars Input untransformed parameters
    /// @return Transformed Parameters
    std::vector<double> TransformParameters(const std::vector<double>& pars) {
        return ApplyTransform(pars, [this](double val, int i) { return TransformSingleParameter(val, i);});
    }

    /// @brief Invert transformation
    /// @param pars Input transformed parameters
    /// @return Untransformed Parameters
    std::vector<double> InvertTransformParameters(const std::vector<double>& pars) {
        return ApplyTransform(pars, [this](double val, int i) { return InvertTransformSingleParameter(val, i); });
    }

  protected:
    /// @brief Apply transformation to single parameter
    /// @param par Parameter value
    /// @param idx Parameter index
    /// @return Transformed parameter
    virtual double TransformSingleParameter(double par, int idx) = 0;

    /// @brief Invert transformation on parameter
    /// @param par Parameter value
    /// @param idx Parameter index
    /// @return Untransformed parameter
    virtual double InvertTransformSingleParameter(double par, int idx) = 0;

    /// @brief Abstract method for applying repeated transformations along the vector
    /// @param pars Parameter Value
    /// @param transformFunc Transformation function to be applied to each individual parameter
    /// @returns Vector of transformed parameters
    std::vector<double> ApplyTransform(std::vector<double> pars,  
                                         const std::function<double(double, int)>& transformFunc){
        std::vector<double> result(pars.size());
        for (int i = 0; i < static_cast<int>(pars.size()); i++) {
            result[i] = transformFunc(pars[i], i);
        }
        return result;
    }


};

/// @brief Scales parameters to x' = (x-mu)/sigma where mu and sigma is the prior mean + error resp
class ScaledParameterTransform : public FunctionalParameterTransform{
  public:

    /// @brief constructor
    /// @param parameter_means Prior means for parameter
    ScaledParameterTransform(std::vector<double> parameter_means, std::vector<double> parameter_errors) :
        FunctionalParameterTransform(),
        par_means(parameter_means),
        par_errs(parameter_errors){}


    /// @brief destructor
    virtual ~ScaledParameterTransform()=default;

  protected:
    /// @brief Transforms single parameter to x' = x-mu/sigma
    /// @param par Parameter value
    /// @param idx Parameter index
    /// @return  scaled parameter
    double TransformSingleParameter(double par, int idx) override{
        return (par - par_means[idx])/par_errs[idx];
    }

    /// @brief Inverts Transform on parameter to give x = x'*sigma + mu
    /// @param par Parameter value
    /// @param idx Parameter index
    /// @return  unscaled parameter

    double InvertTransformSingleParameter(double par, int idx) override{
        return par*par_errs[idx]+par_means[idx];
    }

    /// Parameter prior means
    std::vector<double> par_means;
    /// Parameter prior errors
    std::vector<double> par_errs;
};


/// @brief Scales parameters to sign(x)log(1+|x|)
class ScaledLogTransform : public FunctionalParameterTransform{
  public:

    /// @brief constructor
    /// @param parameter_means Prior means for parameter
    ScaledLogTransform(std::vector<double> parameter_means, std::vector<double> parameter_errors) :
        FunctionalParameterTransform(){}

    /// @brief destructor
    virtual ~ScaledLogTransform()=default;

  protected:
    /// @brief Transforms single parameter to sign(x)log(1+|x|)
    /// @param par Parameter value
    /// @param idx Dummy parameter
    /// @return  scaled parameter
    double TransformSingleParameter(double par, int /*dummy*/) override{
        return std::copysign(std::log(std::abs(par) + 1.0), par);
    }

    /// @brief Transforms single parameter to sign(x)exp(|x|-1)
    /// @param par Parameter value
    /// @param idx Dummy parameter
    /// @return  unscaled parameter

    double InvertTransformSingleParameter(double par, int /*dummy*/) override{
        return std::copysign(std::exp(std::abs(par)) - 1.0, par);
    }
};
