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

bool WARN_ABOUT_MISSING_KEYS = false;

class Table {
private:
    map<string, Distribution> table;

public:
    Distribution& operator[](string b) 
    {
        if (WARN_ABOUT_MISSING_KEYS && table.find(b) == table.end()) {
            // TODO: actually check if key is missing
            cerr << "Warning: \"" << b << "\" is not in the table." << endl;
        }

        table[b].name = b;
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

/*
 * Converts an 80% credence interval into a log-normal distribution.
 */
Distribution CI(double lo, double hi)
{
    double p_m = sqrt(lo * hi);
    double p_s = sqrt(log(hi / lo) / log(10) / 2 / GAUSSIAN_90TH_PERCENTILE);
    Distribution res(p_m, p_s);
    return res;
}

/*
 * Returns a log-normal distribution with all its probability mass at
 * one point.
 */
Distribution CI(double singleton)
{
    return CI(singleton, singleton);
}

void set_globals(Table& t)
{
    t["utility per wealthy human"] =
        t["wealthy human well-being"];
    t["utility per developing-world human"] =
        t["developing-world human well-being"];
    t["utility per factory-farmed animal"] =
        t["factory-farmed animal wellbeing"]
        * t["factory-farmed animal sentience adjustment"];
    t["utility per cage removed"] =
        t["cage-free well-being improvement"]
        * t["factory-farmed animal sentience adjustment"];
    t["utility per wild vertebrate"] =
        t["wild vertebrate well-being"]
        * t["wild vertebrate sentience adjustment"];
    t["utility per insect"] =
        t["insect well-being"]
        * t["insect sentience adjustment"];
    t["utility per hedonium"] =
        t["hedonium well-being"]
        * t["hedonium brains per human brain"];
    t["utility per em"] =
        t["em well-being"]
        * t["ems per human brain"];
    t["utility per paperclip"] =
        t["paperclip maximizer well-being"]
        * t["paperclip maximizers per human brain"];
    t["utility per dolorium"] =
        t["dolorium well-being"]
        * t["dolorium brains per human brain"];

    t["computer brains in far future"] =
        t["accessible stars by computers"]
        * t["years of future"]
        * t["usable wattage per star"]
        * t["brains per watt"];
    t["biology star-years in far future"] =
        t["accessible stars by biology"]
        * t["years of future"];
}

void set_EV_far_future(Table& t)
{
    t["P(humans exist)"] = t["P(fill universe with biology)"];
    t["P(hedonium exists)"] =
        t["P(fill universe with computers)"] * t["P(hedonium)"];
    t["P(ems exist)"] =
        t["P(fill universe with computers)"] * t["P(ems)"];

    t["human weighted utility"] =
        t["P(humans exist)"]
        * t["utility per wealthy human"]
        * t["humans per star"]
        * t["biology star-years in far future"];
    t["hedonium weighted utility"] =
        t["P(hedonium exists)"]
        * t["utility per hedonium"]
        * t["computer brains in far future"];
    t["em weighted utility"] =
        t["P(ems exist)"]
        * t["utility per em"]
        * t["computer brains in far future"];

    t["pos EV of far future"] =
        (t["human weighted utility"]
         + t["hedonium weighted utility"]
         + t["em weighted utility"]).to_lognorm();
    
    t["P(factory farming exists)"] =
        t["P(fill universe with biology)"]
        * t["P(society doesn't care about animals)"]
        * t["P(we have factory farming)"];
    t["P(wild vertebrates exist)"] =
        t["P(fill universe with biology)"]
        * t["P(society doesn't care about animals)"]
        * t["P(we spread WAS)"];
    t["P(insects exist)"] = t["P(wild vertebrates exist)"];
    t["P(simulations exist)"] =
        t["P(fill universe with biology)"]
        * t["P(society doesn't care about animals)"]
        * t["P(we make suffering simulations)"];
    t["P(paperclips exist)"] =
        t["P(fill universe with computers)"] * t["P(paperclip maximizers)"];
    t["P(dolorium exists)"] =
        t["P(fill universe with computers)"] * t["P(dolorium)"];

    t["factory farming weighted utility"] =
        t["P(factory farming exists)"]
        * t["utility per factory-farmed animal"]
        * t["biology star-years in far future"]
        * t["factory farmed animals per star"];
    t["wild vertebrate weighted utility"] =
        t["P(wild vertebrates exist)"]
        * t["utility per wild vertebrate"]
        * t["biology star-years in far future"]
        * t["wild vertebrates per star"];
    t["insect weighted utility"] =
        t["P(insects exist)"]
        * t["utility per insect"]
        * t["biology star-years in far future"]
        * t["insects per star"];
    t["simulation weighted utility"] =
        t["P(simulations exist)"]
        * t["utility per insect"]
        * t["simulations per insect"]
        * t["biology star-years in far future"]
        * t["insects per star"];
    t["paperclip weighted utility"] =
        t["P(paperclips exist)"]
        * t["utility per paperclip"]
        * t["computer brains in far future"];
    t["dolorium weighted utility"] =
        t["P(dolorium exists)"]
        * t["utility per dolorium"]
        * t["computer brains in far future"];

    t["neg EV of far future"] =
        (t["factory farming weighted utility"]
         + t["wild vertebrate weighted utility"]
         + t["insect weighted utility"]
         + t["simulation weighted utility"]
         + t["paperclip weighted utility"]
         + t["dolorium weighted utility"]).to_lognorm();

    t["EV of far future"] =
        t["pos EV of far future"]
        - t["neg EV of far future"];
    
    t["weighted utility of values spreading"] = 
        (t["hedonium scenarios caused by changing values"]
        * t["hedonium weighted utility"]
        + t["dolorium scenarios prevented by changing values"]
        * t["dolorium weighted utility"]
        + t["factory farming scenarios prevented by changing values"]
        * t["factory farming weighted utility"]
        + t["wild vertebrate suffering prevented by changing values"]
        * t["wild vertebrate weighted utility"]
        + t["insect suffering prevented by changing values"]
        * t["insect weighted utility"]
        + t["suffering simulations prevented by changing values"]
         * t["simulation weighted utility"]).to_lognorm();
}

/*
 * Reads a list of confidence intervals into Distributions.
 *
 * Scalar values produce a log-normal distribution with p_m = <value>,
 * p_s = 0; use Distribution::p_m to access the value of the scalar.
*/
Table read_input(string filename)
{
    bool WARN_ABOUT_MISSING_KEYS_SAVED = WARN_ABOUT_MISSING_KEYS; WARN_ABOUT_MISSING_KEYS = false;

    Table t;
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
            t[key] = CI(stof(low_CI), stof(high_CI));
        }
    }

    set_globals(t);
    set_EV_far_future(t);

    WARN_ABOUT_MISSING_KEYS = WARN_ABOUT_MISSING_KEYS_SAVED;
    return t;
}

Distribution veg_estimate_direct(Table& t)
{

    Distribution utility_estimate =
        t["years factory farming prevented per $1000"]
        * t["utility per factory-farmed animal"];
    return utility_estimate;
}

/* Estimates the effect of veg advocacy on the far future. */
Distribution veg_estimate_ff(Table& t)
{
     t["veg-years per $1000"] =
        t["vegetarians per $1000"]
        * t["years spent being vegetarian"];
    t["veg-years directly created per $1000"] =
        t["veg-years per $1000"];
    t["veg-years indirectly created per $1000"] =
        t["veg-years per $1000"]
        * t["annual rate at which vegetarians convert new vegetarians"];
    t["veg-years permanently created per $1000"] =
        (t["veg-years directly created per $1000"]
         + t["veg-years indirectly created per $1000"]).to_lognorm();

    return
        t["weighted utility of values spreading"]
        * t["memetically relevant humans"].reciprocal()
        * t["veg-years permanently created per $1000"];
}

Distribution cage_free_estimate_direct(Table& t)
{
    return t["cage-free total expenditures ($M)"].reciprocal()
        * t["years until cage-free would have happened anyway"]
        * t["millions of cages prevented"]
        * t["proportion of change attributable to campaigns"]
        * t["cage-free years per cage prevented"]
        * t["utility per cage removed"]
        * 1000;
}

Distribution ai_safety_model_1(Table& t)
{
    return t["P(AI-related extinction)"]
        * t["size of FAI community when AGI created"].reciprocal()
        * t["AI researcher multiplicative effect"]
        * t["proportion of bad scenarios averted by doubling total research"]
        * t["cost per AI researcher"].reciprocal()
        * t["EV of far future"]
        * 1000;
}

Distribution ai_safety_model_2(Table& t)
{
    return t["cost per AI researcher"].reciprocal()
        * t["hours to solve AI safety"].reciprocal()
        * t["hours per year per AI researcher"]
        * t["EV of far future"]
        * 1000;
}

Distribution ai_safety_estimate(Table& t)
{
    double divisor = t["Model 1 weight"].p_m + t["Model 2 weight"].p_m;
    return (ai_safety_model_1(t) * t["Model 1 weight"].p_m
            + ai_safety_model_2(t) * t["Model 2 weight"].p_m)
        * (1 / divisor);
}

Distribution targeted_values_spreading_estimate(Table& t)
{
    t["increased probability that AGI is good for animals per $1000 spent"] =
        t["P(friendly AI gets built)"]
        * t["P(AI researchers' values matter)"]
        * t["number of AI researchers when AGI created"].reciprocal()
        * t["values propagation multiplier"]
        * t["cost to convince one AI researcher to care about non-human minds ($)"].reciprocal()
        * 1000;

    return t["weighted utility of values spreading"]
        * t["increased probability that AGI is good for animals per $1000 spent"];
}

void print_results(string name, Distribution prior, Distribution estimate)
{
    Distribution ln = estimate.to_lognorm(); // so p_m and p_s are well-defined
    cout << name << " estimate mean," << ln.mean() << endl;
    cout << name << " estimate p_s," << ln.p_s << endl;
    cout << name << " posterior," << prior.posterior(estimate) << endl;
}

int main(int argc, char *argv[])
{
    try {
        Table t = read_input(argv[1]);
        Distribution lognorm_prior(t["log-normal prior mu"].p_m,
                                   t["log-normal prior sigma"].p_m);
        Distribution pareto_prior(
           lomax_pdf(t["Pareto prior median"].p_m,
                     t["Pareto prior alpha"].p_m));
        Distribution prior = (lognorm_prior * t["log-normal weight"].p_m
                              + pareto_prior * t["Pareto weight"].p_m)
            * (1 / (t["log-normal weight"].p_m + t["Pareto weight"].p_m));

        cout << "EV of far future," << t["EV of far future"].mean() << endl;

        Distribution gd = t["GiveDirectly"];
        Distribution dtw = t["Deworm the World"];
        Distribution veg = veg_estimate_direct(t);
        Distribution veg_ff = veg_estimate_ff(t);
        Distribution cage = cage_free_estimate_direct(t);
        Distribution ai = ai_safety_estimate(t);
        Distribution tvs = targeted_values_spreading_estimate(t);

        print_results("GiveDirectly", prior, gd);
        print_results("DtW", prior, dtw);
        print_results("veg", prior, veg);
        print_results("veg ff", prior, veg_ff);
        print_results("cage free", prior, cage);
        print_results("AI safety", prior, ai);
        print_results("TVS", prior, tvs);
        
    } catch (const char *msg) {
        cerr << msg << endl;
        return 1;
    }

    return 0;
}
