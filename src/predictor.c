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
uint8_t* gshare_pht;
uint32_t gshare_mask; // extract the last ghistorybits
uint32_t extract2_mask = 0x00000003; //extract the last 2 bits


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  switch (bpType) {
  case GSHARE:

    //Gshare
    gshare_pht = calloc (1 << ghistoryBits, sizeof(uint8_t)); 
    gshare_mask = (1 << ghistoryBits) - 1; //set the last ghistoryBits to be 1s

    int i;
    for (i = 0; i < (1 << ghistoryBits); ++i){
      gshare_pht[i] = WN; //initialize all elements to Weakly Not Taken
    }
    break;
  case TOURNAMENT:

    
  case CUSTOM:

    
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
    if ((gshare_pht[(pc ^ global_history) & gshare_mask] & extract2_mask) == ST||
	(gshare_pht[(pc ^ global_history) & gshare_mask] & extract2_mask) == WT){	
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
      if (gshare_pht[(pc ^ global_history) & gshare_mask] != ST){
	gshare_pht[(pc ^ global_history) & gshare_mask] ++; //Move 0->1->2->3
      }
    }

    //if not taken
    else{
      if (gshare_pht[(pc ^ global_history) & gshare_mask] != SN){
	gshare_pht[(pc ^ global_history) & gshare_mask] --; //Move 3->2->1->0
      }
    }

    global_history = global_history << 1 + outcome;
    break;

  case TOURNAMENT:

    
  case CUSTOM:

    
  default:
    break;
  }
    

}

