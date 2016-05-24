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

#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#define NUM_BUCKETS 1000
#define STEP 1.25
#define EXP_OFFSET 100
#define NORMAL_90TH_PERCENTILE 1.2815515655

enum class Type { empty, buckets, lognorm };

class Distribution {
private:
    double cached_mean = 0;
    bool is_mean_cached = false;
    
public:
    std::function<double(double)> pdf;
    std::vector<double> buckets;

    /* using enum instead of subclasses because C++ is stupid */
    Type type;

    /* params for log-normal */
    double p_m; /* exp(mu) */
    double p_s; /* base-10 sigma */

    Distribution();
    Distribution(Type type);
    Distribution(double p_m, double p_s);
    Distribution(std::function<double(double)> pdf);
    double operator[](int index) const;
    double get(int index) const;
    void check_empty() const;
    Distribution operator+(const Distribution& other) const;
    Distribution operator*(const Distribution& other) const;
    Distribution operator*(double scalar) const;
    Distribution reciprocal() const;
    double mean();
    double variance();
    double integrand(const Distribution& measurement, int index, bool ev) const;
    double integral(const  Distribution& measurement, bool ev) const;
    double posterior(const Distribution& measurement) const;
};
