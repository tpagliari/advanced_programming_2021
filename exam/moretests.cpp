#include "stack_pool.hpp"
#include <memory>

int main(){
    
    // testing using unique_ptr in order to be sure
    // that the container do not exhibits memory leak.
    using value_type = std::unique_ptr<int>;

    stack_pool<value_type> pool_u;
    auto l1 = pool_u.new_stack();
    auto l2 = pool_u.new_stack();
    auto l3 = pool_u.new_stack();
    l1 = pool_u.push(std::make_unique<int>(1), l1);

    
    auto c = std::move(pool_u);
    stack_pool<value_type, std::size_t> d;
    d = std::move(c);


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
    
    // testing throws
    stack_pool<double> pool_t(20);
    auto lt = pool.new_stack();
    //pool_t.next(lt);
    //pool_t.value(lt);
    lt = pool_t.pop(lt);
}
