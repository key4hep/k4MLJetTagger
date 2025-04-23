#!/bin/bash

# shell script to submit analysis job to the batch system
echo "Running analysis job ..."
start_time=$(date +%s)

# get input parameters 
JOB=${1} # which steering file to run with the tagger 
FROM_I=${2} # Which index to start with regarding the input root files
NUM_FILES=${3} # number of files to process
FILE_PATTERN=${4} # input file pattern
OUTPUT_DIR=${5} # output directory
OUTPUT_FILE=${6} # output file
TAGGER_DIR=${7} # path to your local clone of k4MLJetTagger
LOCAL=${8} # if true, use k4MLJetTagger locally, otherwise use key4hep stack

# full output file path
OUTPUT=${OUTPUT_DIR}/${OUTPUT_FILE}

# setup the env
if [ "$LOCAL" = true ]; then
  cluster_loc=$(pwd)
  cd $(TAGGER_DIR)
  source ./setup.sh
  k4_local_repo
  cd $cluster_loc
else
  source /cvmfs/sw-nightlies.hsf.org/key4hep/setup.sh
fi

# get input files
root_files=($(ls ${FILE_PATTERN} 2>/dev/null | sort | tail -n +$((FROM_I + 1)) | head -n ${NUM_FILES})) # works fine even if NUM_FILES is larger than the number of files available 
echo "loading root files ..."

# Check if there are any matching files
if [ ${#root_files[@]} -eq 0 ]; then
    echo "No root files found matching the pattern."
    exit 1
fi

pwd 
# make directory
mkdir job

# Copy each file using the Python script
for file in "${root_files[@]}"; do
    # Get the basename of the file
    base_name=$(basename "${file}")
    # Copy the file using the Python script
    python3 /afs/cern.ch/work/f/fccsw/public/FCCutils/eoscopy.py "${file}" "./job/${base_name}"
done
echo "loaded root files ..."

# Extract the pattern (e.g., Hbb, Huu, etc.)
pattern=$(echo "${FILE_PATTERN}" | grep -oE '/H[[:alnum:]]+/')
pattern=${pattern:1:-1} # Remove the leading and trailing slashes

echo "pattern: ${pattern}"

# merge multiple root files to one input file
hadd "./job/merged_input_${pattern}.root" ./job/*.root
preproc_time=$(date +%s)
middle_time=$((preproc_time - start_time))
echo "time before running script: $middle_time seconds"

echo "running script ..."

k4run "${TAGGER_DIR}/k4MLJetTagger/options/${JOB}" --inputFiles="./job/merged_input_${pattern}.root" --outputFile="./job/out.root" 

echo "job done ... "
job_endtime=$(date +%s)
job_time=$((job_endtime - preproc_time)) 
echo "time to run job: $job_time seconds"
# make directory if it does not exist:
if [ ! -d ${OUTPUT_DIR} ]; then
  mkdir -p ${OUTPUT_DIR}
fi


echo "copying output file to: ${OUTPUT}"

# copy file to output dir
python /afs/cern.ch/work/f/fccsw/public/FCCutils/eoscopy.py ./job/out.root ${OUTPUT}
echo "Ran script successfully!"
end_time=$(date +%s)
execution_time=$((end_time - start_time))
echo "Execution time: $execution_time seconds"

