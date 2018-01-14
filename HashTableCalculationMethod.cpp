/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HashTableCalculationMethod.cpp
 * Author: paf
 * 
 * Created on November 5, 2017, 1:56 PM
 */

#include "HashTableCalculationMethod.h"

#include <limits.h>
#include <algorithm>
#include <limits>

//int HashTableCalculationMethod::PHI_VALUE[] = {0, -1, 1, -1, 3, -1, 2, -1}; //TODO: depends when invertible hash is called this might still be needed

int HashTableCalculationMethod::PHI_VALUE[][16] = {
    /*A: */{0, 3, 3, 3, 67, 579, 2627, 10819, 59908, 256600, 1042024, 4183720, 16761256, 67045087, 268180411, 1072721685}, 
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    /*C:*/ {0, 2, 6, 6, 134, 134, 1158, 5254, 54385, 250741, 1036837, 2084069, 10472677, 60782821, 127891685, 932509926}, 
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    /*T: */{0, 0, 12, 12, 12, 268, 2316, 10508, 43339, 239695, 1026463, 2073695, 14651231, 48184159, 115293023, 651475805},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    /*G:*/ {0, 1, 9, 9, 201, 713, 3785, 16073, 48862, 245554, 1031650, 4173346, 4173346, 54461986, 255788578, 792659488},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};


int HashTableCalculationMethod::POW_4_VALUE[] = {1, 4, 16, 64, 256,
                        1024, 4096, 16384, 65536, 262144, 1048576,
                        4194304, 16777216, 67108864, 268435456, 1073741824};

int HashTableCalculationMethod::INV_HASH_MASK[] = {0x0, 0x3, 0xf, 0x3f, 0xff, 
                        0x3ff, 0xfff, 0x3fff, 0xffff, 0x3ffff, 0xfffff, 0x3fffff, 
                        0xffffff, 0x3ffffff, 0xfffffff, 0x3fffffff};


HashTableCalculationMethod::HashTableCalculationMethod() {
}

HashTableCalculationMethod::~HashTableCalculationMethod() {
}

HashTable* HashTableCalculationMethod::calculate(FastADocument* document, int w, int k) {

    // TODO: change so it follows the logic from the text
    // The key is the minimizer hash value and the value is a set of target sequence index, the position of the minimizer and the strand
    
    std::unordered_map<int,std::vector<bioinformatics::Entry>*> *hashTable = new std::unordered_map<int,std::vector<bioinformatics::Entry>*>();
    
    std::vector<Minimizer> minimizerSet;
    
    BioSequence *sequence;
    while ((sequence = document->getNextSequence()) != NULL) {
                
        minimizerSet.clear();
        minimizerSketch(sequence,w,k,minimizerSet);
        
        for (auto it = minimizerSet.begin(); it != minimizerSet.end(); it++) {
            
            bioinformatics::Entry entry;
            entry.sequencePosition = sequence->getSequencePosition();
            entry.i = it->i;
            entry.r = it->r;
            
            std::unordered_map<int,std::vector<bioinformatics::Entry>*>::iterator mapIt = hashTable->find(it->m);
            if (mapIt != hashTable->end()) {
                std::vector<bioinformatics::Entry>* entrySet = mapIt->second;
                entrySet->push_back(entry);
            } else {
                std::vector<Entry>* entrySet = new std::vector<Entry>;
                entrySet->push_back(entry);
                hashTable->insert(std::pair<int, std::vector<bioinformatics::Entry>*>(it->m, entrySet));
            }
        }
        
        delete sequence;
    }
    
    //Fit vectors to the minimum memory size they need
    for(auto it=hashTable->begin(); it!=hashTable->end(); it++){
        
        //TODO: it seems we maybe don't need this part
        /*
        std::set<bioinformatics::Entry> s( it->second->begin(), it->second->end());
	it->second->clear();
	for(auto setIt = s.begin(); setIt!=s.end(); setIt++){
	    it->second->push_back(*setIt);
	}
        */
        
        it->second->shrink_to_fit();
    }
    
    return new HashTable(hashTable);
}


int HashTableCalculationMethod::PHI_function(std::string *seqence, int startIndex, int k) {
    int hashValue = 0;
    
    for (int i = 0; i < k; i++){
        //char c = (*seqence)[startIndex + i];
//        int cHash = PHI_function((*seqence)[startIndex + i]);//TODO: old
        int cHash = PHI_function((*seqence)[startIndex + i],k);
        
        //powf(4.0, k-i-1)
        //(0x1 << (2*(k-i-1)))
        //POW_4_VALUE[k-i-1]
        hashValue +=  POW_4_VALUE[k-i-1]*cHash;
    }
    
//    hashValue = invertibleHash(hashValue,INV_HASH_MASK[k]); //TODO: if 1D
    
    return hashValue;
}

inline int HashTableCalculationMethod::PHI_function(char b, int k) {

    // A 0x41   0100 0001   0
    // T 0x54   0101 0100   2
    // C 0x43   0100 0011   1
    // G 0x47   0100 0111   3
    // mask     0000 0110
 /*   
    int res;    
    if (b == 'A') {
        res = 0;
    } else if (b == 'C') {
        res = 1;
    } else if (b == 'G') {
        res = 2;
    } else if (b == 'T') {
        res = 3;
    }
*/        
    
    //return PHI_VALUE[b & 0x6];//TODO: if only 1D
    return PHI_VALUE[b & 0x6][k];
}

// ALGORITHM 2
int HashTableCalculationMethod::invertibleHash(int x, int m) {

    //int m = (0x1 << p) - 1; //says p=2k
    x = (~x + (x << 21)) & m;
    x = x^x >> 24;
    x = (x + (x << 3) + (x << 8)) & m;
    x = x^x >> 14;
    x = (x +(x << 2) + (x << 4)) & m;
    x = x^x >> 28;
    x = (x + (x << 31)) & m;
    
    return x;
}

// ALGORITHM 1
void HashTableCalculationMethod::minimizerSketch(bioinformatics::BioSequence *sequence, int w, int k, std::vector<Minimizer>& M) {
    
    std::string* raw_sequence = sequence->getSequence();
    std::string* raw_inv_sequence = sequence->getInvertedSequence();

    Minimizer min;
    int u[w-1];
    int v[w-1];
    int minuv[w-1];
    int pos[w-1];
    
    {
        int m = std::numeric_limits<int>::max();
        for (int j = 0; j < w-1; j++) {
            u[j] = PHI_function(raw_sequence, 1 + j, k);
            v[j] = PHI_function(raw_inv_sequence, 1 + j, k);
            
            if (u[j] != v[j]) {
                minuv[j] = std::min(u[j], v[j]);
                m = std::min(m,minuv[j]); 
            }
            else{
                minuv[j] = std::numeric_limits<int>::max();
            }
            
            pos[j] = 1 + j;
        }
        
        for (int j = 0; j < w-1; j++) {

            if (u[j] == m && u[j] < v[j]) {
                min.m = m;
                min.i = pos[j];
                min.r = 0;
                
                M.push_back(min);//a copy is created
            } else if (v[j] == m && v[j] < u[j]) {
                min.m = m;
                min.i = pos[j];
                min.r = 1;
                
                M.push_back(min);//a copy is created
            }
        }    
    }
    
    int current_id = 0;
    
    for (int i = 2, limit = sequence->size()- w-k+1; i < limit; i++) {
        
        u[current_id] = PHI_function(raw_sequence, i + w-1-1, k);
        v[current_id] = PHI_function(raw_inv_sequence, i + w-1-1, k);
        pos[current_id] = i + w-1-1;

        if (u[current_id] != v[current_id]) {
            minuv[current_id] = std::min(u[current_id], v[current_id]);
        }
        else{
            minuv[current_id] = std::numeric_limits<int>::max();
        }

        
        int m = std::numeric_limits<int>::max();
        for (int j = 0; j < w-1; j++) {
            m = std::min(m,minuv[j]); 
        }
        
        for (int j = 0; j < w-1; j++) {

            if (u[j] == m && u[j] < v[j]) {
                min.m = m;
                min.i = pos[j];
                min.r = 0;
                
                M.push_back(min);//a copy is created
            } else if (v[j] == m && v[j] < u[j]) {
                min.m = m;
                min.i = pos[j];
                min.r = 1;
                
                M.push_back(min);//a copy is created
            }
        }
        
        current_id = (current_id+1)%(w-1);
    }
    
    std::set<Minimizer> s(M.begin(), M.end());
    M.clear();
    for(auto setIt = s.begin(); setIt!=s.end(); setIt++){
        M.push_back(*setIt);
    }
    M.shrink_to_fit();    
}
