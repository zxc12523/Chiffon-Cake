#!/bin/bash

while getopts n:m:l: flag
do
    case "${flag}" in 
        n) player_n=${OPTARG};;
        m) host_n=${OPTARG};;
        l) lucky_number=${OPTARG};;
    esac
done

function make_fifo {
    mkfifo "fifo_0.tmp"
    i=1
    while [ $i -le $host_n ]; do
        [ -p "fifo_${i}.tmp" ] || mkfifo "fifo_${i}.tmp"
        i=$(($i+1))
    done
}

function clear_fifo {
    rm -f "fifo_0.tmp"
    i=1
    while [ $i -le $host_n ]; do
        rm -f "fifo_${i}.tmp"
        i=$(($i+1))
    done
}

function create_host {
    for i in $(seq 1 $host_n)
    do
        ./host "-d" "0" "-m" "$i" "-l" "$lucky_number" &
    done
}

function close_host {
    open_fifo
    for i in $(seq 1 $host_n)
    do
        echo "-1 -1 -1 -1 -1 -1 -1 -1" > "fifo_$i.tmp";
    done
}

function open_fifo {
    for i in $(seq 0 $host_n)
    do
       exec {FD}<> "fifo_$i.tmp"
    done
}

make_fifo
open_fifo

create_host

available=()
win_point=()
combination=()

for i in $(seq 0 $host_n)
do
    available[${#available[@]}]=0
done

for i in $(seq 0 $player_n)
do
    win_point[${#win_point[@]}]=0
done

for a in $(seq 1 5)
do
    for ((b=$a+1;b<=6;b++));
    do 
        for ((c=$b+1;c<=7;c++));
        do 
            for ((d=$c+1;d<=8;d++));
            do 
                for ((e=$d+1;e<=9;e++));
                do 
                    for ((f=$e+1;f<=10;f++));
                    do 
                        for ((g=$f+1;g<=11;g++));
                        do 
                            for ((h=$g+1;h<=$player_n;h++))
                            do 
                                combination[${#combination[@]}]="$a $b $c $d $e $f $g $h"
                            done 
                        done 
                    done
                done
            done 
        done 
    done 
done

cnt=0
sol=0

while [ $cnt -lt ${#combination[@]} ] ;
do
    for i in $(seq 1 $host_n)
    do
        if [ ${available[$i]} -eq 0 ]; then
            echo ${combination[$cnt]} > "fifo_$i.tmp";
            available[$i]=1
            cnt=$(($cnt+1))
            break
        fi
    done

    if  read -t 0.05 line < "fifo_0.tmp"; then
        host_id=$(($line | bc))  #host id
        available[host_id]=0

        ma_point=0
        tmp_point=()

        for i in $(seq 0 $player_n)
        do
            tmp_point[${#tmp_point[@]}]=0
        done

        for i in $(seq 1 8)
        do
            read line < "fifo_0.tmp"

            sub=" "
            pos=$(expr index "$line" "$sub")
            len=$((${#line}-$pos))

            id=$((${line:0:$pos} | bc))
            point=$((${line:$pos:$len | bc}))

            win_point[id]=$((${win_point[$id]} + $point))
        done

        sol=$(($sol+1))
    fi
done

while [ $sol -lt ${#combination[@]} ] ;
do
    if  read -t 0.05 line < "fifo_0.tmp"; then
        host_id=$(($line | bc))  #host id
        available[host_id]=0

        ma_point=0
        tmp_point=()

        for i in $(seq 0 $player_n)
        do
            tmp_point[${#tmp_point[@]}]=0
        done

        for i in $(seq 1 8)
        do
            read line < "fifo_0.tmp"

            sub=" "
            pos=$(expr index "$line" "$sub")
            len=$((${#line}-$pos))

            id=$((${line:0:$pos} | bc))
            point=$((${line:$pos:$len | bc}))

            win_point[id]=$((${win_point[$id]} + $point))
        done

        sol=$(($sol+1))
    else
        break
    fi
done

choose=()

for i in $(seq 0 $player_n)
do
    choose[${#choose[@]}]=0
done

cnt=1
p=0

while [ $cnt -le $player_n ]
do
    ma=-1
    for i in $(seq 1 $player_n)
    do
        if [[ ${choose[$i]} -eq 0 && (${win_point[$i]} -gt $ma) ]]; then
            ma=${win_point[$i]}
        fi
    done

    for i in $(seq 1 $player_n)
    do
        if [ ${win_point[$i]} -eq $ma ]; then
            choose[$i]=$cnt
            p=$(($p+1))
        fi
    done

    cnt=$(($cnt+$p))
    p=0
done

for i in $(seq 1 $player_n)
do
    echo $i ${choose[$i]}
done

close_host

wait

clear_fifo

exit 0
