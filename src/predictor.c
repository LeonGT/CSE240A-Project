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


int lhistorySize;
int ghistorySize;
uint64_t* global_table_2;
uint32_t global_mask_2;
uint8_t predictor_mask;;
int ghistoryBits2;


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


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{

  if(bpType == CUSTOM){
    ghistoryBits = 15; //big impact
    lhistoryBits = 10; //small impact
    pcIndexBits = 10; //optimized
    lhistorySize = 512; //optimized
    ghistorySize = 512;
    ghistoryBits2 = 20;
    global_mask_2 = (1 << ghistoryBits2) - 1;
    predictor_mask = (1 << 2) - 1;
  }
  
  //initialize global table (common to all)
  global_history = 0;
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


    //initialize global table (common to all)
    global_table_2 = calloc (1 << ghistorySize, sizeof(uint64_t));
    for (i = 0; i < (1 << ghistorySize); ++i){
      global_table_2[i] = WN; //initialize all elements to Weakly Local
    }
    
    //initialize choice table
    choice_table = calloc (1 << ghistoryBits, sizeof(uint8_t)); 
    for (i = 0; i < (1 << ghistoryBits); ++i){
      choice_table[i] = WN; //initialize all elements to Weakly Local
    }

    //initialize local history table to 000000000000 (lhistoryBits * 0)
    local_history_table = calloc (lhistorySize, sizeof(uint32_t)); 

    //initialize local pattern table
    local_pattern_table = calloc (1 << lhistoryBits, sizeof(uint8_t)); 
    for (i = 0; i < (1 << lhistoryBits); ++i){
      local_pattern_table[i] = WT; //initialize all elements to Weakly Not Taken
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

  int i;
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

    //hit in local
    if (choice_table[(global_history) & global_mask] == SN ||
	choice_table[(global_history) & global_mask] == WN){

      for (i = 0; i < lhistorySize; ++i){
	if (((local_history_table[i] >> lhistoryBits) & pc_mask) == (pc & pc_mask)){
	  
	  if (local_pattern_table[local_history_table[i] & local_mask] == ST ||
	      local_pattern_table[local_history_table[i] & local_mask] == WT){
	    return TAKEN;
	  }
	  else{
	    return NOTTAKEN;
	  }
	}
      }
    }

    //hit in global2
    for (i = 0; i < ghistorySize; ++i){
      if (((global_table_2[i] >> 2) & global_mask_2) == (global_history & global_mask_2)){
	
	if ((global_table_2[i] & predictor_mask) == ST || (global_table_2[i] & predictor_mask) == WT){
	  return TAKEN;
	}
	else{
	  return NOTTAKEN;
	}
      }
    }

    //no hit using traditional global
    if ((global_table[(global_history) & global_mask]) == ST||
	(global_table[(global_history) & global_mask]) == WT){	
      return TAKEN;
    }
    else{
      return NOTTAKEN;
    }          
    
  default:
    return TAKEN;
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
  int i;
  int indexSeenGlobal;
  int indexSeen;
  int moveTemp;
  
  switch (bpType) {
  case GSHARE:

    //if taken
    if (outcome == 1){
      if (global_table[(pc ^ global_history) & global_mask] != ST){
	global_table[(pc ^ global_history) & global_mask] += 1; //Move 0->1->2->3
      }
    }

    //if not taken
    else{
      if (global_table[(pc ^ global_history) & global_mask] != SN){
	global_table[(pc ^ global_history) & global_mask] -= 1; //Move 3->2->1->0
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
	local_pattern_table[local_history_reg & local_mask] += 1;
      }
    }
    //if not taken
    else{
      if (local_pattern_table[local_history_reg & local_mask] != SN){
	local_pattern_table[local_history_reg & local_mask] -= 1;
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
    //printf("Global History is %d\n", global_history & global_mask_2);
    //printf("Curre P is %d\n", (global_history & global_mask_2));
    indexSeenGlobal = -1;
    for (i = 0; i < ghistorySize; ++i){

      //printf("Table P is %ld in state %ld \n", (global_table_2[i] >> 2) & global_mask_2, global_table_2[i] & predictor_mask);
      //
      
      if (((global_table_2[i] >> 2) & global_mask_2) == (global_history & global_mask_2)){
	indexSeenGlobal = i;
	break;	
      }
    }

    if (indexSeenGlobal != -1){
      if (((global_table_2[indexSeenGlobal] & predictor_mask) == ST) || ((global_table_2[indexSeenGlobal] & predictor_mask) == WT)){
	global_prediction = TAKEN;
      }
      else{
	global_prediction = NOTTAKEN;
      }
    }
    else{
        
      if ((global_table[global_history & global_mask]) == ST||
	  (global_table[global_history & global_mask]) == WT){	

	global_prediction = TAKEN;
      }
      else{
	global_prediction = NOTTAKEN;
      }
    }
    global_correctness = (global_prediction == outcome);
 
    //check the local correctness

    indexSeen = -1;
    for (i = 0; i < lhistorySize; ++i){
      //printf("LHT at %d is %d\n", i, local_history_table[i]);
      if (((local_history_table[i] >> lhistoryBits) & pc_mask) == (pc & pc_mask)){
	local_history_reg = local_history_table[0] & local_mask;
	indexSeen = i;
	break;
      }
    }

    if (indexSeen != -1){
      if ((local_pattern_table[local_history_reg]) == ST||
	  (local_pattern_table[local_history_reg]) == WT){	
	local_prediction = TAKEN;
      }
      else{
	local_prediction = NOTTAKEN;
      }
      local_correctness = (local_prediction == outcome);
    }
    else{
      local_correctness = false;
    }
    
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

    if (indexSeenGlobal != -1){
      //printf("Hit\n");
      moveTemp = global_table_2[indexSeenGlobal];

      for (i = indexSeen - 1; i >= 0; --i){
	global_table_2[i+1] = global_table_2[i];
      }
      global_table_2[0] = moveTemp;
    }
    else{
      //printf("Miss\n");
      for (i = ghistorySize - 1; i >= 0; --i){
	global_table_2[i+1] = global_table_2[i];
      }
      global_table_2[0] = (global_history << 2) | WN;
    }

	
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
    //find the pc
    indexSeen = -1;
    for (i = 0; i < lhistorySize; ++i){
      //printf("LHT at %d is %d\n", i, local_history_table[i]);
      if (((local_history_table[i] >> lhistoryBits) & pc_mask) == (pc & pc_mask)){
	indexSeen = i;
	break;
      }
    }
    
    //move the pc to top
    if (indexSeen != -1){
      //printf("Hit\n");
      moveTemp = local_history_table[indexSeen];

      for (i = indexSeen - 1; i >= 0; --i){
	local_history_table[i+1] = local_history_table[i];
      }
      local_history_table[0] = moveTemp;
    }
    else{
      //printf("Miss\n");
      for (i = lhistorySize- 1; i >= 0; --i){
	local_history_table[i+1] = local_history_table[i];
      }
      local_history_table[0] = pc << lhistoryBits;
    }
    
    local_history_reg = local_history_table[0] & local_mask;
    
    //LOCAL
    //if taken
    if (outcome == 1){
      if (local_pattern_table[local_history_reg] != ST){
	local_pattern_table[local_history_reg] += 1;
      }
    }
    //if not taken
    else{
      if (local_pattern_table[local_history_reg] != SN){
	local_pattern_table[local_history_reg] -= 1;
      }
    }
    
    //UPDATE HISTORY
    global_history = (global_history << 1) + outcome;
    local_history_table[0] = (local_history_table[0] & (pc_mask << lhistoryBits)) | (((local_history_table[0] << 1 ) + outcome) & local_mask);


    //printf("Outcome is %d\n", outcome);

    //printf("Entry 0 BEFORE %ld in state %ld \n", (global_table_2[0] >> 2) & global_mask_2, global_table_2[0] & predictor_mask);
    if (outcome == 1){
      if ((global_table_2[0] & predictor_mask) != ST){
	global_table_2[0] = (global_table_2[0] & (global_mask_2 << 2)) | ((global_table_2[0] & predictor_mask) + 1);
      }
    }
    
    //if not taken
    else{
      if ((global_table_2[0] & predictor_mask) != SN){
	global_table_2[0] = (global_table_2[0] & (global_mask_2 << 2)) | ((global_table_2[0] & predictor_mask) - 1);
      }
    }
    //printf("Entry 0 AFTER %ld in state %ld \n", (global_table_2[0] >> 2) & global_mask_2, global_table_2[0] & predictor_mask);
    
    
    break;
    
  default:
    break;
  }
    

}

