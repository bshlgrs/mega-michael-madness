/* 
 * 
 * Tools.cpp
 * ---------
 * 
 * Author: Michael Dickens <mdickens93@gmail.com>
 * Created: 2016-06-19
 * 
 */

#include "QuantitativeModel.h"

#include <cstdarg>

using namespace std;

void error(string message)
{
    stringstream s;
    s << "Error: " << message;
    cerr << s.str() << endl;
    throw s.str();
}

function<double(double)> lomax_pdf(double median, double alpha)
{
    double x_m = median / pow(2, 1.0 / alpha);
    return [x_m, alpha](double x) -> double
    {
        return alpha / x_m / pow(1 + x / x_m, alpha + 1);
    };
}

function<double(double)> lognorm_pdf(double p_m, double p_s)
{
    double mu = log(p_m);
    double s = log(10) * p_s;
    return [mu, s](double x) -> double
    {
        return 1 / (x * s * sqrt(2 * M_PI)) *
            exp(-0.5 * pow((log(x) - mu) / s, 2));
    };
}

function<double(double)> lognorm_cdf(double p_m, double p_s)
{
    double mu = log(p_m);
    double s = log(10) * p_s;
    return [mu, s](double x) -> double
    {
        return 0.5 + 0.5 * erf((log(x) - mu) / (sqrt(2) * s));
    };
}

void normalize(int n, ...)
{
    va_list v1;
    double *vals[n];
    double sum = 0;
    va_start(v1, n);
    for (int i = 0; i < n; i++) {
        vals[i] = va_arg(v1, double *);
        sum += *vals[i];
    }
    va_end(v1);
    for (int i = 0; i < n; i++) {
        *vals[i] /= sum;
    }
}
