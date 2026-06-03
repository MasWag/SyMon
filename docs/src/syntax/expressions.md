Expressions
===========

Expressions describe event patterns. An expression can be an atomic action, a
named expression, a composition of smaller expressions, or a timed expression.

Atomic actions
--------------

An atomic action has the event name followed by parentheses:

```
arrival(sender)
```

Arguments are identifiers separated by commas. They are matched to the event's
signature fields in declaration order: string fields first, then number fields.

An atomic action may also include a guard and updates:

```
arrival(sender, size | sender == "carol@example.net" && size > 0 | count := count + 1)
```

Use an empty guard section when an action only updates variables:

```
arrival(sender | | saved_sender := sender)
```

See [Constraints](./constraints.md) for guard and update syntax.

Composition operators
---------------------

| Construct | Meaning |
| --- | --- |
| `e1 ; e2` | match `e1` followed by `e2` |
| `e1 && e2` | match both expressions |
| `e1 || e2` | match either expression |
| `( e )` | group an expression |

Use parentheses when mixing operators if the intended grouping is not obvious.

Repetition and optional expressions
-----------------------------------

SyMon supports postfix operators and equivalent block forms:

| Postfix form | Block form | Meaning |
| --- | --- | --- |
| `e*` | `zero_or_more { e }` | zero or more repetitions |
| `e+` | `one_or_more { e }` | one or more repetitions |
| `e?` | `optional { e }` | zero or one occurrence |

For example:

```
arrival(sender | sender != "carol@example.net")*;
arrival(sender | sender == "carol@example.net")
```

Choices and conjunction blocks
------------------------------

Use `one_of` for alternatives. Branches after the first use `or`:

```
one_of {
    arrival(sender | sender == "alice@example.com")
} or {
    arrival(sender | sender == "carol@example.net")
}
```

Use `all_of` when all branches must match. Branches after the first use `and`:

```
all_of {
    arrival(sender | sender == "alice@example.com")
} and {
    arrival(sender | sender != "blocked@example.net")
}
```

Both forms require at least two branches.

Timing restrictions
-------------------

Use `within` to restrict the duration of an expression:

```
within [1, 3] {
    arrival(sender);
    arrival(sender)
}
```

Use `%` for the postfix form:

```
(arrival(sender); arrival(sender)) % (<= 3)
```

Timing constraints can be intervals or half guards:

| Form | Meaning |
| --- | --- |
| `[a,b]` | inclusive lower and upper bounds |
| `(a,b)` | exclusive lower and upper bounds |
| `[a,b)` | inclusive lower, exclusive upper |
| `(a,b]` | exclusive lower, inclusive upper |
| `(< a)`, `(<= a)`, `(= a)`, `(<> a)`, `(>= a)`, `(> a)` | single timing guard |

Bounds are numeric expressions. They may use numbers, timing parameters declared
as `param`, parentheses, `+`, and `-`. Use timing parameters with parametric
timing mode; see [Execution modes](../modes.md).

Ignoring events
---------------

Use `ignore` to allow selected event names to occur without being consumed by
the inner expression:

```
ignore arrival {
    arrival(sender | sender == "carol@example.net")
}
```

Multiple event names are separated by commas:

```
ignore arrival, update {
    target
}
```

Next: [File structure](./file-structure.md), [Constraints](./constraints.md),
[Syntax examples](./examples.md), and [Common mistakes](./common-mistakes.md).
