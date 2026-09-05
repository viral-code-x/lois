# # LOIS
# LANGUAGE OPERATED on IS 

**LOIS** is a simple, human-readable programming language designed to make programming and mathematics explicit and easy to understand. 
This language is made for beginners and its tries to keep stuff similiar to most other language so it doesnt make them too much comfortable without rules. 
it has rules  

LOIS uses natural looking syntax such as:

```lois
name is "Shafakat"
age is 15

if age >= 13
then output is "Teenager"
```

---

# Table of Contents

* [Basic Syntax](#basic-syntax)
* [Comments](#comments)
* [Output](#output)
* [Variables](#variables)
* [Numbers](#numbers)
* [Strings](#strings)
* [Booleans](#booleans)
* [Arithmetic](#arithmetic)
* [Comparisons](#comparisons)
* [Logical Operators](#logical-operators)
* [Parentheses and Precedence](#parentheses-and-precedence)
* [Negative Numbers](#negative-numbers)
* [Mathematical Functions](#mathematical-functions)
* [The pi Constant](#the-pi-constant)
* [Sets](#sets)
* [Set Indexing](#set-indexing)
* [Input](#input)
* [If / Then / Else](#if--then--else)
* [But If](#but-if)
* [While Loops](#while-loops)
* [For Loops](#for-loops)
* [Repeat](#repeat)
* [Functions](#functions)
* [Function Calls](#function-calls)
* [Return](#return)
* [Errors](#errors)
* [Reserved Words](#reserved-words)
* [Complete Example](#complete-example)

---

# Basic Syntax

LOIS programs are written as a sequence of statements.

A newline normally separates statements.

```lois
name is "Afnan"
age is 15

output is name
output is age
```

Output:

```text
Afnan
15
```

Spaces and tabs are used as separators outside strings.

---

# Comments

## Single-line comments

Use `note:`.

```lois
note: this line is ignored

output is "Hello"
```

`note:` is case-insensitive:

```lois
NOTE: ignored
Note: also ignored
```

Everything after `note:` on that line is ignored.

---

## Multi-line comments

Use square brackets:

```lois
[
This entire section
is ignored by LOIS.
You can write multiple
lines here.
]

output is "Hello"
```

The closing `]` ends the comment.

---

# Output

LOIS has two main forms of output:

```lois
output is ...
```

and

```lois
output = ...
```

They are not the same thing.

---

## `output is`

`output is` is mainly for **printing something directly** or printing something that is already stored in a variable.

### Direct text

Anything inside double quotes (`" "`) on the same line is printed **straight as it is**.

```lois
output is "Hello World"
```

Output:

```text
Hello World
```

Another example:

```lois
output is "The moon is bright tonight"
```

Output:

```text
The moon is bright tonight
```

LOIS does not try to read the stuff inside the quotes as code.

So:

```lois
output is "1 + 2"
```

prints:

```text
1 + 2
```

It does not calculate it.

Basically, if you put something inside `" "`, LOIS just prints what's inside.

---

## Printing a number directly

You can also directly print a number:

```lois
output is 12
```

Output:

```text
12
```

This is treated as direct output.

So if you want LOIS to actually **calculate** something in the output, use `output =`.

For example:

```lois
output is 1 + 2
```

is not the same as:

```lois
output = 1 + 2
```

The second one tells LOIS to evaluate the arithmetic.

---

# `output =`

`output =` is used when you want LOIS to **calculate an expression and then print the result**.

For example:

```lois
output = 1 + 2
```

Output:

```text
3
```

You don't need to create a variable just to do a calculation.

```lois
output = 10 * 5
```

Output:

```text
50
```

Parentheses work too:

```lois
output = (10 + 5) * 2
```

Output:

```text
30
```

So the simple rule is:

```lois
output is ...
```

→ print it directly

```lois
output = ...
```

→ calculate it first, then print it

---

# Numeric Variables

LOIS currently has two main data types:

* `num` — numbers
* `string` — text

There isn't a separate integer type and decimal/float type.

Numbers are just `num`.

---

## Creating a numeric variable

Use `=` to assign a number:

```lois
x = 8
```

Now `x` contains the number `8`.

You can print it:

```lois
output is x
```

Output:

```text
8
```

---

## Using a numeric variable

Once a variable contains a number, you can use it in calculations.

```lois
x = 8

output = x * 5
```

Output:

```text
40
```

You can also do:

```lois
x = 8

output = x + 2
output = x - 2
output = x * 2
output = x / 2
output = x ^ 2
```

The value of `x` stays `8` unless you change it.

---

## Using multiple variables

You can use multiple numeric variables in one expression:

```lois
x = 8
y = 5

output = x * y + 10
```

Output:

```text
50
```

LOIS calculates the whole expression first and then prints the answer.

---

# Direct Arithmetic vs `output is`

This is one of the important things to understand about LOIS.

If you write:

```lois
output is 12
```

LOIS just outputs:

```text
12
```

But if you write:

```lois
output = 1 + 2
```

LOIS calculates:

```text
1 + 2 = 3
```

and outputs:

```text
3
```

So when doing arithmetic directly inside an output statement, use:

```lois
output = expression
```

---

# Decimals

LOIS only has one numeric type:

```text
num
```

There is no separate `int` and `float`.

A number can still have decimal values.

For example:

```lois
x = 3
```

and:

```lois
output = 3 / 5 upto 3
```

are both using the same `num` system.

---

## `upto`

When you want to control how many decimal places are shown, use:

```lois
output = expression upto number
```

The number after `upto` tells LOIS how many decimal places to keep.

For example:

```lois
x = 3

output = x / 5 upto 3
```

Output:

```text
0.600
```

You don't have to use a variable either:

```lois
output = 3 / 5 upto 3
```

Output:

```text
0.600
```

Another example:

```lois
output = 10 / 3 upto 3
```

Output:

```text
3.333
```

And:

```lois
output = 22 / 7 upto 5
```

Output:

```text
3.14286
```

Basically, `upto` is there when you want to control the decimal output instead of letting LOIS use its normal formatting.

---

## Rounding without `upto`

If you don't use `upto`, LOIS follows its normal rounding rule when a decimal is being reduced to an integer for output.

If the first decimal digit is **5 or greater**, it adds `1` to the integer.

If it is **less than 5**, it ignores the decimal part.

For example:

```text
3.6 → 4
3.4 → 3

7.5 → 8
7.2 → 7
```

So:

```text
3.6
```

becomes:

```text
4
```

while:

```text
3.4
```

becomes:

```text
3
```

If you actually want decimal places instead of this normal rounding behavior, use `upto`.

---

# Strings

Strings are text written inside double quotes:

```lois
message is Hello there
```

The contents are stored as text.

For example:

```lois
message is 1 + 2

output is message
```

Output:

```text
1 + 2
```

LOIS doesn't calculate `1 + 2` here because it is stored as a string.

The quotes tell LOIS:


## note: so you dont put quotes "....." while assigning a variable thats an error 
anything in the same line after the variable assignment is will make it the string stored inside doesnt matter the spaces or anything just not other assignable stuff like function 
 

> this is text, don't treat it as code.

---

# `is` vs `=`

`is` and `=` have different jobs in LOIS.

### `is`

`is` is used when assigning things such as strings:

```lois
message is "Hello"
```

### `=`

`=` is used for numeric values and numeric expressions:

```lois
x = 8
y = x * 5
```

So:

```lois
x = 8

output is x
```

prints:

```text
8
```

And:

```lois
x = 8

output = x * 5
```

prints:

```text
40
```

The important part is that `output is` can print the value already stored in `x`, while `output =` tells LOIS to evaluate an expression.

---

# Undefined Variables

A variable needs to exist before LOIS can use it.

For example:

```lois
output = x * 5
```

will give an error if `x` was never created.

The same applies to:

```lois
output is x
```

If `x` doesn't exist, LOIS can't print its value.

For example, this:

```lois
name is "Alice"

output is age
```

will fail because `age` was never defined.

---

# Quick Reference

| Syntax                  | What it does                          |
| ----------------------- | ------------------------------------- |
| `output is "Hello"`     | Prints the text directly              |
| `output is 12`          | Prints `12` directly                  |
| `output is x`           | Prints the value stored in `x`        |
| `output = 1 + 2`        | Calculates and prints `3`             |
| `output = x * 5`        | Calculates using `x`                  |
| `x = 8`                 | Creates/assigns a numeric variable    |
| `message is Hello`      | Stores text                           |
| `output = 3 / 5 upto 3` | Calculates and keeps 3 decimal places |

The easiest way to remember it:

```lois
output is ...
```

**"Just output this."**

and:

```lois
output = ...
```

**"Calculate this and output the result."**


# Numeric Assignment

`=` is supported as a **numeric assignment**.

```lois
x = 10 + 5
```

This means that `x` must receive a numeric result.

```lois
x = 5 * 2
output is x
```

Output:

```text
10
```

---

## Numeric declaration

You can explicitly declare a numeric variable:

```lois
x is num
```

This creates `x` with the numeric value `0`.

```lois
x is num
output is x
```

Output:

```text
0
```

A numeric variable can then receive numeric input or numeric assignments.

---

## Numeric assignment errors

This is invalid:

```lois
x = "hello"
```

because `=` represents numeric assignment.

LOIS reports that a number was expected.

---

# Numbers

LOIS uses numeric values internally as floating-point numbers.

Examples:

```lois
x is 10
y is 3.14
z is 0.5
```

Numbers can be integers or decimals.

```lois
output is 10
output is 3.14
output is .5
```

---

# Strings

Strings are enclosed in double quotes:

```lois
name is "Viral"
message is "Hello World"
```

Strings can contain spaces:

```lois
message is "Hello, world!"
```

Strings can span multiple lines:

```lois
message is "Hello
World"
```

The newline inside the quotes is preserved.

---

## Unterminated strings

This is invalid:

```lois
message is "Hello
```

because the closing `"` is missing.

LOIS reports an unterminated-string error.

---

# Booleans

LOIS has two boolean literals:

```lois
True
False
```

They are **case-sensitive as boolean literals**.

```lois
x is True
y is False
```

Output:

```lois
output is x
output is y
```

produces:

```text
True
False
```

---

## `true` and `false`

Lowercase forms are not the boolean literals:

```lois
true
false
```

They are treated as ordinary words/variable names.

Therefore:

```lois
x is true
```

attempts to use a variable named `true`.

If it has not been defined, this results in an undefined-variable error.

---

# Arithmetic

LOIS supports:

| Operator | Meaning        | Example |
| -------- | -------------- | ------- |
| `+`      | Addition       | `5 + 2` |
| `-`      | Subtraction    | `5 - 2` |
| `*`      | Multiplication | `5 * 2` |
| `/`      | Division       | `5 / 2` |
| `%`      | Modulo         | `5 % 2` |
| `^`      | Power          | `5 ^ 2` |

Examples:

```lois
output is 5 + 2
output is 5 - 2
output is 5 * 2
output is 10 / 2
output is 10 % 3
output is 2 ^ 3
```

Output:

```text
7
3
10
5
1
8
```

---

## Division by zero

This causes a runtime error:

```lois
output is 10 / 0
```

Error:

```text
LOIS: division by zero
```

---

## Modulo by zero

This also causes an error:

```lois
output is 10 % 0
```

Error:

```text
LOIS: modulo by zero
```

---

# String + Number

`+` can also combine strings with other values.

```lois
name is "Viral"
age is 15

output is name + age
```

The resulting output places a space between the values.

---

# Comparisons

LOIS supports:

| Operator | Meaning               |
| -------- | --------------------- |
| `>`      | greater than          |
| `<`      | less than             |
| `>=`     | greater than or equal |
| `<=`     | less than or equal    |
| `==`     | equal                 |
| `!=`     | not equal             |

Examples:

```lois
output is 10 > 5
output is 10 < 5
output is 10 >= 10
output is 10 <= 10
output is 10 == 10
output is 10 != 5
```

Comparison results are booleans.

---

## Comparing numbers

```lois
age is 20

if age >= 18
then output is "Adult"
```

---

## Equality

Numbers can be compared:

```lois
x is 10
y is 10

output is x == y
```

Strings can also be compared:

```lois
a is "hello"
b is "hello"

output is a == b
```

---

## Different types

Equality requires compatible types.

For example, comparing a number and a boolean causes a type error.

```lois
output is 5 == True
```

---

## `<`, `>`, `<=`, `>=` require numbers

This is invalid:

```lois
output is "hello" > "world"
```

These comparison operators require numeric operands.

---

# Logical Operators

LOIS uses words instead of symbols for logical operations.

## `not`

`not` reverses truth.

```lois
output is not True
```

Output:

```text
False
```

```lois
output is not False
```

Output:

```text
True
```

---

## `and`

Both sides must be truthy.

```lois
x is True
y is False

output is x and y
```

Output:

```text
False
```

---

## `or`

At least one side must be truthy.

```lois
x is True
y is False

output is x or y
```

Output:

```text
True
```

---

## Logical precedence

`and` has higher precedence than `or`.

Therefore:

```lois
True or False and False
```

is interpreted as:

```text
True or (False and False)
```

not:

```text
(True or False) and False
```

---

## Short-circuiting

LOIS short-circuits logical expressions.

For `and`:

```text
False and ...
```

does not need to evaluate the right side.

For `or`:

```text
True or ...
```

does not need to evaluate the right side.

---

# Parentheses and Precedence

Parentheses can be used to explicitly control the order in which LOIS evaluates an expression.

```lois
output = (2 + 3) * 4
```

Output:

```text
20
```

Without parentheses:

```lois
output = 2 + 3 * 4
```

Output:

```text
14
```

This happens because multiplication has higher precedence than addition.

---

## Precedence order

From highest to lowest:

1. Parentheses
2. Unary operations such as `not` and unary `-`
3. `^`
4. `*`, `/`, `%`
5. `+`, `-`
6. Comparisons
7. `and`
8. `or`

Power is **right-associative**.

For example:

```lois
output = 2 ^ 3 ^ 2
```

is interpreted as:

```text
2 ^ (3 ^ 2)
```

and therefore produces:

```text
512
```

not:

```text
64
```

---

# Negative Numbers

Unary `-` is supported for numeric values.

```lois
x = -5

output = x
```

Output:

```text
-5
```

You can also use unary `-` directly in an expression:

```lois
output = -10
```

Or with a variable:

```lois
x = 10

output = -x
```

Output:

```text
-10
```

---

# Mathematical Functions

LOIS provides built-in mathematical functions for common calculations.

| Function   | Meaning                  |   |                |
| ---------- | ------------------------ | - | -------------- |
| `root(x)`  | Square root              |   |                |
| `root3(x)` | Cube root                |   |                |
| `root4(x)` | Fourth root              |   |                |
| `rootn(x)` | nth root                 |   |                |
| `sin(x)`   | Sine                     |   |                |
| `cos(x)`   | Cosine                   |   |                |
| `tan(x)`   | Tangent                  |   |                |
| `log(x)`   | Base-10 logarithm        |   |                |
| `ln(x)`    | Natural logarithm        |   |                |
| `abs(x)`   | Absolute value           |   |                |
| `round(x)` | Round to nearest integer |   |                |
| `floor(x)` | Round down               |   |                |
| `ceil(x)`  | Round up                 |   |                |
| `          | -x                       | ` | Absolute value |
| `x!`       | Factorial                |   |                |

All of these return numeric values.

---

## Square Root

`root()` calculates the square root.

```lois
output = root(25)
```

Output:

```text
5
```

You can also use a variable:

```lois
x = 25

output = root(x)
```

Output:

```text
5
```

---

## Cube Root

`root3()` calculates the cube root.

```lois
output = root3(27)
```

Output:

```text
3
```

---

## Fourth Root

`root4()` calculates the fourth root.

```lois
output = root4(16)
```

Output:

```text
2
```

---

## Any Root

`rootn()` can be used when you want a root other than the built-in square, cube, or fourth root.

The syntax is:

```lois
rootn(number)
```

For example:

```lois
output = root5(32)
```

Output:

```text
2
```

because:

```text
2⁵ = 32
```

You can use any numeric root value:

```lois
output = root4(625)
```

Output:

```text
5
```

This allows you to calculate things such as fifth roots, tenth roots, hundredth roots, and so on.

---

# Trigonometry

LOIS provides:

```lois
sin(x)
cos(x)
tan(x)
```

Trigonometric functions use **radians**.

For example:

```lois
output = sin(0)
output = cos(0)
output = tan(0)
```

Output:

```text
0
1
0
```

You can also use `pi`:

```lois
output = sin(pi / 2)
```

which gives approximately:

```text
1
```

---

# Logarithms

`log()` calculates the base-10 logarithm.

```lois
output = log(100)
```

Output:

```text
2
```

`ln()` calculates the natural logarithm.

```lois
output = ln(1)
```

Output:

```text
0
```

---

# Absolute Value

`abs()` returns the absolute value of a number.

```lois
output = abs(-25)
```

Output:

```text
25
```

LOIS also supports absolute value using `| |`:

```lois
output = |-25|
```

Output:

```text
25
```

The two forms can be used for the same purpose.

---

# Rounding

LOIS provides three rounding functions.

### `round()`

Rounds to the nearest integer.

```lois
output = round(3.6)
```

Output:

```text
4
```

### `floor()`

Rounds down.

```lois
output = floor(3.9)
```

Output:

```text
3
```

### `ceil()`

Rounds up.

```lois
output = ceil(3.1)
```

Output:

```text
4
```

---

# Factorial

Factorial is written using `!`.

```lois
5!
```

means:

```text
5 × 4 × 3 × 2 × 1
```

which equals:

```text
120
```

So you can write:

```lois
output = 5!
```

Output:

```text
120
```

Factorial can also be used with a numeric variable:

```lois
x = 6

output = x!
```

Output:

```text
720
```

It can also be part of a larger expression:

```lois
output = 5! + 3!
```

Output:

```text
126
```

because:

```text
5! = 120
3! = 6
120 + 6 = 126
```

Factorial can be applied to an expression as well:

```lois
output = (3 + 2)!
```

Output:

```text
120
```

because `(3 + 2)` is calculated first.

---

## Factorial rules

Factorial is defined for non-negative whole numbers.

Valid:

```lois
output = 0!
output = 1!
output = 5!
```

`0!` is:

```text
1
```

Factorial cannot be used with a negative number:

```lois
output = (-5)!
```

This produces an error.

It also cannot be used with a non-whole number:

```lois
output = 3.5!
```

This produces an error because factorial is not defined for a non-integer value in LOIS.

---

# Mathematical Function Errors

Mathematical functions require numeric arguments.

For example, this is invalid:

```lois
output = root("hello")
```

because `"hello"` is a string, not a number.

LOIS reports an error similar to:

```text
math function 'root' requires a number
```

---

## Invalid square and fourth roots

`root()` cannot receive a negative number.

```lois
output = root(-1)
```

This produces an error because a real square root of `-1` is not available in LOIS.

`root4()` follows the same restriction:

```lois
output = root4(-16)
```

produces an error.

---

## Invalid logarithms

`log()` and `ln()` require positive numbers.

For example:

```lois
output = log(0)
```

and:

```lois
output = ln(-5)
```

produce errors.

---

## Undefined tangent

`tan()` detects values where tangent is undefined.

For example, tangent is undefined at:

```text
π/2
```

and equivalent angles.

Therefore:

```lois
output = tan(pi / 2)
```

produces a mathematical error rather than an invalid numeric result.

---

## Invalid factorial

Factorial requires a non-negative whole number.

For example:

```lois
output = (-5)!
```

is invalid.

So is:

```lois
output = 3.5!
```

Both produce an error because factorial only accepts non-negative whole numbers.

---

## Unknown functions

Using a function that LOIS does not know produces an error.

For example:

```lois
output = banana(5)
```

will produce an unknown-function error.

---

# The `pi` Constant

LOIS provides `pi` as a built-in mathematical constant.

```lois
output = pi
```

You can use `pi` anywhere a numeric value can be used.

For example:

```lois
radius = 5

output = pi * radius ^ 2
```

This calculates the area of a circle.

You can also use it with trigonometric functions:

```lois
output = sin(pi / 2)
```

`pi` is a numeric constant, not a string or a variable that needs to be created first.


# Sets

LOIS supports set-like collections using `{}`.

```lois
numbers is {1, 2, 3, 4}
```

Output:

```lois
output is numbers
```

produces:

```text
{1,2,3,4}
```

---

## Set elements can be expressions

```lois
numbers is {1 + 1, 2 * 3, 10 / 2}

output is numbers
```

---

## Empty sets

An empty set is valid:

```lois
numbers is {}
```

---

## Set syntax

Elements are separated with commas:

```lois
numbers is {10, 20, 30}
```

This is invalid:

```lois
numbers is {10 20 30}
```

A comma is required between elements.

---

# Set Indexing

LOIS uses **1-based indexing**.

For:

```lois
numbers is {10, 20, 30}
```

the elements are:

```text
numbers1 -> 10
numbers2 -> 20
numbers3 -> 30
```

Example:

```lois
numbers is {10, 20, 30}

output is numbers1
output is numbers2
output is numbers3
```

Output:

```text
10
20
30
```

The index starts at **1**, not 0.

---

## Out-of-range indexing

```lois
numbers is {10, 20, 30}

output is numbers4
```

produces:

```text
LOIS: set index 4 out of range
```

---
# Input

LOIS has two forms of input:

```lois
input is ...
```

and:

```lois
input = ...
```

They are used for different types of input.

---

## String Input

Use `input is` when you want to take **text/string input**.

```lois
input is name
output is name
```

LOIS will prompt for input:

```text
name:
```

If the user enters:

```text
Alice
```

then `name` stores:

```text
Alice
```

and the output is:

```text
Alice
```

No type declaration is needed.

You can also use a different variable name:

```lois
input is message
output is message
```

Whatever the user enters is stored as a string.

---

## Numeric Input

Use `input =` when you want the user to enter a **number**.

```lois
input = age
output is age
```

LOIS will prompt for:

```text
age:
```

If the user enters:

```text
15
```

then `age` stores the numeric value:

```text
15
```

That means you can immediately use it in calculations:

```lois
input = number

output = number * 5
```

If the user enters:

```text
8
```

the output is:

```text
40
```

---

## The Difference

The main difference is the operator:

```lois
input is variable
```

→ expects **string/text input**

```lois
input = variable
```

→ expects **numeric input**

For example:

```lois
input is name
```

can accept:

```text
John Smith
```

while:

```lois
input = age
```

expects something like:

```text
17
```

---

## Invalid Numeric Input

If `input =` is used and the user enters something that is not a valid number, LOIS reports an error.

For example:

```lois
input = age
```

and the user enters:

```text
hello
```

This is invalid because `input =` expects a number.

---

## Using Input in Expressions

Numeric input can be used directly in calculations after it has been stored.

```lois
input = x

output = x ^ 2
```

If the user enters:

```text
5
```

the output is:

```text
25
```

String input can also be printed directly:

```lois
input is name

output is name
```

The value entered by the user is stored in the variable and can then be used by the rest of the program.

---

# If /But if / Else/ Then

`if` is used to run a statement only when a condition is true.

The basic syntax is:

```lois
if condition
then statement
```

For example:

```lois
age = 18

if age >= 18
then output is "Adult"
```

Output:

```text
Adult
```

The condition:

```lois
age >= 18
```

is evaluated first. If it is true, the statement after `then` runs.

---

## Else

`else` runs when the `if` condition is false.

```lois
age = 15

if age >= 18
then output is "Adult"
else output is "Not adult"
```

Output:

```text
Not adult
```

Only one of the two branches executes.

---

## `then` is required

Every `if` statement needs `then` before its statement.

This is invalid:

```lois
if age >= 18
output is "Adult"
```

LOIS expects:

```lois
if age >= 18
then output is "Adult"
```

---

## Conditions

Conditions use the normal comparison operators:

| Operator | Meaning                  |
| -------- | ------------------------ |
| `<`      | Less than                |
| `>`      | Greater than             |
| `<=`     | Less than or equal to    |
| `>=`     | Greater than or equal to |
| `==`     | Equal to                 |
| `!=`     | Not equal to             |

For example:

```lois
x = 10

if x > 5
then output is "Greater"
```

Or:

```lois
x = 10

if x == 10
then output is "Equal"
```

---

## Boolean Conditions

LOIS also supports boolean values:

```lois
has_id is True
```

A boolean can be used directly as a condition:

```lois
has_id is True

if has_id
then output is "Allowed"
```

Remember that `True` and `False` are boolean values. Lowercase `true` and `false` are not the same boolean literals.

---

## Complex Conditions

Conditions can be combined using `and`, `or`, and `not`.

For example:

```lois
age = 20
has_id is True

if age >= 18 and has_id == True
then output is "Allowed"
```

Both conditions must be true because `and` is being used.

You can also use `or`:

```lois
age = 16
has_permission is True

if age >= 18 or has_permission == True
then output is "Allowed"
```

And `not`:

```lois
has_id is False

if not has_id
then output is "No ID"
```

---

# But If

LOIS supports additional conditions using:

```lois
but if
```

This works like an `else if` in many other languages.

For example:

```lois
score = 75

if score >= 90
then output is "A"
but if score >= 80
then output is "B"
but if score >= 70
then output is "C"
else output is "Below C"
```

Output:

```text
C
```

The conditions are checked from top to bottom.

As soon as one condition is true, its statement runs and the remaining branches are skipped.

---

## Multiple `but if` branches

You can have as many `but if` branches as needed.

```lois
temperature = 30

if temperature >= 40
then output is "Very hot"
but if temperature >= 30
then output is "Hot"
but if temperature >= 20
then output is "Warm"
else output is "Cold"
```

Output:

```text
Hot
```

---

## Errors

An `if` statement must have a condition and a statement after `then`.

For example, this is invalid:

```lois
if age >= 18
output is "Adult"
```

because `then` is missing.

A condition also needs to be something LOIS can evaluate as a condition.

For example, using an undefined variable:

```lois
if age >= 18
then output is "Adult"
```

will produce an error if `age` has never been defined.


# While Loops

A `while` loop keeps running as long as its condition is true.

The basic syntax is:

```lois
while condition
then statement
```

The condition can use the normal comparison operators such as:

* `<` — less than
* `>` — greater than
* `<=` — less than or equal to
* `>=` — greater than or equal to
* `==` — equal to
* `!=` — not equal to

So `<=` is **not required** for a `while` loop. You can use whichever condition makes sense.

---

## Basic While Loop

For example:

```lois
x = 0

while x < 5
then x = x + 1
then output is x
```

Output:

```text
1
2
3
4
5
```

Here the loop checks:

```text
x < 5
```

Then it runs the statements in the loop body.

The value of `x` is increased before it is printed, so the first output is `1`.

---

## Using `output is` with a Numeric Variable

`output is` can print a variable directly when that variable already contains a number.

For example:

```lois
x = 0

while x < 5
then x = x + 1
then output is x
```

`x` was created with `=`, so it contains a numeric value. `output is x` simply prints that value.

---

## Using `output =` Instead

If you want to calculate something while printing it, use `output =`.

For example:

```lois
x = 0

while x < 5
then x = x + 1
then output = x + 5
```

This calculates the expression every time the loop runs.

---

## Multiple Statements in a While Loop

A `while` loop can have multiple statements in its body.

```lois
x = 0

while x < 5
then x = x + 1
then output is x
```

Both statements execute every time the loop runs:

```text
x = x + 1
```

updates the value, and:

```text
output is x
```

prints it.

The order matters. Statements execute from top to bottom.

---

## Different Conditions

You can use any supported comparison operator.

### Less than

```lois
while x < 10
then ...
```

Runs while `x` is less than `10`.

### Greater than

```lois
while x > 0
then ...
```

Runs while `x` is greater than `0`.

### Less than or equal to

```lois
while x <= 10
then ...
```

Runs while `x` is less than or equal to `10`.

### Greater than or equal to

```lois
while x >= 1
then ...
```

Runs while `x` is greater than or equal to `1`.

### Not equal

```lois
while x != 10
then ...
```

Runs while `x` is not equal to `10`.

Conditions can also be more complex:

```lois
while x < 10 and x != 5
then ...
```

---

## While Loop Protection

LOIS has a maximum loop-iteration limit of **1,000,000 iterations**.

This prevents an accidentally infinite loop from running forever.

For example, this loop never changes `x`:

```lois
x = 1

while x < 5
then output is x
```

Since `x` stays `1`, the condition never becomes false.

LOIS will eventually stop the loop after reaching the iteration limit.

---

# For Loops

LOIS uses a simple counting `for` loop.

The syntax is:

```lois
for variable <= limit
then statement
```

The loop variable starts at `1` and increases by `1` after each iteration.

---

## Basic For Loop

```lois
for x <= 5
then output is x
```

Output:

```text
1
2
3
4
5
```

The `<=` here is simply the normal **less-than-or-equal-to comparison operator**.

It is not a special requirement of the `for` keyword.

The condition can use the normal comparison operators supported by LOIS.

---

## Different For Conditions

For example:

```lois
for x < 5
then ...
```

uses `<`, meaning **less than**.

```lois
for x <= 5
then ...
```

uses `<=`, meaning **less than or equal to**.

```lois
for x > 5
then ...
```

uses `>`, meaning **greater than**.

```lois
for x >= 5
then ...
```

uses `>=`, meaning **greater than or equal to**.

The exact result depends on the starting value and the automatic increment of the loop variable.

---

## The Loop Variable

The loop variable is automatically initialized to `1`.

For example:

```lois
for x <= 5
then output is x
```

is effectively counting:

```text
1
2
3
4
5
```

The variable is increased by `1` after each iteration.

---

## Using the Loop Variable in Calculations

Because the loop variable is numeric, it can be used in expressions.

```lois
for x <= 5
then output = x * 2
```

Output:

```text
2
4
6
8
10
```

You can also use it with other variables:

```lois
y = 10

for x <= 5
then output = x + y
```

---

## The Limit Can Be a Variable or Expression

The condition can use another numeric variable.

```lois
limit = 5

for x <= limit
then output is x
```

Output:

```text
1
2
3
4
5
```

Expressions can also be used:

```lois
limit = 2

for x <= limit + 3
then output is x
```

Output:

```text
1
2
3
4
5
```

---

## Nested For Loops

`for` loops can be nested.

```lois
for x <= 3
then for y <= 3
then output = x * y
```

The inner loop runs for each iteration of the outer loop.

---

## Changing the Loop Variable

The loop variable is numeric and is automatically controlled by the `for` loop.

Changing it manually can affect the loop's behavior.

For example:

```lois
for x <= 5
then x = x + 2
then output is x
```

The loop variable is still being automatically incremented by the `for` loop as well, so changing it yourself may cause the loop to skip values or behave differently than expected.

# Stop

stop immediately **stops the current loop**.

For example:

```lois
x = 0

while x < 10
then x = x + 1
then if x == 5
then stop
then output is x
```

Once `x` reaches `5`,`  stop` `stops the loop.`

The loop does not continue to the next iteration.

The important idea is:

```text
stop = stop the loop
```

---

# Skip

skip does the opposite of stop.

It **skips the rest of the current iteration** and moves to the next iteration of the loop.

For example:

```lois
x = 0

while x < 5
then x = x + 1
then if x == 3
then skip
then output is x
```

When `x` becomes `3`, `skip` `skips the remaining statements for that iteration.`

So the output is:

```text
1
2
4
5
```

The loop itself does not stop.

The important idea is:

```text
stop = break 
skip = continue
```

---

## Stop vs Skip

| Statement | What it does                                       |
| --------- | -------------------------------------------------- |
| stop      | Stops the loop completely                          |
| skip      | Skips the current iteration and continues the loop |

Think of it like this:

```text
stop
  ↓
STOP THE LOOP
```

while:

```text
skip
  ↓
SKIP THIS ITERATION
  ↓
NEXT ITERATION
```

Both can be used inside `while` and `for` loops.

---

# Repeat

`repeat` executes a statement a specified number of times.

The syntax is:

```lois
repeat count
then statement
```

For example:

```lois
repeat 5
then output is "Hello"
```

Output:

```text
Hello
Hello
Hello
Hello
Hello
```

---

## Repeat with an Expression

The repetition count can be an expression.

```lois
x = 2

repeat x + 3
then output is "Hello"
```

This repeats five times because:

```text
2 + 3 = 5
```

---

## Repeat with Zero or Negative Numbers

If the repetition count is `0` or negative, the body does not execute.

For example:

```lois
repeat 0
then output is "Hello"
```

produces no output.

---

## Repeat Requires a Number

The repeat count must be numeric.

This is invalid:

```lois
repeat "hello"
then output is "Hello"
```

because `"hello"` is a string, not a number.

---

## Loop Control

`stop`  and `skip` can be used to control loops.

The basic difference is:

```text
stop
```

**stops the loop completely.**

```text
skip
```

**skips the current iteration and moves to the next one.**\

# Functions

Functions let you create reusable pieces of code that can take values and produce a result.

LOIS has a simple function system where the expression after `of` determines what the function produces.

---

# Defining a Function

The primary way to define a function is:

```lois
name is function of expression
```

For example:

```lois
square is function of x * x
```

This creates a function named `square`.

The `x` used in the expression becomes the function's parameter.

So:

```lois
square is function of x * x
```

means:

```text
function name: square
parameter: x
result: x * x
```

---

## Another Example

```lois
double is function of x * 2
```

This function takes `x` and produces `x * 2`.

---

# String Functions

Functions can also produce strings.

For example:

```lois
greet is function of "hello"
```

The function produces:

```text
hello
```

A function can also use a parameter inside a string expression:

```lois
greet is function of "hello " + name
```

If:

```lois
name is "Alice"
```

then:

```lois
output is greet(name)
```

produces:

```text
hello Alice
```

---

# Function Parameters

Parameters are determined by the names used in the function expression.

For example:

```lois
add is function of x + y
```

has two parameters:

```text
x
y
```

and produces:

```text
x + y
```

Another example:

```lois
area is function of width * height
```

has:

```text
width
height
```

as its parameters.

---

# Calling Functions

Function calls are **always written with parentheses**.

The basic form is:

```lois
function_name(arguments)
```

For example:

```lois
square is function of x * x

output = square(5)
```

Output:

```text
25
```

You can also use `output is` when directly printing the function's result:

```lois
square is function of x * x

output is square(5)
```

Output:

```text
25
```

There is **no statement-style function call** such as:

```lois
function is square 5
```

That is not valid LOIS syntax.

---

# Multiple Arguments

Functions can have multiple parameters.

For example:

```lois
add is function of x + y
```

The function can be called with:

```lois
output = add(2, 3)
```

Output:

```text
5
```

Another example:

```lois
area is function of width * height

output = area(5, 10)
```

Output:

```text
50
```

The arguments are matched with the parameters in order.

---

# Functions Inside Expressions

A function call can be used wherever an expression can be used.

For example:

```lois
square is function of x * x

output = square(5) + 10
```

Output:

```text
35
```

You can also combine function calls with variables:

```lois
number = 5

output = square(number)
```

Output:

```text
25
```

---

# Functions Returning Strings

A function can return a string just as easily as a number.

For example:

```lois
name is "Alice"

greet is function of "hello " + name

output is greet(name)
```

Output:

```text
hello Alice
```

Another simple example:

```lois
greet is function of "hello"

output is greet()
```

Output:

```text
hello
```

A function with no parameters is called with empty parentheses:

```lois
greet()
```

---

# Alternative Function Syntax

The primary and recommended syntax is:

```lois
square is function of x * x
```

LOIS also supports an alternative form:

```lois
function is square is of x * x
```

Another supported form is:

```lois
function square is of x * x
```

These describe the same basic function.

The primary form is still:

```lois
square is function of x * x
```

because it is the simpler and more readable form.

---

# `returns`

LOIS also has the `returns` form for explicitly describing a function's result.

For example:

```lois
square function x it returns x * x
```

Here, the expression after `returns` is the result of the function.

Another example:

```lois
double function x it returns x * 2
```

The important part is:

```lois
returns expression
```

The expression after `returns` determines the result.

---

## `returns` Without a Value

If `returns` is used without an expression:

```lois
it returns
```

the result is `0`.

For example, a function using an empty return expression produces `0`.

---

# `return`

`return` can also be used as a statement.

A bare:

```lois
return
```

means:

```text
return 0
```

For example:

```lois
return
```

returns `0`.

If a value or expression is being returned, it must come after `returns` in the function-return syntax.

For example:

```lois
returns 5
```

or:

```lois
returns x * 2
```

is valid.

The form:

```lois
it return
```

is invalid.

The keyword is `returns`, not `return`, when an expression follows it.

---

# Function Errors

## Undefined Function

Calling a function that has not been defined produces an error.

For example:

```lois
output = unknown(5)
```

If `unknown` does not exist, LOIS reports an error because the function has not been defined.

---

## Wrong Number of Arguments

A function must receive the correct number of arguments.

For example:

```lois
add is function of x + y
```

requires two arguments.

This is invalid:

```lois
output = add(5)
```

because only one argument was provided.

Likewise, providing too many arguments is invalid:

```lois
output = add(5, 10, 20)
```

because `add` only has two parameters.

---

## Function Parameter Limit

A function can currently have at most **16 parameters**.

---

## Function Argument Limit

A function call can currently provide at most **16 arguments**.

---

## Function Calls Require Parentheses

Function calls must use parentheses.

Correct:

```lois
output = square(5)
```

Correct:

```lois
output is greet(name)
```

Invalid:

```lois
output = square 5
```

Invalid:

```lois
function is square 5
```

The arguments belong inside the parentheses.

---

# Typecasting

LOIS has two main data types:

* `num` — numbers
* `string` — text

Typecasting changes the type of an existing value.

The syntax is:

```lois
variable is type
```

---

## Converting to `num`

For example:

```lois
age is 15
age is num
```

The value of `age` is converted to a number.

It can then be used in numeric expressions:

```lois
age is 15
age is num

output = age + 5
```

Output:

```text
20
```

---

## Converting to `string`

A numeric variable can be converted to a string:

```lois
age = 12
age is string
```

Now `age` is treated as a string.

It can still be printed:

```lois
output is age
```

Output:

```text
12
```

but it is no longer a numeric value.

---

## `is` and `=`

The two operators have different purposes.

### Numeric values

Use `=` for numeric assignment:

```lois
age = 15
x = 10 + 5
result = x * 2
```

### Strings and booleans

Use `is` for strings and boolean values:

```lois
name is "Alice"
ready is True
```

### Typecasting

Use `is` when changing the type:

```lois
age is num
age is string
```

So:

```lois
age = 15
```

creates/assigns a numeric value, while:

```lois
age is num
```

explicitly converts the value to `num`.

---

# Case Sensitivity

LOIS recognizes many language keywords without depending on capitalization.

For example:

```lois
and
AND
And
```

are recognized as the same logical operator.

The boolean literals are deliberately different:

```lois
True
False
```

are boolean values.

Lowercase:

```lois
true
false
```

are ordinary words/variable names and are not the same boolean literals.

---

# Reserved Words

Some words have special meanings in LOIS and are therefore reserved by the language.

Important reserved words include:

```text
is
num
string
function
of
if
then
else
but
while
for
repeat
return
returns
output
input
and
or
not
True
False
stop
skip
```

If you want to print one of these words as literal text, put it inside quotes:

```lois
output is "if"
```

This prints:

```text
if
```

---

# Operators

LOIS supports the following main operators.

## Arithmetic

```text
+
-
*
/
%
^
!
```

## Comparison

```text
>
<
>=
<=
==
!=
```

## Assignment

```text
=
```

## Logical

```text
and
or
not
```

## Grouping

```text
(
)
```

## Absolute Value

```text
| |
```

---

# Sets

LOIS supports sets using braces.

For example:

```lois
numbers is {10, 20, 30}
```

The individual values can be accessed using their numbered names:

```lois
output is numbers1
```

accesses the first value.

```lois
output is numbers2
```

accesses the second value.

```lois
output is numbers3
```

accesses the third value.

---

# Comments

Single-line comments use `note:`:

```lois
note: this is a comment
```

Multi-line comments can be written using brackets:

```lois
[
this is a
multi-line comment
]
```

Comments are ignored by the interpreter.

---

# Nesting and `then`

LOIS does **not** use indentation or spaces to determine nesting.

Spaces are only for readability.

The `then` keyword determines the statement belonging to a control structure.

For example:

```lois
if x > 5
then output is "Large"
```

The `then` connects the following statement to the `if`.

The same idea is used with loops:

```lois
while x < 10
then x = x + 1
then output is x
```

And nested structures can be written directly:

```lois
while x < 10
then if x == 5
then stop
```

You do not need to indent the nested statements.

---

# Stop and Skip

LOIS does not use `break` and `continue`.

Instead, it uses:

```lois
stop
```

and:

```lois
skip
```

## `stop`

`stop` immediately stops the current loop.

```lois
x = 0

while x < 10
then x = x + 1
then if x == 5
then stop
then output is x
```

When `x` reaches `5`, `stop` ends the loop.

The important idea is:

```text
stop = stop the loop
```

---

## `skip`

`skip` skips the current iteration and continues with the next iteration.

```lois
x = 0

while x < 5
then x = x + 1
then if x == 3
then skip
then output is x
```

Output:

```text
1
2
4
5
```

When `x` reaches `3`, `skip` skips the remaining statement for that iteration.

The important difference is:

```text
stop = stop completely
skip = skip this iteration
```

---

# Common Errors

## Undefined Variable

Using a variable that has not been defined:

```lois
output is unknown
```

produces an error.

---

## Invalid Numeric Assignment

`=` expects a numeric expression.

This is invalid:

```lois
x = "hello"
```

because `"hello"` is a string.

Use:

```lois
x is "hello"
```

instead.

---

## Missing `then`

This is invalid:

```lois
if x > 5
output is "yes"
```

The correct syntax is:

```lois
if x > 5
then output is "yes"
```

---

## Invalid Function Call

This is invalid:

```lois
square 5
```

Function calls require parentheses:

```lois
square(5)
```

---

## Wrong Number of Function Arguments

If:

```lois
add is function of x + y
```

then:

```lois
output = add(5)
```

is invalid because the function requires two arguments.

---

## Mathematical Errors

Invalid mathematical operations produce errors.

Examples:

```lois
output = 10 / 0
```

```lois
output = 10 % 0
```

```lois
output = root(-1)
```

```lois
output = log(0)
```

```lois
output = ln(-1)
```

---

# Quick Reference

## Variables

```lois
name is "Alice"
age = 15
ready is True
```

## Typecasting

```lois
age is num
age is string
```

## Output

```lois
output is "Hello"
output is name
output is age
output = 5 + 3
output = age * 2
```

## Input

```lois
input is name
input = age
```

## Arithmetic

```lois
x = 10 + 5
x = 10 - 5
x = 10 * 5
x = 10 / 5
x = 10 % 3
x = 2 ^ 3
x = 5!
```

## Comparisons

```lois
x < 10
x > 10
x <= 10
x >= 10
x == 10
x != 10
```

## Logic

```lois
x > 5 and x < 10
x > 5 or x == 2
not x
```

## If

```lois
if x > 10
then output is "Large"
```

## But If / Else

```lois
if x > 10
then output is "Large"
but if x == 10
then output is "Equal"
else output is "Small"
```

## While

```lois
while x < 10
then x = x + 1
then output is x
```

## For

```lois
for x <= 10
then output is x
```

## Repeat

```lois
repeat 5
then output is "Hello"
```

## Loop Control

```lois
stop
```

Stops the current loop.

```lois
skip
```

Skips the current iteration.

## Functions

Primary definition:

```lois
square is function of x * x
```

Call:

```lois
square(5)
```

Use in output:

```lois
output is square(5)
```

Use in a numeric expression:

```lois
output = square(5) + 10
```

Alternative definition:

```lois
function is square is of x * x
```

or:

```lois
function square is of x * x
```

`returns` form:

```lois
square function x it returns x * x
```

## Sets

```lois
numbers is {10, 20, 30}

output is numbers1
```

## Comments

```lois
note: comment
```

or:

```lois
[
multi-line
comment
]
```

---

# A Complete Function Example

A function can be combined with normal variables and output:

```lois
name is "Alice"

greet is function of "hello " + name
square is function of x * x

output is greet(name)
output is square(5)
output = square(5)
```

Output:

```text
hello Alice
25
25
```

The function definitions come first, and the functions are then called using parentheses.

---

# The Core Idea

The most important distinction in LOIS is the difference between `is` and `=`.

```lois
name is "Alice"
```

stores a string.

```lois
age = 15
```

stores a number.

```lois
age is string
```

changes the type of `age`.

```lois
output is name
```

prints a value directly.

```lois
output = age + 5
```

evaluates a numeric expression and prints the result.

Functions follow the same readable structure:

```lois
square is function of x * x
```

and are always called using parentheses:

```lois
square(5)
```

Finally, LOIS uses `then` for structure rather than indentation:

```lois
if condition
then statement
```

```lois
while condition
then statement
```

```lois
for condition
then statement
```

Spaces and indentation do not determine nesting.
