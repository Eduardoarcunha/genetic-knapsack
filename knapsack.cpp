#include <iostream>
#include <random>
#include <vector>
#include <fstream>

using namespace std;

struct Item
{
    int id;
    int w;
    int v;
};

const int numGenerations = 100;

const int maxItems = 60;
const int maxWeight = 15;
const int maxValue = 100;
const int knapsackCapacity = 100;

const int solutionsByGeneration = 10;
const int numParents = 3;
const int numChildren = solutionsByGeneration - numParents;

const float crossoverRate = 0.8;
const float mutationRate = 0.3;

void generator(int &totalItems, vector<Item> &items, mt19937 &gen);
void firstGeneration(vector<vector<int>> &generation, int totalItems, mt19937 &gen);
void fitness(vector<int> &fittedGeneration, vector<Item> items, vector<vector<int>> generation, int totalItems, int cntGen);
void parentsSelection(vector<vector<int>> &parents, vector<int> fittedGeneration, vector<vector<int>> generation, int cntGen);
void crossover(vector<vector<int>> &children, vector<vector<int>> parents, int totalItems, mt19937 &gen);
void mutation(vector<vector<int>> &children, int totalItems, mt19937 &gen);
void fitMean(vector<float> &fitnessHist, vector<int> fittedGeneration);

int main(int argc, char *argv[]){

    int totalItems;
    int seedValue;
    vector<Item> items;
    vector<vector<int>> generation;
    vector<int> fittedGeneration;
    vector<vector<int>> parents;
    vector<vector<int>> children;
    vector<float> fitnessHist;

    seedValue = atoi(argv[1]);

    mt19937 gen(seedValue);
    generator(totalItems, items, gen);

    firstGeneration(generation, totalItems, gen);

    string outputFileName = "out-knap-" + to_string(seedValue) + ".txt";
    ofstream outputFile(outputFileName);

    // if (!outputFile.is_open()) {
    //     std::cerr << "Error opening output file." << std::endl;
    //     return 1;
    // }

    outputFile << "N " << totalItems << " C " << knapsackCapacity << endl << endl;
    for (int i = 0; i < numGenerations; i++){

        fitness(fittedGeneration, items, generation, totalItems, i);

        fitMean(fitnessHist, fittedGeneration);
        outputFile << "GEN " << i << endl;
        outputFile << "FIT_MEAN " << fitnessHist[i] << endl;
        for (int j = 0; j < solutionsByGeneration; j++){
            for (int k = 0; k < totalItems; k++){
                outputFile << generation[j][k] << " ";
            }
            outputFile << fittedGeneration[j] << " \n";
        }
        outputFile << endl;

        parentsSelection(parents, fittedGeneration, generation, i);
        crossover(children, parents, totalItems, gen);
        mutation(children, totalItems, gen);

        for (int j = 0; j < solutionsByGeneration; j++){
            
            if (j < numParents){
                generation[j] = parents[j];
            } else {
                generation[j] = children[j - numParents];
            }
        }
    }
    outputFile.close();

    return 0;
}

void generator(int &totalItems, vector<Item> &items, mt19937 &gen){
    cout << "Generating items:\n";
    uniform_int_distribution<> distr(1, maxItems+1);
    totalItems = distr(gen);

    for(int i=0; i<totalItems; i++){
        Item newItem;
        newItem.id = i;

        distr.param(uniform_int_distribution<int>::param_type(1, maxWeight+1));
        newItem.w = distr(gen);
        
        distr.param(uniform_int_distribution<int>::param_type(1, maxValue+1));
        newItem.v = distr(gen);

        items.push_back(newItem);

        cout << "Id: " << newItem.id << " Weight: " << newItem.w << " Value: " << newItem.v << endl;
    }
}

void firstGeneration(vector<vector<int>> &generation, int totalItems, mt19937 &gen){

    uniform_int_distribution<> distr(0, 1);

    // cout << "First gen:\n";

    for (int i = 0; i < solutionsByGeneration; i++){
        // cout << "Solution " << i << endl;
        vector<int> solution;
        generation.push_back(solution);

        for (int j = 0; j < totalItems; j++){
            int k = distr(gen);
            generation[i].push_back(k);
            // cout << generation[i][j] << " ";
        }
        // cout << endl;
    }
}

void fitness(vector<int> &fittedGeneration, vector<Item> items, vector<vector<int>> generation, int totalItems, int cntGen){
    // cout << "Fittings:" << endl;
    for (int i = 0; i < solutionsByGeneration; i++){
        int s1 = 0, s2 = 0;
        for (int j = 0; j < totalItems; j++){
            s1 += items[j].v * generation[i][j];
            s2 += items[j].w * generation[i][j];
        }
        if (s2 <= knapsackCapacity){
            if (cntGen == 0){
                fittedGeneration.push_back(s1);
            } else {
                fittedGeneration[i] = s1;
            }
            // cout << i << ": " << s1 << endl;
        } else {
            if (cntGen == 0){
                // fittedGeneration.push_back(0);
                fittedGeneration.push_back(knapsackCapacity - s2);
            } else {
                // fittedGeneration[i] = 0;
                fittedGeneration[i] = knapsackCapacity - s2;
            }
            // cout << i << ": " << 0 << endl;
        }
    }
}

void parentsSelection(vector<vector<int>> &parents, vector<int> fittedGeneration, vector<vector<int>> generation, int cntGen){
    int maxValue;
    int maxValueIdx;

    // cout << "Selected Parents:\n";
    for (int i = 0; i < numParents; i++){
        maxValue = fittedGeneration[0];
        maxValueIdx = 0;

        for (int j = 0; j < solutionsByGeneration; j ++){
            if (fittedGeneration[j] > maxValue) {
                maxValue = fittedGeneration[j];
                maxValueIdx = j;
            }
        }
        if (cntGen == 0){
            parents.push_back(generation[maxValueIdx]);
        } else {
            parents[i] = generation[maxValueIdx];
        }
        
        fittedGeneration[maxValueIdx] = -1;
        // cout << maxValueIdx << " ";
    }
    // cout << endl;
}

void crossover(vector<vector<int>> &children, vector<vector<int>> parents, int totalItems, mt19937 &gen){
    uniform_real_distribution<float> distr(0.0f, 1.0f);

    int cntChildren = 0;
    int k = 0;
    int p1_idx, p2_idx;
    int crossoverPoint = totalItems / 2;
    
    // cout << "New children\n";
    while (cntChildren < numChildren) {
        p1_idx = k % numParents;
        p2_idx = (k+1) % numParents;

        float f = distr(gen);
        if (crossoverRate > f){
            vector<int> child;

            // cout << "Child " << cntChildren << ": ";

            for (int i = 0; i < crossoverPoint; i++){
                // cout << parents[p1_idx][i] << " ";
                child.push_back(parents[p1_idx][i]);
            }
            for (int i = crossoverPoint; i < totalItems; i++){
                // cout << parents[p2_idx][i] << " ";
                child.push_back(parents[p2_idx][i]);
            }
            // cout << endl;

            children.push_back(child);
            cntChildren++;
        }
        k++;
    }
}

void mutation(vector<vector<int>> &children, int totalItems, mt19937 &gen){
    uniform_real_distribution<float> distr(0.0f, 1.0f);
    uniform_real_distribution<> distr2(0, totalItems);

    for (int i = 0; i < numChildren; i++){
        
        float f = distr(gen);
        if (mutationRate > f){
            int j = distr2(gen);
            // cout << j << " ";
            if (children[i][j] == 0){
                children[i][j] = 1;
            } else {
                children[i][j] = 0;
            }
        }
    }
}

void fitMean(vector<float> &fitnessHist, vector<int> fittedGeneration){
    float sum = 0;
    for (int i = 0; i < solutionsByGeneration; i++){
        sum += fittedGeneration[i];
    }
    fitnessHist.push_back(sum / solutionsByGeneration);
}