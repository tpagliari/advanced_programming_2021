#include <iostream>
#include <vector>

// Without exposing the underlining representation
template <typename node_t, typename T, typename N>
class _iterator {
    node_t* pool_ptr; 
    node_t* current_node_ptr;
public:
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;
    using stack_type = N;
    using stack_ptr_type = stack_type*;
    using stack_ref_type = stack_type&;

    using difference_type = std::ptrdiff_t;
    typedef std::forward_iterator_tag iterator_category; 
   
    // end and begin will be the same if they have the 
    // address of the one before first elem. of the stack
    // --> end
    _iterator(stack_type x, node_t* ptr) :
        pool_ptr{ptr}, current_node_ptr{pool_ptr + x - 1} {};
        
    reference operator*() const {
        return current_node_ptr->value;
    }
    _iterator& operator++() {
        current_node_ptr = pool_ptr + current_node_ptr->next - 1;
        return *this;
    }
    _iterator operator++(int){ 
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
    friend bool operator==(const _iterator& it_a, const _iterator& it_b){
        return it_a.current_node_ptr == it_b.current_node_ptr;
    }
    friend bool operator!=(const _iterator& it_a, const _iterator& it_b){
        return !(it_a == it_b);
    }
};


// interface the user will play with ...
template <typename T, typename N = std::size_t>
class stack_pool {
    
    struct node_t {
        T value;
        N next;
        template <typename O>
            node_t(O&& o, N n)
            : value{std::forward<O>(o)}, next{n} {}; 
    };

    // no need to specify template parameters for stack_pool
    class _stack {
        stack_pool* pool_ptr;
        N head;
    public:
        _stack(stack_pool* ptr, N x)
            : pool_ptr{ptr}, head{x} {};
        auto begin() {
            return pool_ptr->begin(head);
        }
        auto end() {
            return pool_ptr->end(head);
        }    
    };

    std::vector<node_t> pool;
    using stack_type = N;
    using value_type = T;
    using size_type = typename std::vector<node_t>::size_type;
    stack_type free_nodes{1};

    node_t& node(stack_type x) noexcept {
        return pool[x - 1]; 
    }
    const node_t& node(stack_type x) const noexcept { 
        return pool[x - 1]; 
    }

    // 2 if statement
    // --> one explicitly done by me
    // --> one internally done by std::vector emplace
    template <typename O>
        stack_type _push(O&& val, stack_type head);

public:
    stack_pool() = default;
    explicit stack_pool(size_type n) {
        reserve(n); // capacity == n
    }

    stack_type new_stack() {
        return end(); 
    }
    
    void reserve(size_type n) {
        pool.reserve(n);
    }
    
    size_type capacity() const {
        return pool.capacity();
    }

    bool empty(stack_type x) const {
        return x == end();
    }

    stack_type end() const noexcept { 
        return stack_type(0); 
    }

    T& value(stack_type x) {
        return node(x).value;
    }
    const T& value(stack_type x) const {
        return node(x).value;
    }

    stack_type& next(stack_type x) {
        return node(x).next;
    }
    const stack_type& next(stack_type x) const {
        return node(x).next;
    }

    stack_type push(const T& val, stack_type head) {
        return _push(val, head);
    }
    stack_type push(T&& val, stack_type head){
        return _push(std::move(val), head);
    }

    stack_type pop(stack_type x);

    // NOTE: pop() does not destroy the nodes,
    // so the size of the pool is still the same.
    // when I push on the slots freed by the free_stack function,
    // push() will select the []-op branch. 
    stack_type free_stack(stack_type x){
        while(x) x = pop(x);
        return x; //end()
    }

    // method to give the user access at the 
    // range based for loop over a stack
    auto stack(_stack head){
        return _stack{this, head}; // call ctor of _stack class
    }

    void display_stack(stack_type x) const;

public:
    using iterator = _iterator<node_t, T, N>;
    using const_iterator = _iterator<node_t, const T, N>;

    iterator begin(stack_type x) {
        return iterator{x, pool.data()}; // using ctor defined in class _iterator
                                         // returns the begin of the stack
    }
    iterator end(stack_type ) { 
        return iterator{0, pool.data()}; // returns the end of the stack
    }

    const_iterator begin(stack_type x) const {
        return const_iterator{x, pool.data()};
    }
    const_iterator end(stack_type ) const {
        return const_iterator{0, pool.data()};
    }

    const_iterator cbegin(stack_type x) const {
        return const_iterator{x, pool.data()};
    }
    const_iterator cend(stack_type ) const {
        return const_iterator{0, pool.data()};
    }
};

template <typename T, typename N>
template <typename O>
N stack_pool<T, N>::_push(O&& val, N head) {
    N tmp{free_nodes};
    if(free_nodes > pool.size()){
        ++free_nodes;
        // --> reserve condition is internally managed by std::vector
        pool.emplace_back(std::forward<O>(val), head);
    }else{
        free_nodes = next(free_nodes);
        //pool[tmp-1] = node_t{std::forward<O>(val), head};
        node(tmp) = node_t{std::forward<O>(val), head};
    }
    //std::cout << "free_nodes " << free_nodes << std::endl;
    return tmp;
};

template <typename T, typename N>
N stack_pool<T, N>::pop(N x){
    if(empty(x)) return x;
    N tmp = next(x);
    next(x) = free_nodes;
    free_nodes = x;
    //std::cout << "free_nodes " << free_nodes << std::endl;
    return tmp;
};

// C++17
template <typename T, typename N>
void stack_pool<T, N>::display_stack(N x) const {
    while(x){
        const auto& [val, next] = stack_pool::node(x);
        std::cout << val << "," << next << " --> ";
        x = stack_pool::next(x);
    }
    std::cout << std::endl;
};
