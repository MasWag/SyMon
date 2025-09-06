#!/usr/bin/env python3
import sys
import re
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
        match = re.search(r'@.*', sender)
        sender_domain = match.group(0)[1:] if match else "none"

        host_match = re.search(r' H=.[^ ]*', line)
        host = host_match.group(0)[3:] if host_match else "none"
        host = host.translate(str.maketrans({'(': None, ')': None}))

        user_match = re.search(r' U=[^ ]*', line)
        user = user_match.group(0)[3:] if user_match else "none"

        auth_match = re.search(r' A=[^ ]*', line)
        auth = auth_match.group(0)[3:] if auth_match else "none"

        print(f"arrival\t{parts[2]}\t{sender}\t{sender_domain}\t{host}\t{user}\t{auth}\t{unix_time}")

    elif "=>" in line:
        parts = line.split()
        date_string = f"{parts[0]} {parts[1]}"
        unix_time = convert_to_unix_time(date_string) - convert_to_unix_time_day(parts[0])

        destination = parts[4]
        match = re.search(r'@.*', destination)
        destination_domain = match.group(0)[1:] if match else "none"

        print(f"delivery\t{parts[2]}\t{destination}\t{destination_domain}\t{unix_time}")

    elif "Completed" in line:
        parts = line.split()
        date_string = f"{parts[0]} {parts[1]}"
        unix_time = convert_to_unix_time(date_string) - convert_to_unix_time_day(parts[0])

        print(f"complete\t{parts[2]}\t{unix_time}")
