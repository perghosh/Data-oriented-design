
# C++ Code Review Checklist

This checklist might look lengthy, but the items are quick to check. It helps assess code quality—not to find bugs, but to spot potential problems. The code could still be well-written.


## 1. Code Formatting
- **Looks Good**: Is the code visually appealing and easy to read?
  - *Why it matters*: Can you spot that developer care about the code?
  - *Check*: Is formatters used this is harder but if not and the code looks nice , it is a good sign.
- **Broken lines**: Are there lines broken just to fit a certain length?
  - *Why it matters*: Broken lines can disrupt flow and readability.
  - *Check*: Look for lines that are broken unnecessarily, especially in comments or long strings.
- **Consistent Style**: Is the code uniformly formatted (e.g., indentation, bracing, line lengths)? Does it follow patterns?
  - *Why it matters*: Consistent formatting improves readability and signals developer care.
  - *Check*: Look for similar code with different styles. It's ok if code in different areas has different styles, but it should be consistent within the same area.
- **Indentation Levels**: Are there excessive nested blocks (deep indentation)?
  - *Why it matters*: Deep indentation suggests complex logic that may need refactoring.
  - *Check*: Flag functions with more than 4-5 levels of nesting.
- **Message Chains**: Are there long chains of method calls (e.g., `obj.a().b().c()`)? Message chains looks nice, but they make code harder to maintain.
  - *Why it matters*: Long message chains indicate tight coupling, making code harder to modify or test.
  - *Check*: Look for chained calls that could be simplified or broken into intermediate variables.
- **Debug-Friendliness**: Does the code include intentional debugging support?
  - *Why it matters*: Debug-friendly code simplifies troubleshooting and reduces time spent on issues. It saves a lot of time.
  - *Check*: Look for debuggcode, try to find out if those that wrote the code understood how to help others to manage it. For example, are there temporary variables that help to understand the code flow?  Assertions that trigger for developer errors?

## 2. Comments
- **Clarity**: Do comments explain *why* code exists, especially for non-obvious logic?
  - *Why it matters*: Comments clarify intent, aiding maintenance and onboarding.
  - *Check*: Verify comments are concise, relevant, and avoid stating the obvious (e.g., avoid `i++ // increment i`). Look for documentation on functions/classes.
- **if and for loops**: Are comments used to explain complex conditions or logic and are they easy to read? When devlopers read code conditionals are important, so comments should be used to clarify them if not obvious.
  - *Why it matters*: Complex conditions can be hard to understand at a glance. 
  - *Check*: Ensure comments clarify the purpose of intricate conditions (e.g., `if (x > 0 && y < 10) // Check if x is positive and y is less than 10`).

## 3. Variables
- **Meaningful Names**: Are variable names descriptive and self-explanatory?
  - *Why it matters*: Clear names reduce guesswork and improve comprehension.
  - *Check*: Avoid vague names (e.g., `tmp`, `data`) and prefer domain-specific names or a combination of type and domain name (e.g., `iUserAge`, `dOrderTotal`).
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

## 4. Bad code
   - **Lots of getters and setters**: Are there many getters and setters that could be simplified?
     - *Why it matters*: Excessive getters/setters can indicate poor encapsulation or design and tight coupling.
     - *Check*: Look for classes with numerous trivial getters/setters that could be replaced with direct access or better abstractions.
   - **Direct member access**: Are there instances where class members are accessed directly instead of through methods?
     - *Why it matters*: Direct access can break encapsulation and lead to maintenance issues.
     - *Check*: Identify cases where class members are accessed directly (e.g., `obj.member`) instead of using methods (e.g., `obj.GetMember()`).
   - **Complex Expressions**: Are there overly complex expressions that could be simplified?

## 5. Templates
- **Effective Use**: Are templates used to improve code reuse without adding complexity?
  - *Why it matters*: Templates enhance flexibility but can reduce readability if overused or make code hard to understand.
  - *Check*: Review template parameters and constraints (e.g., C++20 concepts). Ensure they solve a real problem and aren’t overly generic.

## 6. Inheritance
- **Justification**: Is inheritance used for true “is-a” relationships, or is it overused?
  - *Why it matters*: Misused inheritance creates tight coupling, complicating refactoring.
  - *Check*: Verify inheritance follows the Liskov Substitution Principle. Prefer composition where possible. Flag deep hierarchies or concrete base classes.

## 7. Type Aliases (`using`/`typedef`)
- **Intuitive Names**: Are aliases clear and domain-relevant, or do they obscure meaning?
  - *Why it matters*: Good aliases can clarify intent; but more often confuse readers. Remember that alias are often domain-specific. And domain-specific names is not always good. 
  - *Check*: Ensure names like `using Distance = double;` are meaningful.

## 8. Methods and Functions
- **Redundant naming**: Does a method name unnecessarily repeat the class name or describe its parameters? A method's identity is defined by its name and parameters—not by restating what’s already clear.
  - *Why it matters*: Duplicate names can lead to confusion and errors.
  - *Check*: Ensure method names are distinct and meaningful without duplicating class or parameter context.
- **Concise Names**: Are method names descriptive yet concise, avoiding verbosity?
  - *Why it matters*: Long names (e.g., `calculateTotalPriceAndApplyDiscounts`) suggest methods do too much.
  - *Check*: Ensure names reflect a single purpose (e.g., `calculateTotal`, `ApplyDiscounts`).
- **Single Responsibility**: Does each method perform only one task as implied by its name?
  - *Why it matters*: Methods doing multiple tasks are harder to test and maintain (much harder).
  - *Check*: Flag methods longer than 50-60 lines or with multiple logical tasks.
- **Parameter Count**: Are methods limited to 3-4 parameters?
 - *Why it matters*: Too many parameters complicate method signatures and usage.
 - *Check*: Look for methods with more than 4 parameters. Consider using structs or classes to group related parameters.

## 9. Error Handling
- **Explicit and Debuggable**: Are errors handled clearly?
  - *Why it matters*: Robust error handling prevents crashes and aids debugging.
  - *Check*: Verify consistent error mechanisms and proper logging of issues.

## 10. STL and Standard Library
- **Effective Use**: Does the code leverage STL (e.g., `std::vector`, `std::algorithm`) appropriately? Does the code merge well with the standard library?
  - *Why it matters*: Using STL simplifies code, becuse most C++ knows about STL. It's also well thought out.
  - *Check*: Look for proper use of containers, algorithms, and modern features (e.g., `std::optional`, `std::string_view`). Are stl types used like `value_type`, `iterator`, etc.?

## 11. File and Project Structure
- **Logical Organization**: Are files and directories grouped by module, feature, or layer?
  - *Why it matters*: A clear structure simplifies navigation and scalability.
  - *Check*: Verify meaningful file names, proper header/source separation, and use of header guards or `#pragma once`. Flag circular dependencies.

## 12. Codebase Navigation
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




