File structure
==============

A SyMon specification has a fixed top-level order:

```
var { ... }          optional
init { ... }         optional; supported only with parametric timing constraints
signature ...        one or more
expr name { ... }    optional named expressions
final expression     required
```

Declarations
------------

Use `var` for global variables and timing parameters:

```
var {
    current_sender: string;
    count: number;
    period: param;
}
```

String and number variables can appear in guards and can be updated by atomic
actions. Timing parameters declared with `param` can appear in timing
constraints such as `within (<= period) { ... }`.

Initial constraints
-------------------

Use `init` to constrain initial values:

```
init {
    count = 0 && current_sender != "blocked@example.net"
}
```

`init` is accepted by the syntax, but SyMon only supports it when the timing
constraint type is parametric. In non-parametric or data-parametric timing
modes, remove the `init` block and encode the initialization in the expression.
See [Execution modes](../modes.md) for the mode-level behavior.

Signatures
----------

Every event used in an expression must have a signature:

```
signature arrival {
    sender: string;
    size: number;
}
```

A file must contain at least one `signature`. Inside a signature, declare all
string fields before number fields.

Atomic expressions bind action arguments to signature fields in declaration
order:

```
arrival(sender, size | sender == "carol@example.net" && size > 0)
```

Named expressions
-----------------

Use `expr` to name a reusable expression:

```
expr skip_alice {
    zero_or_more {
        arrival(sender | sender != "alice@example.com")
    }
}

skip_alice;
arrival(sender | sender == "alice@example.com")
```

Named expressions must be declared before the final expression. Keep
dependencies in order so a named expression only refers to expressions that have
already been defined.

Final expression
----------------

The last top-level item must be the final expression. It is not wrapped in an
`expr name { ... }` block unless that named expression is referenced at the end:

```
expr target {
    arrival(sender | sender == "carol@example.net")
}

target
```

Comments and identifiers
------------------------

Line comments start with `#` and continue to the end of the line:

```
# Ignore all arrivals before the target event.
```

Identifiers must start with a letter or `_`, followed by letters, digits, or
`_`.

Next: [Expressions](./expressions.md), [Constraints](./constraints.md),
[Syntax examples](./examples.md), and [Common mistakes](./common-mistakes.md).
