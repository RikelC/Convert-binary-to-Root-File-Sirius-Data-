/*!
 * \file ConvertToRootFile.cc
 * \author Rikel Chakma
 * \date 18/12/2022
 * \brief To convert the binary file in to root file
 *
 */

#include "ReadDataFile.hh"
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <algorithm>

void to_upper(std::string & str);
std::vector<std::string> get_existing_files(std::string dir, std::vector<int> list);

std::vector<std::string> get_existing_files(std::string dir, std::vector<std::string> list); 

std::vector<std::string> get_to_be_processed_files(std::vector<string> list, std::vector<int> subRuns);

std::vector<int> extract_run_numbers_from_input(std::string run_numbers);

std::string *get_current_subrunRange(int l, std::vector<std::string> list);

std::string extract_run_number_from_filename(std::string filename);

std::string extract_subrun_number_from_filename( std::string filename);

std::string get_tree_filename(int l, std::string *t_sub_str, std::string output_dir, std::string runN, std::string subRunN, std::string sum_runs, std::string sum_subRuns,std::vector<std::string> runfiles, bool new_run_flag, int run_increament);

std::string * extract_oFileFormats_substrings(std::string fileFormat);

bool is_a_new_run(int l, std::vector<std::string> list);

void print_help();

struct UserOption{
	UserOption(char * opt, bool f ){
		Option = opt;
		Flag = f;
	}
	char *Option;
	bool Flag;
};

//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
/*!
 * The main function
 */
int main(int argc, char *argv[])
{
	std::ifstream configFile; 
	std::string line,test_line, variable, value; 
	std::string run_numbers;
	std::string subrun_numbers;
	std::string sum_runs;
	std::string sum_subRuns;
	std::string input_dir;
	std::string output_dir;
	std::string treeFileFormat;
	std::string treeName;
	std::string number_of_events_to_process;
	bool equality_sign_found = 0;
	std::vector<std::string> filelist; 

	configFile.open("RunToProcess.txt", std::ifstream::in);
	if(configFile.is_open()){
		while(configFile.good() && !configFile.eof()){
			getline(configFile, line);
			//skip empty line with white spaces			
			std::istringstream str_stream(line);
			str_stream >> test_line;
			if(!str_stream) continue;
			str_stream.unget();
			equality_sign_found = 0;
			variable.clear(); value.clear();
			for(size_t i = 0; i< line.length();i++){
				if(line[i] == '#') break;
				if(line[i] == ' ' || line[i] == '\n') continue;
				else{
					if(line[i] != '='){
						if(!equality_sign_found) variable.append(1, line[i]);
						else value.append(1, line[i]);
					}else{
						equality_sign_found = 1;
					}
				}
			}

			to_upper(variable);

			size_t pos_comment = value.find("//");
			if(pos_comment != std::string::npos) value.erase(pos_comment, value.length());

			if(!variable.empty() && !value.empty()){
				if(variable.compare("RUNNUMBER")==0 || variable.compare("RUNNUMBERS")==0 ){ to_upper(value); run_numbers = value;}
				else if(variable.compare("SUBRUNNUMBER")==0 || variable.compare("SUBRUNNUMBERS")==0 ) subrun_numbers = value;
				else if(variable.compare("SUMRUNS")==0){to_upper(value); sum_runs = value;}
				else if(variable.compare("SUMSUBRUNS")==0){to_upper(value); sum_subRuns = value;}
				else if(variable.compare("INPUTFILEPATH")==0) input_dir = value;
				else if(variable.compare("OUTPUTFILEPATH")==0) output_dir = value;
				else if(variable.compare("TTREEFILENAMEFORMAT")==0) treeFileFormat = value;
				else if(variable.compare("TTREENAME")==0) treeName = value;
				else if(variable.compare("PROCESSNUMBEROFEVENTS")==0){to_upper(value); number_of_events_to_process = value;}
				else if(variable.compare("FILE")==0){filelist.push_back(value);}
			}
		}
		configFile.close();
		// overwrite the commands from Run
		if (argc > 1){
			const int Ncommands = 10;
			bool flag[ Ncommands] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			std::string command[Ncommands] = {"-r", "-s", "-sr", "-ss", "-i", "-o", "-f", "-t", "-n", "-h"};
			std::string command2[Ncommands] = {"--runnumbers", "--subrunnumbers", "--sumruns", "--sumsubruns", "--inputdirectory", "--outputdirectory", "--treefileformat", "--treename", "--numberofeventstoprocess", "--help"};

			//Check if the commands are not correct
			bool found = false;
			std::vector<UserOption> options;
			for(int i = 0; i < argc; i++){

				if(argv[i][0] == '-'){
					found = false;	

					for(int cc =0; cc< Ncommands; cc++){// cc : current command
						if((strcmp(argv[i], command[cc].c_str()) == 0) ||
								(strcmp(argv[i], command2[cc].c_str()) == 0)){
							found = true;
							break;
						}
					}

					if(!found){
						UserOption o(argv[i], found);
						options.push_back(o);
					}
				}

			}

			if(options.size() > 0){
				for(unsigned int i = 0; i< options.size();i++){
					if(options.at(i).Flag == 0){
						cout<<"OPTION : "<<options.at(i).Option <<" is not correct. "<<endl;
					}

				}
				cout<<"\n         For Help Type:"<<endl;
				cout<<"                  'ConvertToRootFile -h'"<<endl;
				cout<<"                            or          "<<endl;
				cout<<"                  'ConvertToRootFile --help'\n"<<endl;
                                options.clear();
				filelist.clear();
				exit(0);
			}


			for(int i = 0; i < argc; i++){
				for(int cc =0; cc< Ncommands; cc++){// cc : current command
					// Check the current command
					if((strcmp(argv[i], command[cc].c_str()) == 0) ||
							(strcmp(argv[i], command2[cc].c_str()) == 0)){
						flag[cc] = true;
						if(cc == 0) run_numbers.clear();
						else if(cc == 1) subrun_numbers.clear();
						else if(cc == 2) sum_runs.clear();
						else if(cc == 3) sum_subRuns.clear();
						else if(cc == 4) input_dir.clear();
						else if(cc == 5) output_dir.clear();
						else if(cc == 6) treeFileFormat.clear();
						else if(cc == 7) treeName.clear();
						else if(cc == 8) number_of_events_to_process.clear();

						else if(cc == 9){
							print_help();
							filelist.clear();
							exit(0);
						}						
						continue;// to skip this entry
					}
					if(flag[cc])
					{
						// Check the current command is swiched off
						for(int oc = 0; oc< Ncommands; oc++){
							if(oc == cc) continue;
							if(strcmp(argv[i], command[oc].c_str()) == 0)
								flag[cc] = false;
						}

						if(flag[cc]){
							if(cc == 0){ if(run_numbers.empty()) run_numbers += std::string(argv[i]); else run_numbers += "," + std::string(argv[i]);}
							else if(cc == 1){ if(subrun_numbers.empty())subrun_numbers += std::string(argv[i]); else subrun_numbers += "," + std::string(argv[i]);}
							else if(cc == 2) sum_runs += std::string(argv[i]);
							else if(cc == 3) sum_subRuns += std::string(argv[i]);
							else if(cc == 4) input_dir += std::string(argv[i]);
							else if(cc == 5) output_dir += std::string(argv[i]);
							else if(cc == 6) treeFileFormat += std::string(argv[i]);
							else if(cc == 7) treeName += std::string(argv[i]);
							else if(cc == 8) number_of_events_to_process += std::string(argv[i]);

						}


					}

				}
			}

			to_upper(sum_runs);
			to_upper(sum_subRuns);
		}
		/*
		   std::cout<<"*****************************************************************"<<std::endl;
		   std::cout<<"Run numbers = "<<run_numbers<<std::endl;
		   std::cout<<"Sub Run numbers = "<<subrun_numbers<<std::endl;
		   std::cout<<"Input file path = "<<input_dir<<std::endl;
		   std::cout<<"Output file path = "<<output_dir<<std::endl;
		   std::cout<<"TTree file format = "<<treeFileFormat<<std::endl;
		   std::cout<<"TTree name = "<<treeName<<std::endl;
		   std::cout<<"Sum Runs = "<<sum_runs<<std::endl;
		   std::cout<<"Sum subRuns = "<<sum_subRuns<<std::endl;
		   std::cout<<"Number of events to process = "<<number_of_events_to_process<<std::endl;
		   std::cout<<"*****************************************************************"<<std::endl;
		   */

		// Check if input parameters are empty or wrong
		if(run_numbers.empty() || subrun_numbers.empty() || sum_runs.empty() || sum_subRuns.empty() || input_dir.empty() || output_dir.empty() || treeFileFormat.empty() || treeName.empty() || number_of_events_to_process.empty()){
			cout<< "\n********************* Error in the input !! ********************\n\n";
			if(run_numbers.empty())cout<<"Check the Run numbers: is empty"<<endl;
			else if(subrun_numbers.empty())cout<<"Check the subRun numbers: is empty"<<endl;
			else if(sum_runs.empty() )cout<<"Check the sum Runs: is empty"<<endl;
			else if(sum_subRuns.empty())cout<<"Check the sum subRuns: is empty"<<endl;
			else if(input_dir.empty())cout<<"Check the input directory: is empty"<<endl;
			else if(output_dir.empty())cout<<"Check the output directory: is empty"<<endl;
			else if(treeFileFormat.empty())cout<<"Check the tree file format: is empty"<<endl;
			else if(treeName.empty())cout<<"Check the tree name: is empty"<<endl;
			else if(number_of_events_to_process.empty())cout<<"Check the number of events to process: is empty"<<endl;
			std::cout<<"\n****************************************************************"<<std::endl;
			filelist.clear();
			exit(0);
		}


		//remove trailling '/'
		if(input_dir[input_dir.length()-1]=='/'){

			input_dir.resize(input_dir.length()-1);
		}
		if(output_dir[output_dir.length()-1]=='/'){

			output_dir.resize(output_dir.length()-1);
		}

		vector<std::string> runfiles;
		if(run_numbers == "FILE")
		{
			runfiles = filelist;
		}
		else{
			// Extract run numbers
			std::vector<int> Runs= extract_run_numbers_from_input(run_numbers);
			//Check for double value if the user had made a mistake in defining the run numbers
			std::vector<int>common_runs;
			for(unsigned int n =0; n<Runs.size();n++){	
				unsigned int counter =0;
				for(unsigned int m =n; m<Runs.size();m++){
					if(n != m){
						if(Runs[n] == Runs[m]){counter++; common_runs.push_back(m);std::cout<<"m "<<m<<std::endl;}
					}
				}
				n += counter;
			}	
			//remove the common runs
			if(common_runs.size() >0){
				unsigned int idx =0;
				for(unsigned int k =0; k<common_runs.size();k++){ Runs.erase(Runs.begin()+common_runs[k]-idx);idx++;}
				common_runs.clear();
			}
			//Extract sub run numbers
			std::vector<int> subRuns = extract_run_numbers_from_input(subrun_numbers);
			//Look for files if they exist in the input directory
			vector<std::string> runfiles_temp =  get_existing_files(input_dir, Runs);
			runfiles =  get_to_be_processed_files(runfiles_temp, subRuns);
			runfiles_temp.clear();
			Runs.clear();
			subRuns.clear();
		}

		// variables for output file names
		std::string treeFileName;
		std::string prev_treeFileName;
		std::string *t_sub_str = extract_oFileFormats_substrings(treeFileFormat);
		ReadDataFile *a;
		int current_runN =0, prev_runN =0, current_subrunN =0, prev_subrunN =0, run_increament =0;
		//---------------
		// do file loop
		//-------------
		for(unsigned int l =0; l < runfiles.size(); l++){
			if(runfiles[l].empty())continue;
			std::string current_file = input_dir + "/" + runfiles[l];
			if(sum_runs == "YES" && runfiles.size() > 1){
				if(l ==0){
					treeFileName = get_tree_filename(l, t_sub_str, output_dir, run_numbers, "all", sum_runs, sum_subRuns, runfiles, true, run_increament);

					a = new ReadDataFile();
					a->initialize_ttree(treeFileName.c_str(), treeName.c_str());

				}
				a->Read(current_file.c_str(), number_of_events_to_process.c_str());
			}// runs summed
			else{//runs not summed
				std::string runN = extract_run_number_from_filename(runfiles[l]);
				std::string subRunN = extract_subrun_number_from_filename(runfiles[l]);
				current_runN = atoi(runN.c_str());
				current_subrunN = atoi(subRunN.c_str());
				if(l ==0){
					treeFileName = get_tree_filename(l, t_sub_str, output_dir, runN, subRunN, sum_runs, sum_subRuns, runfiles, true, run_increament);
				}
				else{
					if(current_runN == prev_runN){// have the same run number
						//first check if they are different runs with same run number
						if(current_subrunN == prev_subrunN){
							run_increament++;

							treeFileName = get_tree_filename(l, t_sub_str, output_dir, runN, subRunN, sum_runs, sum_subRuns, runfiles, true, run_increament);
						}
						else{
							if(is_a_new_run(l, runfiles)){
								run_increament++;
								treeFileName = get_tree_filename(l, t_sub_str, output_dir, runN, subRunN, sum_runs, sum_subRuns, runfiles, true, run_increament);
							}else{
								if(sum_subRuns == "NO"){
									treeFileName = get_tree_filename(l, t_sub_str, output_dir, runN, subRunN, sum_runs, sum_subRuns, runfiles, false, run_increament);
								}

							}
						}
					}
					else{//new run
						run_increament =0;
						treeFileName = get_tree_filename(l, t_sub_str, output_dir, runN, subRunN, sum_runs, sum_subRuns, runfiles, true, run_increament);
					}
				}
				//-----------------
				//  Gru part
				//  ---------------
				if(l ==0){
					a = new ReadDataFile();
					a->initialize_ttree(treeFileName.c_str(), treeName.c_str());
				}
				else{
					if(current_runN == prev_runN){// have the same run number
						//first check if they are different runs with same run number
						if(current_subrunN = prev_subrunN){
							//--------------
							// save old file
							//--------------				
							cout<<"TTree file name: "<<prev_treeFileName<<endl;
							a->save_ttree();
							delete a;  
							a = NULL;
							//-----------------
							// open a new file
							//-----------------
							a = new ReadDataFile();
							a->initialize_ttree(treeFileName.c_str(), treeName.c_str());
						}
						else{//current file with diferent sub run no
							if(is_a_new_run(l, runfiles)){
								//--------------
								// save old file
								//--------------				
								cout<<"TTree file name: "<<prev_treeFileName<<endl;
								a->save_ttree();
								delete a;  
								a = NULL;
								//-----------------
								// open a new file
								//-----------------
								a = new ReadDataFile();
								a->initialize_ttree(treeFileName.c_str(), treeName.c_str());
							}else{
								if(sum_subRuns == "NO"){
									//--------------
									//save old file
									//---------------
									cout<<"TTree file name: "<<prev_treeFileName<<endl;
									a->save_ttree();
									delete a;  
									a = NULL;
									//------------------
									// open a new fiel
									//-------------------
									a = new ReadDataFile();
									a->initialize_ttree(treeFileName.c_str(), treeName.c_str());
								}
							}
						}
					}
					else{//new run
						//--------------
						// save old file
						//--------------				
						if(sum_subRuns == "YES"){
							cout<<"TTree file name: "<<prev_treeFileName<<endl;
							a->save_ttree();
							delete a;  
							a = NULL;
						}
						a = new ReadDataFile();
						a->initialize_ttree(treeFileName.c_str(), treeName.c_str());
					}
				}
				a->Read(current_file.c_str(), number_of_events_to_process.c_str());

				prev_runN = current_runN;
				prev_subrunN = prev_subrunN;
				prev_treeFileName = treeFileName;
			}
		}//file loop
		//--------------
		// save last file
		//--------------				
		cout<<"TTree file name: "<<prev_treeFileName<<endl;
		a->save_ttree();
		delete a;   
		a = NULL;
		runfiles.clear();
		delete[] t_sub_str;


	}else{
		std::cout<<"Error Run.config file not found !!!!!"<<std::endl;
		exit(0);
	}
	filelist.clear();
	return 0;
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
//----------------------------------------
// function to convert to Upper case 
//----------------------------------------
/*! Converts a string of lower case characters string into upper case.
*/
void to_upper(std::string & str){
	std::string str1 = str;
	str.clear();

	for(std::string::size_type i = 0; i < str1.length(); i++){
		if(std::isalpha(str1[i])) str.append(1, std::toupper(str1[i]));
		else str.append(1, str1[i]);
	}
	str1.clear();
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
//------------------------------------------------------------------------------------
// function to get the run and sub run numbers from the specified format in Run.config 
//------------------------------------------------------------------------------------
//! This to extract the run numbers and sub run numbers.
/*! @param run_numbers can be specified in different formats(run number = '119, 120,...', '1-20', '1-20, 21, 40-45', 'all') in the Run.config file. Care must be taken when specifying 'all' as not all the run numbers have the same data format.
*/
std::vector<int> extract_run_numbers_from_input(std::string run_numbers){
	std::vector<int> Runs;
	to_upper(run_numbers);	
	if(run_numbers.compare("ALL") ==0){
		for(int i =0; i < 1000; i++)Runs.push_back(i);
	}
	else{
		while(run_numbers.length() > 0){
			std::size_t pos_coma = run_numbers.find(",");
			if(pos_coma!= std::string::npos){
				std::string val = run_numbers.substr(0,pos_coma); 
				std::size_t pos_dash = val.find("-");
				if(pos_dash !=std::string::npos){
					std::string val1 = val.substr(0,pos_dash);
					std::string val2 = val.substr(pos_dash+1, val.length());
					for(int i = atoi(val1.c_str()) ; i<= atoi(val2.c_str()); i++) Runs.push_back(i);
				}else{
					//single value
					int r = atoi(val.c_str());
					Runs.push_back(r);
				}

				run_numbers.erase(0,pos_coma+1);
			}else{
				//without comma last element
				std::size_t pos_dash = run_numbers.find("-");

				if(pos_dash !=std::string::npos){
					std::string val1 = run_numbers.substr(0,pos_dash);
					std::string val2 = run_numbers.substr(pos_dash+1, run_numbers.length());
					for(int i = atoi(val1.c_str()) ; i<= atoi(val2.c_str()); i++) Runs.push_back(i);
				}else{
					//lonely value
					int r = atoi(run_numbers.c_str());
					Runs.push_back(r);
				}

				run_numbers.erase(0,run_numbers.length());
			}
		}
	}
	return Runs;
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
//----------------------------------------
// function to get the files to processed 
//----------------------------------------
//! To get the name of the files to be processed. 
/*! get_existing_files() functions list all the files of the specified run numbers. This function then further filter the files specified using the subRuns parameter.
 * @param list = get_existing_files()
 * @param subRuns list of sub run numbers specified in the Run.config file.
 */ 
std::vector<std::string> get_to_be_processed_files(std::vector<std::string> list, std::vector<int> subRuns){
	vector<std::string> runfiles;
	bool subRunFound =0;
	//		for(int i =0; i < subRuns.size(); i++) cout<<"subruns to process: "<<subRuns[i]<<endl;
	for(unsigned int l =0; l < list.size(); l++){
		if(list[l].empty())continue;
		subRunFound = 0;
		std::string subRunN = list[l];
		size_t pos = subRunN.find_last_of("s");
		if(pos!=std::string::npos)subRunN.erase(0, pos+2);
		if(subRunN.empty()) subRunN = "0";
		for(unsigned int i = 0; i < subRuns.size(); i++){
			if( atoi(subRunN.c_str()) == subRuns[i]){
				subRunFound = true; break;
			}
		}
		if(subRunFound)	runfiles.push_back(list[l]);
	}
	if(runfiles.size() ==0)cout<<"Please specify run numbers and sub run numbers properly in Run.config"<<endl;
	//	for(int i =0; i < runfiles.size(); i++) cout<<"run to process: "<<runfiles[i]<<endl;
	return runfiles;

}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
//-----------------------------------------------
// function o extract run numbers from filenames 
//-----------------------------------------------
/*! To extract the run number from the file name.
*/
std::string extract_run_number_from_filename(std::string filename){
	std::string runN =  filename;
	std::size_t pos = runN.find_first_of("_");
	if(pos!=std::string::npos)runN.erase(0, pos+1);
	pos = runN.length();
	if(pos!=std::string::npos)runN.erase(4, pos);
	return runN;
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
//-----------------------------------------------
// function to extract subrun numbers from filenames 
//-----------------------------------------------
/*! To extract the sub run number from the file name.
*/
std::string extract_subrun_number_from_filename( std::string filename){
	std::string subRunN = filename;
	std::size_t pos = subRunN.find_last_of("s");
	if(pos!=std::string::npos)subRunN.erase(0, pos+2);
	if(subRunN.empty()) subRunN = "0";
	return subRunN;
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
//-----------------------------------------------
// function to get the range of sub run numbers 
//-----------------------------------------------
//! Get the true range of the sub run numbers.
/*! One could specify run number = 100-150 and sub run number = 0-100 in the Run.config, but, it may be possible that not all the runs have that many sub run files. This function extracts the true range of the sub run rumbers for each run. This is particularly useful in naming the TTree and histogram files.
*/
std::string* get_current_subrunRange(int l, std::vector<std::string> list){
	std::string *srange = new std::string[2];
	srange[0] = extract_subrun_number_from_filename(list[l]);
	srange[1] = srange[0]; // if for loop is not executed
	std::size_t r_length = list[l].length();
	int min = atoi(srange[0].c_str());
	int max = min;
	int c1 =0;
	std::string temp;
	for(unsigned int i =0; i < list.size(); i++){
		if(i == l) continue;
		std::size_t found = list[i].find(list[l]);
		if(found!= std::string::npos){
			std::size_t l_length = list[i].length();
			if(r_length == l_length){
				srange[1]=srange[0];
			}else{
				temp = extract_subrun_number_from_filename(list[i]);
				c1 = atoi(temp.c_str());
				//max val				
				if(c1 > max){
					max = c1;
					srange[1] = temp;
				}
				//min val
				if(c1 < min){
					min = c1;
					srange[0] = temp;
				}
			}
		}
	}
	return srange;

}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
//------------------------------------------------
//extract substrings from the given file format
//-------------------------------------------------
//! To extract the format of the output (TTree and histogram) files.
/*! In the Run.config file one could specify the name of the file format without removing the '(Run)' and '(subRun)'.
*/
std::string * extract_oFileFormats_substrings(std::string fileFormat){
	std::string * sub_str = new std::string[3];
	sub_str[0] = fileFormat;
	std::size_t pos = sub_str[0].find("(Run)");
	if(pos!=std::string::npos)sub_str[0] = sub_str[0].erase(pos,sub_str[0].length() );
	sub_str[1]  = fileFormat;
	std::size_t pos2 = sub_str[1].find("(subRun)");
	if(pos2!=std::string::npos)sub_str[1] = sub_str[1].erase( pos2, sub_str[1].length());
	if(pos!=std::string::npos)sub_str[1] = sub_str[1].erase( 0, pos+5);
	sub_str[2] = fileFormat;
	if(pos2!=std::string::npos)sub_str[2] = sub_str[2].erase(0, pos2+8);


	return sub_str;
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
//------------------------------------------------
// format output ttree file name
//----------------------------------------------
//! Get the name of the Ttree file name.
/*! Depending on the option chosesn in processing the data such as sum runs into one file, for each run, sum the sub runs, the name of the TTree file is generated. If you chose sum runs = all, then, the file name will take (RUN = run numbers), (subRun = all) in the TTree file format. 
*/
std::string get_tree_filename(int l, std::string *t_sub_str, std::string output_dir, std::string runN, std::string subRunN, std::string sum_runs, std::string sum_subRuns,std::vector<std::string> runfiles, bool new_run_flag, int run_increament){
	std::string treeFileName; 
	std::string *sr; 
	if(l ==0){
		if(sum_runs == "YES"){
			treeFileName = output_dir + "/" + t_sub_str[0] + runN + t_sub_str[1] + "all" + t_sub_str[2];
		}
		else{
			if(sum_subRuns == "NO")
				treeFileName = output_dir + "/" + t_sub_str[0] + runN + t_sub_str[1] + subRunN + t_sub_str[2];
			else{
				sr = get_current_subrunRange(l,runfiles);
				if(sr[0] != sr[1])					
					treeFileName = output_dir+ "/" + t_sub_str[0] + runN + t_sub_str[1] + sr[0]+ "-" + sr[1] + t_sub_str[2];
				else
					treeFileName = output_dir+ "/" + t_sub_str[0] + runN + t_sub_str[1] + sr[0] + t_sub_str[2];
				delete[] sr; sr = NULL;
			}
		}
	}else{
		if(!new_run_flag){// have the same run number
			if( sum_subRuns == "NO")
				treeFileName = output_dir + "/" + t_sub_str[0] + runN + t_sub_str[1] + subRunN + t_sub_str[2];
		}
		else{//new run
			if(run_increament ==0){	
				if(sum_subRuns == "NO")
					treeFileName = output_dir + "/" + t_sub_str[0] + runN + t_sub_str[1] + subRunN + t_sub_str[2];
				else{
					sr = get_current_subrunRange(l, runfiles);
					if(sr[0] != sr[1])					
						treeFileName =  output_dir + "/" + t_sub_str[0] + runN + t_sub_str[1] + sr[0] + "-" + sr[1] + t_sub_str[2];
					else 
						treeFileName =  output_dir + "/" + t_sub_str[0] + runN + t_sub_str[1] + sr[0] + t_sub_str[2];
					delete[] sr; sr = NULL;
				}
			}else{
				stringstream ss;
				ss << run_increament;
				string run_inc_str = ss.str();
				if(sum_subRuns == "NO")
					treeFileName = output_dir + "/" + t_sub_str[0] + runN + "+" + run_inc_str + t_sub_str[1] + subRunN + t_sub_str[2];
				else{
					sr = get_current_subrunRange(l, runfiles);
					if(sr[0] != sr[1])					
						treeFileName = output_dir + "/" + t_sub_str[0] + runN + "+" + run_inc_str + t_sub_str[1] + sr[0] + "-" + sr[1] + t_sub_str[2];
					else
						treeFileName = output_dir + "/" + t_sub_str[0] + runN + "+" + run_inc_str + t_sub_str[1] + sr[0]  + t_sub_str[2];
					delete[] sr; sr = NULL;
				}

			}
		}
	}
	return treeFileName;
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
//-----------------------
//Check if it is new run
//-----------------------
//! This is to check if the current run from the list of files to be processed is a new run. 
/*!It is useful because sometimes two or more files can have same run numbers.
*/
bool is_a_new_run(int l, std::vector<std::string> list){

	if(l == 0) return true;
	else if(l > 0){
		//--------------------------------------------------
		//compare with the previous file
		//---------------------------------------------------
		std::string prev_file;
		std::size_t pos = list[l-1].find_last_of("s");
		if(pos!=std::string::npos)prev_file = list[l-1].substr(0, pos+1);

		std::string current_file;
		pos = list[l].find_last_of("s");
		if(pos!=std::string::npos)current_file = list[l].substr(0, pos+1);

		if(current_file.compare(prev_file) == 0){ return false;}
		else{ return true;}
	}
	else {cout<<"put l >= 0"<<endl; return 0;}
}
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
//----------------------------------------------------------
// function to get the existing files in the input directory 
//----------------------------------------------------------
//! To get the existing file names of the listed run numbers given in the list from the input directory. 
/*! Since, neither popen nor Dir works with root version 5 in execution of a macro (without compilation), the nales of all the files in the input directory are written to 'file_names.txt' file using usr/bin/ls. This files is then read and only files having the right format "run_%04d.dat.%*" and specified run numbers are kept.
 * @param list a container for the run numbers specified in the Run.config file.
 * @param dir absolute path of the directory where the data is stored.
 */
std::vector<std::string> get_existing_files(std::string dir, std::vector<std::string> list){
	std::vector<std::string> runfiles;
	std::vector<std::string> files = vector<string>();
	size_t nFiles = list.size();
	DIR *dp;
	struct dirent *dirp;

	if((dp = opendir(dir.c_str())) == NULL)
	{
		cerr << "Error opening " << dir << endl;
	}

	while ((dirp = readdir(dp)) != NULL)
		files.push_back(string(dirp->d_name));
	closedir(dp);
	for(size_t j = 0; j < nFiles; j++){
		for (size_t i = 0;i < files.size();i++){

			int runNo= 0;
			if(sscanf(files[i].c_str(), "run_%04d.dat.%*", &runNo))
			{
				if(runNo == atoi(list[j].c_str())){
					runfiles.push_back(files[i]);
				}
			}  
		}
	}
	files.clear();
	list.clear();
	std::sort(runfiles.begin(), runfiles.end());	
	return runfiles;
}

//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
//----------------------------------------------------------
// function to get the existing files in the input directory 
//----------------------------------------------------------
//! Overloaded function of get_existing_files(std::string input_dir, std::vector<int> list).
/*! To get the existing file names of the listed run numbers given in the list from the input directory.
 * @param list a container for the run numbers specified in the Run.config file.
 * @param dir absolute path of the directory where the data is stored.
 */
std::vector<std::string> get_existing_files(std::string dir, std::vector<int> list){
	std::vector<std::string> runfiles;
	std::vector<std::string> files = vector<string>();
	size_t nFiles = list.size();
	DIR *dp;
	struct dirent *dirp;

	if((dp = opendir(dir.c_str())) == NULL)
	{
		cerr << "Error opening " << dir << endl;
	}

	while ((dirp = readdir(dp)) != NULL)
		files.push_back(string(dirp->d_name));
	closedir(dp);
	for(size_t j = 0; j < nFiles; j++){
		for (size_t i = 0;i < files.size();i++){

			int runNo= 0;
			if(sscanf(files[i].c_str(), "run_%04d.dat.%*", &runNo))
			{
				if(runNo == list[j]){
					runfiles.push_back(files[i]);
				}
			}			 
		}
	}
	files.clear();
	list.clear();
	std::sort(runfiles.begin(), runfiles.end());	
	return runfiles;
}

void print_help(){
	std::cout<<"\n     To convert a run into a root file of given tree format:"
		"\n     Modify the parameters in RunToProcess.txt file"
		"\n                           or                      "
		"\n     Execute via :"
		"\n     ConvertToRootFile [OPTIONS] VALUE"
		"\n     OPTIONS: "
		"\n     -r, --runnumbers"
		"\n                VALUE eg:  1 2 3 or 1-3 or 1,2,3"
		"\n     -s, --subrunnumbers"
		"\n                VALUE eg:  0 1 2 or 0-3 or 0,1,2 or all"
		"\n     -sr, --sumruns"
		"\n                VALUE eg:  yes or no (Case insensitive)"
		"\n     -ss, --sumsubruns"
		"\n                VALUE eg:  yes or no (Case insensitive)"
		"\n     -i, --inputdirectory"
		"\n                VALUE eg:  /data/siriusX/test/acquisition/run/"
		"\n     -o, --outputdirectory" 
		"\n                VALUE eg:  /data/siriusX/test/acquisition/RootFiles"
		"\n     -f, --treefileformat"
		"\n                VALUE eg:  Tree_r-(Run)_s-(subRun)-Numexo2.root.  Warning do not remove (Run) and (subRun) from the format"
		"\n     -t, --treename"
		"\n                VALUE eg:  rawDataTree"
		"\n     -n, --numberofeventstoprocess"
		"\n                VALUE eg:  1000 or all"
		"\n     -h, --help"
		"\n                To display a message of utilization of the program.\n"<<endl;
}
//----------------------------------------------------------
// function to get the existing files in the input directory 
//----------------------------------------------------------
//! To get the existing file names of the listed run numbers given in the list from the input directory. 
/*! Since, neither popen nor Dir works with root version 5 in execution of a macro (without compilation), the nales of all the files in the input directory are written to 'file_names.txt' file using usr/bin/ls. This files is then read and only files having the right format "run_%04d.dat.%*" and specified run numbers are kept.
 * @param list a container for the run numbers specified in the Run.config file.
 * @param dir absolute path of the directory where the data is stored.
 */

/*std::vector<std::string> get_existing_files(std::string dir, vector<int> list){
  size_t nFiles = list.size();
  vector<std::string> runfiles;
//Note neither popen nor Dir works with root version 5
std::ifstream files_inDir;
std::string command = "/usr/bin/ls " + dir + "  >  file_names.txt";
std::string file;
system(command.c_str());
files_inDir.open("file_names.txt");
if(files_inDir.is_open()){
while(files_inDir.good() && !files_inDir.eof()){
getline(files_inDir, file);
if(file.empty())continue;
for(size_t j = 0; j < nFiles; j++){
int runNo= 0;
if(sscanf(file.c_str(), "run_%04d.dat.%*", &runNo))
{
if(runNo == list[j]){
runfiles.push_back(file);
}
}			
}
}
files_inDir.close();
}else{
std::cout<<"filelist could not be opend."<<std::endl;
}

std::sort(runfiles.begin(), runfiles.end());	
return runfiles;
}*/
//---------------ooooooooooooooo---------------ooooooooooooooo---------------ooooooooooooooo---------------//
//----------------------------------------------------------
// function to get the existing files in the input directory 
//----------------------------------------------------------
//! Overloaded function of get_existing_files(std::string input_dir, std::vector<int> list).
/*! To get the existing file names of the listed run numbers given in the list from the input directory.
 * @param list a container for the run numbers specified in the Run.config file.
 * @param dir absolute path of the directory where the data is stored.
 */
/*
   std::vector<std::string> get_existing_files(std::string dir, std::vector<std::string> list){
   size_t nFiles = list.size();
   vector<std::string> runfiles;
//Note neither popen nor Dir works with root version 5
std::ifstream files_inDir;
std::string command = "/usr/bin/ls " + dir + "  >  file_names.txt";
std::string file;
system(command.c_str());
files_inDir.open("file_names.txt");
if(files_inDir.is_open()){
while(files_inDir.good() && !files_inDir.eof()){
getline(files_inDir, file);
if(file.empty())continue;
for(size_t j = 0; j < nFiles; j++){
int runNo= 0;
if(sscanf(file.c_str(), "run_%04d.dat.%*", &runNo))
{
if(file.c_str() == list[j]){
runfiles.push_back(file);
}
}			
}
}
files_inDir.close();
}else{
std::cout<<"filelist could not be opend."<<std::endl;
}
list.clear();
std::sort(runfiles.begin(), runfiles.end());	
return runfiles;
}*/
