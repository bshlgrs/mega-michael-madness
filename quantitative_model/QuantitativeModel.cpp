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

bool WARN_ABOUT_MISSING_KEYS = true;

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

void set_globals(Table& table)
{
    table["utility per wealthy human"] =
        table["wealthy human well-being"];
    table["utility per developing-world human"] =
        table["developing-world human well-being"];
    table["utility per factory-farmed animal"] =
        table["factory-farmed animal wellbeing"]
        * table["factory-farmed animal sentience adjustment"];
    table["utility per cage removed"] =
        table["cage-free well-being improvement"]
        * table["factory-farmed animal sentience adjustment"];
    table["utility per wild vertebrate"] =
        table["wild vertebrate well-being"]
        * table["wild vertebrate sentience adjustment"];
    table["utility per insect"] =
        table["insect well-being"]
        * table["insect sentience adjustment"];
    table["utility per hedonium"] =
        table["hedonium well-being"]
        * table["hedonium brains per human brain"];
    table["utility per em"] =
        table["em well-being"]
        * table["ems per human brain"];
    table["utility per paperclip"] =
        table["paperclip well-being"]
        * table["paperclips per human brain"];
    table["utility per dolorium"] =
        table["dolorium well-being"]
        * table["dolorium brains per human brain"];

    table["computer brains in far future"] =
        table["accessible stars by computers"]
        * table["years of future"]
        * table["usable wattage per star"]
        * table["brains per watt"];
    table["biology star-years in far future"] =
        table["accessible stars by biology"]
        * table["years of future"];
}

void set_EV_far_future(Table& table)
{
    table["P(humans exist)"] = table["P(fill universe with biology)"];
    table["P(hedonium exists)"] =
        table["P(fill universe with computers)"] * table["P(hedonium)"];
    table["P(ems exist)"] =
        table["P(fill universe with computers)"] * table["P(ems)"];

    table["human weighted utility"] =
        table["P(humans exist)"]
        * table["utility per wealthy human"]
        * table["humans per star"]
        * table["biology star-years in far future"];
    table["hedonium weighted utility"] =
        table["P(hedonium exists)"]
        * table["utility per hedonium"]
        * table["computer brains in far future"];
    table["em weighted utility"] =
        table["P(ems exist)"]
        * table["utility per em"]
        * table["computer brains in far future"];

    table["pos EV of far future"] =
        table["human weighted utility"]
        + table["hedonium weighted utility"]
        + table["em weighted utility"];
    
    table["P(factory farming exists)"] =
        table["P(fill universe with biology)"]
        * table["P(society doesn't care about animals)"]
        * table["P(we have factory farming)"];
    table["P(wild vertebrates exist)"] =
        table["P(fill universe with biology)"]
        * table["P(society doesn't care about animals)"]
        * table["P(we spread WAS)"];
    table["P(insects exist)"] = table["P(wild vertebrates exist)"];
    table["P(simulations exist)"] =
        table["P(fill universe with biology)"]
        * table["P(society doesn't care about animals)"]
        * table["P(we make suffering simulations)"];
    table["P(paperclips exist)"] =
        table["P(fill universe with computers)"] * table["P(paperclip)"];
    table["P(dolorium exists)"] =
        table["P(fill universe with computers)"] * table["P(dolorium)"];

    table["factory farming weighted utility"] =
        table["P(factory farming exists)"]
        * table["utility per factory-farmed animal"]
        * table["biology star-years in far future"]
        * table["factory farmed animals per star"];
    table["wild vertebrate weighted utility"] =
        table["P(wild vertebrates exist)"]
        * table["utility per wild vertebrate"]
        * table["biology star-years in far future"]
        * table["wild vertebrates per star"];
    table["insect weighted utility"] =
        table["P(insects exist)"]
        * table["utility per insect"]
        * table["biology star-years in far future"]
        * table["insects per star"];
    table["simulation weighted utility"] =
        table["P(simulations exist)"]
        * table["utility per insect"]
        * table["simulations per insect"]
        * table["biology star-years in far future"]
        * table["insects per star"];
    table["paperclip weighted utility"] =
        table["P(paperclips exist)"]
        * table["utility per paperclip"]
        * table["computer brains in far future"];
    table["dolorium weighted utility"] =
        table["P(dolorium exists)"]
        * table["utility per dolorium"]
        * table["computer brains in far future"];

    table["neg EV of far future"] =
        table["factory farming weighted utility"]
        + table["wild vertebrate weighted utility"]
        + table["insect weighted utility"]
        + table["simulation weighted utility"]
        + table["paperclip weighted utility"]
        + table["dolorium weighted utility"];

    table["EV of far future"] =
        table["pos EV of far future"]
        - table["neg EV of far future"];
}

Table read_input(string filename)
{
    bool WARN_ABOUT_MISSING_KEYS_SAVED = WARN_ABOUT_MISSING_KEYS;
    WARN_ABOUT_MISSING_KEYS = false;

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


    set_globals(table);
    set_EV_far_future(table);
    table["who cares"] =
        // table["utility per wealthy human"]
        // + table["utility per em"];
        table["factory farming weighted utility"]
        + table["wild vertebrate weighted utility"];
    cerr << "SUPER IMPORTANT RESULT IS " << table["who cares"].mean() << endl;

    WARN_ABOUT_MISSING_KEYS = WARN_ABOUT_MISSING_KEYS_SAVED;
    return table;
}

Distribution thl_estimate_direct(Table& table)
{

    Distribution utility_estimate =
        table["THL years factory farming prevented per $1000"]
        * table["utility per factory-farmed animal"];
    return  utility_estimate;
}

Distribution cage_free_estimate_direct(Table& table)
{
    Distribution utility_estimate =
        table["cage-free total expenditures ($M)"].reciprocal()
        * table["years until cage-free would have happened anyway"]
        * table["millions of cages prevented"]
        * table["proportion of change attributable to campaigns"]
        * table["cage-free years per cage prevented"]
        * table["utility per cage removed"]
        * 1000;
    return utility_estimate;
}

Distribution ai_safety_estimate(Table& table)
{
    return table["cost per AI researcher"].reciprocal()
        * table["hours to solve AI safety"].reciprocal()
        * table["hours per year per AI researcher"]
        * 1000
        * table["EV of far future"];
}

int main(int argc, char *argv[])
{
    try {
        // This will obviously break if you're not on Unix.
        Table table = read_input(argv[1]);
        Distribution prior(table["log-normal prior mu"].p_m, table["log-normal prior sigma"].p_m);

        cout << "EV of far future," << table["EV of far future"].mean() << endl;

        Distribution thl = thl_estimate_direct(table);
        Distribution cage = cage_free_estimate_direct(table);
        Distribution ai = ai_safety_estimate(table);
        cout << "THL estimate p_m," << thl.p_m << endl;
        cout << "THL estimate p_s^2," << pow(thl.p_s, 2) << endl;
        cout << "cage free estimate p_m," << cage.p_m << endl;
        cout << "cage free estimate p_s^2," << pow(cage.p_s, 2) << endl;
        cout << "AI safety estimate p_m," << ai.p_m << endl;
        cout << "AI safety estimate p_s^2," << pow(ai.p_s, 2) << endl;
        // cout << "thl_posterior_direct," << prior.posterior(thl) << endl;
        // cout << "cage_free_posterior_direct," << prior.posterior(cage) << endl; // 2531
        cout << "AI_safety_posterior," << prior.posterior(ai) << endl;
        
    } catch (const char *msg) {
        cerr << msg << endl;
    }

    return 0;
}
