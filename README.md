# meta silvae
meta silvae (latin for "beyon forest" and acronym of **M**agister **E**fficiens **T**emperat **A**rbore **silvae**, which translates to "The efficient master mixes trees of the forest", but also "The efficient master mixes trees through [silva](https://github.com/abstract-machine-learning/silva)) is an abstract interpretation based tool for training decision tree classifiers, in particural we aim at enhancing stability-related properties of classifiers.

meta silvae deploies a genetic algorithm to evolve a poluation of decision trees, and internally uses  [silva](https://github.com/abstract-machine-learning/silva) in order to assert fitness of individual trees in terms of accuracy and stability.

## Requirements ##

 - Any C99-compatible C compiler

## Installation
To install meta silvae you need to clone or download the source code files from this repository and compile them. There are no additional requirements nor dependencies:

    git clone https://github.com/abstract-machine-learning/meta-silvae
or:

    wget https://github.com/abstract-machine-learning/meta-silvae/archive/master.zip
    unzip master.zip
then:
    cd meta-silvae/src
    make
    make install
The executable file will be available under `silva/bin/meta-silvae`.

Every piece of code is documented using [Doxygen](http://www.doxygen.nl/). If you have Doxygen installed and wish to generate the documentation pages (HTML), run:

    cd meta-silvae/src
    make doc
Documentation will be available under `meta-silvae/doc/html/index.html`.

## Usage
Run `meta-silvae` without arguments for a quick online help message. Full syntax is

    bin/meta-silvae <training set> <output> [options]
Mandatory arguments:

 - training set     Path to training set file
 - output           Path to output file, silva format

Optional arguments:
  - --fitness                        Fitness function used to estimate performance of a tree
      - linear <magnitude> <correct> <wrong> <stable> <unstable> <no info> <robust> <fragile> <vulnerable> <broken>  Linear combination of standard performance indicators
  - --select                         Criterion used to choose parents for crossover
      - uniform                      Choses a parent with a uniformally distributed probability
      - roulette-wheel               Uses standard Roulette Wheel algorithm
  - --crossover                      Crossover function
      - one-point                    Substitutes a subtree from first parent with a subtree from second parent
  - --mutation                       Mutation function
      - none                         No mutation
      - grow                         Allows trees to grow
      - Z                            Allows both growth and pruning
  - --mutation-base-probability p    Base mutation probability in [0; 1], passed to mutation probability function
  - --mutation-probability           Mutation probability function
      - constant                     Always returns mutation base probability
      - encourage-variance           Returns higher probabilities when variance is low
  - --population-initial-size n      Number of trees in initial population
  - --population-max-size n          Maximum number of trees in the population
  - --population-next-size           Function which computes size of next population
      - constant                     Always returns same size
  - --population-generator           Generator of intial population
      - blank                        Generates single-node trees (every sample in root)
      - from-forest <path>           Reads trees from a forest file
  - --max-iteration n                Maximum number of iterations for genetic algorithm
  - --elitism n                      Number of best individual to copy to next iteration
  - --split-search-aggressiveness p  Fraction of splits to consider during split generation, in [0; 1]
  - --allowed-features               Features allowed for splits
      - all                          Uses every feature
      - uniform n                    Randomly choses n features from a uniform distribution
  - --seed n                         Seed for random number generation

## Data set format
See [dedicated section on our data-collection repository](https://github.com/abstract-machine-learning/data-collection#dataset-format), from which you can also download some ready-to-use [datasets](https://github.com/svm-abstract-verifier/data-collection/tree/master/datasets) and [models](https://github.com/abstract-machine-learning/data-collection/tree/master/models).
