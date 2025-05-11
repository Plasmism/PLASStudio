## PLAS Syntax Specification v0.7

PLAS (`.plas`) is a beginner-focused programming language that bridges visual coding tools and real programming languages. It has two clear syntax modes to guide learners step-by-step:

- **Beginner Style**: Verbose, English-like, structured
- **Core Style**: Concise, streamlined, familiar to programmers

---

### 🔸 File Extension

- `.plas` for beginner style
- `.plasC` for core style (planned)

---

### 📚 Program Structure

```plas
include iostream

create main {
    print "Hello, world!"
}
end program
```

---

### 🖥️ Input & Output

```plas
ask "What is your name?" into name
print "Hello, " + name
```

---

### 📦 Variables & Assignment

Declare variables clearly:

```plas
let x = 10
let message = "Hello!"
```

Optional types for clarity:

```plas
declare number count = 5
declare text username = "Mark"
```

---

### ❓ Conditionals

```plas
if x > 10 then {
    print "Greater than 10"
} else {
    print "10 or less"
}
```

Multiple conditions:

```plas
if x == 0 then {
    print "Zero"
} else if x < 0 then {
    print "Negative"
} else {
    print "Positive"
}
```

Comparisons: `==`, `!=`, `>`, `<`, `>=`, `<=`, logical operators: `and`, `or`, `not`

---

### 🔁 Loops

Repeat fixed number of times:

```plas
repeat 3 times {
    print "PLAS"
}
```

Counting loops:

```plas
let counter = 0
repeat 5 times {
    print counter
    counter = counter + 1
}
```

---

### 📃 Lists

Creating and modifying lists:

```plas
let items = [10, 20, 30]
add 40 to items
```

Accessing list elements:

```plas
get item 0 from items into first_item
print first_item
```

Iterating through lists:

```plas
let index = 0
repeat 3 times {
    get item index from items into current_item
    print current_item
    index = index + 1
}
```

---

### 🛠️ Functions

Creating functions:

```plas
create function greet {
    print "Hello!"
}

call greet
```

Functions with parameters and returns:

```plas
create function add with x, y {
    let result = x + y
    return result
}

call add with 5, 3 into sum
print sum
```

---

### 💾 Memory Management

Explicit memory (advanced users):

```plas
create memory block buffer with 128 bytes
set all bytes in buffer to 0
delete buffer
```
> not added yet
---

### 📁 File I/O

Saving and loading:

```plas
write "Data" to file "file.txt"
read file "file.txt" into contents
```

---

### ⏱️ Timing & Delays

Waiting or pausing execution:

```plas
wait 2000 milliseconds
```

---

### 🚩 Labels & Jumps (basic loops and flow control)

```plas
label start
print "Infinite loop!"
goto start
```

---

### 🧮 Math Utilities

Useful math operations:

```plas
let squared = square of 6
let maximum = max of 5 and 9
let remainder = mod of 10 by 3
```

---

### 🔑 Reserved Keywords

`include`, `main`, `create`, `function`, `call`, `ask`, `print`, `repeat`, `if`, `else`, `then`, `end`, `end program`, `list`, `add`, `get`, `into`, `delete`, `memory`, `declare`, `let`, `return`, `write`, `read file`, `wait`, `label`, `goto`, `square`, `max`, `min`, `abs`, `mod`

---

### 🚀 Full Beginner Example

Combines many core concepts clearly:

```plas
include iostream

create main {
    ask "Enter three numbers:" into a, b, c

    if a >= b and a >= c then {
        print "Largest: " + a
    } else if b >= a and b >= c then {
        print "Largest: " + b
    } else {
        print "Largest: " + c
    }
}
end program
```

---

**End of PLAS Syntax Specification v0.7**
