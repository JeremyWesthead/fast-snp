#!/bin/bash

set -e

#Use environment vars to load the secret bucket link
source .env

fasta=$1

#Pull the existing saves from a bucket here
#   <bucket> --> saves 
#Get the latest saves from the bucket
latest=$(curl -SsL $bucket | jq -r '.objects[].name' | sort | tail -n 1)
echo Using $latest

echo Getting from bucket
curl -SsL $bucket/$latest > $latest
echo
echo untaring
tar xzf $latest
echo
#<><><><><><>


#Get comparisons with the new ones
echo comparing
#Ensure file exists and is empty beforehand
[ -e comparisons.txt ] && rm comparisons.txt
touch comparisons.txt
./fn5 --add $fasta --cutoff 20 --output_file comparisons.txt
echo
#<><><><><><>

#Push new saves to a bucket here
#   saves/* --> <bucket>
output="$(date +%s).tar.gz"
echo compressing
tar czf $output saves
echo
echo uploading
curl -s -X PUT --data-binary "@$(pwd)/$output" $bucket/$output
#<><><><><><>

#Do something with comparions.txt here
#   Parse into DB? 
#   Upload to bucket?
#<><><><><><>