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

typedef map<string, Distribution> Table2;

#define WARN_ABOUT_MISSING_KEYS 0

class Table {
private:
    map<string, Distribution> table;

public:
//    const Distribution& operator [](string b) const {
//        return table[b];
//    }
    Distribution& operator [](string b) {
        if (WARN_ABOUT_MISSING_KEYS) {
            printf("warning: '%s' is not in the table.\n", b.c_str());
        }

        return table[b];
    }

    void print_nicely() {
        for(auto pair: table) {
            string name = pair.first;
            Distribution distribution = pair.second;
            printf("%s,%f,%f\n", name.c_str(), distribution.mean(), distribution.variance());
        }
    }
};

double Distribution::posterior(const Distribution& measurement) const
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
        printf("I wasn't able to open the input file (%s), so I'm exiting.\n", filename.c_str());
        exit(1);
    }

    while (file.good()) {
        getline(file, key, ',');
        getline(file, low_CI, ',');
        getline(file, high_CI);
//        getline(file, comments);
        if (key.length()) {
            table[key] = CI(stof(low_CI), stof(high_CI));
//            printf("I just set '%s'\n", key.c_str());
        }
    }
    return table;
}

void globals(Table& table)
{
   table["utility per factory-farmed animal"] =
       table["factory-farmed animal wellbeing"]
       * table["factory-farmed animal sentience adjustment"];

}

// TODO: not currently called
void ev_far_future(Table& table)
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

    globals(table);
    printf("thl_posterior_direct,%f\n", thl_posterior_direct(table, Distribution(1, 0.75))); // 194.8
    printf("cage_free_posterior_direct,%f\n", cage_free_posterior_direct(table, Distribution(1, 0.75))); // 2531
    table.print_nicely();
    return 0;
}
