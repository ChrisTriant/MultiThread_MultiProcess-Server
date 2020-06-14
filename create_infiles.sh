#! /bin/bash

if [ -z $1 ]
then
    echo -e "ERROR!\nInvalid number of arguments\n"
    exit
else
    diseasesFile=$1
fi

if [ -z $2 ]
then
    echo -e "ERROR!\nInvalid number of arguments\n"
    exit
else
    countriesFile=$2
fi

if [ -z $3 ]
then
    echo -e "ERROR!\nInvalid number of arguments\n"
    exit
else
    input_dir=$3
fi

if [ $4 -le 0 ]
then
    echo -e "numFilesPerDirectory is default set to 10\n"
else
    numFilesPerDirectory=$4
fi


if [ $5 -le 0 ]
then
    echo -e "numRecordPerFile is default set to 10\n"
else
    numRecordsPerFile=$5
fi

rm -rf $input_dir
mkdir $input_dir


Names="Bob
Billy
Alexander
Sarah
Elen
Sofia
Karen
George
Mary
Phillip"

Surnames="Smith
Johnson
Williams
Brown
Jones
Miller
Davis
Wilson
Davis
Garcia
"


names=($Names)
surnames=($Surnames)
state=($State)

num_names=${#names[*]}
num_surnames=${#surnames[*]}

declare -i id=0

while read p; do
    mkdir -p $input_dir/$p/
    for((i=0;i<$numFilesPerDirectory;i++))
    do
        let "day=$RANDOM%30 +1"
        if [ $day -lt 10 ]
        then
            day="0$day"
        fi
        let "month=$RANDOM%12+1"
        if [ $month -lt 10 ]
        then
            month="0$month"
        fi
        let "year=$RANDOM%20+2001"
        date="$day-$month-$year"
        touch $input_dir/$p/$date
    done
    for f in $input_dir/$p/*; do
        lines=$(< $f wc -l)
        for((j=0;j<$(($numRecordsPerFile-$lines));j++)); do
            rand_name=${names[$((RANDOM%num_names))]}
            rand_surname=${surnames[$((RANDOM%num_surnames))]}
            status="ENTER"
            recordid=$id
            disease=$(shuf -r -n 1 $diseasesFile)
            let "age=$RANDOM%120 +1"
            record="$recordid $status $rand_name $rand_surname $disease $age"
            echo "$record" >>$f
            id+=1
            let "exitperc=$RANDOM%100 +1"
            if [ $exitperc -lt 81 ]
            then    
                status="EXIT"
                record="$recordid $status $rand_name $rand_surname $disease $age"
                rand_file=$(ls $input_dir/$p/ | shuf -n 1)
                rand_line=$(< $input_dir/$p/$rand_file wc -l)
                if [ $rand_line != $numRecordsPerFile ]
                then
                    if [ $f == $input_dir/$p/$rand_file ]
                    then
                        j=$(($j +1))
                    fi
                    echo "$record" >> $input_dir/$p/$rand_file
                fi
            fi
        done 
    done
done < $countriesFile
