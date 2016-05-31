/*
 *
 * Distribution.cpp
 * ---------------------
 *
 * Author: Michael Dickens <mdickens93@gmail.com>
 * Created: 2016-05-18
 *
 * Functionality for the Distribution class.
 *
 */

#include "QuantitativeModel.h"

using namespace std;

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

/*
 * Returns the bucket containing the given x value.
 */
int bucket_index(double x)
{
    // return (int) (log(x) / log(STEP) - 0.5) + EXP_OFFSET; // I believe this is more mathematically accurate but Excel does it the other way
    return (int) (log(x) / log(STEP)) + EXP_OFFSET;
}

/*
 * Gives the value in the (geometric) middle of the
 * bucket. Sort-of the opposite of `bucket_index`.
 */
double bucket_value(int index)
{
    return pow(STEP, index - EXP_OFFSET + 0.5);
}

/*
 * Gives value at the bottom of the bucket.
 */
double bucket_min_value(int index)
{
    return pow(STEP, index - EXP_OFFSET);
}

/*
 * Gets the difference in x-values from the beginning to the end of a bucket.
 */
double get_delta(int index)
{
    return pow(STEP, index - EXP_OFFSET + 1) - pow(STEP, index - EXP_OFFSET);
}

/*
 * Default initialization as Buckets type.
 */
Distribution::Distribution() : Distribution(Type::empty) {}

Distribution::Distribution(Type type) : buckets(NUM_BUCKETS, 0)
{
    this->type = type;
}

/*
 * Takes p_m as exp(mu) and p_s and base-10 standard deviation.
 */
Distribution::Distribution(double p_m, double p_s)
{
    this->type = Type::lognorm;
    this->p_m = p_m;
    this->p_s = p_s;
    this->pdf = lognorm_pdf(p_m, p_s);
}

/*
 * Constructs buckets given a probability density function (PDF).
 */
Distribution::Distribution(function<double(double)> pdf) :
    Distribution(Type::buckets)
{
    for (int i = 0; i < NUM_BUCKETS; i++) {
        buckets[i] = pdf(bucket_value(i));
    }
}

void Distribution::check_empty() const
{
    if (type == Type::empty) {
        cerr << "Warning: \"" << name << "\" is empty." << endl;
    }
}

double Distribution::operator[](int index) const
{
    return get(index);
}

double Distribution::get(int index) const
{
    if (type == Type::lognorm) {
        return pdf(bucket_value(index));
    } else {
        return buckets[index];
    }
}

Distribution Distribution::lognorm_from_mean_and_variance(
    double mean,
    double var)
{
    double p_m = mean / sqrt(1 + var / pow(mean, 2));
    double p_s = sqrt(log(1 + var / pow(mean, 2))) / log(10);
    Distribution res(p_m, p_s);
    return res;
}

/*
 * Approximates this distribution using a log-normal distribution. If
 * already lognormal, returns itself.
 */
Distribution Distribution::to_lognorm()
{
    if (type == Type::lognorm) {
        return *this;
    } else {
        return Distribution::lognorm_from_mean_and_variance(
            this->mean(), this->variance());
    }
}

vector<double> Distribution::prefix_sum() const
{
    vector<double> res;
    res.push_back(get(0) * get_delta(0));
    for (int i = 1; i < NUM_BUCKETS; i++) {
        res.push_back(res.back() + get(i) * get_delta(i));
    }
    return res;
}

void Distribution::half_sum(Distribution& res, const Distribution& other, int include_diagonal) const
{
    vector<double> other_prefix_sum = other.prefix_sum();
    // log_STEP(2) gives approximate most steps away a sum can be,
    // then add a constant to be safe
    int band_size = (int) ceil(log(2) / log(STEP) + 3);

    for (int i = 0; i < NUM_BUCKETS; i++) {
        if (i > band_size) {
            double mass = other_prefix_sum[i - band_size - 1] * get(i) * get_delta(i);
            res.buckets[i] += mass / get_delta(i);
        }
        for (int j = max(i - band_size, 0); j < i + include_diagonal; j++) {
            int index = bucket_index(bucket_value(i) + bucket_value(j));
            double mass = get(i) * get_delta(i) * other.get(j) * get_delta(j);
            if (index >= NUM_BUCKETS) {
                index = NUM_BUCKETS - 1;
            }
            res.buckets[index] += mass / get_delta(index);
        }
    }
}

/*
 * Calculates the sum of two probability distributions.
 */
Distribution Distribution::operator+(const Distribution& other) const
{
    check_empty();
    other.check_empty();
    Distribution res(Type::buckets);

    half_sum(res, other, 1);
    other.half_sum(res, *this, 0);
    return res;
}

/*
 * Approximates the difference of two distributions by representing
 * them as log-normal and then taking the difference of the log-normal
 * distributions.
 */
Distribution Distribution::operator-(Distribution& other)
{
    check_empty();
    other.check_empty();
    if (type == Type::lognorm || other.type == Type::lognorm) {
        Distribution x = this->to_lognorm();
        Distribution y = other.to_lognorm();
        return x - y;
    } else {
        double mean1 = this->mean();
        double mean2 = other.mean();
        double var1 = this->variance();
        double var2 = other.variance();
        double new_mean = mean1 - mean2;

        /* The formula for variance is given by
         *   Var(X - Y) = E[X^2] - 2(Cov[X,Y] + E[X]E[Y]) + E[Y^2] - E[X-Y]^2
         */
        double new_var = (var1 + pow(mean1, 2))
            - 2 * mean1 * mean2
            + (var2 + pow(mean2, 2))
            - pow(mean1 - mean2, 2);
        return Distribution::lognorm_from_mean_and_variance(new_mean, new_var);
    }
}

/*
 * Calculates the product of two probability distributions.
 */
Distribution Distribution::operator*(const Distribution& other) const
{
    check_empty();
    other.check_empty();
    if (type == Type::lognorm
        && other.type == Type::lognorm) {
        double new_p_m = p_m * other.p_m;
        double new_p_s = sqrt(pow(p_s, 2) + pow(other.p_s, 2));
        Distribution res(new_p_m, new_p_s);
        return res;
    } else {
        Distribution res(Type::buckets);
        for (int i = 0; i < NUM_BUCKETS; i++) {
            for (int j = 0; j < NUM_BUCKETS; j++) {
                int index = bucket_index(bucket_value(i) * bucket_value(j));
                // yay properties of exponentiation!
                // int index = bucket_index(i + j - EXP_OFFSET);
                double mass = get(i) * get_delta(i) * other.get(j) * get_delta(j);
                if (index >= NUM_BUCKETS) {
                    index = NUM_BUCKETS - 1;
                } else if (index < 0) {
                    index = 0;
                }
                res.buckets[index] += mass / get_delta(index);
            }
        }
        return res;
    }
}

/*
 * Multiplies a distribution by a scalar.
 */
Distribution Distribution::operator*(double scalar) const
{
    check_empty();
    if (this->type == Type::lognorm) {
        Distribution res(p_m * scalar, p_s);
        return res;
    } else {
        /* TODO: test this */
        Distribution res(Type::buckets);
        for (int i = 0; i < NUM_BUCKETS; i++) {
            int index = bucket_index(bucket_value(i) * scalar);
            double density = get(i);
            if (index >= NUM_BUCKETS) {
                index = NUM_BUCKETS - 1;
            } else if (index < 0) {
                index = 0;
            }
            res.buckets[index] += density;
        }
        return res;
    }
}

/*
 * Returns the log-normally-distributed reciprocal of this
 * distribution. That is, if this distribution has probably P at
 * location X, the reciprocal distribution has probability P at
 * location 1/X.
 * 
 * If this distribution is not log-normal, first converts it to a
 * log-normal approximation.
 */
Distribution Distribution::reciprocal()
{
    check_empty();
    if (this->type == Type::lognorm) {
        Distribution res(1 / p_m, p_s);
        return res;
    } else {
        return this->to_lognorm().reciprocal();
    }
}

/*
 * Not const because caches calculated mean.
 */
double Distribution::mean()
{
    if (is_mean_cached) {
        return cached_mean;
    }

    is_mean_cached = true;
    cached_mean = 0;
    if (this->type == Type::lognorm) {
        /* see https://en.wikipedia.org/wiki/Log-normal_distribution#Arithmetic_moments */
        double sigma = log(10) * p_s;
        cached_mean = p_m * exp(0.5 * pow(sigma, 2));
    } else {
        for (int i = 0; i < NUM_BUCKETS; i++) {
            cached_mean += bucket_value(i) * get(i) * get_delta(i);
        }
    }

    return cached_mean;
}

double Distribution::variance()
{
    if (is_variance_cached) {
        return cached_variance;
    }
    if (!is_mean_cached) {
        mean();
    }

    is_variance_cached = true;
    if (this->type == Type::lognorm) {
        /* see https://en.wikipedia.org/wiki/Log-normal_distribution#Arithmetic_moments */
        double sigma = log(10) * p_s;
        cached_variance = pow(cached_mean, 2) * (exp(pow(sigma, 2)) - 1);
    } else {
        double sigma2 = 0;
        for (int i = 0; i < NUM_BUCKETS; i++) {
            sigma2 += pow(bucket_value(i), 2) * get(i) * get_delta(i);
        }
        cached_variance = sigma2 - pow(cached_mean, 2);
    }

    return cached_variance;
}

double Distribution::integrand(Distribution& measurement, int index, bool ev) const
{
    double u = bucket_value(index);
    double prior = get(index);
    double update = 0;
    if (measurement.type == Type::buckets) {
        /* approximate `measurement` with log-normal dist */
        // TODO: just do this directly numerically
        double mean1 = measurement.mean();
        double var = measurement.variance();
        double expmu = mean1 / sqrt(1 + var / pow(mean1, 2));
        double sigma = sqrt(log(1 + var / pow(mean1, 2)));
        update = lognorm_pdf(u, sigma / log(10))(expmu);
    } else {
        update = lognorm_pdf(u, measurement.p_s)(measurement.p_m);
    }

    double res = prior * update;
    if (ev) {
        res *= u;
    }
    return res;
}

double Distribution::integral(Distribution& measurement, bool ev) const
{
    double total = 0;
    double x_lo = pow(STEP, -EXP_OFFSET);
    double x_hi = x_lo * STEP;
    double y_lo = integrand(measurement, 0, ev);
    double y_hi;
    double avg, delta;
    for (int i = 0; i < NUM_BUCKETS; i++) {
        y_hi = integrand(measurement, i, ev);
        avg = (y_lo + y_hi) / 2;
        delta = x_hi - x_lo;
        total += avg * delta;

        x_lo = x_hi;
        x_hi = x_hi * STEP;
        y_lo = y_hi;
    }
    return total;
}

double Distribution::posterior(Distribution& measurement) const
{
    double c = integral(measurement, false);
    return integral(measurement, true) / c;
}
