                    ## Imperative vs declarative

*Distinguishing between declarative and imperative development is important but why?*  
  
Another way to describe this is to clearly separate domain-specific logic from general-purpose code. This separation allows developers, who are proficient in coding, to focus on solving problems within their area of expertise.  

Requiring developers to understand a specific domain before they can effectively code can significantly hinder productivity. In some cases, the domain knowledge required may be so specialized that it becomes impossible to find developers.

This highlights the reason why it is so important to separate between imperative and declarative code.
Developers who write imperative code should prioritize solutions to simplify the work for writing declarative code and make that task as easy as possible.
- **Declarative** = domain-specific logic, code that solves the "what"
- **Imperative** = general-purpose code, code that solves the "how" 

The ultimate success of this approach is demonstrated when domain experts, without requiring any specialized coding skills, can effectively configure the tools to achieve the desired outcomes within their specific domain.  
**Experts will be the best at what they do.**

| Feature | Imperative | Declarative |
|---|---|---|
| Focus | How to do something | What to do |
| Control Flow | Explicitly managed by the programmer | Implicitly managed by the system |
| State Management | Programmer is responsible for state | State is managed automatically |
| Abstraction Level | Lower-level, more control over execution | Higher-level, more abstract |


### Declarative and the domain
Declarative code typically manages domain-specific logic. This code is often hardcoded, meaning it's tailored to a specific use case. To write effective domain code, developers need a understanding of the domain itself. While this can be manageable for simple domains, it's less appealing for developers who prefer coding over domain expertise.

Declarative logic might not be general-purpose code at all, that is often the goal. Browsers have HTML and CSS to display content, databases have SQL, programing editors might be configurable with XML or JSON. Scripting is another approach, like Lua and Python, are frequently used to implement game logic and scientific computations, respectively. These languages offer a flexible and efficient way to automate tasks and create dynamic applications.    

Developing software, it's important to consider future maintenance. Declarative programming paradigms should be easy and flexible to change, even end-users can sometimes adapt through declarative configurations.

### Imperative and domain logic should be kept separate
Imperative programming, which focuses on how to solve a problem, is the opposite of declarative programming, which focuses on what needs to be achieved. It's very important to know this because seemingly minor modification to imperative code, such as adding domain-specific logic, can unintentionally compromise its reusability.  
Developers lacking the necessary skills to maintain general-purpose imperative code will destroy it, and this happens quickly.

### How to destroy code
General-purpose imperative code can easily be destroyed with domain-specific logic, and when this happens you are on the path to project failure or huge problems. This common pitfall is often overlooked, but it can be identified early on by recognizing the signs of such entanglement. By proactively addressing this issue, you can prevent future problems and ensure project success. 

The primary challenge with mixing code types (mix imperative and declarative) is that without understanding the importance of separation, the problem often goes unnoticed. Even when the issue becomes apparent, identifying the root cause can be difficult, leading to a series of smaller problems without a clear solution.

#### It's not rocket science, you know...
It shouldn’t be that difficult to understand though, some domains have incredibly steep learning curves. It takes years of focused study just to gain a basic understanding. How feasible is it for a developer to acquire the necessary knowledge to create practical software solutions in such complex areas? Other domains, while perhaps less complex, may necessitate separation due to factors like frequent changes, evolving regulations, or intricate dependencies. For instance, the sheer volume of interconnected threads in certain systems can make it nearly impossible for developers to fully grasp their relationships.  
Or building applications for a wide audience demands a flexible approach. You must anticipate a variety of user needs and design the software to accommodate them. A one-size-fits-all solution is often not feasible, but maintaining multiple codebases for each customer is unsustainable.  

#### Why is it so difficult to successfully land software projects?

It's a given that developers know how to code, as their primary role is to create software solutions. However, they are not domain experts. Software projects, especially large-scale ones or systems with other high demands are very challenging.  
Landing successful software projects requires more than just the ability to write code. A deep understanding of the chosen programming languages and proficiency in architectural design are important.

For example: Consider Microsoft Office, a behemoth with about 350 million lines of code – easy to understand difficulties there. But complexity isn't solely a matter of scale. Highly mathematical code, even in small amounts (less than 50 lines), can be incredibly challenging to comprehend.

Working with undocumented declarative domain code can be extremely challenging and time-consuming. This is often because it requires knowledge beyond the typical developer skillset, such as domain-specific expertise. This mismatch in required knowledge can contribute significantly to developer burnout. Projects become increasingly vulnerable to failure when bugs begin to emerge, and knowledge is fragmented across developers and domain experts.

#### Programming
Programming is not so much about writing code, it's much more about prioritizing how to manage code. Ask any developer about the time spent writing code, it's a small part.

Think of it like building a house. Writing code is like laying bricks, but the real work lies in designing the blueprints, ensuring the foundation is strong, and making sure the house is safe and functional. Similarly, programming is more about the overall management and maintenance of the codebase than simply writing lines of code in some language.  
**And the buyer is responsible for furnishing the house.** It would be absurd if the builders were also responsible for furnishing the house. How do you think the house value would have been affected if the builders had built the furniture into the walls?