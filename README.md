# Scheme Interpreter

This repository contatins my implementation of interpreter for [Scheme](https://en.wikipedia.org/wiki/Scheme_(programming_language)) language

## Build & Run

In order to build it, you need to have GNU readline package installed.

Then, to build, just run:

```
mkdir build
cd build && cmake ..
make
```

This will create the `scheme` executable you can run!

## Example

Here's an example of what is possible:

```
Scheme Interpreter (v1.0.0) [Thu Feb  1 03:42:07 2024] on linux
>>> (+ 1 2)
3
>>> (define t '(1))
()
>>> t
(1)
>>> (define (add x y)
...     (+ x y)
... )
()
>>> (add 5 8)
13
>>> (define (fact x)
...   (if (< x 2)
...       1
...       (* (fact (- x 1)) x)))
()
>>> (fact 2)
2
>>> (fact 4)
24
```
