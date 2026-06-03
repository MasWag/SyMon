Syntax examples
===============

These examples are complete `.symon` specifications. They are intentionally
small; see the top-level [Examples](../examples.md) page for larger case
studies.

Minimal string guard
--------------------

Detect an arrival from one concrete sender:

```
signature arrival {
    sender: string;
}

arrival(sender | sender == "carol@example.net")
```

Run it in boolean mode:

```
symon -bnf first-carol.symon
```

Counter with an update
----------------------

Store a counter after the first matching event, then check and update it on the
second matching event:

```
var {
    count: number;
}

signature arrival {
    sender: string;
}

arrival(sender | sender == "alice@example.com" | count := 1);
arrival(sender | sender == "alice@example.com" && count = 1 | count := count + 1)
```

Run it in data-parametric mode:

```
symon -dnf two-alice.symon
```

Timing restriction
------------------

Remember an identifier and require another event with the same identifier within
three time units:

```
var {
    first_id: string;
}

signature ping {
    id: string;
}

ping(id | | first_id := id);
within [0, 3] {
    ping(id | id == first_id)
}
```

Run it in boolean mode:

```
symon -bnf ping-within-3.symon
```

Parametric timing with `param` and `init`
-----------------------------------------

Use a timing parameter in a bound and constrain an initial data value:

```
var {
    period: param;
    threshold: number;
}

init {
    threshold > 0
}

signature update {
    value: number;
}

within (<= period) {
    update(value | value >= threshold)
}
```

Run it in parametric timing mode:

```
symon -pnf parametric-period.symon
```

Back to the [Syntax overview](../syntax.md), or see
[Execution modes](../modes.md) for mode-dependent behavior.
