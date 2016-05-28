# mega michael madness

## installation and usage

This needs Ruby and stuff. If you don't have a Ruby environment (with bundle and gems and stuff), get that. Then run:

    bundle install

to install the dependencies.

You probably need to go to the `quantitative_model` directory and run `make` to compile the C program.

Then run it with

    bundle exec ruby server.rb 8080

and it should be running on port 8080.


## TODO

- validate input--if they type "blah" as a number, it should warn them
- display large numbers with scientific notation more consistently
