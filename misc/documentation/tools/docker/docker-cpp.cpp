#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>

int main() {
   std::cout << "Print even numbers between 1 and 5" << std::endl;

   std::vector<int> v{1,2,3,4,5};
   auto even = v | std::views::filter([](int n){ return n%2==0; });
   for(auto x : even) std::cout << x << " ";
   std::cout << "\nready!" << std::endl;
   return 0;
}