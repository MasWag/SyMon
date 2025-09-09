#!/bin/sh

sed 's/x0/current_sender/g;s/A/count/g;s/ == /: /g;s/ = /: /g;'
