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

#define ASSERT(pred)                                                \
    if (!(pred)) {                                                  \
        stringstream s;                                             \
        s << "`" #pred "` failed on Distribution \"" << this->name  \
          << "\", type " << type_to_string(this->type) << " ("      \
          << __FILE__ << ", line " << __LINE__ << ")";              \
        cerr << s.str() << endl;                                    \
        throw s.str();                                              \
    }                                                               

void error(string message)
{
    stringstream s;
    s << "Error: " << message;
    cerr << s.str() << endl;
    throw s.str();
}

string type_to_string(Type type)
{
    switch (type) {
    case Type::empty: return "empty";
    case Type::buckets: return "buckets";
    case Type::lognorm: return "lognorm";
    case Type::double_dist: return "double_dist";
    default: error("Undefined type " + to_string((int) type));
    }
}

void unsupported_operation(string op, const Distribution *left, const Distribution *right)
{
    stringstream s;
    s << "Unsupported operation `" << op << "` on distribution \""
      << left->name << "\"::" << type_to_string(left->type);
    if (right) {
        s << " and \"" << right->name << "\"::" << type_to_string(right->type);
    }
    error(s.str());
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
 * Default initialization as empty type. For sums, an empty
 * distribution behaves as a distribution with all its probability
 * mass at 0; for other operations, behavior is undefined.
 */
Distribution::Distribution() : Distribution(Type::empty) {}

Distribution::Distribution(Type type) : buckets(NUM_BUCKETS, 0)
{
    this->type = type;
}

/*
 * Takes p_m as exp(mu) and p_s and base-10 standard deviation and
 * creates a log-normal distribution.
 */
Distribution::Distribution(double p_m, double p_s)
{
    this->type = Type::lognorm;
    this->p_m = p_m;
    this->p_s = p_s;
    this->pdf = lognorm_pdf(p_m, p_s);
}

/* Initializes a double distribution. */
Distribution::Distribution(Distribution neg, Distribution pos,
                           double pos_weight) :
    Distribution(neg, 1 - pos_weight, pos, pos_weight) {}

Distribution::Distribution(Distribution neg, double neg_weight,
                           Distribution pos, double pos_weight)
{
    this->type = Type::double_dist;
    this->neg = new Distribution(neg);
    this->pos = new Distribution(pos);
    this->neg_weight = neg_weight;
    this->pos_weight = pos_weight;
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

Distribution::~Distribution()
{
    // Maybe this should free neg and pos? It seems to happen
    // automatically?
}

Distribution Distribution::operator=(const Distribution& other)
{
    if (this == &other) {
        return *this;
    }
    cached_mean = other.cached_mean;
    cached_variance = other.cached_variance;
    is_mean_cached = other.is_mean_cached;
    is_variance_cached = other.is_variance_cached;
    pdf = other.pdf;
    name = other.name;
    type = other.type;
    buckets = other.buckets;
    p_m = other.p_m;
    p_s = other.p_s;
    if (neg) {
        delete neg;    
        neg = NULL;
    }
    if (pos) {
        delete pos;   
        pos = NULL;
    } 
    if (other.neg) {
        neg = new Distribution;
        *neg = *other.neg;
    }
    if (other.pos) {
        pos = new Distribution;
        *pos = *other.pos;
    }
    neg_weight = other.neg_weight;
    pos_weight = other.pos_weight;

    return *this;
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
        if (pow(10, p_s) < STEP) {
            // Need special case to handle small p_s or else it will
            // appear that probability density is 0 everywhere
            return bucket_index(p_m) == index ? 1 : 0;
        } else {
            return pdf(bucket_value(index));
        }
    } else {
        return buckets[index];
    }
}

void Distribution::set_name(string op, const Distribution& other)
{
    this->name = op + "(" + other.name + ")";
}

void Distribution::set_name(string op, const Distribution& left,
                            const Distribution& right)
{
    this->name = left.name + " " + op + " " + right.name;
}

/*
 * Creates a log-normal distribution given a mean and variance.
 */
Distribution Distribution::lognorm_from_mean_and_variance(
    double mean,
    double var)
{
    if (mean == 0) {
        /* Special case to handle distributions with probability
           density 0 everywhere. Otherwise we get p_m = p_s = NaN. */
        Distribution res(0, 0);
        return res;
    }
    double p_m = mean / sqrt(1 + var / pow(mean, 2));
    double p_s = sqrt(log(1 + var / pow(mean, 2))) / log(10);
    Distribution res(p_m, p_s);
    return res;
}

/*
 * Approximates this distribution using a log-normal distribution. If
 * already lognormal, returns itself.
 *
 * If this distribution has a negative part, returns a double dist of
 * two log-normal distributions.
 */
Distribution Distribution::to_lognorm()
{
    if (type == Type::lognorm) {
        return *this;
    } else if (type == Type::double_dist) {
        Distribution res(neg->to_lognorm(), neg_weight,
                         pos->to_lognorm(), pos_weight);
        return res;
    } else {
        Distribution res = Distribution::lognorm_from_mean_and_variance(
            this->mean(), this->variance());
        res.set_name("to_lognorm", *this);
        return res;
    }
}

Distribution Distribution::to_double_dist() const
{
    if (type == Type::double_dist) {
        return *this;
    } else {
        /* Make the other half be a log-normal distribution with 0
           probability mass everywhere. */
        Distribution zero(0, 0);
        zero.set_name("zero_half", *this);
        Distribution res(zero, *this, 1);
        res.set_name("to_double_dist", *this);
        return res;
    }
}

/*
 * Flips the positive and negative halves of a distribution.
 */
Distribution Distribution::negate() const
{
    if (type == Type::double_dist) {
        Distribution flipped(*pos, pos_weight,
                             *neg, neg_weight);
        flipped.set_name("-", *this);
        return flipped;
    } else {
        return to_double_dist().negate();
    }
}

/*
 * For a bucket distribution, scales the probability densities in each bucket.
 */
Distribution Distribution::scale_by(double scalar) const
{
    ASSERT(type == Type::buckets);
    Distribution scaled = *this;
    scaled.set_name("scale_by", *this);
    for (int i = 0; i < NUM_BUCKETS; i++) {
        scaled.buckets[i] *= scalar;
    }

    return scaled;
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

/*
 * Computes operations on a triangular set of bucket pairs covering
 * half the pairs, and optionally including the diagonal.
 *
 * res: Holds result. Should be initialized as a bucket dist.
 */
void Distribution::half_op(function<double(double, double)> op,
                           Distribution& res,
                           const Distribution& other,
                           bool include_diagonal) const
{
    ASSERT(this->type != Type::double_dist);
    ASSERT(other.type != Type::double_dist);
    ASSERT(res.type == Type::buckets);
    vector<double> other_prefix_sum = other.prefix_sum();
    // log_STEP(2) gives approximate most steps away a sum can be,
    // then add a constant to be safe
    int band_size = (int) ceil(log(2) / log(STEP) + 3);
    int offset = include_diagonal ? 1 : 0;

    for (int i = 0; i < NUM_BUCKETS; i++) {
        if (i > band_size && get_delta(i) != 0) {
            // For buckets in `other` that are sufficiently smaller
            // than this bucket, the sum/difference lands in the same
            // bucket as the bucket for `this`.
            double mass = other_prefix_sum[i - band_size - 1] * get(i) * get_delta(i);
            res.buckets[i] += mass / get_delta(i);
        }
        for (int j = max(i - band_size, 0); j < i + offset; j++) {
            double x = op(bucket_value(i), bucket_value(j));
            if (x == 0) {
                // Need to handle this special case because no bucket
                // can contain 0
                continue;
            }
            int index = bucket_index(x);
            double mass = get(i) * get_delta(i) * other.get(j) * get_delta(j);
            if (index >= NUM_BUCKETS) {
                index = NUM_BUCKETS - 1;
            } else if (index < 0) {
                index = 0;
            }
            res.buckets[index] += mass / get_delta(index);
        }
    }
}

void Distribution::half_sum(Distribution& res, const Distribution& other,
                            bool include_diagonal) const
{
    half_op([](double x, double y) { return x + y; }, res, other,
            include_diagonal);
}

void Distribution::half_difference(Distribution& res, const Distribution& other,
                             bool include_diagonal) const
{
    half_op([](double x, double y) { return (x - y); }, res, other,
            include_diagonal);
}

/*
 * Returns 1 / x. If x == 0, returns 0.
 */
double safe_reciprocal(double x)
{
    return x == 0 ? 0 : 1 / x;
}

/*
 * Calculates the sum of two probability distributions.
 */
Distribution Distribution::operator+(const Distribution& other) const
{
    /* Raises a warning on empty even though behavior here is
       well-defined. Distributions could be empty on purpose or
       because they are uninitialized. */
    check_empty();
    other.check_empty();
    if (type == Type::lognorm && this->p_m == 0) {
        return other;
    } else if (other.type == Type::lognorm && other.p_m == 0) {
        return *this;
    } else if (type == Type::double_dist && other.type == Type::double_dist) {
        // Produce 4 distributions given by
        //   neg1 + neg2, pos1 + pos2, pos1 - neg2, pos2 - neg1
        // Then sum their buckets.
        Distribution neg1 = *this->neg;
        Distribution pos1 = *this->pos;
        Distribution neg2 = *other.neg;
        Distribution pos2 = *other.pos;
        Distribution sum_nn = neg1 + neg2;
        Distribution sum_pp = pos1 + pos2;
        // TODO: these weights should definitely not all be 1. You end
        // up double-counting the positive side.
        double nn_weight = this->neg_weight * other.neg_weight;
        double pp_weight = this->pos_weight * other.pos_weight;
        double np_weight = this->neg_weight * other.pos_weight;
        double pn_weight = this->pos_weight * other.neg_weight;

        Distribution sum_pn_pos(Type::buckets);
        Distribution sum_np_pos(Type::buckets);
        Distribution sum_pn_neg(Type::buckets);
        Distribution sum_np_neg(Type::buckets);

        // Compute bucketwise differences and store in `sum_pn` and `sum_np`.
        // Don't bother with diagonals b/c result is always 0.
        //
        // half(pos1 - neg2) gives positive part of pos1 - neg2
        // half(neg2 - pos1) gives negative part
        // Similarly for pos2 - neg1
        //
        // WARNING: there could easily be mistakes here
        pos1.half_difference(sum_pn_pos, neg2, false);
        neg2.half_difference(sum_pn_neg, pos1, false);
        pos2.half_difference(sum_np_pos, neg1, false);
        neg1.half_difference(sum_np_neg, pos2, false);

        // Combine buckets for each sum dist
        Distribution neg_res(Type::buckets);
        Distribution pos_res(Type::buckets);

        for (int i = 0; i < NUM_BUCKETS; i++) {
            neg_res.buckets[i] = sum_nn.get(i) * nn_weight
                + sum_pn_neg.get(i) * pn_weight
                + sum_np_neg.get(i) * np_weight;

            pos_res.buckets[i] = sum_pp.get(i) * pp_weight
                + sum_pn_pos.get(i) * pn_weight
                + sum_np_pos.get(i) * np_weight;
        }

        double neg_weight = neg_res.mass();
        double pos_weight = pos_res.mass();
        neg_res = neg_res.scale_by(safe_reciprocal(neg_weight));
        pos_res = pos_res.scale_by(safe_reciprocal(pos_weight));
        
        // If both inputs used log-normal sub-dists, preserve that.
        if (this->neg->type == Type::lognorm
            && other.neg->type == Type::lognorm) {
            neg_res = neg_res.to_lognorm();
        }
        if (this->pos->type == Type::lognorm
            && other.pos->type == Type::lognorm) {
            pos_res = pos_res.to_lognorm();
        }

        // Make so that each sub-dist has probability mass 1
        Distribution res(neg_res, neg_weight,
                         pos_res, pos_weight);
        res.set_name("+", *this, other);
        return res;
    } else if (type == Type::double_dist || other.type == Type::double_dist) {
        /* When one of the operands is a double dist, upcast the other. */
        Distribution res = this->to_double_dist() + other.to_double_dist();
        res.set_name("+", *this, other);
        return res;
    } else {
        Distribution res(Type::buckets);
        half_sum(res, other, true);
        other.half_sum(res, *this, false);
        res.set_name("+", *this, other);
        return res;
    }
}

/*
 * Approximates the difference of two distributions by representing
 * them as log-normal and then taking the difference of the log-normal
 * distributions.
 */
Distribution Distribution::operator-(Distribution& other)
{
    // Don't need `check_empty` because we just call `+` which covers that
    Distribution res;
    if (type == Type::double_dist && other.type == Type::double_dist) {
        Distribution flipped = other.negate();
        res = *this + flipped;
        res.pos->name = this->name;
        res.neg->name = other.name;
    } else {
        Distribution x = this->to_double_dist();
        Distribution y = other.to_double_dist();
        res = x - y;
    }
    res.set_name("-", *this, other);
    return res;
}

/*
 * Calculates the product of two probability distributions.
 */
Distribution Distribution::operator*(const Distribution& other) const
{
    /* Raises a warning on empty even though behavior here is
       well-defined. Distributions could be empty on purpose or
       because they are uninitialized. */
    check_empty();
    other.check_empty();
    Distribution res;
    if (type == Type::lognorm
        && other.type == Type::lognorm) {
        double new_p_m = p_m * other.p_m;
        double new_p_s = sqrt(pow(p_s, 2) + pow(other.p_s, 2));
        Distribution res1(new_p_m, new_p_s);
        res = res1;
    } else if (type == Type::double_dist &&
               other.type == Type::double_dist) {
        Distribution neg = (*this->pos * *other.neg) + (*this->neg * *other.pos);
        Distribution pos = (*this->pos * *other.pos) + (*this->neg * *other.neg);
        if (this->pos->type == Type::lognorm && this->neg->type == Type::lognorm
            && other.pos->type == Type::lognorm && other.neg->type == Type::lognorm) {
            neg = neg.to_lognorm();
            pos = pos.to_lognorm();
        }
        double neg_weight = this->pos_weight * other.neg_weight
            + this->neg_weight * other.pos_weight;
        double pos_weight = this->pos_weight * other.pos_weight
            + this->neg_weight * other.neg_weight;
        Distribution res1(neg, neg_weight, pos, pos_weight);
        res = res1;
    } else if (type == Type::buckets && other.type == Type::buckets) {
        Distribution res1(Type::buckets);
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
                res1.buckets[index] += mass / get_delta(index);
            }
        }
        res = res1;
    } else if (type == Type::double_dist || other.type == Type::double_dist) {
        res = this->to_double_dist() * other.to_double_dist();
    } else {
        unsupported_operation("*", this, &other);
        Distribution empty;
        res = empty;
    }
    res.set_name("*", *this, other);
    return res;
}

/*
 * Multiplies a distribution by a scalar.
 */
Distribution Distribution::operator*(double scalar) const
{
    check_empty();
    Distribution res;
    if (scalar == 0) {
        res = Distribution(0, 0);
    } else if (type == Type::lognorm) {
        Distribution res1(p_m * scalar, p_s);
        res = res1;
    } else if (type == Type::double_dist) {
        Distribution res1(*neg * scalar, neg_weight, *pos * scalar, pos_weight);
        res = res1;
    } else if (type == Type::buckets) {
        /* TODO: test this */
        Distribution res1(Type::buckets);
        for (int i = 0; i < NUM_BUCKETS; i++) {
            int index = bucket_index(bucket_value(i) * scalar);
            double density = get(i);
            if (index >= NUM_BUCKETS) {
                index = NUM_BUCKETS - 1;
            } else if (index < 0) {
                index = 0;
            }
            res1.buckets[index] += density;
        }
        res = res1;
    } else {
        unsupported_operation("scalar multiplication", this, NULL);
    }
    res.set_name(to_string(scalar) + " * ", *this);
    return res;
}

/*
 * Returns the log-normally-distributed reciprocal of this
 * distribution. That is, if this distribution has probably P at
 * location X, the reciprocal distribution has probability P at
 * location 1/X.
 * 
 * If this distribution is not log-normal, first converts it to a
 * log-normal approximation. But you probably shouldn't be calling
 * it on anything that's not log-normal anyway.
 */
Distribution Distribution::reciprocal()
{
    check_empty();
    Distribution res;
    if (type == Type::lognorm) {
        Distribution res1(1 / p_m, p_s);
        res = res1;
    } else {
        res = this->to_lognorm().reciprocal();
    }
    res.set_name("1 / ", *this);
    return res;
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
    if (type == Type::lognorm) {
        /* see https://en.wikipedia.org/wiki/Log-normal_distribution#Arithmetic_moments */
        double sigma = log(10) * p_s;
        cached_mean = p_m * exp(0.5 * pow(sigma, 2));
    } else if (type == Type::double_dist) {
        cached_mean = pos->mean() * pos_weight - neg->mean() * neg_weight;
    } else if (type == Type::buckets) {
        for (int i = 0; i < NUM_BUCKETS; i++) {
            cached_mean += bucket_value(i) * get(i) * get_delta(i);
        }
    } else {
        unsupported_operation("mean", this, NULL);
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
    if (type == Type::lognorm) {
        /* see https://en.wikipedia.org/wiki/Log-normal_distribution#Arithmetic_moments */
        double sigma = log(10) * p_s;
        cached_variance = pow(cached_mean, 2) * (exp(pow(sigma, 2)) - 1);
    } else if (type == Type::double_dist) {
        /* The formula for variance is given by
             Var(X - Y) = E[X^2] - 2(Cov[X,Y] + E[X]E[Y]) + E[Y^2] - E[X-Y]^2 */
        double mean1 = pos->mean() * pos_weight;
        double mean2 = neg->mean() * neg_weight;
        double var1 = pos->variance() * pow(pos_weight, 2);
        double var2 = neg->variance() * pow(neg_weight, 2);
        cached_variance = (var1 + pow(mean1, 2))
            - 2 * mean1 * mean2
            + (var2 + pow(mean2, 2))
            - pow(mean1 - mean2, 2);        
    } else if (type == Type::buckets) {
        double sigma2 = 0;
        for (int i = 0; i < NUM_BUCKETS; i++) {
            sigma2 += pow(bucket_value(i), 2) * get(i) * get_delta(i);
            if (bucket_value(i) > 1e150) {
                error("Values of bucket " + to_string(i) + " and above are too big to store in floats. Please reduce the size of the maximum bucket.");
            }
        }
        cached_variance = sigma2 - pow(cached_mean, 2);
    } else {
        unsupported_operation("variance", this, NULL);
    }

    return cached_variance;
}

/* Returns the log10-standard deviation of the distribution (same as
 * `p_s` for log-normal dists).
 */
double Distribution::log_stdev()
{
    if (type == Type::lognorm) {
        return p_s;
    } else if (type == Type::buckets) {
        return to_lognorm().p_s;
    } else if (type == Type::double_dist) {
        return lognorm_from_mean_and_variance(mean(), variance()).p_s;
    } else {
        unsupported_operation("log_stdev", this, NULL);
        return 0;
    }
}

/* Returns the probability mass of the distribution. Should be 1 for
 * anything except buckets in special situations.
 */
double Distribution::mass()
{
    if (type == Type::buckets) {
        double total = 0;
        for (int i = 0; i < NUM_BUCKETS; i++) {
            total += get(i) * get_delta(i);
        }
        return total;
    } else if (type == Type::lognorm) {
        return 1;
    } else {
        unsupported_operation("mass", this, NULL);
        return 0;
    }
}

double Distribution::integrand(Distribution& measurement, int index,
                               bool ev, int sign) const
{
    double u = bucket_value(index);
    double prior = get(index);
    double update = 0;
    if (measurement.type != Type::double_dist && sign < 0) {
        // Any non-double dist has zero probability mass on the
        // negative side
        return 0;
    }
    if (measurement.type == Type::buckets) {
        // Assume that the true distribution has the same shape as
        // `measurement` but mean `u`. Then see what's the probability
        // of getting the measurement mean that we did.
        // Distribution assumed_true_dist = measurement * (u / measurement.mean());
        // update = assumed_true_dist.get(bucket_index(measurement.mean()));
        double mean1 = measurement.mean();
        double var = measurement.variance();
        double expmu = mean1 / sqrt(1 + var / pow(mean1, 2));
        double sigma = sqrt(log(1 + var / pow(mean1, 2)));
        update = lognorm_pdf(u, sigma / log(10))(expmu);
    } else if (measurement.type == Type::lognorm) {
        // Use the PDF for the log-normal distribution parameterized
        // such that its mean is `u`.
        // double sigma = log(10) * measurement.p_s;
        // double mu = log(u) - 0.5 * pow(sigma, 2);
        // update = lognorm_pdf(exp(mu), measurement.p_s)(measurement.mean());
        update = lognorm_pdf(u, measurement.p_s)(measurement.p_m);
    } else if (measurement.type == Type::double_dist) {
        if (sign < 0) {
            return integrand(*measurement.neg, index, ev);
        } else {
            return integrand(*measurement.pos, index, ev);
        }
    } else {
        unsupported_operation("integrand", &measurement, NULL);
    }

    double res = prior * update;
    if (ev) {
        res *= u;
    }
    return res;
}

double Distribution::integral(Distribution& measurement, bool ev,
                              int sign) const
{
    if (type == Type::buckets || type == Type::lognorm) {
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
    } else if (type == Type::double_dist) {
        double pos_int = pos->integral(measurement, ev,  1);
        double neg_int = neg->integral(measurement, ev, -1);
        return pos_int + neg_int;
    } else {
        unsupported_operation("integral", this, NULL);
        return 0; // to silence compiler warning even though this never happens
    }
}

double Distribution::posterior(Distribution& measurement) const
{
    double c = integral(measurement, false);
    return integral(measurement, true) / c;
}
