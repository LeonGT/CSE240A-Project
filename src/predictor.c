//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

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

uint32_t global_history = 0;
uint8_t* global_table;
uint32_t global_mask; // extract the last ghistorybits
uint32_t counter_mask = 0x00000003; //extract the last 2 bits



//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{

  global_mask = (1 << ghistoryBits) - 1; //set the last ghistoryBits to be 1s
  global_table = calloc (1 << ghistoryBits, sizeof(uint8_t)); 
  int i;
  for (i = 0; i < (1 << ghistoryBits); ++i){
    global_table[i] = WN; //initialize all elements to Weakly Not Taken
  }

    
  switch (bpType) {
  case GSHARE:

    break;

  case TOURNAMENT:

    break;
    
  case CUSTOM:

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
    if ((global_table[(pc ^ global_history) & global_mask] & counter_mask) == ST||
	(global_table[(pc ^ global_history) & global_mask] & counter_mask) == WT){	
      return TAKEN;
    }
    else{
      return NOTTAKEN;
    }
      
  case TOURNAMENT:
  case CUSTOM:
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

    global_history = global_history << 1 + outcome;
    break;

  case TOURNAMENT:

    //GLOBAL
    //if taken
    if (outcome == 1){
      if (global_table[(global_history) & global_mask] != ST){
	global_table[(global_history) & global_mask] ++; //Move 0->1->2->3
      }
    }

    //if not taken
    else{
      if (global_table[(global_history) & global_mask] != SN){
	global_table[(global_history) & global_mask] --; //Move 3->2->1->0
      }
    }
    

    //LOCAL


    //selector

    
    //final update
    global_history = global_history << 1 + outcome;
    
  case CUSTOM:

    
  default:
    break;
  }
    

}

