SyMon --- SYmbolic MONitor
==========================

[![Boost.Test](https://github.com/MasWag/SyMon/actions/workflows/boosttest.yml/badge.svg?branch=master)](https://github.com/MasWag/SyMon/actions/workflows/boosttest.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](./LICENSE)

This is the source code repository for SyMon --- A tool for symbolic monitoring

Demo on Google Colab is [HERE](https://colab.research.google.com/drive/17WNWuA3RxCA51xkDuVfOVeuUbRqHetDz)!!

Usage
-----

### Synopsis

    symon [OPTIONS] -pf [AUTOMATON_FILE] -s [SIGNATURE_FILE] (-i [TIMEDWORD_FILE])

### Options

**-h**, **--help** Print a help message. <br />
**-i** *file*, **--input** *file* Read a timed word from *file*. <br />
**-f** *file*, **--automaton** *file* Read a timed automaton from *file*. <br />
**-s** *file*, **-signature** *pattern* Read a signature from *file*. <br />
**-b**, **-boolean** non-parametric and Boolean mode (default). <br />
**-d**, **-dataparametric** data-parametric mode. <br />
**-p**, **-parametric** fully parametric mode. <br />
**--enable-string-merging** Enable merging of symbolic string valuations. This is supported only in the fully parametric mode. <br />

Example
-------
    
    ./build/symon -f ./example/copy/copy.dot -s ./example/copy/copy.sig < ./example/copy/copy.txt
    ./build/symon -df ./example/copy/copy_data_parametric.dot -s ./example/copy/copy.sig < ./example/copy/copy.txt
    ./build/symon -pf ./example/copy/copy_parametric.dot -s ./example/copy/copy.sig < ./example/copy/copy.txt
    ./build/symon -pf ./example/copy/copy_tpm.dot -s ./example/copy/copy.sig < ./example/copy/copy.txt

The examples used in our CAV 2019 paper is [here](example/cav2019/README.md).

Installation
------------

SyMon is tested on macOS 10.14.4

### Requirements

* C++ compiler supporting C++17 and the corresponding libraries.
* Boost (>= 1.67.0)
* CMake
* Parma Polyhedra Library

### Instructions

```sh
mkdir build 
cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make
```

Limitation in Boolean mode
--------------------------

- The updates of a variable is only `xN := xY`
    - We do not allow assignment of literal or expression.
- The constraint of number expression is only `<expression> <op> <constaint>`
    - This is not the essential problem. Just for simplicity of the implementation.
- In a constraint `s0 <op> s1` of strings, we assume that we know the value of either `s0` or `s1` (i.e., one of them must be constant).
- In a constraint `n0` or `n0 <op> n1` of numbers, we assume that we know the value of both `n0` and `n1` (i.e., both of them must be constant).
    - It is possible to have an unknown variable only if it is not used in the constraint.

Specifications
--------------

- The unobservable action is available only in the fully parametric mode (with **-p**).
- The unobservable action is assigned to id `127`. This will be modified in a future version.

The file format of the signature
--------------------------------

The format of the signature file is as follows.

```
<key0>	<StringSize>	<NumberSize>
<key1>	<StringSize>	<NumberSize>
...
```

For example, the following signature shows that:

- the predicate 0 is open, which takes one string argument; and
- the predicate 1 is put, which takes one string argument and one number argument.

```
open	1	0
put	1	1
```

How to make compile_commands.json
---------------------------------

``` shell
mkdir build
cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
```

References
----------

- [Masaki Waga](http://group-mmm.org/~mwaga/), [Étienne André](https://lipn.univ-paris13.fr/~andre/), and [Ichiro Hasuo](http://group-mmm.org/~ichiro/), **Symbolic Monitoring against Specifications Parametric in Time and Data**, In *Proc. [CAV 2019](http://i-cav.org/2019/)*. [LNCS 11561, pp. 520-539](https://link.springer.com/chapter/10.1007/978-3-030-25540-4_30) [arXiv version](https://arxiv.org/abs/1905.04486)
