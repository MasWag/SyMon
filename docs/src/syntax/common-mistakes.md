Common mistakes
===============

Using `=` for strings
---------------------

Use `==` for string equality:

```
# Wrong
arrival(sender | sender = "carol@example.net")

# Correct
arrival(sender | sender == "carol@example.net")
```

Use `!=` for string disequality:

```
arrival(sender | sender != "blocked@example.net")
```

Using `==` for numbers
----------------------

Use `=` for numeric equality:

```
# Wrong
arrival(sender, size | size == 10)

# Correct
arrival(sender, size | size = 10)
```

Use `<>` for numeric disequality:

```
# Wrong
arrival(sender, size | size != 10)

# Correct
arrival(sender, size | size <> 10)
```

Forgetting a signature
----------------------

Every event name used in an expression needs a `signature`:

```
signature arrival {
    sender: string;
}

arrival(sender)
```

Forgetting the final expression
-------------------------------

Named expressions are definitions, not the final monitored expression. End the
file with the expression to monitor:

```
expr target {
    arrival(sender | sender == "carol@example.net")
}

target
```

Using `init` in the wrong mode
------------------------------

`init` is parsed by the syntax, but SyMon only supports initial constraints with
parametric timing constraints. If SyMon reports that initial constraints are not
supported, remove the `init` block or run the appropriate parametric timing
mode. See [Execution modes](../modes.md).

Mixing up `one_of` and `all_of`
-------------------------------

`one_of` branches are separated by `or`:

```
one_of {
    arrival(sender | sender == "alice@example.com")
} or {
    arrival(sender | sender == "carol@example.net")
}
```

`all_of` branches are separated by `and`:

```
all_of {
    arrival(sender | sender == "alice@example.com")
} and {
    arrival(sender | sender != "blocked@example.net")
}
```

This is wrong:

```
one_of {
    arrival(sender | sender == "alice@example.com")
} and {
    arrival(sender | sender == "carol@example.net")
}
```

Trying to update event fields
-----------------------------

Updates assign to global variables declared in `var`, not to fields declared in
an event signature:

```
var {
    saved_sender: string;
}

signature arrival {
    sender: string;
}

arrival(sender | | saved_sender := sender)
```

Back to the [Syntax overview](../syntax.md), or continue with
[Syntax examples](./examples.md).
