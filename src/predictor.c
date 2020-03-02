//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <inttypes.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Xuanang Li and Xuan Zhang";
const char *studentID   = "A92121357 and A________";
const char *email       = "xul065@ucsd.edu and _________";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int hpBits;  // Number of bits used for History Pattern
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

uint32_t global_history;
uint8_t* global_table;
uint32_t* local_history_table;
uint8_t* local_pattern_table;
uint8_t* choice_table;
uint32_t global_mask; // extract the last ghistorybits
uint32_t pc_mask; // extract the last pcIndexBits
uint32_t local_mask; // extract the last lhistorybits
uint32_t pattern_mask;

//CREDIT: https://gist.github.com/badboy/6267743
//Robert Jenkins' 32 bit integer hash function
uint32_t hash(uint32_t a)
{
    a = (a+0x7ed55d16) + (a<<12);
    a = (a^0xc761c23c) ^ (a>>19);
    a = (a+0x165667b1) + (a<<5);
    a = (a+0xd3a2646c) ^ (a<<9);
    a = (a+0xfd7046c5) + (a<<3);
    a = (a^0xb55a4f09) ^ (a>>16);
    return a;
}

//hash the pc
uint32_t hashPC(uint32_t a)
{
  return hash(a) & pc_mask;
}

//hash the global history
uint32_t hashGH(uint32_t a)
{
  return hash(a) & global_mask;
}

//hash the history pattern
uint32_t hashHP(uint32_t a)
{
  return hash(a & pattern_mask) & local_mask;
}


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{

  if(bpType == CUSTOM){
    ghistoryBits = 9; // Number of bits used for Global History
    lhistoryBits = 12; // Number of bits used for Local History
    pcIndexBits = 10;  // Number of bits used for PC index
    hpBits = 12;
    pattern_mask = (1 << hpBits) - 1;
  }
  
  //initialize global table (common to all)
  global_history = 1;
  global_table = calloc (1 << ghistoryBits, sizeof(uint8_t));
  global_mask = (1 << ghistoryBits) - 1; //set the last ghistoryBits to be 1s
  pc_mask = (1 << pcIndexBits) - 1;
  local_mask = (1 << lhistoryBits) - 1;
  
  int i;
  for (i = 0; i < (1 << ghistoryBits); ++i){
    global_table[i] = WN; //initialize all elements to Weakly Not Taken
  }

    
  switch (bpType) {
  case GSHARE:

    break;

  case TOURNAMENT:

    //initialize choice table
    choice_table = calloc (1 << ghistoryBits, sizeof(uint8_t)); 
    for (i = 0; i < (1 << ghistoryBits); ++i){
      choice_table[i] = WN; //initialize all elements to Weakly Local
    }

    //initialize local history table to 0000000000 (lhistoryBits * 0)
    local_history_table = calloc (1 << pcIndexBits, sizeof(uint32_t)); 

    //initialize local pattern table
    local_pattern_table = calloc (1 << lhistoryBits, sizeof(uint8_t)); 
    for (i = 0; i < (1 << lhistoryBits); ++i){
      local_pattern_table[i] = WN; //initialize all elements to Weakly Not Taken
    }
    
    break;
    
  case CUSTOM:

    //initialize choice table
    choice_table = calloc (1 << ghistoryBits, sizeof(uint8_t)); 
    for (i = 0; i < (1 << ghistoryBits); ++i){
      choice_table[i] = WN; //initialize all elements to Weakly Local
    }

    //initialize local history table to 0000000000 (lhistoryBits * 0)
    local_history_table = calloc (1 << pcIndexBits, sizeof(uint32_t)); 

    //initialize local pattern table
    local_pattern_table = calloc (1 << hpBits, sizeof(uint8_t)); 
    for (i = 0; i < (1 << hpBits); ++i){
      local_pattern_table[i] = WN; //initialize all elements to Weakly Not Taken
    }

    break;
    
  default:
    break;
  }
  

}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{

  // Make a prediction based on the bpType
  switch (bpType) {
  case STATIC:
    return TAKEN;
    
  case GSHARE:
    if ((global_table[(pc ^ global_history) & global_mask] ) == ST||
	(global_table[(pc ^ global_history) & global_mask] ) == WT){	
      return TAKEN;
    }
    else{
      return NOTTAKEN;
    }
      
  case TOURNAMENT:

    //ST and WT mean Global, SN and WN mean Local
    if (choice_table[(global_history) & global_mask] == ST ||
	choice_table[(global_history) & global_mask] == WT){

      if ((global_table[(global_history) & global_mask]) == ST||
	  (global_table[(global_history) & global_mask]) == WT){	
	return TAKEN;
      }
      else{
	return NOTTAKEN;
      }            
    }
    else{

      if (local_pattern_table[local_history_table[pc & pc_mask] & local_mask] == ST ||
	  local_pattern_table[local_history_table[pc & pc_mask] & local_mask] == WT){
	return TAKEN;
      }
      else{
	return NOTTAKEN;
      }
    }

  case CUSTOM:

    //ST and WT mean Global, SN and WN mean Local
    if (choice_table[hashGH(global_history)] == ST ||
	choice_table[hashGH(global_history)] == WT){

      if ((global_table[hashGH(global_history)]) == ST||
	  (global_table[hashGH(global_history)]) == WT){	
	return TAKEN;
      }
      else{
	return NOTTAKEN;
      }            
    }
    else{

      if (local_pattern_table[hashHP(local_history_table[hashPC(pc)])] == ST ||
	  local_pattern_table[hashHP(local_history_table[hashPC(pc)])] == WT){
	return TAKEN;
      }
      else{
	return NOTTAKEN;
      }
    }

  default:
    break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //booleans for tournament predictor
  int global_prediction;
  int local_prediction;
  bool global_correctness;
  bool local_correctness;
  uint32_t local_history_reg;
  
  switch (bpType) {
  case GSHARE:

    //if taken
    if (outcome == 1){
      if (global_table[(pc ^ global_history) & global_mask] != ST){
	global_table[(pc ^ global_history) & global_mask] ++; //Move 0->1->2->3
      }
    }

    //if not taken
    else{
      if (global_table[(pc ^ global_history) & global_mask] != SN){
	global_table[(pc ^ global_history) & global_mask] --; //Move 3->2->1->0
      }
    }

    global_history = (global_history << 1) + outcome;
    break;

  case TOURNAMENT:


    //CHOICE
    //Overloading the counter definition for choice
    //TAKEN means using global, NOTTAKEN means using local
    //Move towards global predictor
    
    //check the global correctness
    if ((global_table[global_history & global_mask]) == ST||
	(global_table[global_history & global_mask]) == WT){	
      global_prediction = TAKEN;
    }
    else{
      global_prediction = NOTTAKEN;
    }
    global_correctness = (global_prediction == outcome);
 
    //check the local correctness
    local_history_reg = local_history_table[pc & pc_mask];
    if ((local_pattern_table[local_history_reg & local_mask]) == ST||
	(local_pattern_table[local_history_reg & local_mask]) == WT){	
      local_prediction = TAKEN;
    }
    else{
      local_prediction = NOTTAKEN;
    }
    local_correctness = (local_prediction == outcome);

    //Move towards global predictor
    if (global_correctness == true && local_correctness == false){
      if (choice_table[global_history & global_mask] != ST){
	choice_table[global_history & global_mask] ++; //Move 0->1->2->3
      }
    }

    //Move towards local predictor
    else if (global_correctness == false && local_correctness == true){
      if (choice_table[global_history & global_mask] != SN){
	choice_table[global_history & global_mask] --; //Move 3->2->1->0
      }
    }

    
    //GLOBAL
    //if taken
    if (outcome == 1){
      if (global_table[global_history & global_mask] != ST){
	global_table[global_history & global_mask] ++; //Move 0->1->2->3
      }
    }
    //if not taken
    else{
      if (global_table[global_history & global_mask] != SN){
	global_table[global_history & global_mask] --; //Move 3->2->1->0
      }
    }


    //LOCAL
    //if taken
    if (outcome == 1){
      if (local_pattern_table[local_history_reg & local_mask] != ST){
	local_pattern_table[local_history_reg & local_mask] ++;
      }
    }
    //if not taken
    else{
      if (local_pattern_table[local_history_reg & local_mask] != SN){
	local_pattern_table[local_history_reg & local_mask] --;
      }
    }
    
    //UPDATE HISTORY
    global_history = (global_history << 1) + outcome;
    local_history_table[pc & pc_mask] = (local_history_table[pc & pc_mask] << 1) + outcome;
    
    break;
    
  case CUSTOM:

    //CHOICE
    //Overloading the counter definition for choice
    //TAKEN means using global, NOTTAKEN means using local
    //Move towards global predictor
    
    //check the global correctness
    if ((global_table[hashGH(global_history)]) == ST||
	(global_table[hashGH(global_history)]) == WT){	
      global_prediction = TAKEN;
    }
    else{
      global_prediction = NOTTAKEN;
    }
    global_correctness = (global_prediction == outcome);
 
    //check the local correctness
    local_history_reg = hashHP(local_history_table[hashPC(pc)]);
    if ((local_pattern_table[local_history_reg]) == ST||
	(local_pattern_table[local_history_reg]) == WT){	
      local_prediction = TAKEN;
    }
    else{
      local_prediction = NOTTAKEN;
    }
    local_correctness = (local_prediction == outcome);

    //Move towards global predictor
    if (global_correctness == true && local_correctness == false){
      if (choice_table[hashGH(global_history)] != ST){
	choice_table[hashGH(global_history)] ++; //Move 0->1->2->3
      }
    }

    //Move towards local predictor
    else if (global_correctness == false && local_correctness == true){
      if (choice_table[hashGH(global_history)] != SN){
	choice_table[hashGH(global_history)] --; //Move 3->2->1->0
      }
    }

    
    //GLOBAL
    //if taken
    if (outcome == 1){
      if (global_table[hashGH(global_history)] != ST){
	global_table[hashGH(global_history)] ++; //Move 0->1->2->3
      }
    }
    //if not taken
    else{
      if (global_table[hashGH(global_history)] != SN){
	global_table[hashGH(global_history)] --; //Move 3->2->1->0
      }
    }


    //LOCAL
    //if taken
    if (outcome == 1){
      if (local_pattern_table[local_history_reg] != ST){
	local_pattern_table[local_history_reg] ++;
      }
    }
    //if not taken
    else{
      if (local_pattern_table[local_history_reg] != SN){
	local_pattern_table[local_history_reg] --;
      }
    }
    
    //UPDATE HISTORY
    global_history = (global_history << 1) + outcome;
    local_history_table[hashPC(pc)] = (local_history_table[hashPC(pc)] << 1) + outcome;

    break;
    
  default:
    break;
  }
    

}

