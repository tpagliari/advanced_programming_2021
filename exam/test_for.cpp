#include "stack_pool.hpp"

int main() {

    stack_pool<int> pool{10};
    auto s1 = pool.new_stack();
    for(auto i=0; i<3; ++i){
        s1 = pool.push(i, s1);
    }
    auto s2 = pool.new_stack();
    s2 = pool.push(300, s2);
    s2 = pool.push(301, s2);
    s1 = pool.push(4, s1);
    // range based for loop
    for(auto& x : pool.stack(s1))
        std::cout << x;
    std::cout << std::endl;

    pool.display_stack(s1);
    pool.display_stack(s2);
}
