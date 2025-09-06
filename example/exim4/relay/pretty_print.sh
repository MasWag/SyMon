#!/bin/sh

sed 's/x0/id/g;s/x1/sender/g;s/x2/host/g;s/x3/user/g;s/x4/auth/g;s/x5/destination/g;s/true//g;s/ == /: /g;'
