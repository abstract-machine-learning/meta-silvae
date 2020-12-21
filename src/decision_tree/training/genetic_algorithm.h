#ifndef DECISION_TREE_TRAINING_GENETIC_ALGORITHM_H
#define DECISION_TREE_TRAINING_GENETIC_ALGORITHM_H

#include <stdio.h>

#include "../decision_tree.h"

#define PATH_SIZE 1024

typedef DecisionTree Individual;
typedef struct population Population;
typedef union fitness_parameters FitnessParameters;
typedef union initial_population_parameters InitialPopulationParameters;
typedef struct status Status;

typedef void (*InitialPopulation)(Population *population, const Dataset training_set, Status *status);
typedef unsigned int (*NextPopulationSize)(Status *status);
typedef double (*Fitness)(const Individual individual, Status *status);
typedef double (*MutationProbability)(const double base_mutation_probability, Status *status);
typedef void (*Mutation)(Individual *individual, Status *status);
typedef void (*Crossover)(Individual *offspring, const Individual parent_a, const Individual parent_b, Status *status);
typedef Individual (*Select)(const Population population, Status *status);
typedef void (*AllowFeatures)(unsigned int *allowed_features, const unsigned int n_features, Status *status);
typedef void (*Callback)(Status *status);


struct population {
    Individual *individuals;
    double *fitness;
    unsigned int size;
};


union fitness_parameters {
    double linear[10];
};


union initial_population_parameters {
    char forest[PATH_SIZE];
};


struct status {
    Population population;
    Population population_swap;
    unsigned int current_iteration;
    Fitness compute_fitness;
    FitnessParameters fitness_parameters;
    Select select;
    Crossover crossover;
    Mutation mutate;
    double base_mutation_probability;
    MutationProbability mutation_probability;
    unsigned int initial_population_size;
    unsigned int max_population_size;
    NextPopulationSize next_population_size;
    InitialPopulation initial_population;
    InitialPopulationParameters initial_population_parameters;
    unsigned int max_iteration;
    unsigned int elitism;
    double split_search_aggressiveness;
    AllowFeatures allow_features;
    unsigned int *allowed_features;
    unsigned int n_allowed_features;
    void *data;
    Callback callback_start;
    Callback callback_before_iter;
    Callback callback_after_iter;
    Callback callback_end;
    unsigned int seed;
};


double fitness_linear(const Individual individual, Status *status);

Individual select_uniform(const Population population, Status *status);

Individual select_roulette_wheel(const Population population, Status *status);

void crossover_one_point(Individual *offspring, const Individual parent_a, const Individual parent_b, Status *status);

void mutation_none(Individual *individual, Status *status);

void mutation_grow(Individual *individual, Status *status);

void mutation_z(Individual *individual, Status *status);

double mutation_probability_constant(const double base_mutation_probability, Status *status);

double mutation_probability_encourage_variance(const double base_mutation_probability, Status *status);

unsigned int next_population_size_constant(Status *status);

void initial_population_blank(Population *population, const Dataset training_set, Status *status);

void initial_population_from_forest(Population *population, const Dataset training_set, Status *status);

void allow_features_all(unsigned int *allowed_features, const unsigned int n_features, Status *status);

void allow_features_uniform(unsigned int *allowed_features, const unsigned int n_features, Status *status);

void callback_status_print(Status *status);



DecisionTree ga_train(Status *status, const Dataset training_set);

void ga_status_init(Status *status);

void ga_status_clear(Status *status);

void ga_parse_options(Status *status, int argc, char *argv[]);

void ga_status_print(FILE *fh, const Status status);

void ga_options_print(FILE *fh);

#endif
