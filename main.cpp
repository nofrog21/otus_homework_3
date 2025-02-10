#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>

#define LOG(s, v) do {std::cout << s << ' ' << (v) << std::endl;} while(0)

template <class T>
struct my_pool_allocator
{
    typedef T value_type;

    static size_t pos;
    static constexpr size_t size = sizeof(T) * 1000;
    static uint8_t data[size];

    my_pool_allocator() noexcept {}
    ~my_pool_allocator() {}

    template <class U>
    my_pool_allocator(const my_pool_allocator<U> &) noexcept {}

    T *allocate(size_t n)
    {
        if (pos + n > size)
            throw std::bad_alloc();

        size_t cur = pos;
        pos += n;
        return reinterpret_cast<T *>(data) + cur;
    }

    void deallocate([[maybe_unused]]T *p, [[maybe_unused]]size_t n) {}

    template <typename U, typename... Args>
    void construct(U *p, Args &&...args)
    {
        new (p) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U *p)
    {
        p->~U();
    }


    template <class U>
    struct rebind
    {
        typedef my_pool_allocator<U> other;
    };
};

template <typename T>
uint8_t my_pool_allocator<T>::data[size];

template <typename T>
size_t my_pool_allocator<T>::pos = 0;

template <class T, class U>
constexpr bool operator==(const my_pool_allocator<T> &a1, const my_pool_allocator<U> &a2) noexcept
{
    return true;
}

template <class T, class U>
constexpr bool operator!=(const my_pool_allocator<T> &a1, const my_pool_allocator<U> &a2) noexcept
{
    return false;
}

template <class T, class Allocator = std::allocator<T>>
class MyList {
    struct Node {
        Node() = default;
        Node(const T& value, Node* next_node)
            : val(value), next(next_node)
        {}

        T val;
        Node* next = nullptr;
    };

    template <typename ValueType>
    class BasicIterator{
        friend class MyList;

        explicit BasicIterator(Node* node)
            : node_(node)
        {
        }

    public:

        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = ValueType*;
        using reference = ValueType&;

        BasicIterator() = default;

        BasicIterator(const BasicIterator<T>& other)
            : node_(other.node_)
        {}
        BasicIterator& operator=(const BasicIterator& rhs) = default;

        [[nodiscard]] bool operator==(const BasicIterator<const T>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator<const T>& rhs) const noexcept {
            return !(*this == rhs);
        }

        [[nodiscard]] bool operator==(const BasicIterator<T>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator<T>& rhs) const noexcept {
            return !(*this == rhs);
        }

        BasicIterator& operator++() noexcept {
            assert(node_ != nullptr);
            node_ = node_->next;
            return *this;
        }

        BasicIterator operator++(int) noexcept {
            auto old_value(*this);
            ++(*this);
            return old_value;
        }

        [[nodiscard]] reference operator*() const noexcept {
            assert(node_ != nullptr);
            return node_->val;
        }

        [[nodiscard]] pointer operator->() const noexcept {
            assert(node_ != nullptr);
            return &(node_->val);
        }

    private:
        Node* node_ = nullptr;
    };
public:
    using Iterator = BasicIterator<T>;
    using ConstIterator = BasicIterator<const T>;

    [[nodiscard]] Iterator begin() noexcept
    {
        return Iterator(head_.next);
    }

    [[nodiscard]] Iterator end() noexcept
    {
        return Iterator();
    }

    [[nodiscard]] ConstIterator begin() const noexcept
    {
        return ConstIterator(head_.next);
    }

    [[nodiscard]] ConstIterator end() const noexcept
    {
        return ConstIterator();
    }

    void push_front(const T& val)
    {
        Node* new_node = node_alloc_.allocate(1);
        node_alloc_.construct(new_node, val, head_.next);
        head_.next = new_node;
        ++size_;
    }

    size_t size() const noexcept
    {
        return size_;
    }

   bool empty() const noexcept
   {
       return size_ == 0;
   }

private:
    Node head_;
    size_t size_ = 0;
    typename Allocator::template rebind<Node>::other node_alloc_;
};

template <class Map>
void FillMap(Map& m)
{
    int factorial = 1;
    m[0] = factorial;
    for (int i = 1; i < 10; ++i) {
        factorial *= i;
        m[i] = factorial;
    }
}

template <class Map>
void PrintMap(const Map& m)
{
    for (const auto& [key, value] : m) {
        std::cout << key << ' ' << value << std::endl;
    }
}

template <class List>
void FillList(List& l)
{
    for (int i = 0; i < 10; ++i) {
        l.push_front(i);
    }
}

template <class List>
void PrintList(const List& l)
{
    for (const auto& el : l) {
        std::cout << el << std::endl;
    }
}

int main()
{
    using namespace std::literals;
    std::cout << "std::allocator map"s << std::endl;
    std::map<int, int> std_alloc_map;
    FillMap(std_alloc_map);
    PrintMap(std_alloc_map);

    std::cout << "my_pool_allocator map"s << std::endl;
    std::map<int, int, std::less<int>, my_pool_allocator<std::pair<const int, int>>> my_alloc_map;
    FillMap(my_alloc_map);
    PrintMap(my_alloc_map);

    std::cout << "std::allocator MyList"s << std::endl;
    MyList<int> std_alloc_list;
    FillList(std_alloc_list);
    PrintList(std_alloc_list);

    std::cout << "my_pool_allocator MyList"s << std::endl;
    MyList<int, my_pool_allocator<int>> my_pool_alloc_list;
    FillList(my_pool_alloc_list);
    PrintList(my_pool_alloc_list);
    return 0;
}
