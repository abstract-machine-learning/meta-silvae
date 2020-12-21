#define DEFAULT_FITNESS fitness_linear
#define DEFAULT_SELECT select_roulette_wheel
#define DEFAULT_CROSSOVER crossover_one_point
#define DEFAULT_MUTATION mutation_z
#define DEFAULT_BASE_MUTATION_PROBABILITY 1.0
#define DEFAULT_MUTATION_PROBABILITY mutation_probability_constant
#define DEFAULT_POPULATION_INITIAL_SIZE 32
#define DEFAULT_POPULATION_MAX_SIZE 32
#define DEFAULT_POPULATION_NEXT_SIZE next_population_size_constant
#define DEFAULT_POPULATION_GENERATOR initial_population_blank
#define DEFAULT_MAX_ITERATION 64
#define DEFAULT_ELITISM 1
#define DEFAULT_SPLIT_SEARCH_AGGRESSIVENESS 0.01
#define DEFAULT_ALLOW_FEATURES allow_features_all
#define DEFAULT_SEED 0

#include "genetic_algorithm.h"

#include <stdlib.h>
#include <string.h>

#include "../../data_structures/stack.h"
#include "../performance/performance.h"
#include "../../forest/forest.h"
#include "../../forest/mapper/silva.h"


/***********************************************************************
 * Internal functions.
 **********************************************************************/

static void population_alloc(Population *population, const unsigned int size) {
    population->individuals = (Individual *) realloc(population->individuals, size * sizeof(Individual));
    population->fitness = (double *) realloc(population->fitness, size * sizeof(double));
}



static Node *choose_subtree(const DecisionTree tree) {
    Node *node = tree.root;

    while (!node_is_leaf(*node)) {
        const double p = (double) rand() / RAND_MAX;
        if (p < 0.3) {
            node = node->left;
        }
        else if (p < 0.6) {
            node = node->right;
        }
        else {
            break;
        }
    }

    return node;
}



static double subscore(const double *probabilities, const unsigned int n_labels) {
    unsigned int i;
    double score = 1.0;

    for (i = 0; i < n_labels; ++i) {
        score -= probabilities[i] * probabilities[i];
    }

    return 1.0 - score;
}



static double split_candidate_score(const Node node, const unsigned int feature, const double threshold, double *buffer) {
    const Dataset dataset = node.tree->dataset;
    const double n_samples = node.last_sample_idx - node.first_sample_idx + 1.0,
                 **samples = (const double **) node.tree->samples;
    const unsigned int *labels = dataset.label_lookup,
                       n_labels = dataset.n_labels;
    unsigned int i, n_left = 0, n_right = 0;

    for (i = 0; i < 2 * n_labels; ++i) {
        buffer[i] = 0.0;
    }

    for (i = node.first_sample_idx; i <= node.last_sample_idx; ++i) {
        const unsigned int label = labels[(samples[i] - dataset.points) / dataset.space_size];
        if (samples[i][feature] <= threshold) {
            buffer[label] += 1.0;
            ++n_left;
        }
        else {
            buffer[n_labels + label] += 1.0;
            ++n_right;
        }
    }

    for (i = 0; i < n_labels; ++i) {
        buffer[i] /= n_left;
        buffer[n_labels + i] /= n_right;
    }

    return (n_left * subscore(buffer, n_labels) + n_right * subscore(buffer + n_labels, n_labels)) / n_samples;
}



static void split_trial(unsigned int *feature, double *threshold, const Node node, double *buffer, Status *status) {
    const Dataset dataset = node.tree->dataset;
    unsigned int i, j;
    double max_score = 0.0;

    *feature = status->allowed_features[rand() % status->n_allowed_features];
     *threshold = node.tree->samples[
        (rand() % (node.last_sample_idx - node.first_sample_idx + 1)) + node.first_sample_idx
    ][*feature];


    for (i = 0; i < status->n_allowed_features; ++i) {
       const unsigned int d = status->allowed_features[i];
       if (dataset.n_projections[d] == 1) {
           continue;
       }
       for (j = 0; j < dataset.n_projections[d] - 1; ++j) {
           double value, score;
           if ((double) rand() / RAND_MAX >= status->split_search_aggressiveness) {
               continue;
           }
           value = ((double) rand() / RAND_MAX) * (dataset.projections[d][j + 1] - dataset.projections[d][j]) + dataset.projections[d][j];
           score = split_candidate_score(node, d, value, buffer);
           if (score > max_score) {
               max_score = score; 
               *feature = d;
               *threshold = value;
           }
       }
    }
}





/***********************************************************************
 * Parsing functions.
 **********************************************************************/

static void parse_fitness(Fitness *fitness, FitnessParameters *fitness_parameters, int argc, char **argv) {
    if (argc > 11 && strcmp(argv[1], "linear") == 0) {
        *fitness = fitness_linear;
        sscanf(argv[2], "%lf", fitness_parameters->linear + 0);
        sscanf(argv[3], "%lf", fitness_parameters->linear + 1);
        sscanf(argv[4], "%lf", fitness_parameters->linear + 2);
        sscanf(argv[5], "%lf", fitness_parameters->linear + 3);
        sscanf(argv[6], "%lf", fitness_parameters->linear + 4);
        sscanf(argv[7], "%lf", fitness_parameters->linear + 5);
        sscanf(argv[8], "%lf", fitness_parameters->linear + 6);
        sscanf(argv[9], "%lf", fitness_parameters->linear + 7);
        sscanf(argv[10], "%lf", fitness_parameters->linear + 8);
        sscanf(argv[11], "%lf", fitness_parameters->linear + 9);
    }
    else {
        fprintf(stderr, "[%s: %d] Unknown type of fitness function \"%s\".\n", __FILE__, __LINE__, argv[1]);
        exit(EXIT_FAILURE);
    }
}



static void parse_select(Select *select, int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "uniform") == 0) {
        *select = select_uniform;
    }
    else if (argc > 1 && strcmp(argv[1], "roulette-wheel") == 0) {
        *select = select_roulette_wheel;
    }
    else {
        fprintf(stderr, "[%s: %d] Unknown type of selection function \"%s\".\n", __FILE__, __LINE__, argv[1]);
        exit(EXIT_FAILURE);
    }
}



static void parse_crossover(Crossover *crossover, int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "one-point") == 0) {
        *crossover = crossover_one_point;
    }
    else {
        fprintf(stderr, "[%s: %d] Unknown type of crossover function \"%s\".\n", __FILE__, __LINE__, argv[1]);
        exit(EXIT_FAILURE);
    }
}



static void parse_mutation(Mutation *mutation, int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "none") == 0) {
        *mutation = mutation_none;
    }
    else if (argc > 1 && strcmp(argv[1], "grow") == 0) {
        *mutation = mutation_grow;
    }
    else if (argc > 1 && strcmp(argv[1], "Z") == 0) {
        *mutation = mutation_z;
    }
    else {
        fprintf(stderr, "[%s: %d] Unknown type of mutation function \"%s\".\n", __FILE__, __LINE__, argv[1]);
        exit(EXIT_FAILURE);
    }
}



static void parse_mutation_base_probability(double *base_mutation_probability, int argc, char **argv) {
    if (argc > 1) {
        sscanf(argv[1], "%lf", base_mutation_probability);
    }
}



static void parse_mutation_probability(MutationProbability *mutation_probability, int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "constant") == 0) {
        *mutation_probability = mutation_probability_constant;
    }
    else if (argc > 1 && strcmp(argv[1], "encourage-variance") == 0) {
        *mutation_probability = mutation_probability_encourage_variance;
    }
    else {
        fprintf(stderr, "[%s: %d] Unknown type of mutation probability \"%s\".\n", __FILE__, __LINE__, argv[1]);
        exit(EXIT_FAILURE);
    }
}



static void parse_population_initial_size(unsigned int *size, int argc, char **argv) {
    if (argc > 1) {
        sscanf(argv[1], "%u", size);
    }
}



static void parse_population_max_size(unsigned int *size, int argc, char **argv) {
    if (argc > 1) {
        sscanf(argv[1], "%u", size);
    }
}



static void parse_population_next_size(NextPopulationSize *next_population_size, int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "constant") == 0) {
        *next_population_size = next_population_size_constant;
    }
    else {
        fprintf(stderr, "[%s: %d] Unknown type of next population size function \"%s\".\n", __FILE__, __LINE__, argv[1]);
        exit(EXIT_FAILURE);
    }
}



static void parse_population_generator(InitialPopulation *initial_population, InitialPopulationParameters *parameters, int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "blank") == 0) {
        *initial_population = initial_population_blank;
    }
    else if (argc > 2 && strcmp(argv[1], "from-forest") == 0) {
        *initial_population = initial_population_from_forest;
        strncpy(parameters->forest, argv[2], PATH_SIZE);
    }
    else {
        fprintf(stderr, "[%s: %d] Unknown type of population generation function \"%s\".\n", __FILE__, __LINE__, argv[1]);
        exit(EXIT_FAILURE);
    }
}



static void parse_max_iteration(unsigned int *max_iteration, int argc, char **argv) {
    if (argc > 1) {
        sscanf(argv[1], "%u", max_iteration);
    }
}



static void parse_elitism(unsigned int *elitism, int argc, char **argv) {
    if (argc > 1) {
        sscanf(argv[1], "%u", elitism);
    }
}



static void parse_split_search_aggressiveness(double *aggressiveness, int argc, char **argv) {
    if (argc > 1) {
        sscanf(argv[1], "%lf", aggressiveness);
    }
}



static void parse_allowed_features(AllowFeatures *allow_feature, unsigned int *n, int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "all") == 0) {
        *allow_feature = allow_features_all;
        *n = 0;
    }
    else if (argc > 2 && strcmp(argv[1], "uniform") == 0) {
        *allow_feature = allow_features_uniform;
        sscanf(argv[2], "%u", n);
    }
    else {
        fprintf(stderr, "[%s: %d] Unknown type of allow features function \"%s\".\n", __FILE__, __LINE__, argv[1]);
        exit(EXIT_FAILURE);
    }
}



static void parse_seed(unsigned int *seed, int argc, char **argv) {
    if (argc > 1) {
        sscanf(argv[1], "%u", seed);
    }
}





/***********************************************************************
 * Pre-made functions.
 **********************************************************************/

double fitness_linear(const Individual individual, Status *status) {
    Performance performance;
    const double *parameters = status->fitness_parameters.linear;

    (void) status;
    decision_tree_performance(&performance, individual, individual.dataset, parameters[0]);

    return parameters[1] * (double) performance.correct / performance.samples
         + parameters[2] * (double) performance.wrong / performance.samples
         + parameters[3] * (double) performance.stable / performance.samples
         + parameters[4] * (double) performance.unstable / performance.samples
         + parameters[5] * (1.0 - (double) (performance.stable + performance.unstable) / performance.samples)
         + parameters[6] * (double) performance.robust / performance.samples
         + parameters[7] * (double) performance.vulnerable / performance.samples
         + parameters[8] * (double) performance.fragile / performance.samples
         + parameters[9] * (double) performance.broken / performance.samples;
}



void crossover_one_point(Individual *offspring, const Individual parent_a, const Individual parent_b, Status *status) {
    Stack S_parent, S_offspring;
    Node *subtree_a = choose_subtree(parent_a),
         *subtree_b = choose_subtree(parent_b);

    (void) status;
    decision_tree_create(offspring, parent_a.dataset);

    stack_create(&S_parent);
    stack_create(&S_offspring);

    stack_push(S_parent, parent_a.root);
    stack_push(S_offspring, offspring->root);

    while (!stack_is_empty(S_parent)) {
        Node *node_parent = stack_pop(S_parent),
             *node_offspring = stack_pop(S_offspring);
        SplitResult split_result;

        if (node_parent == subtree_a && node_parent->tree != subtree_b->tree) {
            node_parent = subtree_b;
        }

        if (node_is_leaf(*node_parent)) {
            continue;
        }

        split_result = node_split(node_offspring, node_parent->feature, node_parent->threshold);
        if (split_result == SPLIT_BOTH) {
            stack_push(S_offspring, node_offspring->right);
            stack_push(S_offspring, node_offspring->left);

            stack_push(S_parent, node_parent->right);
            stack_push(S_parent, node_parent->left);
        }
        else {
            stack_push(S_offspring, node_offspring);
            if (split_result == SPLIT_LEFT) {
                stack_push(S_parent, node_parent->left);
            }
            else {
                stack_push(S_parent, node_parent->left);
            }
        }
    }

    stack_delete(&S_parent);
    stack_delete(&S_offspring);
}



void mutation_grow(Individual *individual, Status *status) {
    Node *node = individual->root;
    unsigned int feature;
    double threshold, *buffer; 

    (void) status;   
    while (!node_is_leaf(*node)) {
        const double entropy_left = node_entropy(*node->left),
                     entropy_right = node_entropy(*node->right);
        const double p = (double) rand() / RAND_MAX;
        node = p < (entropy_left / (entropy_left + entropy_right)) ? node->left : node->right;
    }
    
    buffer = (double *) malloc(2 * individual->dataset.n_labels * sizeof(double));
    split_trial(&feature, &threshold, *node, buffer, status);
    node_split(node, feature, threshold);
    free(buffer);
}



void mutation_z(Individual *individual, Status *status) {
    Node *node = individual->root;
    unsigned int feature;
    double threshold, *buffer;

    (void) status;
    while (!node_is_leaf(*node)) {
        const double entropy = node_entropy(*node),
                     entropy_left = node_entropy(*node->left),
                     entropy_right = node_entropy(*node->right);
        const double p = (double) rand() / RAND_MAX;
        if (p < 1.0 - entropy) {
            node_prune(node);
            return;
        }

        node = p < (entropy_left / (entropy_left + entropy_right)) ? node->left : node->right;
    }

    buffer = (double *) malloc(2 * individual->dataset.n_labels * sizeof(double));
    split_trial(&feature, &threshold, *node, buffer, status);
    node_split(node, feature, threshold);
    free(buffer);
}



void initial_population_blank(Population *population, const Dataset training_set, Status *status) {
    unsigned int i;

    for (i = 0; i < status->max_population_size; ++i) {
        decision_tree_create(population->individuals + i, training_set);
        population->fitness[i] = status->compute_fitness(status->population.individuals[i], status);
        population->size = i + 1;
    }
}



unsigned int next_population_size_constant(Status *status) {
    return status->population.size;
}



double mutation_probability_constant(const double base_mutation_probability, Status *status) {
    (void) status;

    return base_mutation_probability;
}



double mutation_probability_encourage_variance(const double base_mutation_probability, Status *status) {
    double min = 1.0, max = 0.0;
    unsigned int i;

    (void) base_mutation_probability;
    for (i = 0; i < status->population.size; ++i) {
        const double fitness = status->population.fitness[i];

        if (fitness < min) {
            min = fitness;
        }
        if (fitness > max) {
            max = fitness;
        }
    }

    return 1.0 - (max - min);
}



void mutation_none(Individual *individual, Status *status) {
    (void) individual;
    (void) status;
}



Individual select_uniform(const Population population, Status *status) {
    (void) status;

    return population.individuals[rand() % population.size];
}



Individual select_roulette_wheel(const Population population, Status *status) {
    unsigned int i;
    double sum = 0.0,
           p = (double) rand() / RAND_MAX;

    (void) status;
    for (i = 0; i < population.size; ++i) {
        sum += population.fitness[i];
    }
    if (sum == 0.0) {
        return population.individuals[0];
    }

    for (i = 0; i < population.size; ++i) {
        if (p >= population.fitness[i] / sum) {
            return population.individuals[i];
        }
    }

    return population.individuals[0];
}



void initial_population_from_forest(Population *population, const Dataset training_set, Status *status) {
    Forest forest;
    FILE *fh;
    unsigned int i, min;

    fh = fopen(status->initial_population_parameters.forest, "r");
    forest_mapper_silva_load(fh, &forest, training_set);
    fclose(fh);

    min = forest.n_trees < status->max_population_size ? forest.n_trees : status->max_population_size;
    for (i = 0; i < min; ++i) {
        population->individuals[i] = forest.trees[i];
        population->fitness[i] = status->compute_fitness(forest.trees[i], status);
        population->size = i + 1;
    }

}



void allow_features_all(unsigned int *allowed_features, const unsigned int n_features, Status *status) {
    unsigned int i;

    for (i = 0; i < n_features; ++i) {
        allowed_features[i] = i;
    }
    status->n_allowed_features = n_features;
}



void allow_features_uniform(unsigned int *allowed_features, const unsigned int n_features, Status *status) {
    unsigned int i,
                 *features = (unsigned int *) malloc(n_features * sizeof(unsigned int));

    for (i = 0; i < n_features; ++i) {
        features[i] = i;
    }

    for (i = 0; i < status->n_allowed_features; ++i) {
        unsigned int next = rand() % (n_features - i);
        allowed_features[i] = features[next];
        if (next < n_features - i - 1) {
            memmove(features + next, features + next + 1, (n_features - i - next - 1) * sizeof(unsigned int));
        }
    }

    free(features);
}



void callback_status_print(Status *status) {
    ga_status_print(stdout, *status);
    printf("\n");
}



/***********************************************************************
 * Pubic functions.
 **********************************************************************/

DecisionTree ga_train(Status *status, const Dataset training_set) {
    unsigned int i;

    /* Choses features */
    if (status->n_allowed_features == 0) {
        status->n_allowed_features = training_set.space_size;
    }
    status->allowed_features = (unsigned int *) malloc(status->n_allowed_features * sizeof(unsigned int));
    status->allow_features(status->allowed_features, training_set.space_size, status);

    /* Generates initial population */
    status->initial_population(&status->population, training_set, status);

    /* Start */
    if (status->callback_start) {
        status->callback_start(status);
    }

    for (status->current_iteration = 0; status->current_iteration < status->max_iteration; ++status->current_iteration) {
        const unsigned int next_population_size = status->next_population_size(status);
        Population population_buffer;

        if (status->callback_before_iter) {
            status->callback_before_iter(status);
        }

        for (i = 0;  i < status->elitism; ++i) {
            status->population_swap.individuals[i] = status->population.individuals[i];
            status->population_swap.fitness[i] = status->population.fitness[i];
            status->population_swap.size = i + 1;
        }
        for (i = status->elitism; i < next_population_size; ++i) {
            const Individual parent_a = status->select(status->population, status),
                             parent_b = status->select(status->population, status);
            const double mutation_probability = status->mutation_probability(status->base_mutation_probability, status);
            Individual offspring;
            double fitness;
            unsigned int j;

            crossover_one_point(&offspring, parent_a, parent_b, status);
            if ((double) rand() / RAND_MAX < mutation_probability) {
                status->mutate(&offspring, status);
            }
            fitness = status->compute_fitness(offspring, status);

            for (j = 0; j < i; ++j) {
                if (status->population_swap.fitness[j] <= fitness) {
                    break;
                }
            }
            memmove(status->population_swap.individuals + j + 1, status->population_swap.individuals + j, (i - j) * sizeof(Individual));
            memmove(status->population_swap.fitness + j + 1, status->population_swap.fitness + j, (i - j) * sizeof(double));
            status->population_swap.individuals[j] = offspring;
            status->population_swap.fitness[j] = fitness;
            status->population_swap.size = i + 1;
        }
        population_buffer = status->population;
        status->population = status->population_swap;
        status->population_swap = population_buffer;
        for (i = status->elitism; i < status->population_swap.size; ++i) {
            decision_tree_delete(status->population_swap.individuals[i]);
        }

        if (status->callback_after_iter) {
            status->callback_after_iter(status);
        }
    }

    /* End */
    if (status->callback_end) {
        status->callback_end(status);
    }

    /* Frees memory */
    free(status->allowed_features);

    return status->population.individuals[0];
}



void ga_status_init(Status *status) {
    status->population.individuals = NULL;
    status->population.fitness = NULL;
    population_alloc(&status->population, DEFAULT_POPULATION_MAX_SIZE);
    status->population.size = 0;

    status->population_swap.individuals = NULL;
    status->population_swap.fitness = NULL;
    population_alloc(&status->population_swap, DEFAULT_POPULATION_MAX_SIZE);
    status->population_swap.size = 0;

    status->current_iteration = 0;

    status->compute_fitness = DEFAULT_FITNESS;
    status->fitness_parameters.linear[0] = 0.0;
    status->fitness_parameters.linear[1] = 1.0;
    status->fitness_parameters.linear[2] = 0.0;
    status->fitness_parameters.linear[3] = 0.0;
    status->fitness_parameters.linear[4] = 0.0;
    status->fitness_parameters.linear[5] = 0.0;
    status->fitness_parameters.linear[6] = 0.0;
    status->fitness_parameters.linear[7] = 0.0;
    status->fitness_parameters.linear[8] = 0.0;
    status->fitness_parameters.linear[9] = 0.0;

    status->select = DEFAULT_SELECT;
    status->crossover = DEFAULT_CROSSOVER;
    status->mutate = DEFAULT_MUTATION;
    status->base_mutation_probability = DEFAULT_BASE_MUTATION_PROBABILITY;
    status->mutation_probability = DEFAULT_MUTATION_PROBABILITY;
    status->initial_population_size = DEFAULT_POPULATION_INITIAL_SIZE;
    status->max_population_size = DEFAULT_POPULATION_MAX_SIZE;
    status->next_population_size = DEFAULT_POPULATION_NEXT_SIZE;
    status->initial_population = DEFAULT_POPULATION_GENERATOR;
    status->max_iteration = DEFAULT_MAX_ITERATION;
    status->elitism = DEFAULT_ELITISM;
    status->split_search_aggressiveness = DEFAULT_SPLIT_SEARCH_AGGRESSIVENESS;
    status->allow_features = DEFAULT_ALLOW_FEATURES;
    status->data = NULL;
    status->callback_start = NULL;
    status->callback_before_iter = NULL;
    status->callback_after_iter = NULL;
    status->callback_end = NULL;
    status->seed = DEFAULT_SEED;
    srand(status->seed);
}



void ga_status_clear(Status *status) {
    unsigned int i;

    for (i = 0; i < status->population.size; ++i) {
        decision_tree_delete(status->population.individuals[i]);
    }

    free(status->population.individuals);
    free(status->population.fitness);
    free(status->population_swap.individuals);
    free(status->population_swap.fitness);
}



void ga_parse_options(Status *status, int argc, char *argv[]) {
    int i;

    for (i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--fitness") == 0) {
            parse_fitness(&status->compute_fitness, &status->fitness_parameters, argc - i, argv + i);
        }
        else if (strcmp(argv[i], "--select") == 0) {
            parse_select(&status->select, argc - i, argv + i);
        }
        else if (strcmp(argv[i], "--crossover") == 0) {
            parse_crossover(&status->crossover, argc - i, argv + i);
        }
        else if (strcmp(argv[i], "--mutation") == 0) {
            parse_mutation(&status->mutate, argc - i, argv + i);
        }
        else if (strcmp(argv[i], "--mutation-base-probability") == 0) {
            parse_mutation_base_probability(&status->base_mutation_probability, argc - i, argv + i);
        }
        else if (strcmp(argv[i], "--mutation-probability") == 0) {
            parse_mutation_probability(&status->mutation_probability, argc - i, argv + 1);
        }
        else if (strcmp(argv[i], "--population-initial-size") == 0) {
            parse_population_initial_size(&status->initial_population_size, argc - i, argv + i);
        }
        else if (strcmp(argv[i], "--population-max-size") == 0) {
            parse_population_max_size(&status->max_population_size, argc - i, argv + i);
            population_alloc(&status->population, status->max_population_size);
            population_alloc(&status->population_swap, status->max_population_size);
        }
        else if (strcmp(argv[i], "--population-next-size") == 0) {
            parse_population_next_size(&status->next_population_size, argc - i, argv + i);
        }
        else if (strcmp(argv[i], "--population-generator") == 0) {
            parse_population_generator(&status->initial_population, &status->initial_population_parameters, argc - i, argv + i);
        }
        else if (strcmp(argv[i], "--max-iteration") == 0) {
            parse_max_iteration(&status->max_iteration, argc - i, argv + i);
        }
        else if (strcmp(argv[i], "--elitism") == 0) {
            parse_elitism(&status->elitism, argc - i, argv + i);
        }
        else if (strcmp(argv[i], "--split-search-aggressiveness") == 0) {
            parse_split_search_aggressiveness(&status->split_search_aggressiveness, argc - i, argv + i);
        }
        else if (strcmp(argv[i], "--allowed-features") == 0) { 
            parse_allowed_features(&status->allow_features, &status->n_allowed_features, argc - i, argv + i);
        }
        else if (strcmp(argv[i], "--seed") == 0) {
            parse_seed(&status->seed, argc - i, argv + i);
            srand(status->seed);
        }
    }
}



void ga_status_print(FILE *fh, const Status status) {
    unsigned int i;

    fprintf(fh, "Iteration %u / %u\n", status.current_iteration + 1, status.max_iteration);
    fprintf(fh, "Population size: %u / %u\n", status.population.size, status.max_population_size);
    fprintf(fh, "Fitness: [%.2g", status.population.fitness[0]);
    for (i = 1; i < status.population.size; ++i) {
        fprintf(fh, ", %.2g", status.population.fitness[i]);
    }
    fprintf(fh, "]\n");
}



void ga_options_print(FILE *fh) {
    const unsigned int name_length = 32,
                       option_length = 28;
    fprintf(fh, "Options:\n");
    fprintf(fh, "  %-*s Fitness function used to estimate performance of a tree\n", name_length, "--fitness");
    fprintf(fh, "      linear <magnitude> <correct> <wrong> <stable> <unstable> <no info> <robust> <fragile> <vulnerable> <broken>  Linear combination of standard performance indicators\n");

    fprintf(fh, "  %-*s Criterion used to choose parents for crossover\n", name_length, "--select");
    fprintf(fh, "      %-*s Choses a parent with a uniformally distributed probability\n", option_length, "uniform");
    fprintf(fh, "      %-*s Uses standard Roulette Wheel algorithm\n", option_length, "roulette-wheel");

    fprintf(fh, "  %-*s Crossover function\n", name_length, "--crossover");
    fprintf(fh, "      %-*s Substitutes a subtree from first parent with a subtree from second parent\n", option_length, "one-point");

    fprintf(fh, "  %-*s Mutation function\n", name_length, "--mutation");
    fprintf(fh, "      %-*s No mutation\n", option_length, "none");
    fprintf(fh, "      %-*s Allows trees to grow\n", option_length, "grow");
    fprintf(fh, "      %-*s Just too long to describe...\n", option_length, "Z");

    fprintf(fh, "  %-*s Base mutation probability in [0; 1], passed to mutation probability function\n", name_length, "--mutation-base-probability p");

    fprintf(fh, "  %-*s Mutation probability function\n", name_length, "--mutation-probability");
    fprintf(fh, "      %-*s Always returns mutation base probability\n", option_length, "constant");
    fprintf(fh, "      %-*s Returns higher probabilities when variance is low\n", option_length, "encourage-variance");

    fprintf(fh, "  %-*s Number of trees in initial population\n", name_length, "--population-initial-size n");

    fprintf(fh, "  %-*s Maximum number of trees in the population\n", name_length, "--population-max-size n");

    fprintf(fh, "  %-*s Function which computes size of next population\n", name_length, "--population-next-size");
    fprintf(fh, "      %-*s Always returns same size\n", option_length, "constant");

    fprintf(fh, "  %-*s Generator of intial population\n", name_length, "--population-generator");
    fprintf(fh, "      %-*s Generates single-node trees (every sample in root)\n", option_length, "blank");
    fprintf(fh, "      %-*s Reads trees from a forest file\n", option_length, "from-forest <path>");

    fprintf(fh, "  %-*s Maximum number of iterations for genetic algorithm\n", name_length, "--max-iteration n");

    fprintf(fh, "  %-*s Number of best individual to copy to next iteration\n", name_length, "--elitism n");

    fprintf(fh, "  %-*s Fraction of splits to consider during split generation, in [0; 1]\n", name_length, "--split-search-aggressiveness p");

    fprintf(fh, "  %-*s Features allowed for splits\n", name_length, "--allowed-features");
    fprintf(fh, "      %-*s Uses every feature\n", option_length, "all");
    fprintf(fh, "      %-*s Randomly choses n features from a uniform distribution\n", option_length, "uniform n");

    fprintf(fh, "  %-*s Seed for random number generation\n", name_length, "--seed n");
}
