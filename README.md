CCS for C++
===========

This is a C++ implementation of [CCS][1]. 

There's a presentation about the language [here][2], but it's a little sketchy
without someone talking along with it.

[1]: http://github.com/hellige/ccs
[2]: http://hellige.github.io/ccs

Syntax quick reference
----------------------

#### @context (a.b c, d)

Sets "context" for the rest of the ruleset. Must appear prior to any other
rules. Equivalent to:

    a.b c, d { /* rest of file */ }

#### name = 123

Property definition. Property _name_ will have the specified value in the
current context. The syntax of values is conventional: _true_, _false_,
64-bit signed integers (may be specified in hex as _0x1a2bc_), doubles,
and strings.

#### 'name with spaces' = 'value'

_name_ may be enclosed in quotes and written as a string literal if it
contains spaces or other non-identifier characters.

#### @override name = 123

Overriding property definition. For a given property, any definition marked
_@override_ will take precedence over any normal definition, regardless of
the relative specificities of the two definitions. If more than one
_@override_ definition applies in a particular context, the most specific wins.

#### 'VAR: ${VAR}' <br> "literal: \${VAR}"

Strings may be enclosed in single or double quotes. Environment variables may
be interpolated with ```${VAR}```. Special characters may be escaped with a
backslash. Recognized escape sequences include: ```\t \n \r \' \" \\ \$```.
A string may be broken across multiple lines by ending a line with a single 
backslash.

#### 'double: ", single: \'' <br> "double: \", single: '"

The non-delimiting quote character need not be escaped.

#### @import "..." <br> a b.c d : @import "..."

Import a ruleset into the current ruleset. The entire imported ruleset will be
nested in the current context, so for example the second example above further
constrains all imported settings to the context ```a b.c d```.

#### @constrain a/b.c/d.e.f

Further constrain the context. This is equivalent to further constraining the
context using the ```context.constrain(...)``` API call.

Of course, this is most useful when applied only in a particular selected
context, in which case it allows for some additional reuse and
modularity within CCS rulesets (activating additional sets of rules in
particular cases, for example). 

#### _selector_ : @import "..." <br> _selector_ : @constrain _..._ <br> _selector_ : _name_ = _value_ <br> _selector_ { _rules_ }

Constrain rules to apply only in the selected context. _rule_ is any of:
an import, a property setting, a constraint, or a selector and further nested
rules. Selector syntax is documented below...

#### a.b.c

Matches a constraint of type ```a``` with values ```b``` and ```c```. Note
that this matches only a single constraint with _both_ values. Multiple
constraints of the same type are allowed (thus preserving monotonicity), so
```a.b.c``` is not equivalent to ```a.b a.c``` in every case.

### a/c

_Rarely needed._

Matches a simultaneous occurence of constraints of type ```a``` and type
```c```. Again, this is not generally equivalent to ```a c```, as it only
matches when the two constraints are applied in the same single step.

#### a.b c.d <br> a.b, c.d <br> a.b > c.d

Conjunction, disjunction, and descendant selection, respectively. The first
form matches in any context containing _both_ ```a.b``` and ```c.d```.
The second matches in any context containing _either one_.

The third form matches in any context containing ```c.d```, _itself_ in a
context containing ```a.b```. This form is infrequently used in CCS (although it
is of course the default operator in CSS).

Precedence of operators is as follows, from highest to lowest: 

  - ```>``` (descendant)
  - juxtaposition (conjunction)
  -  ```,``` (disjunction)

Parentheses may be used to enforce grouping. So, for example, ```a, c > d e```
is equivalent to ```a, ((c > d) e)```. Since ```>``` is infrequently used,
the rules are generally simple and intuitive.


Note that rules may be separated by semicolons for clarity, but this is always
completely optional and will never affect the way the ruleset is parsed.
    

TODO
----

* Aggregate values: lists, maps.
* For aggregate values, allow modification of inherited value as well as
  replacement??
