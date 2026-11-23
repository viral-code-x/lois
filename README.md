# LOIS

## Language Orienting ItSelf
![image alt](LOIS LOGO.png)
**A tutorial manual-inspired programming language designed to make programming more approachable, readable, and understandable for people.**

LOIS is an experimental programming language built around a simple idea:

**Programming should not feel like learning how to speak to a machine before you can express an idea.**

The goal of LOIS is not to make code shorter, compete with Python, or replace established programming languages. Its goal is to make the fundamentals of programming easier to recognise and understand, especially for people who have never written code before.

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [The Philosophy Behind LOIS](#2-the-philosophy-behind-lois)
3. [What Makes LOIS Different?](#3-what-makes-lois-different)
4. [Getting Started](#4-getting-started)
5. [Your First LOIS Program](#5-your-first-lois-program)
6. [Basic Syntax and Structure](#6-basic-syntax-and-structure)
7. [Variables and Assignment](#7-variables-and-assignment)
8. [Input and Output](#8-input-and-output)
9. [Mathematical Expressions and Operators](#9-mathematical-expressions-and-operators)
10. [Sets and Indexed Data](#10-sets-and-indexed-data)
11. [Conditions](#11-conditions)
12. [Loops](#12-loops)
13. [Functions](#13-functions)
14. [Built-in Mathematical Functions](#14-built-in-mathematical-functions)
15. [Comments](#15-comments)
16. [Complete Examples](#16-complete-examples)
17. [Common Mistakes](#17-common-mistakes)
18. [Current Capabilities and Limitations](#18-current-capabilities-and-limitations)
19. [Quick Syntax Reference](#19-quick-syntax-reference)
20. [Project Information](#20-project-information)

---

# 1. Introduction

LOIS stands for **Language Orienting ItSelf**.

It is a tutorial manual-inspired programming language that attempts to make programming more approachable by using syntax that communicates what a program is doing in a way people can recognise.

Consider a simple task: displaying a message.

In many programming languages, a beginner might encounter:

```python
print("Hello world")
```

Or:

```javascript
console.log("Hello world");
```

Or:

```php
echo "Hello world";
```

These commands are familiar to experienced programmers. But what do they communicate to someone who has never programmed?

Does `print` mean a printer? What is being echoed? Why is a command called `console.log`?

The commands make sense once someone learns the programming language. But the beginner must first learn what those commands mean.

LOIS approaches this differently:

```lois
output is Hello world
```

The intention is immediately visible.

The program is producing output. The word `is` connects the action with its content. Even without previous programming experience, someone can make a reasonable interpretation of the statement.

That is the central idea behind LOIS.

# 2. The Philosophy Behind LOIS

## Programming languages are designed for computers. But who are they communicating with?

The basic foundations of programming are not necessarily unfamiliar to people.

We already understand many of the ideas behind programming:

- Giving information.
- Receiving information.
- Making decisions.
- Repeating actions.
- Performing calculations.
- Organising information.
- Breaking a problem into smaller steps.

These are concepts people encounter in everyday life, mathematics, and education.

Yet learning programming can feel unnecessarily complicated because the learner must understand both the programming concept and the language-specific commands used to express it.

I began thinking about a question:

**If the foundations of programming are already understandable, why does expressing them in code have to feel so unfamiliar?**

Even beginner-friendly languages such as Python and Lua use established programming conventions. These languages are powerful, practical, and excellent tools. However, their syntax still expects the learner to become familiar with programming terminology and conventions.

For someone who has never written code, a command such as `echo`, `print`, or `console.log` may not immediately explain its purpose.

They are instructions that make sense after learning the language.

I wanted to experiment with a different approach.

## Introducing LOIS

Instead of trying to make programming shorter, I tried to make it more readable.

Instead of asking how few characters a program could use, I asked how clearly a person could understand what a statement was doing.

**Output**

```lois
output is Hello world
```

**Repetition**

```lois
repeat 5
then output is Hello
```

**Conditions**

```lois
if age > 18
then output is Adult
```

**Mathematics**

```lois
total = 10 + 20 * 3
output = total
```

**Loops**

```lois
i is 1

while i <= 5
then output = i
then i = i + 1
```

These examples use familiar words and explicit connections between statements.

## Why the keyword `then`?

Many programming languages use braces, indentation, or other structural conventions to determine which statements belong to a condition or loop.

LOIS uses the keyword `then` to make the relationship explicit.

For example:

```lois
if age > 18
then output is Adult
```

The word `then` communicates that the following statement belongs to the condition.

It is not necessary to understand a particular indentation style to recognise the relationship.

LOIS treats this as an explicit step in the language's structure.

## Why `is`?

LOIS uses `is` for a readable form of assignment and text-oriented statements.

```lois
name is Alex
```

A beginner can interpret this as:

"The name is Alex."

This is one of the central design choices of LOIS.

## Why not just use Python?

Python is an excellent programming language. Lua is an excellent programming language. Neither needs to be made less capable to justify the existence of LOIS.

The objective is different.

LOIS is not trying to be easier than Python in every technical sense. It is exploring whether a programming language can make its basic operations more self-explanatory to someone who has never learned programming.

The focus is not on replacing existing languages.

It is on approaching the learning experience from another direction.

**LOIS is an experiment in making programming syntax orient itself toward the person reading it.**

# 3. What Makes LOIS Different?

| Design choice | LOIS approach |
|---|---|
| Output | `output is Hello` |
| Text assignment | `name is Alex` |
| Numeric assignment | `age = 17` |
| Conditions | `if condition` |
| Repetition | `repeat` |
| Loop bodies | Explicit `then` statements |
| Mathematical expressions | Familiar mathematical operators |
| Collections | Sets using `{ }` |
| Set access | One-based indexing using `( )` |
| Functions | Expression-based definitions |
| Source files | `.is` |

These choices are intended to reduce the distance between what a beginner wants to express and what they must write.

They do not eliminate the need to learn programming concepts. They aim to make those concepts easier to recognise.

# 4. Getting Started

## Requirements

To build and run LOIS from source, you need:

- A C compiler supported by the project.
- CMake.
- A terminal or command-line environment.
- The LOIS source code.

## Clone the repository

```bash
git clone https://github.com/viral-code-x/lois.git
cd lois
```

## Build with CMake

If your checkout includes the project's CMake configuration, use:

```bash
cmake -S . -B build
cmake --build build
```

On a typical Linux or GitHub Codespaces environment, the executable is expected at:

```bash
./build/lois
```

The exact executable location can vary depending on the build system and generator.

## Run a LOIS program

Create a file with the `.is` extension:

```text
hello.is
```

Write:

```lois
output is Hello world
```

Run it:

```bash
./build/lois hello.is
```

Expected output:

```text
Hello world
```

# 5. Your First LOIS Program

Let's start with a complete example.

Create `first_program.is`:

```lois
name is Alex
age = 17

output is "Hello, " name
output is "Your age is " age
```

Expected output:

```text
Hello, Alex
Your age is 17
```

This program demonstrates:

- Text assignment.
- Numeric assignment.
- Output statements.
- Quoted text.
- Variable substitution.
- Combining text and variables in output.

The important thing is that each statement is intended to be understandable on its own.

You do not need to learn an unfamiliar output command before displaying a message.

# 6. Basic Syntax and Structure

## 6.1 Statements

A LOIS program consists of statements.

Each statement describes an operation, assignment, condition, loop, or other action.

For example:

```lois
name is Alex
age = 17
output is Hello name
```

Statements are separated by newlines.

## 6.2 Indentation is not the way LOIS connects statements

LOIS uses explicit `then` statements for the bodies of supported conditions and loops.

This is valid:

```lois
if age > 18
then output is Adult
```

Do not rely on indentation to define a statement body.

The keyword `then` establishes the connection.

## 6.3 Case sensitivity

Variable and function names are case-sensitive.

For example:

```lois
age is 17
output = age
```

Here, `age` and `Age` are different names.

Some built-in mathematical function names are handled without case sensitivity.

## 6.4 Names

Identifiers can begin with a letter or underscore and can contain letters, numbers, and underscores.

Examples:

```lois
age is 17
student_name is Alex
_total = 100
number1 = 25
```

Avoid spaces in variable names.

# 7. Variables and Assignment

Variables store values that can be used later.

LOIS provides different assignment forms.

## 7.1 Text-oriented assignment with `is`

```lois
name is Alex
country is Bangladesh
message is Hello
```

This form is intended for readable assignment of text.

A bare word after `is` in this assignment context is treated as text.

For example:

```lois
name is Alex
```

Stores the text `Alex`.

Do not assume that writing `first is second` copies the value of a variable called `second`. In this form, a bare word is treated as a text literal.

## 7.2 Numeric assignment with `=`

Use `=` for evaluated numeric expressions.

```lois
age = 17
marks = 85
total = 10 + 20
```

The expression is evaluated and its result is assigned.

```lois
x = 10
y = 20
result = x + y

output = result
```

Output:

```text
30
```

## 7.3 Initialising a variable with a type

The `num` and `text` type forms are supported in assignment contexts.

For example:

```lois
age is num
name is text
```

These initialise variables with their respective types.

The numeric form starts at zero, while the text form starts as an empty string.

## 7.4 Changing values

A variable can be reassigned.

```lois
score = 10
score = score + 5
output = score
```

Output:

```text
15
```

## 7.5 Increment shorthand

LOIS supports `++` as a shorthand for increasing a numeric variable by one.

```lois
i is 1
i++
output = i
```

Output:

```text
2
```

This is equivalent to:

```lois
i = i + 1
```

# 8. Input and Output

Input and output allow a program to communicate with the person using it.

## 8.1 Output

Use `output` to display information.

```lois
output is Hello world
```

You can also display variables:

```lois
name is Alex
output is name
```

When an unquoted word in an output statement matches an existing variable, LOIS substitutes the variable's value.

When an unquoted word does not match a variable, it is treated as literal text.

For example:

```lois
name is Alex
output is Hello name
```

Output:

```text
Hello Alex
```

## 8.2 Combining text and variables

LOIS allows text and variable names to be placed next to each other in an output statement.

You do not need to use `+` just to insert a variable into a message.

```lois
name is afnan

output is "your name is " name
output is hello world
```

Expected output:

```text
your name is afnan
hello world
```

The output statement is made of separate pieces:

- Quoted text is displayed literally.
- An unquoted word matching a variable displays that variable's value.
- An unquoted word that does not match a variable is displayed as ordinary text.

This means that quoted text can contain a word that is also a variable name without substituting that variable.

For example, if `name` is a variable:

```lois
name is Alex

output is "The word name is written here."
output is "The person's name is " name
```

Expected output:

```text
The word name is written here.
The person's name is Alex
```

**Important:** Quotation marks distinguish literal text from variable references. Spaces inside quoted text are preserved. Use a space at the end of a quoted piece when the following piece should be separated by a space.

## 8.3 Quoted text

Quoted strings can be used when you want to define text explicitly.

```lois
output is "Welcome to LOIS"
```

Quotation marks tell the output system that the enclosed content is literal text.

## 8.4 Output with expressions

Use `=` when you want to evaluate an expression.

```lois
age = 17
output = age + 1
```

Output:

```text
18
```

The `=` form evaluates the expression and displays its result.

## 8.5 Input

Use `input` to receive information.

Text input:

```lois
input is name
```

Numeric input:

```lois
input = age
```

The distinction is important:

- `input is` reads text.
- `input =` reads a numeric value.

## 8.6 Input with a prompt

You can provide a prompt using `with`.

```lois
input is name with "Enter your name: "
output is "Hello " name
```

Example interaction:

```text
Enter your name: Alex
Hello Alex
```

For numeric input:

```lois
input = age with "Enter your age: "
output = age
```

# 9. Mathematical Expressions and Operators

LOIS uses familiar mathematical operators.

The intention is to make mathematical expressions feel similar to the notation people already encounter in calculators and mathematics.

## 9.1 Basic arithmetic

| Operator | Meaning | Example |
|---|---|---|
| `+` | Addition | `10 + 5` |
| `-` | Subtraction | `10 - 5` |
| `*` | Multiplication | `10 * 5` |
| `/` | Division | `10 / 5` |
| `%` | Remainder | `10 % 3` |
| `^` | Exponentiation | `2 ^ 3` |

Example:

```lois
a = 10
b = 5

output = a + b
output = a - b
output = a * b
output = a / b
```

Output:

```text
15
5
50
2
```

## 9.2 Order of operations

LOIS evaluates mathematical expressions using operator precedence.

For example:

```lois
result = 2 + 3 * 4
output = result
```

Output:

```text
14
```

Multiplication is evaluated before addition.

Parentheses can be used to control the order:

```lois
result = (2 + 3) * 4
output = result
```

Output:

```text
20
```

## 9.3 Remainder

The `%` operator calculates the remainder after division.

```lois
output = 10 % 3
```

Output:

```text
1
```

This is useful for checking whether a number is even or odd.

```lois
number = 8

if number % 2 == 0
then output is Even
else output is Odd
```

## 9.4 Comparisons

| Operator | Meaning |
|---|---|
| `==` | Equal to |
| `!=` | Not equal to |
| `>` | Greater than |
| `<` | Less than |
| `>=` | Greater than or equal to |
| `<=` | Less than or equal to |

Example:

```lois
age = 20

if age >= 18
then output is Adult
```

## 9.5 Logical operators

LOIS supports:

- `and`
- `or`
- `not`
- `!`

Example:

```lois
age = 20
has_id = True

if age >= 18 and has_id
then output is Allowed
```

Using `not`:

```lois
is_raining is False

if not is_raining
then output is No rain
```

Logical operators use truthiness. Numbers, strings, booleans, and sets can be evaluated as true or false according to whether they represent a non-empty or non-zero value.

## 9.6 Comparison chains

LOIS supports chained comparisons.

```lois
age = 16

if 13 <= age <= 18
then output is Teenager
```

This expresses a range in a way that resembles ordinary mathematics.

## 9.7 Factorial

The postfix `!` operator represents factorial.

```lois
output = 5!
```

Output:

```text
120
```

Factorial is supported for non-negative integers.

The prefix `!` can also be used as logical NOT:

```lois
output = !True
```

# 10. Sets and Indexed Data

LOIS uses sets to organise multiple values under one variable.

A set can contain multiple values, including numbers, text, booleans, and other sets.

## 10.1 Creating a set

Use curly braces `{ }`.

```lois
numbers{1, 2, 3, 4, 5}
```

You can also use a set literal in an expression:

```lois
numbers = {1, 2, 3, 4, 5}
```

An empty set can be written as:

```lois
numbers{}
```

Or as an empty set literal:

```lois
numbers = {}
```

## 10.2 Accessing elements

LOIS uses parentheses for set indexing.

```lois
numbers{10, 20, 30}

output = numbers(1)
output = numbers(2)
output = numbers(3)
```

Output:

```text
10
20
30
```

**LOIS set indexing starts at 1, not 0.**

| Index | Value |
|---:|---:|
| 1 | 10 |
| 2 | 20 |
| 3 | 30 |

This is an intentional design choice in the language's current set system.

## 10.3 Changing an element

An existing element can be replaced through indexed assignment.

```lois
numbers{10, 20, 30}

numbers(2) = 99

output = numbers(2)
```

Output:

```text
99
```

## 10.4 Adding an element through indexing

An element can be appended by assigning to the next available index.

```lois
numbers{10, 20, 30}

numbers(4) = 40
```

The new set contains:

```text
{10, 20, 30, 40}
```

You cannot skip an index when appending. For example, assigning to index 6 when the set contains only three elements is not a valid append operation.

## 10.5 Mixed values

Sets can contain different value types.

```lois
information{
    "Alex",
    17,
    True
}
```

This demonstrates a set containing:

1. Text.
2. A number.
3. A boolean.

## 10.6 Nested sets

Sets can contain other sets.

```lois
matrix = {
    {1, 2},
    {3, 4}
}
```

Nested indexing is supported:

```lois
output = matrix(1)(2)
```

Output:

```text
2
```

## 10.7 Set rules

- Set indexing starts at 1.
- Indexes must be positive integers.
- Reading an index requires that the element already exists.
- Existing elements can be replaced.
- The next available index can be used to append.
- Indexes cannot be skipped during append.
- Nested sets are supported.
- Set values are copied when read, including nested set contents.

The current implementation uses dynamically allocated set storage. It does not impose a small fixed number of elements as the language's set size.

# 11. Conditions

Conditions allow a program to make decisions.

LOIS uses `if`, `else if`, `but if`, and `else`.

## 11.1 The `if` statement

```lois
age = 20

if age >= 18
then output is Adult
```

If the condition is true, the statement after `then` is executed.

## 11.2 The `else` statement

Use `else` to define what happens when the condition is false.

```lois
age = 15

if age >= 18
then output is Adult
else output is Minor
```

## 11.3 The `else if` statement

Use `else if` to check another condition when the previous condition was false.

```lois
marks = 75

if marks >= 80
then output is Excellent
else if marks >= 60
then output is Good
else if marks >= 40
then output is Passed
else output is Failed
```

The conditions are checked in order. The first matching branch is selected.

## 11.4 The `but if` statement

LOIS also supports `but if` as an additional conditional branch.

In the current implementation, ordinary `else if` checks are considered before the `but if` branches. A matching `but if` branch is used when none of those ordinary branches has matched.

Example:

```lois
score = 75

if score >= 90
then output is Excellent
else if score >= 80
then output is Very good
but if score >= 70
then output is Good
else output is Keep practising
```

Use this form when you specifically want the additional `but if` behaviour. For ordinary decision chains, `else if` is the simpler and more familiar choice.

## 11.5 Conditions with logical operators

```lois
age = 20
has_permission = True

if age >= 18 and has_permission
then output is Access granted
else output is Access denied
```

## 11.6 Conditional expressions

LOIS also supports conditional expressions inside an expression.

```lois
age = 20
status = if age >= 18 then "Adult" else "Minor"

output is status
```

This evaluates the selected branch and produces its value.

# 12. Loops

Loops allow a program to repeat instructions.

LOIS provides `while`, `for`, and `repeat`.

The `then` keyword connects the loop with the statements it executes.

## 12.1 The `while` loop

A `while` loop repeats as long as its condition remains true.

```lois
i is 1

while i <= 5
then output = i
then i = i + 1
```

Output:

```text
1
2
3
4
5
```

The loop:

1. Starts with `i` equal to 1.
2. Checks whether `i` is less than or equal to 5.
3. Displays the current value.
4. Increases `i` by 1.
5. Checks the condition again.

The body can contain multiple `then` statements.

## 12.2 The `for` loop

A `for` loop is useful when you know the starting value, ending value, and step.

```lois
for i = 1, 5, i++
then output = i
```

Output:

```text
1
2
3
4
5
```

The general form is:

```lois
for variable = start, finish, step
then statement
```

For example, the step can also be written as an assignment:

```lois
for i = 1, 5, i = i + 1
then output = i
```

The current range-style loop includes its endpoint.

## 12.3 Counting backwards

A descending loop can use a negative step.

```lois
for i = 5, 1, i = i - 1
then output = i
```

Output:

```text
5
4
3
2
1
```

## 12.4 Conditional `for` loops

The current parser also supports a condition-style `for` form.

```lois
for i = 1, i <= 5, i++
then output = i
```

This uses the condition as the loop's continuation test.

## 12.5 The `repeat` loop

Use `repeat` when you want to execute something a fixed number of times.

```lois
repeat 5
then output is Hello
```

Output:

```text
Hello
Hello
Hello
Hello
Hello
```

This is useful when the exact number of repetitions is known.

## 12.6 `stop`

Use `stop` to exit the current loop.

```lois
i is 1

while i <= 10
then if i == 5
then stop
then output = i
then i++
```

`stop` ends the loop instead of continuing its repetitions.

## 12.7 `skip`

Use `skip` to skip the remaining statements in the current loop iteration and continue with the next iteration.

For example:

```lois
i is 0

while i < 5
then i++
then if i == 3
then skip
then output = i
```

The value 3 is skipped.

## 12.8 Loop limitations

The current runtime has a maximum iteration limit of 1,000,000 for `while` and `for` loops.

This is an implementation safeguard, not a claim that every possible loop should be limited to that number forever.

# 13. Functions

Functions allow reusable calculations and operations to be defined once and called when needed.

LOIS currently supports expression-based function definitions.

## 13.1 Defining a function

```lois
Square(x) = x * x
```

This defines a function named `Square` with one parameter, `x`.

## 13.2 Calling a function

```lois
Square(x) = x * x

output = Square(5)
```

Output:

```text
25
```

Function calls use parentheses.

## 13.3 Functions with multiple parameters

```lois
Add(a, b) = a + b

output = Add(10, 20)
```

Output:

```text
30
```

## 13.4 Another function definition form

LOIS also supports the `function of` form for a single parameter:

```lois
Square is function of x * x

output = Square(5)
```

## 13.5 Calling a function as a statement

A function call can also be used as a statement:

```lois
Add(a, b) = a + b

Add(10, 20)
```

The current interpreter displays the resulting value for a function-call statement.

## 13.6 Function behaviour and current scope

Current LOIS functions are expression-based. Their result comes from evaluating the expression in the definition.

They are not currently general multi-statement function bodies.

The language's `return` statement should not be confused with a conventional function return mechanism: in the current interpreter, a `return` statement evaluates and displays its expression rather than terminating a function body and returning a value to its caller.

Function and variable names are case-sensitive. The current implementation supports up to 256 function definitions.

# 14. Built-in Mathematical Functions

LOIS includes several built-in mathematical functions.

These functions accept one numeric argument.

| Function | Purpose |
|---|---|
| `abs(x)` | Absolute value |
| `round(x)` | Round to the nearest integer |
| `floor(x)` | Round downward |
| `ceil(x)` | Round upward |
| `root(x)` | Square root |
| `root3(x)` | Cube root |
| `root4(x)` | Fourth root |
| `root5(x)` | Fifth root |
| `sin(x)` | Sine |
| `cos(x)` | Cosine |
| `tan(x)` | Tangent |
| `log(x)` | Base-10 logarithm |
| `ln(x)` | Natural logarithm |
| `rad(x)` | Convert degrees to radians |

## Examples

```lois
output = abs(-10)
output = round(4.7)
output = floor(4.7)
output = ceil(4.2)
```

Output:

```text
10
5
4
5
```

## Roots

```lois
output = root(25)
output = root3(27)
output = root4(16)
output = root5(32)
```

Output:

```text
5
3
2
2
```

## Trigonometry

LOIS's trigonometric functions use degrees by default.

```lois
output = sin(30)
output = cos(60)
output = tan(45)
```

For calculations using radians, use `rad()`:

```lois
angle = rad(90)
output = sin(angle)
```

The `pi` constant is also available:

```lois
output = pi
```

## Logarithms

```lois
output = log(100)
output = ln(1)
```

The logarithm functions require positive input values.

Mathematical functions also validate their domains. For example, square roots of negative numbers are rejected.

# 15. Comments

Comments allow you to write explanations in your source code without treating them as executable statements.

LOIS supports two comment forms.

## 15.1 Single-line comments

Use `note:`.

```lois
note: This is a comment

age = 17
output = age
```

The text after `note:` is ignored until the end of the line.

The keyword is case-insensitive.

## 15.2 Multiline comments

Use square brackets.

```lois
[
This is a multiline comment.
It can contain several lines.
The interpreter ignores this section.
]

output is Hello world
```

The current lexer closes a multiline comment at the first closing `]`. Nested multiline comments are not supported.

# 16. Complete Examples

The following examples combine the features introduced above.

## 16.1 Hello World

```lois
output is Hello world
```

## 16.2 Personal information

```lois
name is Alex
age = 17
country is Bangladesh

output is "Name: " name
output is "Age: " age
output is "Country: " country
```

Expected output:

```text
Name: Alex
Age: 17
Country: Bangladesh
```

## 16.3 Simple calculator

```lois
a = 20
b = 10

output is Addition:
output = a + b

output is Subtraction:
output = a - b

output is Multiplication:
output = a * b

output is Division:
output = a / b
```

Output:

```text
Addition:
30
Subtraction:
10
Multiplication:
200
Division:
2
```

## 16.4 Even or odd

```lois
number = 17

if number % 2 == 0
then output is Even
else output is Odd
```

## 16.5 Age classifier

```lois
age = 17

if age < 13
then output is Child
else if age <= 19
then output is Teenager
else output is Adult
```

## 16.6 Sum of numbers from 1 to 10

```lois
i is 1
total = 0

while i <= 10
then total = total + i
then i++

output is "Total: " total
```

Output:

```text
Total: 55
```

## 16.7 Working with a set

```lois
numbers{10, 20, 30, 40, 50}

output is "First: " numbers(1)
output is "Third: " numbers(3)

numbers(3) = 99

output is "Updated third: " numbers(3)
```

## 16.8 A simple reusable function

```lois
Square(x) = x * x

output = Square(4)
output = Square(5)
output = Square(10)
```

Output:

```text
16
25
100
```

## 16.9 A simple grade classifier

```lois
marks = 85

if marks >= 80
then output is Grade A
else if marks >= 70
then output is Grade B
else if marks >= 60
then output is Grade C
else if marks >= 40
then output is Grade D
else output is Fail
```

# 17. Common Mistakes

This section explains mistakes that beginners may encounter.

| Mistake | Correct approach |
|---|---|
| Using `print` | Use `output` |
| Using `echo` | Use `output` |
| Using `console.log` | Use `output` |
| Using indentation as the body structure | Use `then` |
| Using `=` for equality | Use `==` |
| Using `==` for assignment | Use `=` or `is`, depending on the value |
| Accessing a set from index 0 | Start at index 1 |
| Calling a function without parentheses | Use `FunctionName(arguments)` |
| Appending to a set while skipping indexes | Append at the next available index |
| Using a non-numeric value in a numeric calculation | Make sure the expression evaluates to a number |

## 17.1 Confusing `is` and `=`

These two forms have different purposes.

```lois
name is Alex
```

This assigns text.

```lois
age = 17
```

This assigns a numeric value.

For comparisons, use:

```lois
if age == 17
then output is Correct
```

## 17.2 Forgetting `then`

Incorrect:

```text
if age > 18
output is Adult
```

Correct:

```lois
if age > 18
then output is Adult
```

## 17.3 Using zero-based indexing

Incorrect:

```lois
numbers{10, 20, 30}
output = numbers(0)
```

Correct:

```lois
numbers{10, 20, 30}
output = numbers(1)
```

The first element is at index 1.

## 17.4 Forgetting to update a loop variable

A `while` loop must eventually reach a point where its condition becomes false, unless it is intentionally designed to continue.

For example:

```lois
i is 1

while i <= 5
then output = i
then i++
```

Without `i++`, the value of `i` would never change.

## 17.5 Using `+` unnecessarily in output

You do not need to concatenate ordinary output text and variables using `+`.

Instead of relying on:

```lois
name is Alex
output is "Hello " + name
```

Use the language's text-and-variable output form:

```lois
name is Alex
output is "Hello " name
```

Quoted text is literal, and an unquoted matching variable name is substituted.

# 18. Current Capabilities and Limitations

LOIS is an evolving programming language. Its current implementation supports a growing collection of fundamental programming features, but it should not be presented as a mature general-purpose language.

## Current capabilities

The implementation includes:

- A custom lexer.
- A custom parser.
- An interpreter.
- Numeric and text values.
- Boolean values.
- Variable assignment.
- Arithmetic expressions.
- Comparison and logical operators.
- Conditional statements.
- Conditional expressions.
- `while`, `for`, and `repeat` loops.
- Loop control using `stop` and `skip`.
- Expression-based functions.
- Built-in mathematical functions.
- Sets and nested sets.
- One-based set indexing.
- Set element replacement and append operations.
- Text and numeric input.
- Output formatting.
- Single-line and multiline comments.
- Runtime error handling.

## Current implementation limits

The implementation currently has several explicit limits:

| Area | Current implementation |
|---|---|
| Variables | Maximum of 256 |
| Functions | Maximum of 256 |
| `while` loop | Maximum of 1,000,000 iterations |
| `for` loop | Maximum of 1,000,000 iterations |
| Input buffer | 1,024 bytes |
| Output buffer | 1 MiB |
| Error buffer | 1 MiB |
| Set indexing | Positive integer indexes, starting at 1 |

These are implementation details and may change as the language develops.

## Features not to assume are available

The current implementation should not be assumed to provide:

- A complete standard library.
- A general module-loading system.
- Classes or object-oriented programming.
- Dictionaries or general key-value collections.
- General file input/output from LOIS programs.
- General multi-statement function bodies.
- Conventional function-return semantics.
- A complete exception-handling system.
- A `for each` collection iteration system.
- Unlimited loop execution.

The parser recognises `add math` and `add game`, but the current interpreter does not implement actual module loading for these statements.

LOIS's purpose is to develop a readable programming foundation. Its capabilities should be described according to what is actually implemented, not what may be planned for the future.

# 19. Quick Syntax Reference

This section provides a compact reference for common LOIS syntax.

## Output

```lois
output is Hello world
output is "Hello world"
output is "Hello " name
output = 10 + 20
```

## Variables

```lois
name is Alex
age = 17
number is num
message is text
```

## Input

```lois
input is name
input = age
input is name with "Enter your name: "
input = age with "Enter your age: "
```

## Arithmetic

```lois
result = 10 + 5
result = 10 - 5
result = 10 * 5
result = 10 / 5
result = 10 % 3
result = 2 ^ 3
```

## Conditions

```lois
if condition
then statement
else statement
```

## Multiple conditions

```lois
if condition1
then statement1
else if condition2
then statement2
else statement3
```

## Additional conditional branch

```lois
if condition1
then statement1
but if condition2
then statement2
else statement3
```

## While loop

```lois
while condition
then statement
```

## For loop

```lois
for i = start, finish, i++
then statement
```

## Repeat loop

```lois
repeat 5
then statement
```

## Loop control

```lois
stop
skip
```

## Sets

```lois
numbers{1, 2, 3}
numbers = {1, 2, 3}
numbers(1)
numbers(2) = 99
numbers(4) = 40
```

## Functions

```lois
Square(x) = x * x
output = Square(5)
```

## Mathematical functions

```lois
abs(x)
round(x)
floor(x)
ceil(x)
root(x)
root3(x)
root4(x)
root5(x)
sin(x)
cos(x)
tan(x)
log(x)
ln(x)
rad(x)
```

## Comments

Single-line:

```lois
note: Single-line comment
```

Multiline:

```lois
[
Multiline comment
]
```

# 20. Project Information

**Project:** LOIS

**Full name:** Language Orienting ItSelf

**File extension:** `.is`

**Implementation:** C

**Project type:** Experimental interpreted programming language

**Repository:** [https://github.com/viral-code-x/lois](https://github.com/viral-code-x/lois)

## The idea behind the name

LOIS is short for **Language Orienting ItSelf**.

The name reflects the project's central philosophy: rather than expecting every beginner to immediately adapt to the conventions of a programming language, LOIS experiments with a language that tries to orient its syntax toward the person reading it.

It is not a claim that programming can be learned without effort.

It is an attempt to make the first interaction with programming feel more familiar.

## Final thought

Programming is a way of expressing ideas through instructions.

People already understand many of the ideas that programming is built upon. They understand numbers, decisions, repetition, information, and cause and effect.

Perhaps the first challenge should not be teaching someone to memorise unfamiliar commands.

Perhaps it should be giving them a language in which they can recognise what they are saying.

That is what LOIS is trying to explore.

**LOIS — Language Orienting ItSelf.**

*A programming language that tries to speak in a way any person can understand.*

