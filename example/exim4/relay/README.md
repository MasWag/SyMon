---
author: Masaki Waga
title: Relay Detection with SyMon
---

This is an example to detect SMTP relay using
[SyMon](https://github.com/MasWag/SyMon/). See [this
document](https://www.exim.org/exim-html-3.20/doc/html/spec_46.html#SEC813)
for the detail of the relevant configuration of Exim4. This example
consists of a Python script `convert_log.py` to clean up and construct a
log file for SyMon and the files required by SyMon.

# Requirements

- [SyMon](https://github.com/MasWag/SyMon/)
- Python3
- Emacs
  - to tangle the resulting files from relay.org

# Usage

The usage of the generated files is as follows:

``` bash
sudo cat /var/log/exim4/mainlog |
    ./convert_log.py |
    ./relay.symon |
    ./pretty_print.sh
```

In the converted log, the timestamp is the duration since the beginning
of the day in seconds.

You can re-generate `relay.symon` by `make`. You can also generate the
document with `make doc`

# Events

We monitor the following events:

- `arrival`: The event showing the message arrival. This is shown as
  `<=` in the raw log.
- `delivery`: The event showing the normal message delivery. This is
  shown as `=>` in the raw log
- `complete`: The event shows the end of a transaction. This is shown as
  `Completed` in the raw log

Each event is tied with a string `id` corresponding to the unique
identifier of the message.

In addition, arrival event has the following fields:

- `sender`: The email address of the sender, e.g.,
  `example@example.com`.
- `sender_domain`: The domain of the sender email address, e.g.,
  `example.com`.
- `host`: The name of the source host, i.e., the `H` field.
- `auth`: The authentication information, i.e., the `A` field.
- `user`: The user of the event, i.e., the `U` field.

The corresponding signature for SyMon is as follows:

``` symon
signature arrival {
    id: string;
    sender: string;
    sender_domain: string;
    host: string;
    auth: string;
    user: string;
}
```

In addition to `id`, delivery event has the following fields:

- `destination`: The email address of the destination.
- `destination_domain`: The domain of the destination email address.

The corresponding signature for SyMon is as follows (i.e. the event
`arrival` is augmented with three string fields and zero numeric
fields):

``` symon
signature delivery {
    id: string;
    destination: string;
    destination_domain: string;
}
```

Overall, we use the following signature to specify the monitored events.

``` symon
signature arrival {
    id: string;
    sender: string;
    sender_domain: string;
    host: string;
    auth: string;
    user: string;
}
signature delivery {
    id: string;
    destination: string;
    destination_domain: string;
}
signature complete {
    id: string;
}
```

# Specification

What we want to do in the monitoring process is as follows:

- It waits for `arrival` of the (parameterized) current id.
  - If the sender's domain is `example.com`, it is good and we stop
  - Otherwise, we assign `sender`, `host`, and `user` to the internal
    variables for reporting.
- After the `arrival`, it waits for `delivery` or `complete` with the
  current id.
  - We stop tracking the id after these events.
  - If we detect a `delivery` whose the destination domain is not
    `example.com`, it is a relay and we raise an alert.
    - We also save `destination` for reporting.

Overall, we need the following (global) variables.

``` symon
var {
    current_id: string;
    current_sender: string;
    current_host: string;
    current_user: string;
    current_auth: string;
    current_destination: string;
}
```

First, we ignore events before tracking the current transaction.

``` symon
zero_or_more {
    one_of {
        arrival(id, sender, sender_domain, host, auth, user)
    } or {
        delivery(id, destination, destination_domain)
    } or {
        complete(id)
    }
}
```

Then, we detect a relevant message arrival from non-internal domain
(i.e., other than `example.com`).

``` symon
arrival(id, sender, sender_domain, host, auth, user | sender_domain != "example.com" | current_id := id; current_sender := sender; current_host := host; current_user := user)
```

After that, we ignore events with `id` not the `current_id`.

``` symon
zero_or_more {
    one_of {
        arrival(id, sender, sender_domain, host, auth, user | id != current_id)
    } or {
        delivery(id, destination, destination_domain | id != current_id)
    } or {
        complete(id | id != current_id)
    }
}
```

Finally, we deem that it is a relay if the destination is not our
internal addresses (i.e., `example.com`).

``` symon
delivery(id, destination, destination_domain | id == current_id && destination_domain != "example.com" | current_destination := destination)
```

Overall, the following shows the specification.

``` symon

var {
    current_id: string;
    current_sender: string;
    current_host: string;
    current_user: string;
    current_auth: string;
    current_destination: string;
}

signature arrival {
    id: string;
    sender: string;
    sender_domain: string;
    host: string;
    auth: string;
    user: string;
}
signature delivery {
    id: string;
    destination: string;
    destination_domain: string;
}
signature complete {
    id: string;
}

zero_or_more {
    one_of {
        arrival(id, sender, sender_domain, host, auth, user)
    } or {
        delivery(id, destination, destination_domain)
    } or {
        complete(id)
    }
};
arrival(id, sender, sender_domain, host, auth, user | sender_domain != "example.com" | current_id := id; current_sender := sender; current_host := host; current_user := user);
zero_or_more {
    one_of {
        arrival(id, sender, sender_domain, host, auth, user | id != current_id)
    } or {
        delivery(id, destination, destination_domain | id != current_id)
    } or {
        complete(id | id != current_id)
    }
};
delivery(id, destination, destination_domain | id == current_id && destination_domain != "example.com" | current_destination := destination)
```
