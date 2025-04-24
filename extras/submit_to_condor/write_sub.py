### MUST CHANGE this default values to your own!

# data you want to process
input_data_path = "/eos/experiment/fcc/prod/fcc/ee/test_spring2024/240gev/{pattern}/CLD_o2_v05/rec/{prod}/*/{pattern}_rec_*.root"
data_pattern = {"Huu": "00016774", "Hdd": "00016771", "Hss": "00016777", "Hcc": "00016780", "Hbb": "00016783", "Hgg": "00016768", "Htautau": "00016765"} # the key is the "pattern" and the value is the "prod" number
 
num_files = 5 # How many files to process per job
start_indices = range(0, 100, num_files) # total of 100 files per flavor with 1000 events each

# output paths
output_base = "/eos/experiment/fcc/ee/datasets/CLD_fullsim_tagging_key4hep_dummy_folder/" 
output_log = "/afs/cern.ch/work/s/saaumill/public/condor/std-condor/" # condor logs (stdout, stderr, log)

# select what job you want to run (submitJob_JetObsWriter.sh or submitJob_JetTagsWriter.sh)
job = "writeJetConstObs.py" # writeJetTags.py # which steering file to use
local_path_to_tagger = "/afs/cern.ch/work/s/saaumill/public/k4MLJetTagger/" # CHANGE your path here (used for path to your steering file (job) and in case of local==True, the k4MLJetTagger is locally used and NOT from the stack)

# if you want to run k4MLJetTagger locally or from the latest stack
local = True

# condor settings
acounting_group = "group_u_FCC.local_gen" 
job_flavour = "longlunch" # check out options here: https://batchdocs.web.cern.ch/local/submit.html

### DO NOT CHANGE ANYTHING BELOW THIS LINE

def generate_sub_file():
    """
    Create a .sub file for the JetObsWriter job submission to condor.
    The file will contain the header and the content with the arguments for the job. 
    Please make sure to use the correct paths for the input and output files and the correct executable.
    THIS function currently generates the .sub file for the JetObsWriter job submission.
    """

    
    # Prepare the header of the file
    header = f"""# run commands for submitting jet tagging jobs on condor

executable    = submitJob.sh
#requirements = (OpSysAndVer =?= "CentOS7")
# here you specify where to put .log, .out and .err files
output                = {output_log}job.$(ClusterId).$(ProcId).out
error                 = {output_log}job.$(ClusterId).$(ProcId).err
log                   = {output_log}job.$(ClusterId).$(ClusterId).log

+AccountingGroup = {acounting_group}
+JobFlavour    = {job_flavour}
"""

    # Prepare the content with arguments
    content = ""
    for pattern in data_pattern:
        job_counter = 0
        for start_index in start_indices:
            input_pattern = input_data_path.format(pattern=pattern, prod=data_pattern[pattern])
            output_file = f"{pattern}_{job_counter}.root"
            arguments = f"{job} {start_index} {num_files} \'{input_pattern}\' {output_base} {output_file} {local_path_to_tagger} {local}"
            content += f"arguments=\"{arguments}\"\nqueue\n"
            job_counter += 1

    # Write to the .sub file
    with open("condor.sub", "w") as file:
        file.write(header)
        file.write(content)

# Run the function to generate the file
generate_sub_file()
