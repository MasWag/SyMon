Getting Started
===============

Let's look at some examples of SyMon usage. See also [this document](./syntax.md) for the details of the specification language supported by SyMon. In this document, we assume that SyMon can be executed by `symon`. Replace `symon` with the actual executable path if your environment differs. Installation instructions are available [here](./install.md).

Consider the following as the monitored log. It is also in `example/exim4/frequent/example.log`.

```
arrival	alice@example.com	0.0
arrival	bob@example.org	0.3
arrival	alice@example.com	0.5
arrival carol@example.net	0.7
arrival	alice@example.com	0.8
arrival	alice@example.com	1.0
arrival	bob@example.org	1.2
arrival	alice@example.com	1.5
arrival	alice@example.com	1.7
arrival	alice@example.com	1.9
arrival	carol@example.net	2.0
arrival	alice@example.com	2.2
arrival	alice@example.com	2.4
arrival	bob@example.org	2.5
arrival	alice@example.com	2.7
arrival	carol@example.net	2.9
arrival	alice@example.com	3.1
arrival	bob@example.org	3.25
arrival	alice@example.com	3.3
arrival	bob@example.org	3.4
```

Signature declaration
---------------------

In SyMon, we first must declare the signature, i.e., the kind of events with the associated data. In the above log, we have only one kind of log (`arrival`) associated with a string-valued email address and a timestamp. For such events, the signature declaration is as follows.

```
signature arrival {
    sender: string;
}
```

Your first example
------------------

Let's detect the first occurrence of an email from `carol@example.net`. This can be done by 1) skipping the arrival of a message not from `carol@example.net` and 2) ending with the arrival of a message from `carol@example.net`. The following expression encodes such a specification.

```
zero_or_more {
    arrival( sender | sender != "carol@example.net" )
};
arrival( sender | sender == "carol@example.net" )
```

`zero_or_more` means possible repetition of the expression inside. This is the same notion of Kleene star as in regular expressions. In this example, SyMon first finds a repetition of message arrivals that are not from `carol@example.net`. Then, we detect the message arrival from `carol@example`. Alternatively, one can also write it using `*` operation as follows.

```
arrival( sender | sender != "carol@example.net" )*;
arrival( sender | sender == "carol@example.net" )
```

In SyMon's specification language, the signature declaration must be put before the expression. The following is a complete specification.

```
signature arrival {
    sender: string;
}

zero_or_more {
    arrival(sender | sender != "carol@example.net" )*
};
arrival(sender | sender == "carol@example.net" )
```

An example of the usage and the result is as follows.

```
symon -nf ./first-carol.symon < example.log
```

```
@0.700000.	(time-point 3)	true
```

The result shows that SyMon detected the given specification at time 0.7, corresponding to the fourth event (time-point is 0-origin) in the given log.

Timing constraints
------------------

We can impose timing constraints on the monitored log. For example, one can detect all the arrivals of emails from `carol@example.net` after 1 time unit as follows.

```
within (> 1) {
    zero_or_more {
        arrival(sender)
    };
    arrival(sender | sender == "carol@example.net" )
}
```

An example of the usage and the result is as follows.

```
symon -nf ./carol-after-1.symon < example.log
```

```
@2.000000.	(time-point 10)	true
@2.900000.	(time-point 15)	true
```

(Concrete) Variables
--------------------

SyMon supports variables. For instance, one can count the number of message arrivals from `alice@example.com` and detect the 11th arrival as follows. We note that in SyMon, we use `==` for equality of strings while `=` for equality of numbers.

```
var {
    count: number;
}

init {
    count = 0
}

signature arrival {
    sender: string;
}

one_or_more {
    one_of {
        arrival( sender | sender != "alice@example.com" && count <= 10 )
    } or {
        arrival( sender | sender == "alice@example.com" && count < 10 | count := count + 1 )
    }
};
arrival( sender | sender == "alice@example.com" && count = 10 | count := count + 1 )
```

An example of the usage and the result is as follows.

```
symon -pnf ./alice-11.symon < example.log
```

```
@3.1.	(time-point 16)	Num: A = 11	Clock: true
```

Parametric Variables
--------------------

SyMon also supports parametric variables, i.e., variables whose values are not explicitly specified, and symbolically handled based on constraints. For instance, one can detect the arrivals of more than 10 messages from the same sender within 3 time units as follows.

```
var {
    current_sender: string;
    count: number;
}

signature arrival {
    sender: string;
}

one_or_more {
    arrival(sender)
};
# Nondeterministically start counting for current_sender
arrival( sender | sender == current_sender | count := 1 );
within (<= 3) {
    one_or_more {
        one_of {
            arrival( sender | sender != current_sender && count <= 10 )
        } or {
            arrival( sender | sender == current_sender && count < 10 | count := count + 1 )
        }
    };
    arrival( sender | sender == current_sender && count = 10 | count := count + 1 )
}
```

An example of the usage and the result is as follows.

```
symon -dnf ./frequent.symon < example.log
```

```
@3.300000.	(time-point 18)	x0 == alice@example.com	A = 11
```
