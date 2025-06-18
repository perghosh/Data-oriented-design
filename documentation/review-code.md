
# C++ Code Review Checklist

This checklist ensures C++ code is readable, maintainable, and high-quality. Use it to evaluate code systematically, focusing on clarity, structure, and best practices.

## 1. Code Formatting
- **Consistent Style**: Is the code uniformly formatted (e.g., indentation, bracing, line lengths)? Does it follow a style guide?
  - *Why it matters*: Consistent formatting improves readability and signals developer care.
  - *Check*: Look for mixed tabs/spaces, inconsistent brace styles, or chaotic layout.
- **Indentation Levels**: Are there excessive nested blocks (deep indentation)?
  - *Why it matters*: Deep indentation suggests complex logic that may need refactoring.
  - *Check*: Flag functions with more than 3-4 levels of nesting.
- **Message Chains**: Are there long chains of method calls (e.g., `obj.a().b().c()`)?
  - *Why it matters*: Long message chains indicate tight coupling, making code harder to modify or test.
  - *Check*: Look for chained calls that could be simplified or broken into intermediate variables.
- **Spacing**: Is whitespace used effectively (e.g., around operators, after commas)?
  - *Why it matters*: Proper spacing enhances readability and reduces visual clutter.
  - *Check*: Ensure consistent spacing (e.g., `int x = 5;` vs. `int x=5;`).

## 2. Comments
- **Clarity**: Do comments explain *why* code exists, especially for non-obvious logic?
  - *Why it matters*: Comments clarify intent, aiding maintenance and onboarding.
  - *Check*: Verify comments are concise, relevant, and avoid stating the obvious (e.g., avoid `i++ // increment i`). Look for documentation on functions/classes.

## 3. Variables
- **Meaningful Names**: Are variable names descriptive and self-explanatory?
  - *Why it matters*: Clear names reduce guesswork and improve comprehension.
  - *Check*: Avoid vague names (e.g., `tmp`, `data`) and prefer domain-specific names (e.g., `iUserAge`, `dOrderTotal`).
- **Abbreviations**: Are abbreviations minimal and widely understood?
  - *Why it matters*: Excessive or obscure abbreviations confuse readers.
  - *Check*: Flag cryptic abbreviations (e.g., `usrMngr` vs. `userManager`).
- **Scope and Isolation**: Are variables declared close to their point of use?
  - *Why it matters*: Localized variables reduce mental overhead and minimize errors.
  - *Check*: Look for variables declared far from usage or reused across unrelated scopes.
- **Magic Numbers/Strings**: Are hardcoded values replaced with named constants?
  - *Why it matters*: Magic numbers (e.g., `42`) obscure intent and hinder maintenance.
  - *Check*: Ensure constants like `const int MAX_USERS = 100;` are used.
- **Use of `auto`**: Is `auto` used judiciously, or does it obscure variable types?
  - *Why it matters*: Overuse of `auto` can make debugging harder by hiding types.
  - *Check*: Verify `auto` is used for clear cases (e.g., iterators, lambdas) but not where type clarity is critical (e.g., `auto x = GetValue();`).

## 4. Templates
- **Effective Use**: Are templates used to improve code reuse without adding complexity?
  - *Why it matters*: Templates enhance flexibility but can reduce readability if overused or make code hard to understand.
  - *Check*: Review template parameters and constraints (e.g., C++20 concepts). Ensure they solve a real problem and aren’t overly generic.

## 5. Inheritance
- **Justification**: Is inheritance used for true “is-a” relationships, or is it overused?
  - *Why it matters*: Misused inheritance creates tight coupling, complicating refactoring.
  - *Check*: Verify inheritance follows the Liskov Substitution Principle. Prefer composition where possible. Flag deep hierarchies or concrete base classes.

## 6. Type Aliases (`using`/`typedef`)
- **Intuitive Names**: Are aliases clear and domain-relevant, or do they obscure meaning?
  - *Why it matters*: Good aliases clarify intent; poor ones confuse readers.
  - *Check*: Ensure names like `using Distance = double;` are meaningful.

## 7. Methods and Functions
- **Concise Names**: Are method names descriptive yet concise, avoiding verbosity?
  - *Why it matters*: Long names (e.g., `calculateTotalPriceAndApplyDiscounts`) suggest methods do too much.
  - *Check*: Ensure names reflect a single purpose (e.g., `calculateTotal`, `ApplyDiscounts`).
- **Single Responsibility**: Does each method perform only one task as implied by its name?
  - *Why it matters*: Methods doing multiple tasks are harder to test and maintain.
  - *Check*: Flag methods longer than 20-30 lines or with multiple logical tasks.

## 8. Error Handling
- **Explicit and Debuggable**: Are errors handled clearly (e.g., exceptions, error codes, `std::expected`)?
  - *Why it matters*: Robust error handling prevents crashes and aids debugging.
  - *Check*: Verify consistent error mechanisms and proper logging of issues.

## 9. STL and Standard Library
- **Effective Use**: Does the code leverage STL (e.g., `std::vector`, `std::algorithm`) appropriately?
  - *Why it matters*: STL reduces bugs and improves performance compared to custom implementations.
  - *Check*: Look for proper use of containers, algorithms, and modern features (e.g., `std::optional`, `std::string_view`). Flag reinvented wheels (e.g., custom arrays). Are stl types used like value_type, iterator, etc.?

## 10. File and Project Structure
- **Logical Organization**: Are files and directories grouped by module, feature, or layer?
  - *Why it matters*: A clear structure simplifies navigation and scalability.
  - *Check*: Verify meaningful file names, proper header/source separation, and use of header guards or `#pragma once`. Flag circular dependencies.

## 11. Codebase Navigation
- **Ease of Exploration**: Is the code easy to navigate and test?
  - *Why it matters*: A navigable codebase speeds up development and debugging.
  - *Check*: Ensure clear module boundaries, consistent naming, and testable units. Verify unit tests exist for critical functionality.

---

## Regular Expressions for Code Review

Usefull regular expressions to find potential issues in C++ code. These patterns can help identify areas that may need refactoring or review.
Note that it doesn't mean that code is bad, but it may need some attention.

### Common
- `^\s{20}if\s*\(` - Find `if` statements with 20 spaces before them.  
- `^[ \t]*[a-zA-Z_][\w:<>, \t*&]*\s+([a-zA-Z_][a-zA-Z_0-9]{20,})\s*\(` - Find function definitions with 20 or more characters in the function name.  
- `\b[a-zA-Z][a-zA-Z0-9_]{19,}\b(?=\s*\()` - Find function names with 20 or more characters.  

### Magic Numbers
- `\b[0-9]{2,}\b(?!\s*[;,\)])` - Numbers with 2+ digits not at the end of statements.  
- `\b[0-9]+\.[0-9]+\b` - Floating point literals.  

### Bad Variables
- `\b[a-z]\b(?=\s*[=\[])` - Single letter variables being assigned/accessed.  
- `\b[a-zA-Z_][a-zA-Z0-9_]*[0-9]+[a-zA-Z_][a-zA-Z0-9_]*` - Variables with numbers in the middle.  
- `\b[a-z]{2,3}\b(?=\s*[=\[])` - Very short variable names (2-3 chars).  

### Memory
- `\bnew\s+(?!std::)` - Raw `new` without `std::` (potential memory leak).  
- `\bdelete\s+(?!std::)` - Raw `delete`.  
- `\bmalloc\s*\(` - C-style allocation.  
- `\bfree\s*\(` - C-style deallocation.  

### Complexity
- `^\s{32,}` - Lines indented 32+ spaces (8 levels deep).  
- `^\s*for\s*\([^)]*\)\s*\{\s*for\s*\([^)]*\)\s*\{\s*for` - Triple nested loops.  
- `^\s*if\s*\([^)]*\)\s*\{\s*if\s*\([^)]*\)\s*\{\s*if` - Triple nested `if`s.  

### Function Calls
- `\([^)]{100,}\)` - Function calls/definitions with 100+ chars in parameters.  
- `\([^)]*,[^)]*,[^)]*,[^)]*,[^)]*,[^)]*\)` - 6+ parameters (count commas).  

### Casting to Check
- `\(void\s*\*\)` - C-style void pointer casts.  
- `\([a-zA-Z_]\w*\s*\*\)` - C-style pointer casts.  
- `\breinterpret_cast\s*<` - Dangerous casts.  

### Lines to Check
- `^.{120,}$` - Lines longer than 120 characters.  
- `[&|]{3,}` - Multiple logical operators chained.  
- `[=!<>]{3,}` - Complex comparison chains.




## Rgular expression to find code blocks
```regex
[\w-]*\n[\s\S]*?\n

```

- `^\s{20}if\s*\(` - If statement with 20 spaces before it
- `^[ \t]*[a-zA-Z_][\w:<>, \t*&]*\s+([a-zA-Z_][a-zA-Z_0-9]{20,})\s*\(` - Function definition with 20 or more characters in the function name
- `\b[a-zA-Z][a-zA-Z0-9_]{19,}\b(?=\s*\()` one more
- `\b([a-zA-Z_][a-zA-Z_0-9]{20,})\b\s*=` - Variable assignment with 20 or more characters
- `` - find #define statements to check if you understand the constant names
- `\b\w+\s*=\s*\d+(\.\d+)?\b` - Find variable assignments with numbers


