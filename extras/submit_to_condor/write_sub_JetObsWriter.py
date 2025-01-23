def generate_sub_file():
    # Define file patterns and other fixed values
    data_pattern = {"Huu": "00016774", "Hdd": "00016771", "Hss": "00016777", "Hcc": "00016780", "Hbb": "00016783", "Hgg": "00016768", "Htautau": "00016765"}
    num_files = 5
    start_indices = range(0, 100, num_files) # total of 100 files per flavor with 1000 events each
    base_command = "/eos/experiment/fcc/prod/fcc/ee/test_spring2024/240gev/{pattern}/CLD_o2_v05/rec/{prod}/*/{pattern}_rec_*.root"
    output_base = "/eos/experiment/fcc/ee/datasets/CLD_fullsim_tagging_from_key4hep/"
    
    # Prepare the header of the file
    header = """# run commands for analysis,

# here goes your shell script
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
