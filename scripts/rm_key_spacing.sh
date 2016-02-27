#!/bin/sh

#if [[ "$2" != '' ]]
#then
#   exec > $2
#   exec 2>&1
#fi

outFile="${1}.no_delim"

echo $outFile

exec > $outFile

cat $1 | tr -d " "

#echo $trunc_file


#while IFS= read -r LINE; 
#do
#   $line=""
#   $(line)+= "$(grep -o -e=*[A-Za-z0-9]*  $LINE)"
#   echo $line
#
#done < $file_name

