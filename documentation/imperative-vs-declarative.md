## Imperative vs declarative

*Distinguishing between declarative and imperative development is important but why?*  

Imperative and Declarative are different programming paradigms, likely named differently to enhance clarity and emphasize their contrasting approaches.
A similar and perhaps more intuitive concept is that of a producer and a consumer. The producer generates something, while the consumer utilizes it.

Example:
When you dine at a restaurant, you become a consumer of culinary creations and the restaurant's services. The restaurant itself acts as a consumer, buying ingredients from suppliers. 
These suppliers, in turn, rely on producers like farmers and fishermen to provide the raw materials. This illustrates a chain of consumption, where each entity refines and 
adds value to the product before passing it along. Importantly, within this chain, everyone plays dual roles as both consumers and producers.

While the path of food from farm to table is relatively straightforward, comprehending the intricate journey of software development presents a significant challenge.
Computers, at their core, operate on machine code, a language far from human understanding. To bridge this gap, a multi-layered refinement process is necessary.
This involves a complex interplay between hardware, operating systems, development environments, components, applications, documentation, and ultimately, the end user. 
Unlike the linear progression of food, software development involves intricate overlaps and feedback loops, making it a highly dynamic and non-linear process.

Software refinement is also inherently costly and carries significant risks. Key considerations include technical debt, software usability, need for adaptations, effective knowledge transfer and documentation.

Framing the software refinement process through the lenses of **imperative and declarative** approaches may help to get better understanding.
Provide a valuable framework for decision-making at each step, helping to improve the final outcome.

Consider a restaurant scenario: if a customer receives a cold dish, it would be unacceptable for them to interfere with the kitchen's operations by attempting to reheat it themselves.
This not only disrupts the carefully coordinated efforts of the kitchen staff but also risks compromising the quality of the meals they produce. 
Such individual actions, if repeated by multiple guests, would quickly lead to chaos. In a well-defined and optimized work process, if intervened, even with the best of intentions,
can inadvertently disrupt the established flow and undermine the overall efficiency of the system.

Codebases lacking a well-defined structure, riddled with numerous dependencies, or deeply intertwined with a complex domain are notoriously difficult to maintain.
The primary challenge arises not from the code's inherent complexity, but from the intricate web of dependencies and the challenges of managing interactions between the various components that constitute the final system.

#### Hacks
Most developers knows about the term "hack". A 'hack' in programming typically refers to a solution applied inappropriately. The issue often lies not with the solution itself, but rather with its placement within the codebase.  

## Moving parts, where to place logic, domain vs general code.

Requiring developers to understand a specific domain before they can effectively code can significantly hinder productivity. In some cases, the domain knowledge required may be so specialized that it becomes impossible to find developers.

While the terms 'declarative' and 'imperative' may not always provide a precise categorization, it highlights the importance of separating concerns within software systems.
The concept of separating concerns within software systems is fundamental, regardless of domain complexity.

This division of labor enhances structure, improves developer focus, and enables a modular architecture.
Producer-consumer relationships are common throughout software projects, enabling a modular and scalable architecture.
Adhering to these principles, while avoiding 'hacks' that compromise the intended separation of concerns, is essential for building maintainable and adaptable systems.
Isolating functionalities improves code quality and developer productivity.

Developers who write imperative code should prioritize solutions to simplify the work for writing declarative code and make that task as easy as possible.

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

### Imperative code, don't destroy it
Imperative programming, which focuses on how to solve a problem, the opposite of declarative programming. 
It's very important to know this because seemingly minor modification to imperative code, such as adding domain-specific logic, can unintentionally compromise its reusability.  
Not knowing how to maintain general-purpose imperative code will destroy it, and this happens quickly.

#### It's not rocket science
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

### So why call it Imperative or declarative?
Imperativ och Deklarativ är olika typ av kod men varför kalla det för olika saker.
Lättare att förstå är producent och konsument. Producenten är den som skapar något, konsumenten använder det.
Skall du gå på restaurang och äta mat, är du konsument till kocken och restaurangen. Restaurangen är i sin tur konsument till de som säljer varor för att laga maten. De som säljer mat till restaranger är i sin tur konsumenter till de som producerar maten, bönder, fiskare etc.
En lång kedja med steg där varje steg förädlar något till nästa steg. Samtidigt är alla både konsumenter och producenter i förädlingskedjan.

Följa matens väg från jord till bord är enklare än att följa mjukvara även om det handlar om samma sak.
Datorer är användbara om man lyckas instruera dem till att utföra saker på ett sätt som fungerar för människor. 
Innan maskinkoden som är det enda datorer försår, är användbar för slutanvändare, då har den förädlats i flera steg.
Exempelvis: Hårdvara - operativsystem - utvecklingsmiljöer - komponenter - applikation - dokumentation - slutanvändare. Fyra av stegen är mjukvara.
Alla är de steg som används för att producera system för användare. Att förädla mjukvara är inte lika linjärt som att förädla mat. Områden går in i varandra och blandas.

Förädla mjukvara är också kostsamt och riskabelt. Termer som teknisk skuld, mjukvarans användbarhet, anpassningar, kunskapsöverföring, dokumentation är viktiga att tänka på.

Imperativ och deklarativ kan här eventuellt hjälpa till för att förstå riskerna bättre. Med termerna har steg i förädlingsprocessen lättare att förstå hur de skall tänka för att inte trampa fel.

Restaurangexemplet igen. En som köpt mat tycker maten är kall, skulle konsumenten gå in i köket och själv värma maten, då förstör konsumenten för de som producerar mat som optimerat sitt arbete. Gör fler gäster samma sak blir det kaos i köket.
I en optimerad arbetsprocess kan det för utomstående vara svårt att förstå vad man inte bör göra för att förstöra flödet.

Skrivs mycket kod som mixas med en svår domän är risken stor att koden blir svår att underhålla. Det beror inte på att koden i sig är svår utan regelverket som styr koden.


NEW:

Imperative and Declarative are different programming paradigms, likely named differently to enhance clarity and emphasize their contrasting approaches.
A similar and perhaps more intuitive concept is that of a producer and a consumer. The producer generates something, while the consumer utilizes it.

Example:
When you dine at a restaurant, you become a consumer of culinary creations and the restaurant's services. The restaurant itself acts as a consumer, buying ingredients from suppliers. 
These suppliers, in turn, rely on producers like farmers and fishermen to provide the raw materials. This illustrates a chain of consumption, where each entity refines and 
adds value to the product before passing it along. Importantly, within this chain, everyone plays dual roles as both consumers and producers.

While the path of food from farm to table is relatively straightforward, comprehending the intricate journey of software development presents a significant challenge.
Computers, at their core, operate on machine code, a language far from human understanding. To bridge this gap, a multi-layered refinement process is necessary.
This involves a complex interplay between hardware, operating systems, development environments, components, applications, documentation, and ultimately, the end user. 
Unlike the linear progression of food, software development involves intricate overlaps and feedback loops, making it a highly dynamic and non-linear process.

Software refinement is also inherently costly and carries significant risks. Key considerations include technical debt, software usability, need for adaptations, effective knowledge transfer and documentation.

Framing the software refinement process through the lenses of imperative and declarative approaches may help to get better understanding.
Provide a valuable framework for decision-making at each step, helping to improve the final outcome.

Consider a restaurant scenario: if a customer receives a cold dish, it would be unacceptable for them to interfere with the kitchen's operations by attempting to reheat it themselves.
This not only disrupts the carefully coordinated efforts of the kitchen staff but also risks compromising the quality of the meal. 
Such individual actions, if repeated by multiple guests, would quickly lead to chaos. In a well-defined and optimized work process, if intervened, even with the best of intentions,
can inadvertently disrupt the established flow and undermine the overall efficiency of the system.

Codebases lacking a well-defined structure, riddled with numerous dependencies, or deeply intertwined with a complex domain are notoriously difficult to maintain.
The primary challenge arises not from the code's inherent complexity, but from the intricate web of dependencies and the challenges of managing interactions between the various components that constitute the final system.


Imperative and Declarative are different types of code and maybe are named differently to make it easier to understand.




Similar concept but maybe easier to understand is producer and consumer. The producer is the one who creates something, the consumer uses it.
If you go to a restaurant and eat food, you are a consumer of the chef and the restaurant. The restaurant is in turn a consumer of those who sell goods to cook the food. Those who sell food to restaurants are in turn consumers of those who produce the food, farmers, fishermen, etc.
A long chain of steps where each step refines something to the next step. At the same time, everyone is both a consumer and a producer in the refining chain.

Following the path of food from farm to table is easier than following software even though it is about the same thing.
Computers are useful if you manage to instruct them to do things in a way that works for humans.
Before the machine code, which is the only thing computers provide, is useful for end users, it has been refined in several steps.
For example: Hardware - operating system - development environments - components - application - documentation - end user. Four of the steps are software.
All are the steps used to produce systems for users. Refining software is not as linear as refining food. Areas overlap and mix.

Refining software is also costly and risky. Terms such as technical debt, software usability, adaptations, knowledge transfer, documentation are important to consider.

Imperative and declarative can possibly help here to understand the risks better. With the terms, it is easier to understand how steps in the refining process should think in order not to make mistakes.

The restaurant example again. If someone who has bought food thinks the food is cold, the consumer should go into the kitchen and heat the food himself, then the consumer ruins the work of those who produce food that has optimized their work. If more guests do the same thing, there will be chaos in the kitchen.
In an optimized work process, it can be difficult for outsiders to understand what not to do to disrupt the flow.



If a lot of code is produced without a clear structure, mixed with different dependencies or mixed with a difficult domain, there is a high risk that the code will be difficult to maintain. This is not because the code itself is difficult, but because of the parts used inte the final system.




### Hot to spot problems in code
- Är det möjligt att följa hur data flödet ser ut i koden. Hur enkelt det är att nå olika ställen i koden.
- Hur djupt ner i tech stacken domänen går.
- Hur väl olika logiker är separerade.
You are in trouble if
- Domain specific DTO objects contains some sort of lists and is mixed with general-purpose operations like working with files, database, etc.
- You have a advanced domain with undocumented code.
- Methods that do more than one thing and where they are mixed with domain-specific logic.
- Domain-specific logic is mixed with general-purpose operations like working with files, database

### Why is this so diffuclt to manage?
- De flesta utvecklare behöver skriva något lite större och trassla till det innan de förstår konsekvenserna av att blanda imperativt och deklarativt. Det tar tid att lära sig och det tar tid att förstå vilken typ av kunskap som behövs.
- 

https://iso25000.com/index.php/en/iso-25000-standards/iso-25010





## Imperative vs declarative

*Distinguishing between declarative and imperative development is important but why?*  

Imperative and Declarative are different programming paradigms, likely named differently to enhance clarity and emphasize their contrasting approaches.
A similar and perhaps more intuitive concept is that of a producer and a consumer. The producer generates something, while the consumer utilizes it.

Example:
When you dine at a restaurant, you become a consumer of culinary creations and the restaurant's services. The restaurant itself acts as a consumer, buying ingredients from suppliers. 
These suppliers, in turn, rely on producers like farmers and fishermen to provide the raw materials. This illustrates a chain of consumption, where each entity refines and 
adds value to the product before passing it along. Importantly, within this chain, everyone plays dual roles as both consumers and producers.

While the path of food from farm to table is relatively straightforward, comprehending the intricate journey of software development presents a significant challenge.
Computers, at their core, operate on machine code, a language far from human understanding. To bridge this gap, a multi-layered refinement process is necessary.
This involves a complex interplay between hardware, operating systems, development environments, components, applications, documentation, and ultimately, the end user. 
Unlike the linear progression of food, software development involves intricate overlaps and feedback loops, making it a highly dynamic and non-linear process.

Software refinement is also inherently costly and carries significant risks. Key considerations include technical debt, software usability, need for adaptations, effective knowledge transfer and documentation etc.

Framing the software refinement process through the lenses of **imperative and declarative** approaches may help to get better understanding.
Provide a valuable framework for decision-making at each step, helping to improve the final outcome.

Consider a restaurant scenario: if a customer receives a cold dish, it would be unacceptable for them to interfere with the kitchen's operations by attempting to reheat it themselves.
This not only disrupts the carefully coordinated efforts of the kitchen staff but also risks compromising the quality of the meals they produce. 
Such individual actions, if repeated by multiple guests, would quickly lead to chaos. In a well-defined and optimized work process, if intervened, even with the best of intentions,
can inadvertently disrupt the established flow and undermine the overall efficiency of the system.

Codebases lacking a well-defined structure, riddled with numerous dependencies, or deeply intertwined with a complex domain are notoriously difficult to maintain beacause its a mess.
The primary challenge arises not from the code's inherent complexity, but from the intricate web of dependencies and the challenges of managing interactions between the various components that constitute the final system.

### Hacks
Most developers knows about the term "hack". A 'hack' in programming typically refers to a solution applied inappropriately. The issue often lies not with the solution itself, but rather with its placement within the codebase.  

## Moving parts, where to place logic, domain vs general code.

The domain is a important factor in software development, given that software's primary function is to solve problems within a specific domain.

Requiring developers to understand a specific domain before they can effectively code can significantly hinder productivity. In some cases, the domain knowledge required may be so specialized that it becomes impossible to find developers.
While the terms 'declarative' and 'imperative' may not always provide a precise categorization, if there is a requirement for domain knowledge to work in code it highlights the importance of separating concerns within software systems.
If the domain is to advanced and mixed in code, it's may be impossible to work with in it.
While the domain may be learnable, it places a considerable cognitive burden on developers, forcing them to memorize and understand unfamiliar logic, which can be mentally taxing.

By leveraging the distinction between imperative and declarative (producer-consumer) code, we can isolate and simplify development tasks, potentially reducing the reliance on in-depth domain expertise.

Division of labor enhances structure, improves developer focus, and enables a modular architecture.
Producer-consumer relationships are common throughout software projects, enabling a modular and scalable architecture.
Adhering to these principles, while avoiding 'hacks' that compromise the intended separation of concerns, is essential for building maintainable and adaptable systems.
Isolating functionalities improves code quality and developer productivity.

Developers who write imperative code should prioritize solutions to simplify the work for writing declarative code and make that task as easy as possible.

| Feature | Imperative | Declarative |
|---|---|---|
| Focus | How to do something | What to do |
| Control Flow | Explicitly managed by the programmer | Implicitly managed by the system |
| State Management | Programmer is responsible for state | State is managed automatically |
| Abstraction Level | Lower-level, more control over execution | Higher-level, more abstract |


### Declarative and the domain
Declarative code typically manages domain-specific logic. This code is often hardcoded, meaning it's tailored to a specific use case within the domain. 

Declarative logic might not be general-purpose code at all, that is often the goal. Browsers have HTML and CSS to display content, databases have SQL, programing editors might be configurable with XML or JSON. Scripting is another approach, like Lua and Python, are frequently used to implement game logic and scientific computations, respectively. These languages offer a flexible and efficient way to automate tasks and create dynamic applications.    

Developing software, it's important to consider future maintenance. Declarative programming paradigms should be easy and flexible to change, even end-users can sometimes adapt through declarative configurations.

### Imperative code, don't destroy it
Imperative programming, which focuses on how to solve a problem, the opposite of declarative programming. 
It's very important to know this because seemingly minor modification to imperative code, such as adding domain-specific logic, can unintentionally compromise its reusability.  
Not knowing how to maintain general-purpose imperative code will destroy it, and this happens quickly.

#### It's not rocket science
It shouldn’t be that difficult to understand though, some domains have incredibly steep learning curves. It takes years of focused study just to gain a basic understanding. How feasible is it for a developer to acquire the necessary knowledge to create practical software solutions in such complex areas? Other domains, while perhaps less complex, may necessitate separation due to factors like frequent changes, evolving regulations, or intricate dependencies. For instance, the sheer volume of interconnected threads in certain systems can make it nearly impossible for developers to fully grasp their relationships.  
Or building applications for a wide audience demands a flexible approach. You must anticipate a variety of user needs and design the software to accommodate them. A one-size-fits-all solution is often not feasible, but maintaining multiple codebases for each customer is unsustainable.  


## How to spot problems
- The level of difficulty in maintaining code can depend on how tightly coupled it is to the specific domain. Identifying code sections that interact with long-term storage like disks. If this data handling logic is domain-specific, it's a sign the domain knowledge might be pervasive throughout the codebase. This can make changes more complex down the line.  
- The difficulty in tracing the flow of data in code should be considered a warning sign. It often suggests that data is scattered throughout the codebase, making it challenging to understand and maintain.
- Methods that perform multiple, unrelated tasks (lack of separation of concerns) significantly hinder code maintainability. Refactoring and making behavioral changes become significantly more challenging.



Självklart beror det mycket på vilken typ av program och kod det handlar om. Men så svårt är det faktiskt inte och se problem i en kodmassa.
En teknik är att försöka leta upp de delar i koden som hanterar data och att data skrivs eller läses från något disk eller annan långsiktig lagring.
Är den här delen domänspecific är sannolikheten hög att domänen ligger inbakad i resten av koden också. Inte bra.