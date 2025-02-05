/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007

 * Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>
 * Everyone is permitted to copy and distribute verbatim copies
 * of this license document, but changing it is not allowed.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * -----------------------------------------------------------------
 *
 * Author  : Sergio Barrachina-Muñoz and Francesc Wilhelmi
 * Created : 2016-12-05
 * Updated : $Date: 2017/03/20 10:32:36 $
 *           $Revision: 1.0 $
 *
 * -----------------------------------------------------------------
 * File description: this file contains functions related to the agents' operation
 *
 * - This file contains the methods used by the multi-armed bandits (MAB) framework
 */

#include "../../list_of_macros.h"
#include "action_selection_strategies/epsilon_greedy.h"
#include "action_selection_strategies/thompson_sampling.h"

#ifndef _AUX_MABS_
#define _AUX_MABS_

class MultiArmedBandit {

	// Public items
	public:

		int agent_id;
		int save_agent_logs;
		int print_agent_logs;
		int num_actions;
		int action_selection_strategy;

		// Generic variables to all the learning strategies
		int initial_reward;
		int num_iterations;

		// Arms statistics
		double *reward_per_arm;
		double *cumulative_reward_per_arm;
		double *average_reward_per_arm;
		double *estimated_reward_per_arm;
		int *times_arm_has_been_selected;

		// e-greedy specific variables
		double initial_epsilon;
		double epsilon;

	// Methods
	public:

		/******************/
		/******************/
		/*  MAIN METHODS  */
		/******************/
		/******************/

		/*
		 * UpdateRewardStatistics(): updates the statistics maintained for each arm
		 * INPUT:
		 * 	- action_ix: index of the action to be updated
		 * 	- reward: last reward observed from the action of interest
		 **/
		void UpdateArmStatistics(int action_ix, double reward){

			if(action_ix >= 0) { // Avoid indexing errors
				// Update the reward for the chosen arm
				reward_per_arm[action_ix] = reward;
				// Update the times the chosen arm has been selected
				times_arm_has_been_selected[action_ix] += 1;
				// Update the cumulative reward for the chosen arm
				cumulative_reward_per_arm[action_ix] += reward;
				// Update the average reward for the chosen arm
				average_reward_per_arm[action_ix] = cumulative_reward_per_arm[action_ix] /
					times_arm_has_been_selected[action_ix];
				// Update the estimated reward per arm
				estimated_reward_per_arm[action_ix] = ((estimated_reward_per_arm[action_ix]
					* times_arm_has_been_selected[action_ix])
					+ reward) / (times_arm_has_been_selected[action_ix] + 2);
			} else {
				printf("[MAB] ERROR: The action ix (%d) is not correct!\n", action_ix);
				exit(EXIT_FAILURE);
			}
		}

		/*
		 * SelectNewAction(): selects a new action according to the chosen action selection strategy
		 * OUTPUT:
		 *  - action_ix: index of the selected action
		 */
		int SelectNewAction() {
			int action_ix;
			// Select an action according to the chosen strategy: TODO improve this part (now it is hardcoded)
			//action_selection_strategy = STRATEGY_EGREEDY;
			switch(action_selection_strategy) {
				/*
				 * epsilon-greedy strategy:
				 */
				case STRATEGY_EGREEDY:{
					// Update epsilon
					epsilon = initial_epsilon / sqrt( (double) num_iterations);
					// Pick an action according to e-greedy
					action_ix = PickArmEgreedy(num_actions, average_reward_per_arm, epsilon);
					// Increase the number of iterations
					num_iterations ++;
					break;
				}
				/*
				 * Thompson sampling strategy:
				 */
				case STRATEGY_THOMPSON_SAMPLING:{
					// Pick an action according to Thompson sampling
					action_ix = PickArmThompsonSampling(num_actions,
						estimated_reward_per_arm, times_arm_has_been_selected);
					// Increase the number of iterations
					num_iterations ++;
					break;
				}
				default:{
					printf("[MAB] ERROR: '%d' is not a correct action-selection strategy!\n", action_selection_strategy);
					PrintAvailableActionSelectionStrategies();
					exit(EXIT_FAILURE);
				}
			}
			return action_ix;
		}

		/*************************/
		/*************************/
		/*  PRINT/WRITE METHODS  */
		/*************************/
		/*************************/

		/*
		 * PrintOrWriteStatistics: prints (or writes) the statistics of each arm
		 * INPUT:
		 * 	- write_or_print: variable to indicate whether to print on the  console or to write on the the output logs file
		 * 	- aggent_logger: logger object to write on the output file
		 * 	- sim_time: simulation time
		 */
		void PrintOrWriteStatistics(int write_or_print, Logger &agent_logger, double sim_time) {
			// Write or print according the input parameter "write_or_print"
			switch(write_or_print){
				// Print logs in console
				case PRINT_LOG:{
					if(print_agent_logs){
						printf("%s Reward per arm: ", LOG_LVL3);
						for(int n = 0; n < num_actions; n++){
							printf("%f  ", reward_per_arm[n]);
						}
						printf("\n%s Cumulative reward per arm: ", LOG_LVL3);
						for(int n = 0; n < num_actions; n++){
							printf("%f  ", cumulative_reward_per_arm[n]);
						}
						printf("\n%s Times each arm has been selected: ", LOG_LVL3);
						for(int n = 0; n < num_actions; n++){
							printf("%d  ", times_arm_has_been_selected[n]);
						}
						printf("\n");
					}
					break;
				}
				// Write logs in agent's output file
				case WRITE_LOG:{
					if(save_agent_logs) fprintf(agent_logger.file, "%.15f;A%d;%s;%s Reward per arm: ",
						sim_time, agent_id, LOG_C00, LOG_LVL3);
					for(int n = 0; n < num_actions; n++){
						 if(save_agent_logs){
							 fprintf(agent_logger.file, "%f  ", reward_per_arm[n]);
						 }
					}
					if(save_agent_logs) fprintf(agent_logger.file, "\n%.15f;A%d;%s;%s Cumulative reward per arm: ",
						sim_time, agent_id, LOG_C00, LOG_LVL3);
					for(int n = 0; n < num_actions; n++){
						 if(save_agent_logs){
							 fprintf(agent_logger.file, "%f  ", cumulative_reward_per_arm[n]);
						 }
					}
					fprintf(agent_logger.file, "\n%.15f;A%d;%s;%s Times each arm has been selected: ",
									sim_time, agent_id, LOG_C00, LOG_LVL3);
					for(int n = 0; n < num_actions; n++){
						if(save_agent_logs){
							fprintf(agent_logger.file, "%d ", times_arm_has_been_selected[n]);
						}
					}
					if(save_agent_logs) fprintf(agent_logger.file, "\n");
					break;
				}
			}
		}

		/*
		 * PrintAvailableLearningMechanisms(): prints the available ML mechanisms types
		 */
		void PrintAvailableActionSelectionStrategies(){
			printf("%s Available types of action-selection strategies:\n", LOG_LVL2);
			printf("%s STRATEGY_EGREEDY (%d)\n", LOG_LVL3, STRATEGY_EGREEDY);
			printf("%s STRATEGY_THOMPSON_SAMPLING (%d)\n", LOG_LVL3, STRATEGY_THOMPSON_SAMPLING);
		}


		/***********************/
		/***********************/
		/*  AUXILIARY METHODS  */
		/***********************/
		/***********************/

		/*
		 * InitializeVariables(): initializes the variables used by the MAB
		 */
		void InitializeVariables(){
			// TODO: generate file that stores algorithm-specific variables
			initial_epsilon = 1;
			epsilon = initial_epsilon;
			initial_reward = 0;
			num_iterations = 1;
			// Initialize the rewards assigned to each arm
			reward_per_arm = new double[num_actions];
			cumulative_reward_per_arm = new double[num_actions];
			average_reward_per_arm = new double[num_actions];
			estimated_reward_per_arm = new double[num_actions];
			// Initialize the array containing the times each arm has been played
			times_arm_has_been_selected = new int[num_actions];
			for(int i = 0; i < num_actions; i++){
				reward_per_arm[i] = initial_reward;	// Set the initial reward
				cumulative_reward_per_arm[i] = initial_reward;
				average_reward_per_arm[i] = initial_reward;
				estimated_reward_per_arm[i] = initial_reward;
				times_arm_has_been_selected[i] = 0;
			}
		}

};

#endif
