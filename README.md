# mega michael madness

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
