Execution modes
===============

SyMon can run the same `.symon` syntax with different monitoring modes. The
mode controls how data values and timing constraints are interpreted. It is not
part of the grammar itself.

Using `.symon` files
--------------------

Use `-n` when the input specification uses the `.symon` syntax documented in
[Syntax](./syntax.md):

```
symon -nf spec.symon
```

Mode flags can be combined with `-n` and `-f`:

| Mode | Command form | Meaning |
| --- | --- | --- |
| Boolean | `symon -bnf spec.symon` or `symon -nf spec.symon` | non-parametric monitoring with concrete data and concrete timing bounds |
| Data-parametric | `symon -dnf spec.symon` | symbolic monitoring for data values, with concrete timing bounds |
| Parametric timing | `symon -pnf spec.symon` | symbolic monitoring for data values and timing parameters |

At most one of `-b`, `-d`, and `-p` can be used. If none is given, SyMon uses
boolean mode.

Boolean mode
------------

Boolean mode uses concrete string and number values. Timing bounds must be
concrete numeric bounds. Use this mode for specifications that do not need
symbolic data values or unknown timing constants.

Matches are reported with the timestamp, time-point, and concrete valuations for
variables when relevant.

Data-parametric mode
--------------------

Data-parametric mode keeps string and number variables symbolic when their
values are not fixed by the input. It is useful for specifications such as
"some sender appears more than ten times" where the sender is not known in
advance.

Timing bounds are still concrete in this mode. Declarations such as
`period: param;` should be reserved for parametric timing mode.

Parametric timing mode
----------------------

Parametric timing mode keeps both data values and timing parameters symbolic.
Declare timing parameters in `var` using `param`:

```
var {
    period: param;
}
```

Then use them in timing constraints:

```
within (<= period) {
    arrival(sender)
}
```

Matches include symbolic data valuations and timing constraints. SyMon prints
numeric data constraints after `Num:` and timing-parameter constraints after
`Clock:`.

Initial constraints
-------------------

The `init` block is syntactically valid in `.symon` files, but SyMon only
supports it in parametric timing mode:

```
var {
    threshold: number;
    period: param;
}

init {
    threshold > 0
}
```

Use `init` for initial constraints on data variables. Use timing parameters in
timing bounds such as `within (<= period) { ... }`; do not use `init` to
constrain timing parameters.

Unobservable actions
--------------------

`unobservable` is special-cased by SyMon. Do not declare it as a normal
`signature`. It can be used as an internal transition, for example when an
expression needs to wait without consuming a visible event:

```
var {
    saved: number;
}

within (= 2) {
    unobservable( | | saved := saved )
}
```

Developer note: internally, SyMon represents unobservable transitions with
action id `127`.

See also [Syntax examples](./syntax/examples.md) for complete small
specifications.
