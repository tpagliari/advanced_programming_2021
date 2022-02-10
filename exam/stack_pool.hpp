#include <iostream>
#include <vector>

/*
 * Without exposing the underlining representation
 */
template <typename node_t, typename T, typename N>
class _iterator {
    node_t* first_pool_node_ptr; 
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
    
    _iterator(stack_type x, node_t* ptr) noexcept :
        first_pool_node_ptr{ptr}, current_node_ptr{first_pool_node_ptr + x - 1} {};
        
    reference operator*() const {
        return current_node_ptr->value;
    }
    _iterator& operator++() {
        current_node_ptr = first_pool_node_ptr + current_node_ptr->next - 1;
        return *this;
    }
    _iterator operator++(int){ 
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
    friend bool operator==(const _iterator& it_a, const _iterator& it_b) noexcept {
        return it_a.current_node_ptr == it_b.current_node_ptr;
    }
    friend bool operator!=(const _iterator& it_a, const _iterator& it_b) noexcept {
        return !(it_a == it_b);
    }
};

template <typename T, typename N = std::size_t>
class stack_pool {
    
    struct node_t {
        T value;
        N next;
        template <typename O>
            node_t(O&& o, N n)
            : value{std::forward<O>(o)}, next{n} {}; 
    };

    /*
     * This class allows stack_pool to have begin() and end() public methods.
     * From this class, I can access begin(stack type ) and 
     * end(stack_type ) of the particular instance. This can be done thanks
     * to the pool_ptr that is initialized as 'this' of the particular
     * instance of stack_pool<T,N>. 
     */  
    class _stack {
        stack_pool* pool_ptr; // Need pool_ptr also in nested class
                              // nested cls has no special access to the this pointer 
                              // of the enclosing class.
                              // without pool_ptr there's no way to know on which 
                              // instance of pool_stack the method has to be invoked
                              // (same logic in the _iterator)
        N head;
    public:
        _stack(stack_pool* ptr, N x) noexcept
            : pool_ptr{ptr}, head{x} {};
        auto begin() noexcept {
            return pool_ptr->begin(head);
        }
        auto end() noexcept {
            return pool_ptr->end(head);
        }    
    };

    std::vector<node_t> pool;
    using stack_type = N;
    using value_type = T;
    using size_type = typename std::vector<node_t>::size_type;
    stack_type free_nodes{end()};

    /*
     * Functions that defines a one-to-one correspondence between
     * a particular stack and the vector it is stored in.
     */
    node_t& node(stack_type x) noexcept {
        return pool[x - 1]; 
    }
    const node_t& node(stack_type x) const noexcept { 
        return pool[x - 1]; 
    }

    /*
     * Forwarding referenced in order to have a push method that is
     * able to accept both const& value and rvalue without
     * code-duplication of the body function.
     */
    template <typename O>
        stack_type _push(O&& val, stack_type head);

    /*
     * This function is used to perform checkings for logic errors
     * eventually committed by the user, e.g. popping an empty stack.
     */
    void check_logic_error(stack_type x, const std::string& message) const {
        if(empty(x)) 
            throw std::out_of_range(message);
    }

public:
    /*
     * Default constructor that construct a new instance of
     * stack_pool<T,N> without allocated memory.
     */
    stack_pool() noexcept = default; // noexcept:

    /*
     * Custom constructor that creates a new instance of
     * stack_pool<T,N> with initial capacity equals to n
     *
     * This ctor may throw if the given argument exceeds the maximum number
     * of elements the container std::vector is able to hold.
     */
    explicit stack_pool(size_type n) {
        reserve(n);
    }
    
    /*
     * This method creates a new empty stack,
     * by setting its head to end.
     */
    stack_type new_stack() noexcept {
        return end(); 
    }
    
    /*
     * stack_pool::reserve calls the vector::reserve method.
     * It throws if the given argument n exceeds the maximum number 
     * of elements the container std::vector is able to hold.
     */
    void reserve(size_type n) { 
        pool.reserve(n);
    }
    
    /*
     * Returns the current capacity of the std::vector pool
     */
    size_type capacity() const noexcept {
        return pool.capacity();
    }

    /*
     * A stack is empty if its head is equal to its end.
     * This method checks if the stack is empty.
     */
    bool empty(stack_type x) const noexcept {
        return x == end();
    }

    /*
     * this method returns the end of a stack in the pool,
     * the end is the past-the-last element of the stack.
     */
    stack_type end() const noexcept { 
        return stack_type(0); 
    }

    /*
     * stack_pool::value throws exceptions if the given
     * argument x is not a valid index of the std::vector pool.
     * This causes segmentation fault.
     *
     * A logic error could be appling value on an empty stack.
     * This invokes node[-1] which is an attempt to access
     * elements out of defined range.
     */
    T& value(stack_type x) {
        check_logic_error(x, "Requested value on empty stack");
        return node(x).value;
    }
    const T& value(stack_type x) const {
        check_logic_error(x, "Requested value on empty stack");
        return node(x).value;
    }

    /*
     * stack_pool::next throws exceptions if the given
     * argument x is not a valid index of the std::vector pool.
     * This causes segmentation fault.
     *
     * A logic error could be appling next on an empty stack.
     * This invokes node[-1] which is an attempt to access
     * elements out of defined range.
     */
    stack_type& next(stack_type x) {
        check_logic_error(x, "Requested next on empty stack");
        return node(x).next;
    }
    const stack_type& next(stack_type x) const { 
        check_logic_error(x, "Requested next on empty stack");
        return node(x).next;
    }

    stack_type push(const T& val, stack_type head) {
        return _push(val, head);
    }
    stack_type push(T&& val, stack_type head){
        return _push(std::move(val), head);
    }

    stack_type pop(stack_type x);

    /*
     * stack_type::free_stack takes a given stack
     * and, by popping all of it's nodes, returns an empty stack
     * We can say that the stack is now freed.
     *
     * This method cannot throw exceptions because pop is
     * proteced by the control in the while loop.
     * Still, the user should be careful and be sure to pass 
     * the head of the stack as argument.
     */
    stack_type free_stack(stack_type x) noexcept {
        // NOTE: pop() does not destroy the nodes,
        // so the size of the pool is still the same.
        // when I push on the slots freed by the free_stack function,
        // push() will select the []-op branch. 
        while(x) x = pop(x);
        return x;
    }

    /*
     * Method that gives the user access at the 
     * range based for loop over a stack
     */
    auto stack(stack_type head) noexcept{
        return _stack{this, head}; // call ctor of _stack class
    }

    void display_stack(stack_type x) const;

public:
    using iterator = _iterator<node_t, T, N>;
    using const_iterator = _iterator<node_t, const T, N>;

    iterator begin(stack_type x) noexcept {
        return iterator{x, pool.data()}; // calls ctor defined in class _iterator
                                         // returns the begin of the stack
    }
    iterator end(stack_type ) noexcept { 
        return iterator{0, pool.data()}; // returns the end of the stack
    }

    const_iterator begin(stack_type x) const noexcept {
        return const_iterator{x, pool.data()};
    }
    const_iterator end(stack_type ) const noexcept {
        return const_iterator{0, pool.data()};
    }

    const_iterator cbegin(stack_type x) const noexcept {
        return const_iterator{x, pool.data()};
    }
    const_iterator cend(stack_type ) const noexcept{
        return const_iterator{0, pool.data()};
    }
};

/*
 * stack_pool::push throws an exception if the argument head
 * is not a valid index of the pool. 
 *
 * If the argument passed is a valid index but it
 * is not the actual head of the stack, the linked structure
 * wont be linear no more and the behaviour of the stack_pool
 * is no longer predictable
 * Still, this would be an illogic error from the user, which should
 * use the container properly.
 *
 * vector::emplace_back can throw exceptions.
 * Internally, it looks for space to allocate if needed.
 * If the memory needed exceed the memory the container is able to hold,
 * it has to throw. 
 *
 * Also, it constructs the object of type T,
 * and this class may have a throwing ctor.
 */
template <typename T, typename N>
template <typename O>
N stack_pool<T, N>::_push(O&& val, N head) {
    if(empty(free_nodes)){
        pool.emplace_back(std::forward<O>(val), head); 
        return static_cast<stack_type>(pool.size());
    }else{
        auto tmp = free_nodes;
        free_nodes = next(free_nodes);
        //pool[tmp-1] = node_t{std::forward<O>(val), head};
        node(tmp) = node_t{std::forward<O>(val), head};
        return tmp;
    }
};

/*
 * stack_pool::pop throws an exception if the argument x
 * is not a valid index of the pool vector. 
 * This causes segmentation fault.
 *
 * If the argument x is a valid index but it
 * is not the head of the stack, the linked structure of the stack
 * will be broked and the behaviour of the data structure is
 * no longer predictible. 
 * Still, this would be an illogic error from the user, which should
 * use the container properly.
 *
 * A logic error that the user may do here is appling stack_pool::pop
 * on an empty stack. The user should be inform that the stack
 * is empty. So in this case I may throw exception.
 *
 */
template <typename T, typename N>
N stack_pool<T, N>::pop(N x){
    N tmp = next(x); // internally checks for logic error
    next(x) = free_nodes;
    free_nodes = x;
    return tmp;
};

/*
 * This method allows the user to print a stack in the pool.
 *
 * std::cout may throw exceptions, so I would not mark the 
 * following method noexcept. For example, if the quantity of available
 * memory is running low, cout may throw because it
 * allocates new memory.
 */
template <typename T, typename N>
void stack_pool<T, N>::display_stack(N x) const {
    while(x){
        const auto& [val, next] = stack_pool::node(x);
        std::cout << val << "," << next << " --> ";
        x = stack_pool::next(x);
    }
    std::cout << std::endl;
};
