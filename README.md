# mega michael cause prioritization madness

[Quantitative models offer a superior approach in determining which interventions to support.](http://mdickens.me/2016/04/06/expected_value_estimates_you_can_%28maybe%29_take_literally/) However, naive cost-effectiveness estimates [have big problems](http://blog.givewell.org/2011/08/18/why-we-cant-take-expected-value-estimates-literally-even-when-theyre-unbiased/). In particular:

1.  They don’t give stronger consideration to more robust estimates.
2.  They don’t always account for all relevant factors.

This is an implementation of [Michael Dickens' attempt](http://mdickens.me/2016/05/17/a_complete_quantitative_model_for_cause_selection/) to buid a quantitative model for cause selection which does not have these limitations.

The model makes estimates by using expected-value calculations to produce probability distributions of utility values. It then uses these estimates as evidence to update a prior over the effectiveness of different interventions. Treating estimates as evidence updating a prior means that interventions with more robust evidence of effectiveness have better posteriors.

This app is deployed at [http://mdickens.me/causepri-app/](http://mdickens.me/causepri-app/).

You can add new models by editing this repo. We're very happy to help you out with that.

This version was implemented by Michael Dickens and Buck Shlegeris.


## Installation and Usage

This needs Ruby and stuff. If you don't have a Ruby environment (with bundle and gems and stuff), get that. Then run:

    bundle install

to install the dependencies.

You probably need to go to the `quantitative_model` directory and run `make` to compile the C++ program.

Then run it with

    bundle exec ruby server.rb 8080

and it should be running on port 8080.

## Using the Backend

You can run the C++ backend independently of the frontend by calling

    ./a.out inputs.txt

`inputs.txt` contains the inputs needed for the backend model. Right now it's missing a lot of inputs but you can add them if you want to use them.

### How to Add a New Intervention

To add calculations for a new intervention, you'll need to modify `quantitative_model/QuantitativeModel.cpp`.

1. Add a function to estimate the expected value of the intervention. Look at `targeted_values_spreading_estimate(Table& t)` or any of the other estimate functions to see how expected value estimates work. They access `Distribution` objects stored in the table `t`. `Distribution` objects support addition, multiplication by other distributions and by scalars, and reciprocals (via `.reciprocal()`).
2. In `main`, call your new function and store the result in a `Distribution` variable. You may then call `print_results` on the variable (look at how `print_results` is used for the other estimates in `main`).
