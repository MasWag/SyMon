#!/bin/sh

sed 's/Num:/\nNum:/;s/Clock:/\nClock:/;' |
    sed '/Num:/ s/A/threshold/g;' |
    sed '/Clock:/ s/A/period/g;' |
    sed '/Clock:/ s/B/last_interval/g;' |
    sed 'N;N;s/\n//g;'
