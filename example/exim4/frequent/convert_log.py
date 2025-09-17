#!/usr/bin/env python3
import sys
from time import mktime, strptime

def convert_to_unix_time(date_string):
    try:
        time_struct = strptime(date_string, "%Y-%m-%d %H:%M:%S")
        return int(mktime(time_struct))
    except ValueError:
        return "unknown"

def convert_to_unix_time_day(day_string):
    try:
        time_struct = strptime(day_string, "%Y-%m-%d")
        return int(mktime(time_struct))
    except ValueError:
        return "unknown"

for line in sys.stdin:
    if "<=" in line:
        parts = line.split()
        date_string = f"{parts[0]} {parts[1]}"
        unix_time = convert_to_unix_time(date_string) - convert_to_unix_time_day(parts[0])

        sender = parts[4]

        print(f"arrival\t{sender}\t{unix_time}")
