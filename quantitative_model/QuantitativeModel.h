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
#include <sstream>
#include <string>
#include <vector>

#define NUM_BUCKETS 1000
#define CENTER_OFFSET ((NUM_BUCKETS) / 2)
#define STEP 1.25
#define EXP_OFFSET 100
#define GAUSSIAN_90TH_PERCENTILE 1.2815515655

enum class Type { empty, buckets, lognorm, double_dist };

std::string type_to_string(Type type);

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
    double integrand(Distribution& measurement, int index, bool ev, int sign=1) const;
    
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

    /* Params for double log-normal or double
     * buckets. Sub-distributions need to be pointers because a class
     * can't contain itself.
     */
    Distribution *neg = NULL;
    Distribution *pos = NULL;
    double neg_weight;
    double pos_weight; /* between 0 and 1 */

    Distribution();
    Distribution(Type type);
    Distribution(double p_m, double p_s);
    Distribution(Distribution neg, Distribution pos, double pos_weight);
    Distribution(Distribution neg, double neg_weight,
                 Distribution pos, double pos_weight);
    Distribution(std::function<double(double)> pdf);
    ~Distribution();
    Distribution operator=(const Distribution& other);
    void check_empty() const;
    Distribution fix_empty() const;
    double operator[](int index) const;
    double get(int index) const;
    void set_name(std::string op, const Distribution& other);
    void set_name(std::string op, const Distribution& left, const Distribution& right);
    Distribution to_lognorm();
    Distribution to_double_dist() const;
    Distribution scale_by(double scalar) const;
    Distribution operator+(const Distribution& other) const;
    Distribution operator-(Distribution& other);
    Distribution operator*(const Distribution& other) const;
    Distribution operator*(double scalar) const;
    Distribution reciprocal();
    double mean();
    double variance();
    double integral(Distribution& measurement, bool ev, int sign=1) const;
    double posterior(Distribution& measurement) const;

    static Distribution lognorm_from_mean_and_variance(double mean, double var);
};

std::function<double(double)> lomax_pdf(double median, double alpha);
std::function<double(double)> lognorm_pdf(double p_m, double p_s);
