Constraints and updates
=======================

Atomic actions can contain a guard, updates, or both:

```
event(args | constraints | updates)
event(args | constraints)
event(args | | updates)
```

Constraints
-----------

Use `&&` to combine constraints:

```
arrival(sender, size | sender == "carol@example.net" && size > 0)
```

String constraints use `==` and `!=`:

| Construct | Meaning |
| --- | --- |
| `x == y` | string equality |
| `x != y` | string disequality |
| `x == "literal"` | compare with a string literal |
| `"literal" != y` | compare a literal with a variable |

String literals use double quotes.

Numeric constraints use `<`, `<=`, `=`, `<>`, `>=`, and `>`:

| Construct | Meaning |
| --- | --- |
| `n < m` | less than |
| `n <= m` | less than or equal |
| `n = m` | numeric equality |
| `n <> m` | numeric disequality |
| `n >= m` | greater than or equal |
| `n > m` | greater than |

Numeric expressions may use identifiers, numbers, parentheses, `+`, and `-`:

```
amount + fee <= limit - 1
```

Numbers may be integers or decimals, with an optional leading `-`.

Updates
-------

Use `:=` to update global variables declared in `var`:

```
arrival(sender | sender == current_sender | count := count + 1)
```

Separate multiple updates with semicolons:

```
arrival(sender, amount | amount > 0 | saved_sender := sender; total := total + amount)
```

String variables can be updated from a string literal or a string-valued
identifier. Number variables can be updated from a numeric expression.

Updates target global variables, not event fields declared in a signature.

Guards without updates
----------------------

When an action only checks a condition, use one `|`:

```
arrival(sender | sender != "blocked@example.net")
```

Updates without guards
----------------------

When an action only performs updates, keep the empty guard section:

```
arrival(sender | | saved_sender := sender)
```

Next: [Expressions](./expressions.md), [File structure](./file-structure.md),
[Syntax examples](./examples.md), and [Common mistakes](./common-mistakes.md).
