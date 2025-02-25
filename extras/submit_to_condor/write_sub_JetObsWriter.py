def generate_sub_file():
    """
    Create a .sub file for the JetObsWriter job submission to condor.
    The file will contain the header and the content with the arguments for the job. 
    Please make sure to use the correct paths for the input and output files and the correct executable.
    THIS function currently generates the .sub file for the JetObsWriter job submission.
    """


    # Define file patterns and other fixed values
    num_files = 5 # How many files to process per job
    start_indices = range(0, 100, num_files) # total of 100 files per flavor with 1000 events each
    base_command = "/eos/experiment/fcc/prod/fcc/ee/test_spring2024/240gev/{pattern}/CLD_o2_v05/rec/{prod}/*/{pattern}_rec_*.root" # YOUR DATA HERE
    # data_pattern = {"Huu": "00016774", "Hdd": "00016771", "Hss": "00016777", "Hcc": "00016780", "Hbb": "00016783", "Hgg": "00016768", "Htautau": "00016765"}
    # output_base = "/eos/experiment/fcc/ee/datasets/CLD_fullsim_tagging_from_key4hep/" # YOUR OUTPUT PATH HERE

    # data_pattern = {"Huu": "00016808", "Hcc": "00016810"} 
    # output_base = "/eos/experiment/fcc/ee/datasets/CLD_fullsim_tagging_debug_tracks/with_fixesPV/" # new data with fixed PV beamspot params

    # data_pattern = {"Huu": "00016808"} 
    # output_base = "/eos/experiment/fcc/ee/datasets/CLD_fullsim_tagging_debug_tracks/with_fixesPV/with_HiggsMCPV/" # as above, but saving jet_MCPV_x etc using the Higgs, not the daughters

    # data_pattern = {"Huu": "00016774"}
    # output_base = "/eos/experiment/fcc/ee/datasets/CLD_fullsim_tagging_debug_tracks/checking_firstprod_Juraj_00016774/saving_MCPV_fromDaughters/" # checking the first production campaign from Juraj

    # data_pattern = {"Huu": "00016774"}
    # output_base = "/eos/experiment/fcc/ee/datasets/CLD_fullsim_tagging_debug_tracks/checking_firstprod_Juraj_00016774/saving_MCPV_fromHiggs/" # checking the first production campaign from Juraj

    data_pattern = {"Huu": "00016823"}
    output_base = "/eos/experiment/fcc/ee/datasets/CLD_fullsim_tagging_debug_tracks/with_fixedSmearing/" # new data with fixed smearing. Manually checked that MC PV from daughters and Higgs are the same.


    """ NOTE
    The different numbers in the input path file refer to the different production campaigns. 
    Taking Huu: 
    - 00016774: First big production campaign from Juraj with newest key4hep stack to validate my tagger (done)
    - 00016808: Fixing PV beamspot params done by Juraj
    - 00016823: Fixing the double smearing in the vertex by Juraj

    In 00016808, the MC PV are not correct!! Let's see if they are in 00016774, because I have not checked the PV back then.

    """

    
    # Prepare the header of the file
    header = """# run commands for analysis,

# here goes your shell script (!! YOUR JOB !!)
executable    = submitJob_JetObsWriter.sh
#requirements = (OpSysAndVer =?= "CentOS7")
# here you specify where to put .log, .out and .err files
output                = /afs/cern.ch/work/s/saaumill/public/condor/std-condor/job.$(ClusterId).$(ProcId).out
error                 = /afs/cern.ch/work/s/saaumill/public/condor/std-condor/job.$(ClusterId).$(ProcId).err
log                   = /afs/cern.ch/work/s/saaumill/public/condor/std-condor/job.$(ClusterId).$(ClusterId).log

+AccountingGroup = "group_u_FCC.local_gen"
+JobFlavour    = "longlunch"
"""

    # Prepare the content with arguments
    content = ""
    for pattern in data_pattern:
        job_counter = 0
        for start_index in start_indices:
            input_pattern = base_command.format(pattern=pattern, prod=data_pattern[pattern])
            output_file = f"{output_base}{pattern}_{job_counter}.root"
            arguments = f"{start_index} {num_files} \'{input_pattern}\' {output_base} {output_file}"
            content += f"arguments=\"{arguments}\"\nqueue\n"
            job_counter += 1

    # Write to the .sub file
    with open("jetobswriter.sub", "w") as file:
        file.write(header)
        file.write(content)

# Run the function to generate the file
generate_sub_file()
