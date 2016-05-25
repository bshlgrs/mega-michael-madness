/*
 *
 * QuantitativeModel.cpp
 * ---------------------
 *
 * Author: Michael Dickens <mdickens93@gmail.com>
 * Created: 2016-05-19
 *
 * Port of my quantitative model spreadsheet for people who don't have
 * Excel.
 *
 */

#include "QuantitativeModel.h"

using namespace std;

#define WARN_ABOUT_MISSING_KEYS 0

class Table {
private:
    map<string, Distribution> table;

public:
    Distribution& operator[](string b) 
    {
        if (WARN_ABOUT_MISSING_KEYS) {
            // TODO: actually check if key is missing
            cout << "Warning: '" << b << "' is not in the table." << endl;
        }

        return table[b];
    }

    void print_nicely() 
    {
        for (auto pair: table) {
            string name = pair.first;
            Distribution distribution = pair.second;
            cout << name << ", " << distribution.mean() << ", "
                 << distribution.variance() << endl;
        }
    }
};

double Distribution::posterior(Distribution& measurement) const
{
    double c = integral(measurement, false);
    return integral(measurement, true) / c;
}

/*
 * Converts an 80% credence interval into a log-normal distribution.
 */
Distribution CI(double lo, double hi)
{
    double p_m = sqrt(lo * hi);
    double p_s = sqrt(log(hi / lo) / log(10) / 2 / NORMAL_90TH_PERCENTILE);
    Distribution res(p_m, p_s);
    return res;
}

Table read_input(string filename)
{
    Table table;
    ifstream file(filename);
    string key, comments, low_CI, high_CI;

    if (!file.good()) {
        cerr << "I wasn't able to open the input file (" << filename << "), so I'm exiting." << endl;
        exit(1);
    }

    while (file.good()) {
        getline(file, key, ',');
        getline(file, low_CI, ',');
        getline(file, high_CI);
        if (key.length()) {
            table[key] = CI(stof(low_CI), stof(high_CI));
        }
    }
    return table;
}

void set_globals(Table& table)
{
   table["utility per factory-farmed animal"] =
       table["factory-farmed animal wellbeing"]
       * table["factory-farmed animal sentience adjustment"];
}

// TODO: not currently called
void set_ev_far_future(Table& table)
{
    table["P(humans exist)"] = table["P(fill universe with biology)"];
    table["P(hedonium exists)"] =
        table["P(fill universe with computers)"] * table["P(hedonium)"];
    table["p(ems exist)"] =
        table["p(fill universe with computers)"] * table["p(ems)"];


    table["p(paperclips exist)"] =
        table["p(fill universe with computers)"] * table["p(paperclip)"];
    table["p(dolorium exists)"] =
        table["p(fill universe with computers)"] * table["p(dolorium)"];
}

double thl_posterior_direct(Table& table, const Distribution& prior)
{

    Distribution utility_estimate =
        table["THL years factory farming prevented per $1000"]
        * table["utility per factory-farmed animal"];
    return prior.posterior(utility_estimate);
}

double cage_free_posterior_direct(Table& table, const Distribution& prior)
{
    Distribution utility_estimate =
        table["cage-free total expenditures ($M)"].reciprocal()
        * table["years until cage-free would have happened anyway"]
        * table["millions of cages prevented"]
        * table["proportion of change attributable to campaigns"]
        * table["cage-free years per cage prevented"]
        * table["utility per cage removed"]
        * 1000;
    Distribution r = table["cage-free total expenditures ($M)"].reciprocal();
    return prior.posterior(utility_estimate);
}

int main(int argc, char *argv[])
{
    // This will obviously break if you're not on Unix.
    Table table = read_input(argv[1]);
    Distribution prior(table["log-normal prior mu"].p_m, table["log-normal prior sigma"].p_m);

    set_globals(table);
    cout << "thl_posterior_direct," << thl_posterior_direct(table, prior) << endl; // 194.8
    cout << "cage_free_posterior_direct," << cage_free_posterior_direct(table, prior) << endl; // 2531
    return 0;
}
