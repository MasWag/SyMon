SyMon's specification language
==============================

In SyMon, a specification language based on timed regular expressions (TREs) [ACM02] can be used. The following shows some of the syntax of SyMon's specification language.

```
expr : atomic
     | expr_variable
     | expr && expr
     | all_of { expr } and { expr }
     | expr || expr
     | one_of { expr } and { expr }
     | expr ; expr
     | expr *
     | zero_or_more { expr }
     | expr +
     | one_or_more { expr }
     | expr ?
     | optional { expr }
     | expr % timing_constraint
     | within timing_constraint { expr }
     | ( expr )

atomic : identifier ( args | guard | updates )
       | identifier ( args | guard )
       | identifier ( args )

guard : string_or_variable == string_or_variable
      | string_or_variable != string_or_variable
      | number_or_variable > number_or_variable
      | number_or_variable >= number_or_variable
      | number_or_variable <> number_or_variable
      | number_or_variable <= number_or_variable
      | number_or_variable < number_or_variable
      | guard && guard
```

Reference
---------

- [ACM02] Timed regular expressions. Eugene Asarin, Paul Caspi, and Oded Maler, Journal of the ACM, Volume 49 Issue 2, March 2002, Pages 172-206
