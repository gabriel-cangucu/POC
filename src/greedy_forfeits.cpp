#include <bits/stdc++.h>
#include "../json-develop/single_include/nlohmann/json.hpp"

using json = nlohmann::json;


struct Params {
    int instanceID;
    int numberOfItems;
    int numberOfPairs;
    int budget;
    std::vector<int> weights;
    std::vector<int> profits;
    std::vector<int> forfeits;
    std::vector<std::array<int, 2>> forfeitPairs;
};


int argmax(std::map<int, double> values) {
    auto maxRatio = -std::numeric_limits<double>::infinity();
    int i;

    for (auto& pair : values) {
        int item = pair.first;
        double ratio = pair.second;

        if (ratio > maxRatio) {
            maxRatio = ratio;
            i = item;
        }
    }

    return i;
}


bool valueInSolution(int value, std::vector<int>& solution) {
    if (std::find(solution.begin(), solution.end(), value) != solution.end())
        return true;
    else
        return false;
}


std::vector<int> greedyForfeits(struct Params* params) {
    std::vector<int> solution;
    int residualBudget = params->budget;

    while (int(solution.size()) < params->numberOfItems) {
        std::vector<int> possibleItems;

        for (int i = 0; i < params->numberOfItems; i++)
            // If i is not in the solution and has a weight no greater than the residual budget
            if (params->weights[i] <= residualBudget && !valueInSolution(i, solution))
                possibleItems.push_back(i);
        
        if (possibleItems.empty())
            return solution;

        std::map<int, double> ratios;

        for (auto& i : possibleItems) {
            int newProfit = params->profits[i];

            for (int k = 0; k < params->numberOfPairs; k++) {
                std::array<int, 2> pair = params->forfeitPairs[k];

                // Forfeit pairs {i, j} such that j is already in the solution
                if (pair[0] == i && valueInSolution(pair[1], solution))
                    newProfit -= params->forfeits[k];
                
                else if (pair[1] == i && valueInSolution(pair[0], solution))
                    newProfit -= params->forfeits[k];
            }

            ratios[i] = newProfit / params->weights[i];
        }

        int bestItem = argmax(ratios);

        if (ratios[bestItem] < 0)
            // The best item will decrease the value of the knapsack, so we don't include it
            return solution;
        
        solution.push_back(bestItem);
        residualBudget -= params->weights[bestItem];
    }

    return solution;
}


void removeLastChoices(std::vector<int>& solution, double beta) {
    /*
    Removes the last beta*|S| items from the solution S.
    */

    int n = solution.size();

    int begin = n - std::round(beta*n);
    int end = n;

    solution.erase(std::next(solution.begin(), begin), std::next(solution.begin(), end));
}


int greedyForfeitsSingle(struct Params* params, std::vector<int>& solution) {
    int residualBudget = params->budget;
    for (auto& i : solution) residualBudget -= params->weights[i];

    std::vector<int> possibleItems;

    for (int i = 0; i < params->numberOfItems; i++)
        // If i is not in the solution and has a weight no greater than the residual budget
        if (params->weights[i] <= residualBudget && !valueInSolution(i, solution))
            possibleItems.push_back(i);

    std::map<int, double> ratios;

    for (auto& i : possibleItems) {
        int newProfit = params->profits[i];

        for (int k = 0; k < params->numberOfPairs; k++) {
            std::array<int, 2> pair = params->forfeitPairs[k];

            // Forfeit pairs {i, j} such that j is already in the solution
            if (pair[0] == i && valueInSolution(pair[1], solution))
                newProfit -= params->forfeits[k];
            
            else if (pair[1] == i && valueInSolution(pair[0], solution))
                newProfit -= params->forfeits[k];
        }

        ratios[i] = newProfit / params->weights[i];
    }

    int bestItem = argmax(ratios);
    return bestItem;
}


std::vector<int> greedyForfeitsInit(struct Params* params, std::vector<int>& solution) {
    std::vector<int> newSolution(solution);

    int residualBudget = params->budget;
    for (auto& i : newSolution) residualBudget -= params->weights[i];

    while (int(newSolution.size()) < params->numberOfItems) {
        std::vector<int> possibleItems;

        for (int i = 0; i < params->numberOfItems; i++)
            // If i is not in the solution and has a weight no greater than the residual budget
            if (params->weights[i] <= residualBudget && !valueInSolution(i, newSolution))
                possibleItems.push_back(i);
        
        if (possibleItems.empty())
            return newSolution;

        std::map<int, double> ratios;

        for (auto& i : possibleItems) {
            int newProfit = params->profits[i];

            for (int k = 0; k < params->numberOfPairs; k++) {
                std::array<int, 2> pair = params->forfeitPairs[k];

                // Forfeit pairs {i, j} such that j is already in the solution
                if (pair[0] == i && valueInSolution(pair[1], newSolution))
                    newProfit -= params->forfeits[k];
                
                else if (pair[1] == i && valueInSolution(pair[0], newSolution))
                    newProfit -= params->forfeits[k];
            }

            ratios[i] = newProfit / params->weights[i];
        }

        int bestItem = argmax(ratios);

        if (ratios[bestItem] < 0)
            // The best item will decrease the value of the knapsack, so we don't include it
            return newSolution;
        
        newSolution.push_back(bestItem);
        residualBudget -= params->weights[bestItem];
    }

    return newSolution;
}


std::vector<int> carouselForfeits(struct Params* params, int alpha, double beta) {
    auto solution = greedyForfeits(params);
    removeLastChoices(solution, beta);

    for (unsigned i = 0; i < alpha*solution.size(); i++) {
        // Removes the oldest choice
        solution.erase(solution.begin());

        int bestItem = greedyForfeitsSingle(params, solution);
        solution.push_back(bestItem);
    }

    auto newSolution = greedyForfeitsInit(params, solution);
    return newSolution;
}


bool fileIsEmpty(std::string fileName) {
    std::ifstream f(fileName);
    return f.peek() == std::ifstream::traits_type::eof();
}


void storeResultsToCSV(
    std::vector<int>& solution, std::string fileName, struct Params* params, double timeElapsed
) {
    int totalProfit = 0;
    int forfeitsPaid = 0;

    for (auto& i : solution)
        totalProfit += params->profits[i];

    for (int k = 0; k < params->numberOfPairs; k++) {
        std::array<int, 2> pair = params->forfeitPairs[k];

        if (valueInSolution(pair[0], solution) && valueInSolution(pair[1], solution)) {
            totalProfit -= params->forfeits[k];
            forfeitsPaid++;
        }
    }

    std::ofstream resultsFile;
    // Opening in append mode
    resultsFile.open(fileName, std::ios_base::app);

    // Adding a header if the file was just created
    if (fileIsEmpty(fileName))
        resultsFile << "n,id,sol,#forf.,time(s)\n";

    resultsFile << std::fixed << std::setprecision(2);
    
    resultsFile << params->numberOfItems << ',';
    resultsFile << params->instanceID << ',';
    resultsFile << totalProfit << ',';
    resultsFile << forfeitsPaid << ',';
    resultsFile << timeElapsed << '\n';

    resultsFile.close();
}


int main()
{
    std::ifstream inputFile("instances.json");
    json instances = json::parse(inputFile);

    for (unsigned t = 0; t < instances.size(); t++) {
        Params* params = new Params();

        params->instanceID    = instances[t].at("instance_id");
        params->numberOfItems = instances[t].at("number_of_items");
        params->numberOfPairs = instances[t].at("number_of_pairs");
        params->budget        = instances[t].at("budget");
        params->weights       = instances[t].at("weights").get<std::vector<int>>();
        params->profits       = instances[t].at("profits").get<std::vector<int>>();
        params->forfeits      = instances[t].at("forfeits").get<std::vector<int>>();
        params->forfeitPairs  = instances[t].at("forfeit_pairs").get<std::vector<std::array<int, 2>>>();

        std::vector<int> solution;
        std::chrono::steady_clock::time_point beginTime, endTime;
        double timeElapsed;

        // Greedy Forfeits
        beginTime = std::chrono::steady_clock::now();
        solution = greedyForfeits(params);
        endTime = std::chrono::steady_clock::now();

        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count();
        timeElapsed /= 1000;

        storeResultsToCSV(solution, "greedy_results.csv", params, timeElapsed);

        // Carousel Forfeits
        beginTime = std::chrono::steady_clock::now();
        solution = carouselForfeits(params, 2, 0.05);
        endTime = std::chrono::steady_clock::now();

        timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count();
        timeElapsed /= 1000;

        storeResultsToCSV(solution, "carousel_results.csv", params, timeElapsed);

        delete params;
    }

    inputFile.close();
    return 0;
}
