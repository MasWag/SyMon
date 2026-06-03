SyMon syntax
============

This page is a compact guide to SyMon's specification language. For a
step-by-step introduction, see [Getting Started](./getting_started.md). For
complete small specifications, see [Syntax examples](./syntax/examples.md). For
mode-dependent behavior, see [Execution modes](./modes.md).

Minimal example
---------------

```
signature arrival {
    sender: string;
}

arrival(sender | sender == "carol@example.net")
```

A specification must declare at least one signature and end with one final
expression. The final expression is the pattern that SyMon monitors.

File shape
----------

SyMon files have this top-level order:

```
var { ... }          optional
init { ... }         optional; supported only with parametric timing constraints
signature ...        one or more
expr name { ... }    optional named expressions
final expression     required
```

See [File structure](./syntax/file-structure.md) for declarations, comments,
identifiers, named expressions, and the final expression.

Expression constructs
---------------------

| Construct | Meaning | Details |
| --- | --- | --- |
| `event(args)` | atomic action | [Expressions](./syntax/expressions.md) |
| `event(args | guard | updates)` | guarded action with updates | [Constraints](./syntax/constraints.md) |
| `e1 ; e2` | sequence | [Expressions](./syntax/expressions.md) |
| `e1 && e2` | conjunction | [Expressions](./syntax/expressions.md) |
| `e1 || e2` | disjunction | [Expressions](./syntax/expressions.md) |
| `e*` / `zero_or_more { e }` | zero or more repetitions | [Expressions](./syntax/expressions.md) |
| `e+` / `one_or_more { e }` | one or more repetitions | [Expressions](./syntax/expressions.md) |
| `e?` / `optional { e }` | optional expression | [Expressions](./syntax/expressions.md) |
| `one_of { e1 } or { e2 }` | choose one branch | [Expressions](./syntax/expressions.md) |
| `all_of { e1 } and { e2 }` | require all branches | [Expressions](./syntax/expressions.md) |
| `within [a,b] { e }` | timing restriction on `e` | [Expressions](./syntax/expressions.md) |
| `e % (> c)` | postfix timing restriction | [Expressions](./syntax/expressions.md) |
| `ignore event { e }` | ignore selected events while matching `e` | [Expressions](./syntax/expressions.md) |

Constraints and updates
-----------------------

| Construct | Meaning | Details |
| --- | --- | --- |
| `x == y` | string equality | [Constraints](./syntax/constraints.md) |
| `x != y` | string disequality | [Constraints](./syntax/constraints.md) |
| `n = m` | numeric equality | [Constraints](./syntax/constraints.md) |
| `n <> m` | numeric disequality | [Constraints](./syntax/constraints.md) |
| `n < m`, `n <= m`, `n >= m`, `n > m` | numeric comparisons | [Constraints](./syntax/constraints.md) |
| `g1 && g2` | guard conjunction | [Constraints](./syntax/constraints.md) |
| `x := expr` | update a declared variable | [Constraints](./syntax/constraints.md) |

Common mistakes
---------------

If a specification parses differently than expected, check
[Common mistakes](./syntax/common-mistakes.md). The most frequent issues are
using the wrong equality operator for strings or numbers, forgetting the final
expression, and writing `one_of { ... } and { ... }` instead of
`one_of { ... } or { ... }`.

Complete examples
-----------------

See [Syntax examples](./syntax/examples.md) for complete small `.symon`
specifications covering guards, updates, timing restrictions, parametric timing,
and initial constraints.

Reference
---------

- [ACM02] Timed regular expressions. Eugene Asarin, Paul Caspi, and Oded Maler, Journal of the ACM, Volume 49 Issue 2, March 2002, Pages 172-206
