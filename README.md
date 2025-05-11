# PLAS Studio

**An AI-powered code generator that transforms human readable .plas files into real C++ code.**

PLAS is a tool I made that lets you write `.plas` files — basically simplified, readable instructions — and turns them into real C++ using AI.

It’s not a real language, and it’s definitely not a real compiler, but it works. You write stuff like:

```plas
ask "What's your name?" into name
print "Hello, " + name
```

and it spits out valid C++ code.

I built this using a free $1 API credit from Together.ai.

---

##  What it does

- You type `.plas` pseudocode  
- It sends that to a language model  
- You get back C++ that matches your intent  
- Copy it, compile it, or just learn from it  

---

## Example

```plas
ask "Enter a number?" into num
if num > 10
print "That's a big number!"
else
print "That's a small number."
end
```

Turns into:

```cpp
#include <iostream>
#include <string>

int main() {
    std::string input;
    int num;

    std::cout << "Enter a number?" << std::endl;
    std::getline(std::cin, input);

    try {
        num = std::stoi(input);
    } catch (const std::invalid_argument& e) {
        std::cout << "That's not a valid number." << std::endl;
        return 1;
    }

    if (num > 10) {
        std::cout << "That's a big number!" << std::endl;
    } else {
        std::cout << "That's a small number." << std::endl;
    }

    return 0;
}
```

---

## How to run it

```bash
visual studio
```

> You’ll need an API key for Together.ai or whatever model provider you plug in.

---

## License

AGPL-3.0. You can use this, learn from it, even build on it — just keep it open-source.

---

## Made by [@Plasmism](https://github.com/Plasmism)

> ai was used to write the readme and some parts of the code.
>
> ai is a great tool, but it doesn't replace hard work.
