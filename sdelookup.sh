#!/bin/bash

declare -A TABLE_TYPES

TABLE_TYPES=( ["200"]="Old Method"
              ["400"]="PGID (new) Method"
              ["600"]="PGID 0 Only (new) Method"
              ["1000"]="Direct Mapping (w/ case)"
              ["1200"]="Offset Mapping (w/case)"
		      ["1400"]="Stride Mapping (w/case)"
              ["1600"]="Stride Block Mapping (w/case)"
              ["1800"]="Lookup Table Mapping (w/case)"
              ["2000"]="Multiple PGID Table lookup (w/case)"
              ["2200"]="Dynamic Direct Mapping (w/ case)"
              ["2400"]="Dynamic Offset Mapping (w/case)"
		      ["2600"]="Dynamic Stride Mapping (w/case)"
              ["2800"]="Dynamic Stride Block Mapping (w/case)"
              ["3000"]="Dynamic Lookup Table Mapping (w/case)"
              ["3200"]="Dynamic Multiple PGID Table lookup (w/case)"
              ["4000"]="Full Direct Mapping (w/ case)"
              ["4200"]="Full Pure Intra Direct Mapping (w/ case)"
              ["4400"]="Full Offset Mapping (w/case)"
              ["4600"]="Full Pure Intra Offset Mapping (w/case)"
		      ["4800"]="Full Stride Mapping (w/case)"
              ["5000"]="Full Pure Intra Stride Mapping (w/case)"
              ["5200"]="Full Stride Block Mapping (w/case)"
              ["5400"]="Full Pure Stride Block Mapping (w/case)"
              ["5600"]="Full Lookup Table Mapping (w/case)"
              ["5800"]="Full Pure Lookup Table Mapping (w/case)"
              ["6000"]="Full Multiple PGID Table lookup (w/case)"
              ["6200"]="Full Old method (no case)"
            )
METHODS="200 400 600 1000 1200 1400 1600 1800 2000 2200 2400 2600 2800 3000 3200 4000 4200 4400 4600 4800 5000 5200 5400 5600 5800 6000 6200"

#"${!b[@]}"
for x in ${METHODS}; do

    echo $x "-->" ${TABLE_TYPES["$x"]}
    y=$(($x+10))

    /opt/intel/sde/sde64          \
        -pinlit2                  \
        -debugtrace               \
        -start_ssc_mark $x:1     \
        -stop_ssc_mark  $y:1     \
        -log:mt                   \
        -log:focus_thread 1       \
        -dt_symbols 1             \
        -dt_flush                 \
        -mix                      \
        -footprint                \
        -length 2000000000        \
        -- ./a.out > /dev/null 2>&1
    inst=$(cat sde-debugtrace-out.txt | grep INS | wc -l)
    inst=$(($inst-2))
    inst_val=$(cat sde-debugtrace-out.txt | grep INS)
    echo $inst instructions
    echo "$inst_val"
    cat   sde-footprint.txt
done

exit
