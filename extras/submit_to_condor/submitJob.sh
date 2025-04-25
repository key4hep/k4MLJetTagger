#!/bin/bash
##
## Copyright (c) 2020-2024 Key4hep-Project.
##
## This file is part of Key4hep.
## See https://key4hep.github.io/key4hep-doc/ for further info.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##


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
if [[ "${LOCAL,,}" == "true" ]]; then
  cluster_loc=$(pwd)
  echo "[Info] Running k4MLJetTagger locally ... from ${TAGGER_DIR}"
  cd ${TAGGER_DIR}
  source ./setup.sh
  k4_local_repo
  cd $cluster_loc
else
  source /cvmfs/sw-nightlies.hsf.org/key4hep/setup.sh
fi

# get input files
root_files=($(ls ${FILE_PATTERN} 2>/dev/null | sort | tail -n +$((FROM_I + 1)) | head -n ${NUM_FILES})) # works fine even if NUM_FILES is larger than the number of files available
echo "[Info] Loading root files ..."

# Check if there are any matching files
if [ ${#root_files[@]} -eq 0 ]; then
    echo "[Error] No root files found matching the pattern."
    exit 1
fi

# make directory
mkdir -p job

# Copy each file using the Python script
for file in "${root_files[@]}"; do
    # Get the basename of the file
    base_name=$(basename "${file}")
    # Copy the file using the Python script
    python3 /afs/cern.ch/work/f/fccsw/public/FCCutils/eoscopy.py "${file}" "./job/${base_name}"
done
echo "[Info] Loaded root files ..."

# Extract the pattern (e.g., Hbb, Huu, etc.)
pattern=$(echo "${FILE_PATTERN}" | grep -oE '/H[[:alnum:]]+/')
pattern=${pattern:1:-1} # Remove the leading and trailing slashes

# Check if the pattern is empty
if [ -z "${pattern}" ]; then
    echo "[Error] Failed to extract pattern from FILE_PATTERN: ${FILE_PATTERN}"
    exit 1
fi

echo "[Info] Found following from pattern: ${pattern}"

# merge multiple root files to one input file
hadd "./job/merged_input_${pattern}.root" ./job/*.root
preproc_time=$(date +%s)
middle_time=$((preproc_time - start_time))
echo "[Info] Time before running script: $middle_time seconds"

# Run the script
cmd="k4run \"${TAGGER_DIR}/k4MLJetTagger/options/${JOB}\" --inputFiles=\"./job/merged_input_${pattern}.root\" --outputFile=\"./job/out.root\""
echo "[INFO] Running command: $cmd"
eval $cmd
echo "[Info] Job done ... "
job_time=$(($(date +%s) - preproc_time))
echo "[Info] Time to run job: $job_time seconds"

# make directory if it does not exist:
if [ ! -d "${OUTPUT_DIR}" ]; then
  mkdir -p "${OUTPUT_DIR}"
fi

# copy file to output dir
echo "[Info] Copying output file to: ${OUTPUT}"
python3 /afs/cern.ch/work/f/fccsw/public/FCCutils/eoscopy.py ./job/out.root ${OUTPUT}

# Finish info
echo "[Info] Ran script successfully!"
execution_time=$(($(date +%s) - start_time))
echo "[Info] Execution time: $execution_time seconds"

