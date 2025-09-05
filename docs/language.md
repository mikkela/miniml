# Mini-ML (Step 1) — Language Specification

This document defines the **core syntax, types, and semantics** of Mini-ML in its initial version (Step 1).  
It is intentionally small: lambda calculus + integers, `let`, `if`, and function application.

## Syntax

Expressions:

```
e ::= x                     -- variable
    | n                     -- integer literal
    | \x -> e               -- lambda (function)
    | e e                   -- application (left-associative)
    | let x = e in e        -- local binding
    | if e then e else e    -- conditional expression
```

- **Variables** are identifiers like `x`, `f`, etc.
- **Integer literals** are decimal integers.
- **Lambda abstraction** introduces a single parameter.
- **Application** is left-associative: `f a b` = `(f a) b`.
- **Let-binding** introduces a local name for use in the body.
- **If-expressions** require a boolean condition and return the value of one of their branches.

Precedence and associativity:

- Application binds tighter than `let` and `if`.
- Parentheses may be used to group expressions.

## Types

Type expressions:

```
τ ::= Int
    | Bool
    | α                   -- type variable
    | τ -> τ              -- function type
```

- `Int` — integers
- `Bool` — booleans (currently only used in `if`; literals and operators are future work)
- `α, β, …` — type variables introduced by inference
- `τ1 -> τ2` — function type from `τ1` to `τ2`

## Type System

Typing judgments have the form `Γ ⊢ e : τ`.

- **Variable**
  ```
  (x : τ) ∈ Γ
  -----------------
  Γ ⊢ x : τ
  ```

- **Integer literal**
  ```
  -----------------
  Γ ⊢ n : Int
  ```

- **Lambda**
  ```
  Γ, x : α ⊢ e : β
  -----------------------
  Γ ⊢ \x -> e : α -> β
  ```

- **Application**
  ```
  Γ ⊢ e1 : α -> β     Γ ⊢ e2 : α
  -------------------------------
           Γ ⊢ e1 e2 : β
  ```

- **Let** (no generalization yet)
  ```
  Γ ⊢ e1 : τ1
  Γ, x : τ1 ⊢ e2 : τ2
  -------------------
  Γ ⊢ let x = e1 in e2 : τ2
  ```

- **If**
  ```
  Γ ⊢ e0 : Bool
  Γ ⊢ e1 : τ     Γ ⊢ e2 : τ
  ---------------------------
  Γ ⊢ if e0 then e1 else e2 : τ
  ```

## Semantics

- **Evaluation strategy** is strict (call-by-value).
- **Lambda** creates a function (a closure in later runtimes).
- **Application** evaluates the function and its argument, then applies the function.
- **Let** evaluates the right-hand side, then extends the environment for the body.
- **If** evaluates the condition; if nonzero (true) evaluates the “then” branch, otherwise the “else” branch.
- **Integers** are first-class values.

## Example Programs

### Identity
```ml
let id = \x -> x in id 42
-- type: Int
-- evaluates to 42
```

### Currying
```ml
let add = \a -> \b -> a + b in add 2 3
-- type: Int
-- evaluates to 5
```

### Conditionals (future, once Bool literals are added)
```ml
if true then 42 else 0
-- type: Int
-- evaluates to 42
```