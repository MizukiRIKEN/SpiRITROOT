//#include "/mnt/spirit/analysis/user/tsangc/create_submit.C"
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <TString.h>
#include <vector>
#include "ConfigListToConfig.C"

const int num_jobs_in_queue = 4; // only allow this amount of jobs on ember for other users

void run_general_reco(const std::string& t_config_list, int t_start_from = 0, int t_num_jobs_to_be_submitted = num_jobs_in_queue)
{
	TString workDir   = gSystem -> Getenv("VMCWORKDIR");
  	TString parDir    = workDir + "/parameters/";

	ConfigListIO config_list;
	config_list.FillFromText(t_config_list);

	for(unsigned i = t_start_from; i < config_list.Size(); ++i)
	{
		// end the script if too many jobs are submitted
		if(i - t_start_from >= t_num_jobs_to_be_submitted)
			return;	

		// name of the input mc files
		TString input_name(config_list.GetElement(i, "Filename"));

		// name of the output reco files
		TString output_name(input_name);// + "_WithBDC");


		// right now we assume number of produced particles = total num / 6 as 6 being the numbers of available cocktail particles
		// start simulation
		std::cout << "Start simulation with output " << output_name << "\n";
		TString command = TString::Format("sbatch ./embedMacros/submit_general_reco.sh \"%s\" %d \"%s\" \"%s\"", output_name.Data(), i + num_jobs_in_queue, t_config_list.c_str(), input_name.Data());

		std::cout << " With command " << command << "\n";
		system(command.Data());
		std::cout << "Job submitted!\n";
	}

}