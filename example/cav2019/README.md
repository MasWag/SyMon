Experiments Data for CAV 2019
=============================

This is a document on the experiment data for the CAV 2019 paper [WAH].
We implemented and evaluated our tool SyMon. We compiled SyMon using GCC 7.3.0. See also [READMD.md](../README.md).

Experimental raw data
---------------------

We conducted the experiments on an Amazon EC2 c4.large instance (2.9 GHz Intel Xeon E5-2666 v3, 2 vCPUs, and 3.75 GiB RAM) that runs Ubuntu 18.04 LTS (64 bit).
We compiled PPL [BHZ08] with the following options.

> --disable-dependency-tracking --enable-coefficients=checked-int64 --disable-silent-rules --enable-optimization=speed

The DOT and signature files are in this directory. The Evaluation data is available from the following link.

- [Evaluation data for "Symbolic Monitoring against Specifications Parametric in Time and Data"](https://www.researchgate.net/publication/336086078_Evaluation_data_for_Symbolic_Monitoring_against_Specifications_Parametric_in_Time_and_Data)

Description of the Benchmarks
-----------------------------

### Copy

Benchmark Copy is inspired by the monitoring of variable updates much like the scenario in [BDSV14]. The action is `update` and `update` has one string and one integer values. The string value is the name of the updated variable and the integer value is the updated value.

### Dominant

Benchmark Dominant is inspired by the monitoring of withdrawals from bank accounts of various users much like the scenario in [BKZ17]. The action is `withdraw` and `withdraw` has one string and one integer values. The string value is the user name and the integer value is the amount of the withdrawals.

### Periodic

Benchmark Periodic is inspired by a parameter identification of periodic withdrawals from one bank account. The action is `withdraw` and `withdraw` has one integer value. The the integer value is the amount of the withdrawals.


Calling SyMon
-------------

For all case studies, the following command was used for PTPM:

>  ./build/symon -pf [ptda.dot] < [timed_data_word.txt]

References
----------

<dl>
<dt>[WAH]</dt>
<dd>Masaki Waga, Étienne André, and Ichiro Hasuo, Symbolic Monitoring against Specifications Parametric in Time and Data, To appear in Proc. CAV 2019.</dd>
<dt>[BDSV14]</dt>
<dd>Brim, L., Dluhoš, P., Šafránek, D., & Vejpustek, T. (2014). STL⁎: Extending signal temporal logic with signal-value freezing operator. Information and Computation, 236, 52-67.</dd>
<dt>[BKZ17]</dt>
<dd>Basin, D. A., Klaedtke, F., & Zalinescu, E. (2017, December). The MonPoly Monitoring Tool. In RV-CuBES (pp. 19-28).</dd>
</dl>
