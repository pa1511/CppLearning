/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   QueryMapper.cpp
 * Author: papric
 * 
 * Created on December 17, 2017, 11:46 AM
 */

#include "QueryMapper.h"
#include <stdlib.h>   
#include <time.h>   
#include <iostream>

using namespace bioinformatics;

QueryMapper::QueryMapper() {
}

QueryMapper::~QueryMapper() {
}

void QueryMapper::mapQuerySequence(HashTable *H, FastADocument *targetFastADoc,
        BioSequence *q, PAF *output, int w, int k, int epsilon){
    
    int MATCHING_BASES_MIN_COUNT = 100;
    int MIN_MAPPING_SUBSET_SIZE = 4;

    std::vector<ATuple> A;

    
    {//added block so Minimizers sets get removed from memory as soon as they are no longer needed
        HashTableCalculationMethod method;
        std::vector<Minimizer> queryMinimizerSet0;
        std::vector<Minimizer> queryMinimizerSet1;
        method.minimizerSketch(q, w, k,queryMinimizerSet0,queryMinimizerSet1);

        auto hashTable0 = H->getHashTableRaw0();
        auto hashTable1 = H->getHashTableRaw1();

        ATuple tuple;

        fillASame(queryMinimizerSet0, hashTable0, A, tuple);
        fillADiff(queryMinimizerSet0, hashTable1, A, tuple);
        fillASame(queryMinimizerSet1, hashTable1, A, tuple);
        fillADiff(queryMinimizerSet1, hashTable0, A, tuple);
    }    
    // std::printf("Finished building A\n");
    
    std::sort(A.begin(), A.end()); 
    std::vector<ATuple>* lisC;

     srand (time(NULL));

    
    int b = 0;
    for (int e = 0, size = A.size(); e < size; e++){
        if ((e == size - 1) ||
                (A[e + 1].t != A[e].t) || (A[e + 1].r != A[e].r) ||
                (A[e + 1].c - A[e].c >= epsilon)) {
    
            if(e-b>=MIN_MAPPING_SUBSET_SIZE){
                
                //LongestIncreasingSubsequence(A, b, e, lisC);
                lisC = lis(A,b,e);
                
                int N = lisC->size();
                int matchingBases = N * k;

                /*
                std::cout << "Lis group" << std::endl;
                for(int k=b; k<=e; k++){
                    std::cout << "T: " << A[k].t << " R: " << unsigned(A[k].r) 
                            << " C: " << A[k].c << " I: " << A[k].i << std::endl;
                }
                std::cout << std::endl;
                std::cout << "Lis result" << std::endl;
                
                for(auto lisIter = lisC->begin(); lisIter!=lisC->end(); lisIter++){
                    std::cout << "T: " << lisIter->t << " R: " << unsigned(lisIter->r) 
                            << " C: " << lisIter->c << " I: " << lisIter->i << std::endl;
                }
                std::cout << std::endl;
                std::cout << std::endl;
                */
                
                if (N >= MIN_MAPPING_SUBSET_SIZE && matchingBases >= MATCHING_BASES_MIN_COUNT) {
                    output->print(q, targetFastADoc, &((*lisC)[0]), &((*lisC)[N-1]), N);
                }
                
                /*if ((*lisC)[0].i > (*lisC)[lisC->size()-1].i ||  rand() % 1000 == 0) {
                    for (int g = 0; g < lisC->size(); g++) {
                        printf("(i=%d, c=%d)  ", (*lisC)[g].i, (*lisC)[g].c);
                    }

                    printf("\n\n");
                }
                
                
                
                for (int g = 1; g < lisC->size(); g++) {
                    if ((*lisC)[g-1].i <= (*lisC)[g].i) continue;
                    printf("(i=%d,g=%d,b=%d,e=%d)  ", (*lisC)[g-1].i, g-1, b, e);
                    printf("(i=%d,g=%d,b=%d,e=%d)  ", (*lisC)[g].i, g, b, e);
                    printf("\n\n");
                }*/
                
                delete lisC;
            }
            
            b = e + 1;
        }
    }
}

void QueryMapper::fillASame(std::vector<Minimizer>& queryMinimizerSet, std::unordered_map<int, std::vector<bioinformatics::Entry>*> * hashTable, std::vector<ATuple>& A, ATuple& tuple){
    for (auto qMsIt = queryMinimizerSet.begin(); qMsIt != queryMinimizerSet.end(); qMsIt++) { 
        auto hashEntry = hashTable->find(qMsIt->m);
        if (hashEntry == hashTable->end())
            continue;
        
        auto entrySet = hashEntry->second;
        
        for (auto entryIt = entrySet->begin(); entryIt != entrySet->end(); entryIt++) {
                tuple.t = entryIt->sequencePosition;
                tuple.r = 0;
                tuple.c = qMsIt->i - entryIt->i;
                tuple.i = entryIt->i;
                A.push_back(tuple);
        }
    }
}

void QueryMapper::fillADiff(std::vector<Minimizer>& queryMinimizerSet, std::unordered_map<int, std::vector<bioinformatics::Entry>*> * hashTable, std::vector<ATuple>& A, ATuple& tuple){
    for (auto qMsIt = queryMinimizerSet.begin(); qMsIt != queryMinimizerSet.end(); qMsIt++) { 
        auto hashEntry = hashTable->find(qMsIt->m);
        if (hashEntry == hashTable->end())
            continue;
        
        auto entrySet = hashEntry->second;
        
        for (auto entryIt = entrySet->begin(); entryIt != entrySet->end(); entryIt++) {
            tuple.t = entryIt->sequencePosition;
            tuple.r = 1;
            tuple.c = qMsIt->i + entryIt->i;
            tuple.i = entryIt->i;
            A.push_back(tuple);
        }
    }
}

/*
void QueryMapper::LongestIncreasingSubsequence(std::vector<ATuple>& A, int b, int e, std::vector<ATuple>& ret) {
    
    int n=e-b;
    
    std::vector<int> tail(n+1, 0);
    std::vector<int> prev(n+1, -1);
    
    int len = 1;
    
    for (int i = 1; i < n; i++) {
        if (A[i+b].i < A[tail[0]+b].i) {
            tail[0] = i;
        } else if (A[tail[len - 1]+b].i < A[i+b].i) {
            prev[i] = tail[len - 1];
            tail[len++] = i;
        } else {
            int pos = 0;
            for (int j = 1; j < n; j++) {
                if (A[pos+b].i < A[j+b].i) {
                    pos = j;
                }
            }
            prev[i] = tail[pos - 1];
            tail[pos] = i;
        }
    }
    
    for (int i = tail[len - 1]; i >= 0; i = prev[i]) {
        ret.insert(ret.begin(), A[i+b]);
    }
}
*/

std::vector<ATuple>* QueryMapper::lis(std::vector<ATuple>& array, int b, int e) {

    std::vector<ATuple>* s0 = new std::vector<ATuple>();
    s0->push_back(array[b]);

    std::vector<std::vector<ATuple>*> solutions;
    solutions.push_back(s0);


    for (int i = b+1; i <= e; i++) {

        ATuple elem = array[i];

        std::vector<ATuple>* longest_solution = solutions.back();
        ATuple last_max = longest_solution->back();

        if (last_max.i <= elem.i) {
            std::vector<ATuple>* new_longest_solution = new std::vector<ATuple>(*longest_solution);
            new_longest_solution->push_back(elem);
            solutions.push_back(new_longest_solution);
        } else {
            for (auto solution = solutions.begin(); solution < solutions.end(); solution++) {
                int sol_size = (*solution)->size();

                ATuple last = (*solution)->at(sol_size - 1);

                if (sol_size == 1) {
                    if (elem.i < last.i) {
                        (*solution)->clear();
                        (*solution)->push_back(elem);
                    }
                } else {
                    ATuple second_last = (*solution)->at(sol_size - 2);

                    if (elem.i >= second_last.i && elem.i < last.i) {
                        (*solution)->clear();
                        (*solution)->insert((*solution)->end(), (*(solution - 1))->begin(), (*(solution - 1))->end());
                        (*solution)->push_back(elem);
                    }
                }
            }
        }
    }

    std::vector<ATuple>* solution = solutions.back();
    solutions.pop_back();

    //free memory
    while (!solutions.empty()) {
        std::vector<ATuple>* s = solutions.back();
        solutions.pop_back();
        delete s;
    }

    return solution;
}