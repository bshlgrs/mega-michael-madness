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
#include <cassert>

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
    assert(lo <= hi);
    double lo_sign = lo < 0 ? -1 : 1;
    double hi_sign = hi < 0 ? -1 : 1;
    double p_m = sqrt(lo * hi);
    double p_s = sqrt(log(hi / lo) / log(10) / 2 / GAUSSIAN_90TH_PERCENTILE);
    if (lo_sign != hi_sign) {
        error("Interval (" + to_string(lo) + ", " + to_string(hi)
              + ") should not cross 0.");
    }
    Distribution lognorm(p_m, p_s);
    if (lo_sign < 0 && hi_sign < 0) {
        return lognorm.negate();
    } else {
        return lognorm;
    }
}

/*
 * Returns a log-normal distribution with all its probability mass at
 * one point.
 */
Distribution CI(double singleton)
{
    return CI(singleton, singleton);
}

/*
 * Reads a list of confidence intervals into Distributions.
 *
 * Scalar values produce a log-normal distribution with p_m = <value>,
 * p_s = 0; use Distribution::p_m to access the value of the scalar.
*/
Table read_input(string filename)
{
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

    return t;
}

Distribution sum_models(Table& t, string names[], int length,
                        Distribution& per1k_converter)
{
    Distribution sum;
    for (int i = 0; i < length; i++) {
        string per1k = names[i] + " per $1000";
        t[per1k] = t[names[i]] * per1k_converter;
        cout << per1k << "," << t[per1k].mean() << endl;
        if (i == 0) sum = t[per1k];
        else sum = sum + t[per1k];
    }

    return sum.to_lognorm();
}

void set_prior(Table& t)
{
    t["lognorm prior"] = Distribution(t["log-normal prior mu"].p_m,
                                      t["log-normal prior sigma"].p_m);
    t["Pareto prior"] = Distribution(
        lomax_pdf(t["Pareto prior median"].p_m,
                  t["Pareto prior alpha"].p_m));

    t["prior"] = t["lognorm prior"].mixture(t["log-normal weight"].p_m,
                                            t["Pareto prior"],
                                            t["Pareto weight"].p_m);
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

    t["human weighted utility"] =
        t["P(humans exist)"]
        * t["utility per wealthy human"]
        * t["humans per star"]
        * t["biology star-years in far future"];
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
    t["hedonium weighted utility"] =
        t["P(hedonium exists)"]
        * t["utility per hedonium"]
        * t["computer brains in far future"];
    t["em weighted utility"] =
        t["P(ems exist)"]
        * t["utility per em"]
        * t["computer brains in far future"];
    t["paperclip weighted utility"] =
        t["P(paperclips exist)"]
        * t["utility per paperclip"]
        * t["computer brains in far future"];
    t["dolorium weighted utility"] =
        t["P(dolorium exists)"]
        * t["utility per dolorium"]
        * t["computer brains in far future"];

    t["EV of far future"] = (
        t["human weighted utility"]
        + t["factory farming weighted utility"]
        + t["wild vertebrate weighted utility"]
        + t["insect weighted utility"]
        + t["simulation weighted utility"]
        + t["hedonium weighted utility"]
        + t["em weighted utility"]
        + t["paperclip weighted utility"]
        + t["dolorium weighted utility"]
        );

    t["weighted utility of values spreading"] = (
        t["hedonium scenarios caused by changing values"]
        * t["hedonium weighted utility"]
        + t["dolorium scenarios prevented by changing values"]
        * t["dolorium weighted utility"].negate()
        + t["factory farming scenarios prevented by changing values"]
        * t["factory farming weighted utility"].negate()
        + t["wild vertebrate suffering prevented by changing values"]
        * t["wild vertebrate weighted utility"].negate()
        + t["insect suffering prevented by changing values"]
        * t["insect weighted utility"].negate()
        + t["suffering simulations prevented by changing values"]
        * t["simulation weighted utility"].negate()
                                                 );
}

Distribution amf_estimate_direct(Table& t)
{
    // TODO: might need to convert to lognorm
    return t["AMF impact from improving health"]
        + (t["AMF cost per life saved ($K)"].reciprocal()
           * t["adjusted life expectancy"]
           * t["QALYs per life-year saved"]);
}

Distribution veg_ads_estimate_direct(Table& t)
{
    Distribution utility_estimate =
        t["years factory farming prevented per $1000"]
        * t["utility per factory-farmed animal"].negate();
    return utility_estimate;
}

/* Estimates the effect of veg advocacy on the far future. */
Distribution veg_ads_estimate_ff(Table& t)
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

    return t["weighted utility of values spreading"]
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

Distribution gfi_estimate_direct(Table& t)
{
    // Note: GFI does multiple things so these models may be additive
    // rather than disjunctive
    
    string model_names[] = {
        "GFI value from accelerating cultured meat",
        "GFI value from helping startups",
        "GFI value from corporate engagement",
    };

    t[model_names[0]] =
        t["number of people who'd switch to cultured meat (millions)"]
        * 1000000
        * t["years cultured meat accelerated by GFI per year"]
        * t["factory farming years caused per human year"];

    t[model_names[1]] =
        t["mean revenue of startups GFI supports ($K)"]
        * 1000
        * t["number of startups GFI can support per year"]
        * t["money per person-year spent on animal products"].reciprocal()
        * t["factory farming years caused per human year"]
        * t["proportion of startup success attributable to GFI"];

    cerr << model_names[1] << ": " << t[model_names[1]].mean() << endl;

    t[model_names[2]] =
        t["factory farming years displaced at restaurants and grocery stores by GFI"];

    Distribution to_1k_converter = t["GFI budget ($K)"].reciprocal();

    t["GFI years factory farming prevented per $1000"] = sum_models(t, model_names, 3, to_1k_converter);

    t["GFI suffering prevented per $1000"] =
        t["GFI years factory farming prevented per $1000"]
        * t["utility per factory-farmed animal"].negate();

    return t["GFI suffering prevented per $1000"];
}

Distribution gfi_estimate_ff(Table& t)
{
    t["GFI value shifts per $1000"] =
        t["GFI years factory farming prevented per $1000"]
        * t["factory farming years caused per human year"].reciprocal()
        * t["speciesism reduction caused by not eating animals"];

    return t["weighted utility of values spreading"]
        * t["memetically relevant humans"].reciprocal()
        * t["GFI value shifts per $1000"];
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
    return t["P(AI-related extinction)"]
        * t["cost per AI researcher"].reciprocal()
        * t["hours to solve AI safety"].reciprocal()
        * t["hours per year per AI researcher"]
        * t["EV of far future"]
        * 1000;
}

/*
 * Based on the principle that AGI has a binary outcome (see
 * https://www.facebook.com/groups/aisafety/permalink/619701371527314/),
 * and marginal research only matters if it pushes us over the
 * threshold from "not enough research" to "enough research".
 */
Distribution ai_safety_model_3(Table& t)
{
    // TODO: Right now variance in `diff` doesn't matter for the final result
    
    // TODO: This looks wrong. P(outcome changes) decreases when we
    // match the intervals more tightly.
    t["years research needed to solve AI safety"].should_preserve_lognormal = false;
    Distribution diff = t["years research needed to solve AI safety"]
        - t["years research done by hard takeoff date"];
    double research_caused =
        (t["cost per AI researcher"].reciprocal() * 1000).p_m;
    t["P($1000 of marginal research matters)"] = Distribution(diff.mass(0, research_caused), 0);

    t["P(outcome changes per $1000)"] =
        t["P($1000 of marginal research matters)"]
        * t["P(hard takeoff)"]
        * t["P(AGI bad by default)"];

    return t["P(outcome changes per $1000)"]
        * t["EV of far future"];
}

Distribution ai_safety_estimate(Table& t)
{
    double w1 = t["Model 1 weight"].p_m;
    double w2 = t["Model 2 weight"].p_m;
    double w3 = t["Model 3 weight"].p_m;
    normalize(3, &w1, &w2, &w3);

    return ai_safety_model_1(t).scale_by(w1)
        .mixture(ai_safety_model_2(t).scale_by(w2))
        .mixture(ai_safety_model_3(t).scale_by(w3));
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

Distribution ace_estimate_general(Table& t)
{
    t["relative value of ACE money moved"] =
        ((t["proportion ACE money moved between effective animal charities"]
          * t["relative improvement between top animal charities"])
         + (t["proportion ACE money moved to effective animal charities"]
            * t["relative improvement from money moved to effective animal charities"])).to_lognorm()
        * t["ACE total money moved ($K)"]
        * t["ACE marginal money moved relative to total money moved"];
    
    t["relative value of ACE intervention research"] =
        t["relative improvement between animal interventions"]
        * t["money put into interventions that ACE researches ($K)"]
        * t["proportion of money moved by ACE report"];

    // TODO: call sum_models
    string outputs[] = {
        "relative value of ACE money moved",
        "relative value of ACE intervention research",
    };
    for (int i = 0; i < sizeof(outputs)/sizeof(string); i++) {
        string total = outputs[i];
        string per1k = outputs[i] + " per $1000";
        t[per1k] = t[total] * t["ACE budget ($K)"].reciprocal();

        cout << per1k << "," << t[per1k].mean() << endl;
    }

    return (t[outputs[0] + " per $1000"]
            + t[outputs[1] + " per $1000"]).to_lognorm();
}

Distribution ace_estimate_direct(Table& t)
{
    return ace_estimate_general(t) * t["veg posterior"].mean();
}

Distribution ace_estimate_ff(Table& t)
{
    return ace_estimate_general(t) * t["veg ff posterior"].mean();
}

Distribution reg_estimate(Table& t)
{
    t["REG weighted money raised"] = 
        t["REG money raised for AMF"] * t["AMF posterior"]
        + t["REG money raised for veg advocacy"] * t["veg posterior"]
        + t["REG money raised for veg advocacy"] * t["veg ff posterior"]
        + t["REG money raised for AI safety"] * t["AI safety posterior"]
        + t["REG money raised for TVS"] * t["TVS posterior"]
        + t["REG money raised for ACE"] * t["ACE estimate mean"]
        + t["REG money raised for ACE"] * t["ACE ff estimate mean"];

    return t["REG weighted money raised"]
        * t["REG ratio of future money moved to historical money moved"]
        * t["REG budget ($K)"].reciprocal();
}

void print_results(Table& t, string name, Distribution prior, Distribution estimate)
{
    Distribution ln = estimate.to_lognorm(); // so p_s is well-defined
    double posterior = prior.posterior(estimate);
    if (t[name + " RFMF factor"].type != Type::empty) {
        posterior *= t[name + " RFMF factor"].p_m;
    }
    t[name + " posterior"] = CI(posterior);
    cout << name << " estimate mean," << estimate.mean() << endl;
    cout << name << " estimate p_s," << ln.log_stdev() << endl;
    cout << name << " posterior," << posterior << endl;
}

void print_results_no_posterior(Table& t, string name, Distribution estimate)
{
    Distribution ln = estimate.to_lognorm(); // so p_s is well-defined
    double ev = estimate.mean();
    if (t[name + " RFMF factor"].type != Type::empty) {
       ev *= t[name + " RFMF factor"].p_m;
    }
    cout << name << " estimate mean," << ev << endl;
    cout << name << " estimate p_s," << ln.log_stdev() << endl;
}

int main(int argc, char *argv[])
{
    try {
        Table t = read_input(argv[1]);
        set_prior(t);
        set_globals(t);
        set_EV_far_future(t);
        Distribution prior = t["prior"];

        cout << "EV of far future," << t["EV of far future"].mean() << endl;
        cout << "weighted utility of values spreading," << t["weighted utility of values spreading"].mean() << endl;

        Distribution gd = t["GiveDirectly"];
        print_results(t, "GiveDirectly", prior, gd);

        Distribution dtw = t["Deworm the World"];
        print_results(t, "DtW", prior, dtw);

        Distribution amf = amf_estimate_direct(t);
        print_results(t, "AMF", prior, amf);

        Distribution veg = veg_ads_estimate_direct(t);
        Distribution veg_ff = veg_ads_estimate_ff(t);
        print_results(t, "veg", prior, veg);
        print_results(t, "veg ff", prior, veg_ff);

        Distribution cage = cage_free_estimate_direct(t);
        print_results(t, "cage free", prior, cage);

        Distribution gfi = gfi_estimate_direct(t);
        Distribution gfi_ff = gfi_estimate_ff(t);
        print_results(t, "GFI", prior, gfi);
        print_results(t, "GFI ff", prior, gfi_ff);

        Distribution ai = ai_safety_estimate(t);
        print_results(t, "AI safety", prior, ai);

        Distribution tvs = targeted_values_spreading_estimate(t);
        print_results(t, "TVS", prior, tvs);

        Distribution ace = ace_estimate_direct(t);
        Distribution ace_ff = ace_estimate_ff(t);
        print_results_no_posterior(t, "ACE", ace);
        print_results_no_posterior(t, "ACE ff", ace_ff);
        
    } catch (const char *msg) {
        cerr << msg << endl;
        return 1;
    }

    return 0;
}
