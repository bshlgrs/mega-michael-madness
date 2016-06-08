/*
 *
 * QuantitativeModel.h
 * -------------------
 *
 * Author: Michael Dickens <mdickens93@gmail.com>
 * Created: 2016-05-18
 *
 */

#define _USE_MATH_DEFINES // for M_PI

#include <algorithm> /* for min/max */
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#define NUM_BUCKETS 2000
#define CENTER_OFFSET ((NUM_BUCKETS) / 2)
#define STEP 1.25
#define EXP_OFFSET 100
#define GAUSSIAN_90TH_PERCENTILE 1.2815515655

enum class Type { empty, buckets, lognorm, double_lognorm };

class Distribution {
private:
    double cached_mean = 0;
    double cached_variance = 0;
    bool is_mean_cached = false;
    bool is_variance_cached = false;
    
    std::vector<double> prefix_sum() const;
    void half_op(std::function<double(double, double)> op, Distribution& res, const Distribution& other, bool include_diagonal) const;
    void half_sum(Distribution& res, const Distribution& other, bool include_diagonal) const;
    void half_difference(Distribution& res, const Distribution& other, bool include_diagonal) const;
    double integrand(Distribution& measurement, int index, bool ev) const;
    
public:
    std::function<double(double)> pdf;
    std::string name;

    /* Using enum instead of subclasses because C++ is stupid. Would
     * save memory to put params for different types inside a union
     * but who cares.
     */
    Type type;

    /* param for buckets */
    std::vector<double> buckets;

    /* params for log-normal */
    double p_m; /* exp(mu) */
    double p_s; /* base-10 sigma */

    /* params for double log-normal */
    Distribution *neg;
    Distribution *pos;
    double pos_weight; /* between 0 and 1; neg weight is 1 - pos_weight */

    Distribution();
    Distribution(Type type);
    Distribution(double p_m, double p_s);
    Distribution(Distribution neg, Distribution pos, double pos_weight);
    Distribution(std::function<double(double)> pdf);
    ~Distribution();
    void check_empty() const;
    double operator[](int index) const;
    double get(int index) const;
    Distribution to_lognorm();
    Distribution operator+(const Distribution& other) const;
    Distribution operator-(Distribution& other);
    Distribution operator*(const Distribution& other) const;
    Distribution operator*(double scalar) const;
    Distribution reciprocal();
    double mean();
    double variance();
    double integral(Distribution& measurement, bool ev) const;
    double posterior(Distribution& measurement) const;

    static Distribution lognorm_from_mean_and_variance(double mean, double var);
};

std::function<double(double)> lomax_pdf(double median, double alpha);
std::function<double(double)> lognorm_pdf(double p_m, double p_s);
